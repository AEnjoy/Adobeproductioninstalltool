#pragma once
// �ر� MFC ��һЩ�����Ҿ����ɷ��ĺ��Ե����ؾ�����Ϣ
#define _AFX_ALL_WARNINGS
//#include <afxwin.h>         // MFC ��������ͱ�׼���
//#include <afxext.h>         // MFC ��չ
//#include <afxdisp.h>        // MFC �Զ�����
//#define _AFXDLL
#include "resource.h"
#include "..\duilib\UIlib.h"
#pragma comment(lib, "../duilib/DuiLib.lib")//x86
#pragma comment(lib, "lib/libcrypto.lib")
#pragma comment(lib, "lib/libssl.lib")
#pragma comment(lib, "DXGI.lib")
//#pragma comment(lib,"Explorerframe.lib")
using namespace DuiLib;
//����
#include "include/releaseHelper.h"
#include "include/download.h"
#include "shobjidl.h"

//#include "include/md5.h"

#define includejson //��Դ����json��
//#define includeinstallerfile //ѹ��������exe
#define purl "https://raw.fastgit.org/AEnjoy/Adobeproductioninstalltool/config/file.ini"

//"https://gcore.jsdelivr.net/gh/AEnjoy/Adobeproductioninstalltool@config/file.ini"

typedef struct {
	string  file_format_version,ver, url, savename, md5, password, execname, api_version;
	int32_t needspace/*KB*/, zipspace/*KB*/;
} iinfo;

