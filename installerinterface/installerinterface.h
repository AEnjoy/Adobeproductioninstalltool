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
using namespace DuiLib;
//����
#include "include/releaseHelper.h"
#include "include/download.h"
//#include "include/md5.h"

#define includejson //��Դ����json��
//#define includeinstallerfile //ѹ��������exe
#define purl "http://127.0.0.1/file.ini"

typedef struct {
	string  file_format_version,ver, url, savename, md5, password, execname, api_version;
	int32_t needspace/*KB*/, zipspace/*KB*/;
} iinfo;
void _parserjsonfile(iinfo*& a, const char* filename = "\\file.ini");
wstring folder_open_dialog();
