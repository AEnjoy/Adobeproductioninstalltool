// UnPack.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "releaseHelper.h"
#include <iostream>
#include "resource.h"
#include <Windows.h>
#include <direct.h>
#include <corecrt_io.h>
#pragma warning(disable:4996)
using namespace std;
int main()
{
    CReleaseDLL releasehelper;
    char OutPath[MAX_PATH];
    GetTempPathA(MAX_PATH, OutPath);
    string s = OutPath; s += "adobeinstalltool";//末尾有"\"
    releasehelper.FreeResFile(IDR_FILE1, "FILE", "package.7z");
    releasehelper.FreeResFile(IDR_FILE2, "FILE", "7za.exe");
    CreateDirectoryA(s.c_str(), 0);
    system(("7za x package.7z -y -o\"" + s + "\"").c_str());
    remove("package.7z");
    remove("7za.exe");
    if (access("Photoshop2021.7z", 0) != -1)
    {
        char cmd[500];
        sprintf(cmd, "copy Photoshop2021.7z \"%s\" /Y", OutPath);
        system(cmd);
    }
    chdir(s.c_str());
    system("start installerinterface.exe");
    return 0;
}