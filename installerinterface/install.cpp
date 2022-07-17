#include "framework.h"
#include "CMainWnd.h"
#include <iostream>
#include <cstdio>
#include <string>
#include <atlbase.h>
#include <shobjidl.h>
#include <windows.h>
#include <atlstr.h>
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
    GetPrivateProfileStringW(v, L"execname", L"", t.GetBuffer(MAX_PATH), MAX_PATH, f);
    a->execname = CT2A(t.GetString());
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