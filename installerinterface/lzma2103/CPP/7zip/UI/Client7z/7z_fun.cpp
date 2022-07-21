// main.cpp

#include "StdAfx.h"
#include "..\..\..\..\..\framework.h"
#include "..\..\..\..\..\CMainWnd.h"
using namespace std;
#include "../../../Common/MyWindows.h"
#include "../../../Common/Defs.h"
#include "../../../Common/MyInitGuid.h"
#include "../../../Common/IntToString.h"
#include "../../../Common/StringConvert.h"
#include "../../../Windows/DLL.h"
#include "../../../Windows/FileDir.h"
#include "../../../Windows/FileFind.h"
#include "../../../Windows/FileName.h"
#include "../../../Windows/NtCheck.h"
#include "../../../Windows/PropVariant.h"
#include "../../../Windows/PropVariantConv.h"
#include "../../Common/FileStreams.h"
#include "../../Archive/IArchive.h"
#include "../../IPassword.h"
#include "../../../../C/7zVersion.h"
extern
HINSTANCE g_hInstance;
HINSTANCE g_hInstance = 0;

// You can find full list of all GUIDs supported by 7-Zip in Guid.txt file.
// 7z format GUID: {23170F69-40C1-278A-1000-000110070000}

#define DEFINE_GUID_ARC(name, id) DEFINE_GUID(name, \
  0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, id, 0x00, 0x00);

enum
{
    kId_Zip = 1,
    kId_BZip2 = 2,
    kId_7z = 7,
    kId_Xz = 0xC,
    kId_Tar = 0xEE,
    kId_GZip = 0xEF
};

// use another id, if you want to support other formats (zip, Xz, ...).
// DEFINE_GUID_ARC (CLSID_Format, kId_Zip)
// DEFINE_GUID_ARC (CLSID_Format, kId_BZip2)
// DEFINE_GUID_ARC (CLSID_Format, kId_Xz)
// DEFINE_GUID_ARC (CLSID_Format, kId_Tar)
// DEFINE_GUID_ARC (CLSID_Format, kId_GZip)
DEFINE_GUID_ARC(CLSID_Format, kId_7z)

using namespace NWindows;
using namespace NFile;
using namespace NDir;
static const char* const kCopyrightString =
"\n"
"7-Zip"
" (" kDllName " client)"
" " MY_VERSION
" : " MY_COPYRIGHT_DATE
"\n";

static const char* const kHelpString =
"Usage: 7zcl.exe [a | l | x] archive.7z [fileName ...]\n"
"Examples:\n"
"  7zcl.exe a archive.7z f1.txt f2.txt  : compress two files to archive.7z\n"
"  7zcl.exe l archive.7z   : List contents of archive.7z\n"
"  7zcl.exe x archive.7z   : eXtract files from archive.7z\n";


static void Convert_UString_to_AString(const UString& s, AString& temp)
{
    int codePage = CP_OEMCP;
    /*
    int g_CodePage = -1;
    int codePage = g_CodePage;
    if (codePage == -1)
      codePage = CP_OEMCP;
    if (codePage == CP_UTF8)
      ConvertUnicodeToUTF8(s, temp);
    else
    */
    UnicodeStringToMultiByte2(temp, s, (UINT)codePage);
}

static FString CmdStringToFString(const char* s)
{
    return us2fs(GetUnicodeString(s));
}

static void Print(const char* s)
{
    fputs(s, stdout);
}

static void Print(const AString& s)
{
    Print(s.Ptr());
}

static void Print(const UString& s)
{
    AString as;
    Convert_UString_to_AString(s, as);
    Print(as);
}

static void Print(const wchar_t* s)
{
    Print(UString(s));
}

static void PrintNewLine()
{
    Print("\n");
}

static void PrintStringLn(const char* s)
{
    Print(s);
    PrintNewLine();
}

static void PrintError(const char* message)
{
    Print("Error: ");
    PrintNewLine();
    Print(message);
    PrintNewLine();
}

static void PrintError(const char* message, const FString& name)
{
    PrintError(message);
    Print(name);
}


