#include "framework.h"
#include "installerinterface.h"
#include "CMainWnd.h"
#define MAX_LOADSTRING 100
static const char* filepath = "file.ini";
#pragma comment(lib,"ntdll.lib")
CMainWnd* pFrame = new CMainWnd();
LPBYTE g_lpResourceZIPBuffer = NULL;
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
inline  void  EnableMemLeakCheck(int Breakpoint = 0) {
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
	if (Breakpoint)
		_CrtSetBreakAlloc(1912);
}
void InitResource()
{
	// 资源类型
	CPaintManagerUI::SetResourceType(UILIB_ZIPRESOURCE);
	// 资源路径
	//CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath());
	//CResourceManager::GetInstance()->LoadResource(_T("main.xml"), NULL);
	//CDuiString strResourcePath = CPaintManagerUI::GetInstancePath();
	//strResourcePath += _T("res\\");
	//CPaintManagerUI::SetResourcePath(strResourcePath.GetData());
	//CDuiString strResourcePath = CPaintManagerUI::GetInstancePath();
	// 加载资源
	HRSRC hResource = ::FindResource(CPaintManagerUI::GetResourceDll(), MAKEINTRESOURCE(IDR_FILE3), _T("FILE"));
	if (hResource == NULL)return;
	DWORD dwSize = 0;
	HGLOBAL hGlobal = ::LoadResource(CPaintManagerUI::GetResourceDll(), hResource);
	if (hGlobal == NULL)
	{
		::FreeResource(hResource);return;
	}
	dwSize = ::SizeofResource(CPaintManagerUI::GetResourceDll(), hResource);
	if (dwSize == 0)return;
	g_lpResourceZIPBuffer = new BYTE[dwSize];
	if (g_lpResourceZIPBuffer != NULL)
	{
		::CopyMemory(g_lpResourceZIPBuffer, (LPBYTE)::LockResource(hGlobal), dwSize);
	}
	::FreeResource(hResource);
	CPaintManagerUI::SetResourceZip(g_lpResourceZIPBuffer, dwSize);
}

auto APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow) ->int
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	EnableMemLeakCheck();//防止内存泄露
	HRESULT Hr = ::CoInitialize(NULL);
	CPaintManagerUI::SetInstance(hInstance);
	InitResource();
	
	if (pFrame == NULL)
		return 0;
//CReleaseDLL releasehelper;
#ifdef includejson
#ifdef DEBUG
	releasehelper.FreeResFile(IDR_FILE1, "FILE", ::filepath);
	SetFileAttributesA(::filepath, FILE_ATTRIBUTE_HIDDEN);
#endif // !DEBUG
#endif // includejson
	//releasehelper.FreeResFile(IDR_FILE2, "FILE", kDllName);
	//SetFileAttributesA(kDllName, FILE_ATTRIBUTE_HIDDEN);

	pFrame->Create(NULL, _T("Installer"), UI_WNDSTYLE_DIALOG, 0L, 0, 0, 900, 600);
	pFrame->CenterWindow();
	::ShowWindow(*pFrame, SW_SHOW);
	CPaintManagerUI::MessageLoop();
	::CoUninitialize();
	//GdiplusShutdown(gdiplusToken);
	_CrtDumpMemoryLeaks();
	return 0;
}
