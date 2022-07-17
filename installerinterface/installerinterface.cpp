#include "framework.h"
#include "installerinterface.h"
#include "CMainWnd.h"
#define MAX_LOADSTRING 100
static const char* filepath = "file.ini";
#pragma comment(lib,"ntdll.lib")
CMainWnd* pFrame = new CMainWnd();

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
	//CPaintManagerUI::SetResourceType(UILIB_FILE);
	// 资源路径
	//CPaintManagerUI::SetInstance(hInstance);
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath());
	CResourceManager::GetInstance()->LoadResource(_T("main.xml"), NULL);
	CDuiString strResourcePath = CPaintManagerUI::GetInstancePath();
	strResourcePath += _T("res\\");
	CPaintManagerUI::SetResourcePath(strResourcePath.GetData());
	//CDuiString strResourcePath = CPaintManagerUI::GetInstancePath();
	// 加载资源
	/*switch (CPaintManagerUI::GetResourceType())
	{
	case UILIB_FILE:
	{
		//strResourcePath += _T("res\\");
		//CPaintManagerUI::SetResourcePath(strResourcePath.GetData());
		// 加载资源管理器
		CResourceManager::GetInstance()->LoadResource(_T("main.xml"), NULL);
		break;
	}
	case UILIB_RESOURCE:
	{
		strResourcePath += _T("res\\");
		CPaintManagerUI::SetResourcePath(strResourcePath.GetData());
		// 加载资源管理器
		CResourceManager::GetInstance()->LoadResource(_T("IDR_RES"), _T("xml"));
		break;
	}
	case UILIB_ZIP:
	{
		strResourcePath += _T("res\\");
		CPaintManagerUI::SetResourcePath(strResourcePath.GetData());
		CPaintManagerUI::SetResourceZip(_T("res.zip"), true);
		// 加载资源管理器
		CResourceManager::GetInstance()->LoadResource(_T("main.xml"), NULL);
		break;
	}
	case UILIB_ZIPRESOURCE:
	{
		strResourcePath += _T("res\\");
		CPaintManagerUI::SetResourcePath(strResourcePath.GetData());

		HRSRC hResource = ::FindResource(CPaintManagerUI::GetResourceDll(), _T("IDR_ZIPRES"), _T("ZIPRES"));
		if (hResource != NULL) {
			DWORD dwSize = 0;
			HGLOBAL hGlobal = ::LoadResource(CPaintManagerUI::GetResourceDll(), hResource);
			if (hGlobal != NULL) {
				dwSize = ::SizeofResource(CPaintManagerUI::GetResourceDll(), hResource);
				if (dwSize > 0) {
					CPaintManagerUI::SetResourceZip((LPBYTE)::LockResource(hGlobal), dwSize);
					// 加载资源管理器
					CResourceManager::GetInstance()->LoadResource(_T("main.xml"), NULL);
				}
			}
			::FreeResource(hResource);
		}
	}
	break;
	}
*/
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

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	EnableMemLeakCheck();//防止内存泄露
#ifndef DEBUG
	if (!ExeIsAdmin()) 
	{
		MessageBoxA(NULL,"!)请使用管理员权限运行安装程序.","Error:", MB_ICONHAND);
		exit(1);
	}
#endif // DEBUG


	if (getwinverdwBuildNumber() < 17763)
	{
		MessageBoxA(NULL, "!) PS2022系统要求：Windows 10 1909(17763)及以上.", "Error:", MB_ICONHAND);
		exit(1);
	}
	HRESULT Hr = ::CoInitialize(NULL);
	CPaintManagerUI::SetInstance(hInstance);
	InitResource();
	
	if (pFrame == NULL)
		return 0;
CReleaseDLL releasehelper;
#ifdef includejson
#ifdef DEBUG
	releasehelper.FreeResFile(IDR_FILE1, "FILE", ::filepath);
	SetFileAttributesA(::filepath, FILE_ATTRIBUTE_HIDDEN);
#endif // !DEBUG
#endif // includejson
	//releasehelper.FreeResFile(IDR_FILE2, "FILE", kDllName);
	SetFileAttributesA(kDllName, FILE_ATTRIBUTE_HIDDEN);

	pFrame->Create(NULL, _T("Installer"), UI_WNDSTYLE_DIALOG, 0L, 0, 0, 900, 600);
	pFrame->CenterWindow();
	::ShowWindow(*pFrame, SW_SHOW);
	CPaintManagerUI::MessageLoop();
	::CoUninitialize();
	//GdiplusShutdown(gdiplusToken);
	_CrtDumpMemoryLeaks();
	return 0;
}
