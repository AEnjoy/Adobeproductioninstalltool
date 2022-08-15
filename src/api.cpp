#include "framework.h"
#include "CMainWnd.h"
#include <iostream>
#include <cstdio>
#include <string>
#include <atlbase.h>
#include <shobjidl.h>
#include <windows.h>
#include <atlstr.h>
#include <DXGI.h>
#include <io.h>
using namespace std;
using namespace DuiLib;
string WCharToMByte(LPCWSTR lpcwszStr)
{
    string str;
    DWORD dwMinSize = 0;
    LPSTR lpszStr = NULL;
    dwMinSize = WideCharToMultiByte(CP_OEMCP, NULL, lpcwszStr, -1, NULL, 0, NULL, FALSE);
    if (0 == dwMinSize)
    {
        return FALSE;
    }
    lpszStr = new char[dwMinSize];
    WideCharToMultiByte(CP_OEMCP, NULL, lpcwszStr, -1, lpszStr, dwMinSize, NULL, FALSE);
    str = lpszStr;
    delete[] lpszStr;
    return str;
}
wstring UTF8ToUnicode(const char* strSrc)
{
    std::wstring wstrRet;

    if (NULL != strSrc)
    {
        int len = MultiByteToWideChar(CP_UTF8, 0, strSrc, -1, NULL, 0) * sizeof(WCHAR);
        WCHAR* strDst = new(std::nothrow) WCHAR[len + 1];
        if (NULL != strDst)
        {
            MultiByteToWideChar(CP_UTF8, 0, strSrc, -1, strDst, len);
            wstrRet = strDst;;
            delete[]strDst;
        }
    }

    return wstrRet;
}
string UnicodeToAnsi(const WCHAR* strSrc)
{
    string strRet;

    if (NULL != strSrc)
    {
        int len = WideCharToMultiByte(CP_ACP, 0, strSrc, -1, NULL, 0, NULL, NULL);
        char* strDst = new(std::nothrow) char[len + 1];
        if (NULL != strDst)
        {
            WideCharToMultiByte(CP_ACP, 0, strSrc, -1, strDst, len, NULL, NULL);
            strRet = strDst;
            delete[]strDst;
        }
    }

    return strRet;
}
/*文件是否存在*/
bool FindFirstFileExists(LPCTSTR lpPath, DWORD dwFilter=0) {
    return _waccess(lpPath, dwFilter) != -1;
}
DWORD getwinverdwBuildNumber() //int such as 19041
{
    typedef void(__stdcall* NTPROC)(DWORD*, DWORD*, DWORD*);
    HINSTANCE hinst = LoadLibrary(_T("ntdll.dll"));
    DWORD dwMajor, dwMinor, dwBuildNumber;
    NTPROC proc = (NTPROC)GetProcAddress(hinst, "RtlGetNtVersionNumbers");
    proc(&dwMajor, &dwMinor, &dwBuildNumber);
    FreeLibrary(hinst);
    return dwBuildNumber;
}
int CountvAdapersMounts() {
    IDXGIFactory* pFactory; IDXGIAdapter* pAdapter;
    int iAdapterNum = 0;
    HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&pFactory));
    if (FAILED(hr))
        return 0;
    while (pFactory->EnumAdapters(iAdapterNum, &pAdapter) != DXGI_ERROR_NOT_FOUND)iAdapterNum++;
    return iAdapterNum;
}
string UTF8ToAnsi(const char* strSrc)
{
    return UnicodeToAnsi(UTF8ToUnicode(strSrc).c_str());
}
void _parserjsonfile(iinfo *&a,const char* filename) {
    //https://blog.csdn.net/normallife/article/details/52661632
    //string c = "..//";c=c + filename;
    TCHAR szCurrentDirectory[MAX_PATH];
    GetCurrentDirectoryW(MAX_PATH, szCurrentDirectory);
    CString f = szCurrentDirectory;f=f +filename;// c.c_str();
    a = new iinfo;
    CString t,v;
    GetPrivateProfileStringW(L"system",L"versionlist",L"",t.GetBuffer(), MAX_PATH, f);
    GetPrivateProfileStringW(t, L"leastversion", L"", v.GetBuffer(MAX_PATH), MAX_PATH, f);
    GetPrivateProfileStringW(v, L"file_format_version", L"", t.GetBuffer(MAX_PATH), MAX_PATH, f);
    a->file_format_version = CT2A(t.GetString());
    GetPrivateProfileStringW(v, L"url", L"", t.GetBuffer(MAX_PATH), MAX_PATH, f);
    a->url = CT2A(t.GetString());
    GetPrivateProfileStringW(v, L"savename", L"", t.GetBuffer(MAX_PATH), MAX_PATH, f);
    a->savename = CT2A(t.GetString());
    GetPrivateProfileStringW(v, L"md5", L"", t.GetBuffer(MAX_PATH), MAX_PATH, f);
    a->md5 = CT2A(t.GetString());
    GetPrivateProfileStringW(v, L"password", L"", t.GetBuffer(MAX_PATH), MAX_PATH, f);
    a->password = CT2A(t.GetString());
    {
        GetPrivateProfileStringW(v, L"execname", L"", t.GetBuffer(MAX_PATH), MAX_PATH, f);
        a->execname = UTF8ToAnsi(CT2A(t.GetString()));
    }
    GetPrivateProfileStringW(v, L"api_version", L"", t.GetBuffer(MAX_PATH), MAX_PATH, f);
    a->api_version = CT2A(t.GetString());
    a->zipspace = GetPrivateProfileIntW(v, L"zip", 0, f);
    a->needspace = GetPrivateProfileIntW(v, L"needspace", 0, f);
    a->ver = CT2A(v.GetString());
}
wstring folder_open_dialog()
{
    wstring ret;
    CComPtr<IFileOpenDialog> spFileOpenDialog;
    if (SUCCEEDED(spFileOpenDialog.CoCreateInstance(__uuidof(FileOpenDialog)))) {
        FILEOPENDIALOGOPTIONS options;
        if (SUCCEEDED(spFileOpenDialog->GetOptions(&options))) {
            spFileOpenDialog->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
            if (SUCCEEDED(spFileOpenDialog->Show(nullptr))) {
                CComPtr<IShellItem> spResult;
                if (SUCCEEDED(spFileOpenDialog->GetResult(&spResult))) {
                    wchar_t* name;
                    if (SUCCEEDED(spResult->GetDisplayName(SIGDN_FILESYSPATH, &name))) {
                        ret = name;
                        CoTaskMemFree(name);
                    }
                }
            }
        }
    }
    return move(ret);
}
bool isVmemorysatisfied(size_t &memsize,wstring &devicesname) {
    IDXGIFactory* pFactory;IDXGIAdapter* pAdapter;
    vector <IDXGIAdapter*> vAdapters;            // 显卡  
    int iAdapterNum = 0;
    bool flag=false;
    HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&pFactory));
    if (FAILED(hr))
        return flag;
    while (pFactory->EnumAdapters(iAdapterNum, &pAdapter) != DXGI_ERROR_NOT_FOUND)
    {
        vAdapters.push_back(pAdapter);
        iAdapterNum++;
    }
    for (auto i : vAdapters) {
        DXGI_ADAPTER_DESC adapterDesc;
        i->GetDesc(&adapterDesc);
        auto t = (adapterDesc.DedicatedVideoMemory/* + adapterDesc.DedicatedSystemMemory + adapterDesc.SharedSystemMemory*/) / 1024 / 1024;
        if (t > 2048)
        {
            flag = true;
            memsize = t;
            devicesname=adapterDesc.Description;
        }
        if (!flag)
        {
            memsize = t;
            devicesname = adapterDesc.Description;
        }
    }
    return flag;
}
BOOL ExeIsAdmin()
{
    HANDLE hToken;
    DWORD dwStatus;
    DWORD dwAccessMask;
    DWORD dwAccessDesired;
    DWORD dwACLSize;
    DWORD dwStructureSize = sizeof(PRIVILEGE_SET);
    PACL pACL = NULL;
    PSID psidAdmin = NULL;
    BOOL bReturn = FALSE;
    PRIVILEGE_SET ps;
    GENERIC_MAPPING GenericMapping;
    PSECURITY_DESCRIPTOR psdAdmin = NULL;
    SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;
    if (!ImpersonateSelf(SecurityImpersonation))
        goto LeaveIsAdmin;
    if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hToken))
    {
        if (GetLastError() != ERROR_NO_TOKEN)
            goto LeaveIsAdmin;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
            goto LeaveIsAdmin;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
            goto LeaveIsAdmin;
    }
    if (!AllocateAndInitializeSid(&SystemSidAuthority, 2,
        SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS,
        0, 0, 0, 0, 0, 0, &psidAdmin))
        goto LeaveIsAdmin;
    psdAdmin = LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
    if (psdAdmin == NULL)
        goto LeaveIsAdmin;
    if (!InitializeSecurityDescriptor(psdAdmin,
        SECURITY_DESCRIPTOR_REVISION))
        goto LeaveIsAdmin;
    dwACLSize = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) +
        GetLengthSid(psidAdmin) - sizeof(DWORD);
    pACL = (PACL)LocalAlloc(LPTR, dwACLSize);
    if (pACL == NULL)
        goto LeaveIsAdmin;
    if (!InitializeAcl(pACL, dwACLSize, ACL_REVISION2))
        goto LeaveIsAdmin;
    dwAccessMask = ACCESS_READ | ACCESS_WRITE;
    if (!AddAccessAllowedAce(pACL, ACL_REVISION2, dwAccessMask, psidAdmin))
        goto LeaveIsAdmin;
    if (!SetSecurityDescriptorDacl(psdAdmin, TRUE, pACL, FALSE))
        goto LeaveIsAdmin;
    if (!SetSecurityDescriptorGroup(psdAdmin, psidAdmin, FALSE))
        goto LeaveIsAdmin;
    if (!SetSecurityDescriptorOwner(psdAdmin, psidAdmin, FALSE))
        goto LeaveIsAdmin;
    if (!IsValidSecurityDescriptor(psdAdmin))
        goto LeaveIsAdmin;
    dwAccessDesired = ACCESS_READ;
    GenericMapping.GenericRead = ACCESS_READ;
    GenericMapping.GenericWrite = ACCESS_WRITE;
    GenericMapping.GenericExecute = 0;
    GenericMapping.GenericAll = ACCESS_READ | ACCESS_WRITE;
    if (!AccessCheck(psdAdmin, hToken, dwAccessDesired,
        &GenericMapping, &ps, &dwStructureSize, &dwStatus, &bReturn))
        goto LeaveIsAdmin;
    if (!RevertToSelf())
        bReturn = FALSE;