static HRESULT IsArchiveItemProp(IInArchive* archive, UInt32 index, PROPID propID, bool& result)
{
    NCOM::CPropVariant prop;
    RINOK(archive->GetProperty(index, propID, &prop));
    if (prop.vt == VT_BOOL)
        result = VARIANT_BOOLToBool(prop.boolVal);
    else if (prop.vt == VT_EMPTY)
        result = false;
    else
        return E_FAIL;
    return S_OK;
}

static HRESULT IsArchiveItemFolder(IInArchive* archive, UInt32 index, bool& result)
{
    return IsArchiveItemProp(archive, index, kpidIsDir, result);
}


static const wchar_t* const kEmptyFileAlias = L"[Content]";


//////////////////////////////////////////////////////////////
// Archive Open callback class


class CArchiveOpenCallback :
    public IArchiveOpenCallback,
    public ICryptoGetTextPassword,
    public CMyUnknownImp
{
public:
    MY_UNKNOWN_IMP1(ICryptoGetTextPassword)

        STDMETHOD(SetTotal)(const UInt64* files, const UInt64* bytes);
    STDMETHOD(SetCompleted)(const UInt64* files, const UInt64* bytes);

    STDMETHOD(CryptoGetTextPassword)(BSTR* password);

    bool PasswordIsDefined;
    UString Password;

    CArchiveOpenCallback() : PasswordIsDefined(false) {}
};

STDMETHODIMP CArchiveOpenCallback::SetTotal(const UInt64* /* files */, const UInt64* /* bytes */)
{
    return S_OK;
}

STDMETHODIMP CArchiveOpenCallback::SetCompleted(const UInt64* /* files */, const UInt64* /* bytes */)
{
    return S_OK;
}

STDMETHODIMP CArchiveOpenCallback::CryptoGetTextPassword(BSTR* password)
{
    if (!PasswordIsDefined)
    {
        // You can ask real password here from user
        // Password = GetPassword(OutStream);
        // PasswordIsDefined = true;
        PrintError("Password is not defined");
        return E_ABORT;
    }
    return StringToBstr(Password, password);
}



static const char* const kIncorrectCommand = "incorrect command";

//////////////////////////////////////////////////////////////
// Archive Extracting callback class

static const char* const kTestingString = "Testing     ";
static const char* const kExtractingString = "Extracting  ";
static const char* const kSkippingString = "Skipping    ";
static const char* const kReadingString = "Reading     ";

static const char* const kUnsupportedMethod = "Unsupported Method";
static const char* const kCRCFailed = "CRC Failed";
static const char* const kDataError = "Data Error";
static const char* const kUnavailableData = "Unavailable data";
static const char* const kUnexpectedEnd = "Unexpected end of data";
static const char* const kDataAfterEnd = "There are some data after the end of the payload data";
static const char* const kIsNotArc = "Is not archive";
static const char* const kHeadersError = "Headers Error";

struct CArcTime
{
    FILETIME FT;
    UInt16 Prec;
    Byte Ns100;
    bool Def;

    CArcTime()
    {
        Clear();
    }

    void Clear()
    {
        FT.dwHighDateTime = FT.dwLowDateTime = 0;
        Prec = 0;
        Ns100 = 0;
        Def = false;
    }

    bool IsZero() const
    {
        return FT.dwLowDateTime == 0 && FT.dwHighDateTime == 0 && Ns100 == 0;
    }

    int GetNumDigits() const
    {
        if (Prec == k_PropVar_TimePrec_Unix ||
            Prec == k_PropVar_TimePrec_DOS)
            return 0;
        if (Prec == k_PropVar_TimePrec_HighPrec)
            return 9;
        if (Prec == k_PropVar_TimePrec_0)
            return 7;
        int digits = (int)Prec - (int)k_PropVar_TimePrec_Base;
        if (digits < 0)
            digits = 0;
        return digits;
    }

    void Write_To_FiTime(CFiTime& dest) const
    {
#ifdef _WIN32
        dest = FT;
#else
        if (FILETIME_To_timespec(FT, dest))
            if ((Prec == k_PropVar_TimePrec_Base + 8 ||
                Prec == k_PropVar_TimePrec_Base + 9)
                && Ns100 != 0)
            {
                dest.tv_nsec += Ns100;
            }
#endif
    }

