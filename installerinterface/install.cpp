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
bool isVmemorysatisfied(size_t &memsize) {
    IDXGIFactory* pFactory;IDXGIAdapter* pAdapter;
    vector <IDXGIAdapter*> vAdapters;            // ÏÔ¿¨  
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
        auto t=adapterDesc.DedicatedVideoMemory / 1024 / 1024;
        if (t > 2048)
        {
            flag = true;
            memsize = t;
        }
        if(!flag)
            memsize = t;
    }
    return flag;
}