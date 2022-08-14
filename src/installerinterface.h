#pragma once
// 关闭 MFC 的一些常见且经常可放心忽略的隐藏警告消息
#define _AFX_ALL_WARNINGS
//#include <afxwin.h>         // MFC 核心组件和标准组件
//#include <afxext.h>         // MFC 扩展
//#include <afxdisp.h>        // MFC 自动化类
//#define _AFXDLL
#include "resource.h"
#include "..\duilib\UIlib.h"
#pragma comment(lib, "../duilib/DuiLib.lib")//x86
#pragma comment(lib, "lib/libcrypto.lib")
#pragma comment(lib, "lib/libssl.lib")
#pragma comment(lib, "DXGI.lib")
//#pragma comment(lib,"Explorerframe.lib")
using namespace DuiLib;
//功能
#include "include/releaseHelper.h"
#include "include/download.h"
#include "shobjidl.h"

//#include "include/md5.h"

#define includejson //资源含有json项
//#define includeinstallerfile //压缩包含于exe
#define purl "https://raw.fastgit.org/AEnjoy/Adobeproductioninstalltool/config/file.ini"

//"https://gcore.jsdelivr.net/gh/AEnjoy/Adobeproductioninstalltool@config/file.ini"

typedef struct {
	string  file_format_version,ver, url, savename, md5, password, execname, api_version;
	int32_t needspace/*KB*/, zipspace/*KB*/;
} iinfo;