    void Set_From_Prop(const PROPVARIANT& prop)
    {
        FT = prop.filetime;
        unsigned prec = 0;
        unsigned ns100 = 0;
        const unsigned prec_Temp = prop.wReserved1;
        if (prec_Temp != 0
            && prec_Temp <= k_PropVar_TimePrec_1ns
            && prop.wReserved3 == 0)
        {
            const unsigned ns100_Temp = prop.wReserved2;
            if (ns100_Temp < 100)
            {
                ns100 = ns100_Temp;
                prec = prec_Temp;
            }
        }
        Prec = (UInt16)prec;
        Ns100 = (Byte)ns100;
        Def = true;
    }
};

class CArchiveExtractCallback:
  public IArchiveExtractCallback,
  public ICryptoGetTextPassword,
  public CMyUnknownImp
{
public:
  MY_UNKNOWN_IMP1(ICryptoGetTextPassword)

  // IProgress
  STDMETHOD(SetTotal)(UInt64 size);
  STDMETHOD(SetCompleted)(const UInt64 *completeValue);

  // IArchiveExtractCallback
  STDMETHOD(GetStream)(UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode);
  STDMETHOD(PrepareOperation)(Int32 askExtractMode);
  STDMETHOD(SetOperationResult)(Int32 resultEOperationResult);

  // ICryptoGetTextPassword
  STDMETHOD(CryptoGetTextPassword)(BSTR *aPassword);
  
private:
  CMyComPtr<IInArchive> _archiveHandler;
  FString _directoryPath;  // Output directory
  UString _filePath;       // name inside arcvhive
  FString _diskFilePath;   // full path to file on disk
  bool _extractMode;
  struct CProcessedFileInfo
  {
    CArcTime MTime;
    UInt32 Attrib;
    bool isDir;
    bool Attrib_Defined;
  } _processedFileInfo;

  COutFileStream *_outFileStreamSpec;
  CMyComPtr<ISequentialOutStream> _outFileStream;

public:
  void Init(IInArchive *archiveHandler, const FString &directoryPath);
  
  UInt64 NumErrors;
  bool PasswordIsDefined;
  UString Password;
  UInt64 FileSize;
  CArchiveExtractCallback() : PasswordIsDefined(false) {}
};

void CArchiveExtractCallback::Init(IInArchive *archiveHandler, const FString &directoryPath)
{
  NumErrors = 0;
  _archiveHandler = archiveHandler;
  _directoryPath = directoryPath;
  NName::NormalizeDirPathPrefix(_directoryPath);
}