LeaveIsAdmin:
    if (pACL) LocalFree(pACL);
    if (psdAdmin) LocalFree(psdAdmin);
    if (psidAdmin) FreeSid(psidAdmin);
    return bReturn;
}
progress::progress() {
    m_hImageList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 0, 0);
}
progress::~progress() {
    ImageList_Destroy(m_hImageList);
}
void progress::Attach(HWND hWnd) {
    m_hWnd = hWnd;
	if (NULL == m_pTaskBarlist)
	{
		    CoCreateInstance(
			CLSID_TaskbarList, NULL, CLSCTX_ALL,
			IID_ITaskbarList3, (void**)&m_pTaskBarlist);
	}
	m_pTaskBarlist->SetProgressState(m_hWnd, TBPF_INDETERMINATE);
}
void progress::SetProgressValue(ULONGLONG ullCompleted, ULONGLONG ullTotal)
{
    m_pTaskBarlist->SetProgressValue(m_hWnd, ullCompleted, ullTotal);
}
void progress::SetProgressState(TBPFLAG tbpFlags = TBPF_NOPROGRESS) {
    m_pTaskBarlist->SetProgressState(m_hWnd, TBPF_NOPROGRESS);
}
string& replace_all(string& src, const string& old_value, const string& new_value) {
    for (string::size_type pos(0); pos != string::npos; pos += new_value.length()) {
        if ((pos = src.find(old_value, pos)) != string::npos) {
            src.replace(pos, old_value.length(), new_value);
        }
        else break;
    }
    return src;
}