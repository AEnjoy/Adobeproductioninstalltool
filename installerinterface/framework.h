// header.h: 标准系统包含文件的包含文件，
// 或特定于项目的包含文件
//

#pragma once
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
// Windows 头文件
#include <windows.h>
// C 运行时头文件
#include <iostream>
#include <string>
using namespace std;
#include <memory.h>
#include <tchar.h>
#include <Shlobj.h>
#include <Commctrl.h>
#define ACCESS_READ 1 
#define ACCESS_WRITE 2 
#define kDllName "extracting.dll" //7-zip dll
int Exfile(const char* filename, const char* outdir/*为空时解压在当前目录*/, std::string passwords /*NULL时没有密码*/);
DWORD getwinverdwBuildNumber();
BOOL FindFirstFileExists(LPCTSTR lpPath, DWORD dwFilter = FALSE);
int get_file_md5(const string& file_name, string& md5_value);
string WCharToMByte(LPCWSTR lpcwszStr);