STDMETHODIMP CArchiveExtractCallback::SetTotal(UInt64  size )
{
  FileSize = size;
  //extern CMainWnd* pFrame;
  //pFrame->m_pProgressBar->SetValue(FileSize);
  return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::SetCompleted(const UInt64 * completeValue )
{
   extern CMainWnd* pFrame;
  int t=(*completeValue)*100/FileSize;
  pFrame->m_pProgressBar->SetValue(t);
  return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::GetStream(UInt32 index,
    ISequentialOutStream **outStream, Int32 askExtractMode)
{
  *outStream = 0;
  _outFileStream.Release();

  {
    // Get Name
    NCOM::CPropVariant prop;
    RINOK(_archiveHandler->GetProperty(index, kpidPath, &prop));
    
    UString fullPath;
    if (prop.vt == VT_EMPTY)
      fullPath = kEmptyFileAlias;
    else
    {
      if (prop.vt != VT_BSTR)
        return E_FAIL;
      fullPath = prop.bstrVal;
    }
    _filePath = fullPath;
  }

  if (askExtractMode != NArchive::NExtract::NAskMode::kExtract)
    return S_OK;

  {
    // Get Attrib
    NCOM::CPropVariant prop;
    RINOK(_archiveHandler->GetProperty(index, kpidAttrib, &prop));
    if (prop.vt == VT_EMPTY)
    {
      _processedFileInfo.Attrib = 0;
      _processedFileInfo.Attrib_Defined = false;
    }
    else
    {
      if (prop.vt != VT_UI4)
        return E_FAIL;
      _processedFileInfo.Attrib = prop.ulVal;
      _processedFileInfo.Attrib_Defined = true;
    }
  }

  RINOK(IsArchiveItemFolder(_archiveHandler, index, _processedFileInfo.isDir));

  {
    _processedFileInfo.MTime.Clear();
    // Get Modified Time
    NCOM::CPropVariant prop;
    RINOK(_archiveHandler->GetProperty(index, kpidMTime, &prop));
    switch (prop.vt)
    {
      case VT_EMPTY:
        // _processedFileInfo.MTime = _utcMTimeDefault;
        break;
      case VT_FILETIME:
          _processedFileInfo.MTime.Set_From_Prop(prop);
        break;
      default:
        return E_FAIL;
    }

  }
  {
    // Get Size
    NCOM::CPropVariant prop;
    RINOK(_archiveHandler->GetProperty(index, kpidSize, &prop));
    UInt64 newFileSize;
    /* bool newFileSizeDefined = */ ConvertPropVariantToUInt64(prop, newFileSize);
  }

  
  {
    // Create folders for file
    int slashPos = _filePath.ReverseFind_PathSepar();
    if (slashPos >= 0)
      CreateComplexDir(_directoryPath + us2fs(_filePath.Left(slashPos)));
  }

  FString fullProcessedPath = _directoryPath + us2fs(_filePath);
  _diskFilePath = fullProcessedPath;

  if (_processedFileInfo.isDir)
  {
    CreateComplexDir(fullProcessedPath);
  }
  else
  {
    NFind::CFileInfo fi;
    if (fi.Find(fullProcessedPath))
    {
      if (!DeleteFileAlways(fullProcessedPath))
      {
        PrintError("Cannot delete output file", fullProcessedPath);
        return E_ABORT;
      }
    }
    
    _outFileStreamSpec = new COutFileStream;
    CMyComPtr<ISequentialOutStream> outStreamLoc(_outFileStreamSpec);
    if (!_outFileStreamSpec->Open(fullProcessedPath, CREATE_ALWAYS))
    {
      PrintError("Cannot open output file", fullProcessedPath);
      return E_ABORT;
    }
    _outFileStream = outStreamLoc;
    *outStream = outStreamLoc.Detach();
  }
  return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::PrepareOperation(Int32 askExtractMode)
{
  _extractMode = false;
  switch (askExtractMode)
  {
    case NArchive::NExtract::NAskMode::kExtract:  _extractMode = true; break;
  };
  switch (askExtractMode)
  {
    case NArchive::NExtract::NAskMode::kExtract:  Print(kExtractingString); break;
    case NArchive::NExtract::NAskMode::kTest:  Print(kTestingString); break;
    case NArchive::NExtract::NAskMode::kSkip:  Print(kSkippingString); break;
  };
  Print(_filePath);
  return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::SetOperationResult(Int32 operationResult)
{
  switch (operationResult)
  {
    case NArchive::NExtract::NOperationResult::kOK:
      break;
    default:
    {
      NumErrors++;
      Print("  :  ");
      const char *s = NULL;
      switch (operationResult)
      {
        case NArchive::NExtract::NOperationResult::kUnsupportedMethod:
          s = kUnsupportedMethod;
          break;
        case NArchive::NExtract::NOperationResult::kCRCError:
          s = kCRCFailed;
          break;
        case NArchive::NExtract::NOperationResult::kDataError:
          s = kDataError;
          break;
        case NArchive::NExtract::NOperationResult::kUnavailable:
          s = kUnavailableData;
          break;
        case NArchive::NExtract::NOperationResult::kUnexpectedEnd:
          s = kUnexpectedEnd;
          break;
        case NArchive::NExtract::NOperationResult::kDataAfterEnd:
          s = kDataAfterEnd;
          break;
        case NArchive::NExtract::NOperationResult::kIsNotArc:
          s = kIsNotArc;
          break;
        case NArchive::NExtract::NOperationResult::kHeadersError:
          s = kHeadersError;
          break;
      }
      if (s)
      {
        Print("Error : ");
        Print(s);
      }
      else
      {
        char temp[16];
        ConvertUInt32ToString(operationResult, temp);
        Print("Error #");
        Print(temp);
      }
    }
  }

  if (_outFileStream)
  {
      if (_processedFileInfo.MTime.Def)
      {
          CFiTime ft;
          _processedFileInfo.MTime.Write_To_FiTime(ft);
          _outFileStreamSpec->SetMTime(&ft);
      }
    RINOK(_outFileStreamSpec->Close());
  }
  _outFileStream.Release();
  if (_extractMode && _processedFileInfo.Attrib_Defined)
    SetFileAttrib_PosixHighDetect(_diskFilePath, _processedFileInfo.Attrib);
  PrintNewLine();
  return S_OK;
}


STDMETHODIMP CArchiveExtractCallback::CryptoGetTextPassword(BSTR *password)
{
  if (!PasswordIsDefined)
  {
    // You can ask real password here from user
    // Password = GetPassword(OutStream);
    // PasswordIsDefined = true;
    PrintError("Password is not defined");
    return E_ABORT;
  }
  return StringToBstr(Password, password);
}



//////////////////////////////////////////////////////////////
// Archive Creating callback class

struct CDirItem : public NWindows::NFile::NFind::CFileInfoBase
{
    UString Path_For_Handler;
    FString FullPath; // for filesystem

    CDirItem(const NWindows::NFile::NFind::CFileInfo& fi) :
        CFileInfoBase(fi)
    {}
};

static void GetStream2(const wchar_t *name)
{
  Print("Compressing  ");
  if (name[0] == 0)
    name = kEmptyFileAlias;
  Print(name);
}

#if defined(_UNICODE) && !defined(_WIN64) && !defined(UNDER_CE)
#define NT_CHECK_FAIL_ACTION PrintError("当前Windows版本不受支持"); return 1;
#endif

int Exfile(const char* filename, const char* outdir/*为空时解压在当前目录*/, std::string passwords /*NULL时没有密码*/) {
    NT_CHECK

#ifdef ENV_HAVE_LOCALE
        MY_SetLocale();
#endif
    FString dllPrefix;
    dllPrefix = NDLL::GetModuleDirPrefix();
    NDLL::CLibrary lib;
    if (!lib.Load(dllPrefix + FTEXT(kDllName)))
    {
        PrintError("Error: " kDllName " 文件加载失败");
        return 1;
    }
    Func_CreateObject createObjectFunc = (Func_CreateObject)lib.GetProc("CreateObject");
    char c = 0;
    UString password;
    bool passwordIsDefined = false;
    CObjectVector<FString> params;
    if (passwords != "NULL") {
        password = GetUnicodeString(passwords.c_str());
        passwordIsDefined = true;
    }

    AString a(filename/**/);
    params.Add(CmdStringToFString(a));
    const FString& archiveName = params[0];
    CMyComPtr<IInArchive> archive;
    createObjectFunc(&CLSID_Format, &IID_IInArchive, (void**)&archive);
    CInFileStream* fileSpec = new CInFileStream;
    CMyComPtr<IInStream> file = fileSpec;
    fileSpec->Open(archiveName);
    {
        CArchiveOpenCallback* openCallbackSpec = new CArchiveOpenCallback;
        CMyComPtr<IArchiveOpenCallback> openCallback(openCallbackSpec);
        openCallbackSpec->PasswordIsDefined = passwordIsDefined;
        openCallbackSpec->Password = password;

        const UInt64 scanSize = 1 << 23;
        archive->Open(file, &scanSize, openCallback);
    }
    //解压
    CArchiveExtractCallback* extractCallbackSpec = new CArchiveExtractCallback;
    CMyComPtr<IArchiveExtractCallback> extractCallback(extractCallbackSpec);
    extractCallbackSpec->Init(archive, FString(outdir)); // second parameter is output folder path
    extractCallbackSpec->PasswordIsDefined = passwordIsDefined;
    extractCallbackSpec->Password = password;
    archive->Extract(NULL, (UInt32)(Int32)(-1), false, extractCallback);
    return 0;
}

// Main function
// 
//int MY_CDECL main(int numArgs, const char* args[]);