#include "CMainWnd.h"
#include "framework.h"
#include <atlstr.h>
#include <string>
#include <cstdio>
#include <atlbase.h>
#include <shobjidl.h>
#include <windows.h>
#include <io.h>
#include <direct.h>
#include "include/pthreads/pthread.h"
using namespace std;
extern int downloadprog;//下载进度
int allowclose = 1;
iinfo* i_= nullptr;
/*文件是否存在*/
BOOL FindFirstFileExists(LPCTSTR lpPath, DWORD dwFilter) {
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(lpPath, &fd);
	BOOL bFilter = (FALSE == dwFilter) ? TRUE : fd.dwFileAttributes & dwFilter;
	BOOL RetValue = ((hFind != INVALID_HANDLE_VALUE) && bFilter) ? TRUE : FALSE;
	FindClose(hFind);
	return RetValue;
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

void* CMainWnd::ThreadFunc(void* arg)
{
	CMainWnd* thiz = static_cast<CMainWnd*>(arg);
	thiz->InstallStart();
	return nullptr;
}
void* CMainWnd::ThreadFunc1(void* arg)
{
	CMainWnd* thiz = static_cast<CMainWnd*>(arg);
	thiz->downloadpack();
	return nullptr;
}
CMainWnd::CMainWnd()
	: m_pStepTabLayout(NULL)
	, nowver(NULL)
	, dir(NULL)
	, needspace(NULL)
	, extspace(NULL)
	,m_pProgressBar(NULL)
	, m_download(NULL)
	,pver(NULL)
	, nver(NULL)
	,next3(NULL)
	,next2(NULL)
	,t0(NULL)
{
}

CDuiString CMainWnd::GetSkinFile()
{
	try
	{
		return CDuiString(_T("main.xml"));
	}
	catch (...)
	{
		throw "CMainWnd::GetSkinFile";
	}
}

LPCTSTR CMainWnd::GetWindowClassName() const
{
		return _T("Install");
}

void CMainWnd::InitWindow()
{
		WindowImplBase::InitWindow();
		Init();
}
void CMainWnd::progress_() {
	Sleep(500);
	m_download->SetValue(downloadprog);
}
void CMainWnd::downloadpack()
{
	next3->SetVisible(false);
	downLoad(32, downloadurl, t0->savename);
	downloadfinishflag = true;
	pthread_t th; int* thread_ret = NULL;
	pthread_create(&th, NULL, CMainWnd::ThreadFunc, this);
}
void CMainWnd::dirchanged(bool once)
{
	CString installdir = dir->GetText().GetData();
	DWORD dwTotalClusters;
	DWORD dwFreeClusters;
	DWORD dwSectPerClust;
	DWORD dwBytesPerSect;
	GetDiskFreeSpaceW(installdir.Left(2), &dwSectPerClust, &dwBytesPerSect, &dwFreeClusters, &dwTotalClusters);
	t = (static_cast<float>((DWORD64)dwFreeClusters) * (DWORD64)dwSectPerClust * (DWORD64)dwBytesPerSect) / 1024 / 1024 / 1024;
	char t5[30];
	sprintf(t5, "%0.2f GB", t);
	extspace->SetText(CString(t5));
	if (t * 1024 * 1024 < t0->needspace)
	{
		MessageBoxA(GetHWND(), "!)磁盘剩余空间不足,请重新选择.", "Error:", MB_ICONERROR);
		next2->SetVisible(false);
	}
	else next2->SetVisible();
	/*
	if (once) {
		if (_waccess(installdir, 0) == -1)
			_wmkdir(installdir);
	}
	else {
		if (_waccess(installdir, 0) == -1) {
			MessageBoxA(GetHWND(), "安装目录不存在\n已自动创建", "目录不存在", MB_ICONEXCLAMATION);
			_wmkdir(installdir);
		}
	}
	*/
}
void CMainWnd::Notify(TNotifyUI& msg)
{
	if (msg.sType == _T("click"))
	{//msg.pSender == m_pBtnOneClick
		if (msg.pSender->GetName() == _T("closebtn") && allowclose==0) {
			MessageBoxW(NULL,L"正在安装,不允许退出",L"Error:", MB_ICONHAND);
			return;
		}
		if (msg.pSender->GetName() == _T("last1"))
		{
			//InstallStart();
			m_pStepTabLayout->SelectItem(0);
			return;
		}
		if (msg.pSender->GetName() == _T("next1"))
		{
			//InstallStart();
			m_pStepTabLayout->SelectItem(1);
			return;
		}
		if (msg.pSender->GetName() == _T("cdir"))
		{CString strFolderPath=folder_open_dialog().c_str();
		if (strFolderPath.GetLength() > 2)
		{
			dir->SetText(strFolderPath);
			dirchanged(false);
		}
		}
		if (msg.pSender->GetName() == _T("checkp")) {
#ifndef DEBUG
if( downLoad(1,purl,"file.ini"))
{
	SetFileAttributesA("file.ini", FILE_ATTRIBUTE_HIDDEN);
	_parserjsonfile(i_, "\\file.ini");
	t0 = i_ ;
}
else {
	MessageBoxA(GetHWND(), "获取云端版本失败!", "Error:", MB_ICONERROR);
}
#else 
			_parserjsonfile(i_, "\\file.ini");
#endif // DEBUG

			CString s = i_->ver.c_str();
			pver->SetText(s);
			MessageBoxW(NULL,L"最新版本:\n"+s,L"操作完成", 0);
			string s1 = to_string(float(i_->needspace / 1024/1024)) + "GB";
			CString nt = s1.c_str();
			needspace->SetText(nt);
		}
		if (msg.pSender->GetName() == _T("next2"))//下一步安装
		{
			m_pStepTabLayout->SelectItem(2);
			pthread_t th; int* thread_ret = NULL;
			pthread_create(&th, NULL, CMainWnd::ThreadFunc,this);
			//next3->SetVisible(false);
			//pthread_join(th, (void**)&thread_ret);
			//Sleep(5500);
			//InstallStart();
			return;
		}
		if (msg.pSender->GetName() == _T("next3")) {
			if (needdowload)
			{
				if (downloadfinishflag)
				{
					pthread_t th; int* thread_ret = nullptr;
					pthread_create(&th, NULL, CMainWnd::ThreadFunc, this);
				}
				else {
					MessageBoxW(NULL, L"资源下载未完成,请稍后再试.", L"请等待:", MB_ICONERROR);
				}
			}
			else
				m_pStepTabLayout->SelectItem(3);
			//DeleteFileA(kDllName);
#ifndef DEBUG
//DeleteFileA("file.ini");
#endif // DEBUG
		}
		if (msg.pSender->GetName() == _T("next4"))
		{
			exit(0);
		}
		
	}
	else if (msg.sType == _T("textchanged")) {
		//if (msg.pSender->GetName() == _T("dir"))
		
	}
	WindowImplBase::Notify(msg);
}

void CMainWnd::OnFinalMessage(HWND hWnd)
{
	try
	{
		WindowImplBase::OnFinalMessage(hWnd);
		PostQuitMessage(0);
		delete this;
	}
	catch (...)
	{
		throw "CMainWnd::OnFinalMessage";
	}
}
void CMainWnd::Init()
{	//初始化控件
	{
	m_pStepTabLayout = static_cast<CAnimationTabLayoutUI*>(m_pm.FindControl(_T("stepTabLayout")));
	nowver = static_cast<CCheckBoxUI*>(m_pm.FindControl(_T("nowver")));
	dir = static_cast<CEditUI*>(m_pm.FindControl(_T("dir")));
	needspace = static_cast<CLabelUI*>(m_pm.FindControl(_T("needspace")));
	extspace = static_cast<CLabelUI*>(m_pm.FindControl(_T("extspace")));
	m_pProgressBar = static_cast<CProgressUI*>(m_pm.FindControl(_T("install")));
	m_download = static_cast<CProgressUI*>(m_pm.FindControl(_T("download")));
	pver = static_cast<CLabelUI*>(m_pm.FindControl(_T("pver")));
	nver = static_cast<CLabelUI*>(m_pm.FindControl(_T("nowver")));
	next3 = static_cast<CButtonUI*>(m_pm.FindControl(_T("next3")));
	next2 = static_cast<CButtonUI*>(m_pm.FindControl(_T("next2")));
	}
#ifdef includejson
	
	

	_parserjsonfile(t0,"\\file.ini");
	nver->SetText(CString(t0->ver.c_str()));
	char t5[30];
	sprintf(t5, "%0.2f GB", float(t0->needspace / static_cast<double>(1024) / 1024));
	needspace->SetText(CString(t5));
	next3->SetVisible(false);
#else
	nver->SetText(L"NULL");
#endif // includejson
	m_pProgressBar->SetValue(0);
	m_download->SetValue(0);
	dirchanged(true);
}
void CMainWnd::InstallStart()
{
	next3->SetVisible(false);
	allowclose = 0;//不允许关闭安装窗口
	CString resfilename = t0->savename.c_str();
	downloadurl = t0->url;
	goto check;
check:
#ifndef DEBUG
	string md5;
	get_file_md5(t0->savename, md5);
#endif // !DEBUG
#ifndef includeinstallerfile
	if (!FindFirstFileExists(resfilename)
#ifndef DEBUG
		||(md5 != strlwr((char*)t0->md5.c_str()))
#endif // !DEBUG		
		)
	{
		MessageBoxW(GetHWND(), L"MD5检测失败或安装资源文件不存在,\n点击 ＂确定＂ 重新下载...", L"Error:", MB_ICONHAND);
		needdowload = true;
		DeleteFileW(resfilename);
		pthread_t th; int* thread_ret = nullptr;
		pthread_create(&th, NULL, CMainWnd::ThreadFunc1, this);
		next3->SetVisible(true);
		return;
		//pthread_join(th, (void**)&thread_ret);
		//while (!downloadfinishflag);
		//goto check;
	}
		
	//MD5 md5;
	//md5.reset();
	//ifstream i(t0->savename);
	//md5.update(i);


#else
	CString strPath;
	GetModuleFileName(NULL, strPath.GetBufferSetLength(MAX_PATH + 1), MAX_PATH + 1);
	resfilename = strPath;
#endif // includeinstallerfile
	CString t = dir->GetText().GetData();
	if (t0->password == "")t0->password = "NULL";
	string t3 = WCharToMByte(t.GetBuffer());//+ "\\Photoshop"
#ifndef DEBUG
	Exfile(t0->savename.c_str(),t3.c_str(), t0->password);
#endif
	string command = t3+"\\";
	command += t0->execname;
	//MessageBoxA(0, ("cmd /c \"" + command + "\"").c_str(), "Debug:", MB_OK);
	system(("cmd /c \""+command+"\"").c_str());
	InstallFinished();
}

void CMainWnd::InstallFinished()
{
allowclose = 1;
needdowload = false;
next3->SetVisible(true);
}
