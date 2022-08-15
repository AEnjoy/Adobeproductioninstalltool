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
#include <fstream>
using namespace std;
extern int downloadprog;//下载进度
int allowclose = 1;
int funcflag = -1;
iinfo* i_= nullptr;
static inline auto warningflag()->CControlUI* {
	auto warnning = new CControlUI;//! 
	warnning->SetMaxHeight(24);
	warnning->SetMinHeight(24);
	warnning->SetMaxWidth(24);
	warnning->SetMinWidth(24);
	warnning->SetBkImage(L"warning.png");
	return warnning;
}
static inline auto errorflag()->CControlUI* {
	auto error = new CControlUI;//X
	error->SetMaxHeight(24);
	error->SetMinHeight(24);
	error->SetMaxWidth(24);
	error->SetMinWidth(24);
	error->SetBkImage(L"error.png");
	return error;
}
static inline auto acceptflag()->CControlUI* {
	auto accept = new CControlUI;//√ 
	accept->SetMaxHeight(24);
	accept->SetMinHeight(24);
	accept->SetMaxWidth(24);
	accept->SetMinWidth(24);
	accept->SetBkImage(L"accept.png");
	return accept;
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
void* CMainWnd::ThreadFunc2(void* arg)
{
	CMainWnd* thiz = static_cast<CMainWnd*>(arg);
	thiz->environmental_inspection();
	return nullptr;
}
void* CMainWnd::ThreadFunc3(void* arg)
{
	CMainWnd* thiz = static_cast<CMainWnd*>(arg);
	MessageBoxA(0, "正在检查云端更新...", "Info:", MB_ICONINFORMATION);
	thiz->checkp();
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
	,list(nullptr)
	,speed(nullptr)
	, ProgressHelper(nullptr)
{
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
	//Sleep(500);
	m_download->SetValue(downloadprog);
}
void CMainWnd::environmental_inspection()
{
	//info = L"正在检查运行环境";
	//ShowNotification(appname, info);
	auto line = new CHorizontalLayoutUI;
	auto pLine = new CListContainerElementUI;
	pLine->SetTag(0);
	pLine->SetFixedHeight(24);
	auto text = new CLabelUI; //text->SetTextColor(0xff00ff00);
	pLine->Add(text);
	if (!ExeIsAdmin()) {
		text->SetText(L"W:建议使用管理员权限运行.");
		text->SetToolTip(L"W:!建议使用管理员权限运行,没有管理员权限将只能安装到用户目录.");
		pLine->Add(warningflag());
	}
	else {
		text->SetText(L"I:使用管理员权限安装.");
		pLine->Add(acceptflag());
	}
	list->Add(pLine);
	auto pLine1 = new CListContainerElementUI;
	auto text1 = new CLabelUI;// text1->SetTextColor(0xff00ff00);
	pLine1->SetTag(1);
	pLine1->SetFixedHeight(24);
	pLine1->Add(text1);
	auto t = getwinverdwBuildNumber();
	if (t < 17763) {
		text1->SetText(L"E:当前系统版本过低.");
		text1->SetToolTip(L"E:!Adobe CC 2022及以上所需要Windows10 build 17763及更高,当前系统版本不满足最低运行要求.");
		pLine1->Add(errorflag());
		next2->SetVisible(false);
	}
	else if (t >= 17763 && t < 19041) {
		text1->SetText(L"W:系统版本较旧,但满足最低运行需求.");
		text1->SetToolTip(L"W:!Adobe CC 2022及以上建议使用windows10 build 19041及更高版本.");
		pLine1->Add(warningflag());
	}
	else {
		text1->SetToolTip(L"I:系统版本需求满足windows10 build 19041及更高版本.");
		text1->SetText(L"I:系统版本满足推荐.");
		pLine1->Add(acceptflag());
	}
	list->Add(pLine1);
	auto pLine2 = new CListContainerElementUI;
	auto text2 = new CLabelUI; //text2->SetTextColor(0xff00ff00);
	pLine2->SetTag(2);
	pLine2->SetFixedHeight(24);
	pLine2->Add(text2);
	size_t memsize; wstring devicesname;
	if (!isVmemorysatisfied(memsize, devicesname))
	{
		char strsize[250];
#ifdef _WIN64
		sprintf(strsize, "W:您当前电脑所使用的显卡(核显)(最低)未满足PS最低显存所需(或未安装显卡驱动),可能无法使用某些功能.当前显存为:%lldMB 设备描述:%s", memsize, WCharToMByte(devicesname.c_str()).c_str());
#else
sprintf(strsize, "W:您当前电脑所使用的显卡(核显)(最低)未满足PS最低显存所需(或未安装显卡驱动),可能无法使用某些功能.当前显存为:%dMB 设备描述:%s", memsize, WCharToMByte(devicesname.c_str()).c_str());
#endif // _WIN64
		text2->SetText(L"W:显存不足");
		text2->SetToolTip(CString(strsize));
		pLine2->Add(warningflag());
	}
	else {
		char strsize[250];
#ifdef _WIN64
		sprintf(strsize, "I:显卡(核显)(最低)满足PS最低显存所需.当前显存为:%lldMB 当前GPU:%s", memsize, WCharToMByte(devicesname.c_str()).c_str());
#else
sprintf(strsize, "I:显卡(核显)(最低)满足PS最低显存所需.当前显存为:%dMB 当前GPU:%s", memsize, WCharToMByte(devicesname.c_str()).c_str());
#endif
		
		text2->SetText(L"I:显存充足");
		text2->SetToolTip(CString(strsize));
		pLine2->Add(acceptflag());
	}
	list->Add(pLine2);
#ifndef _WIN64
#ifdef _WIN64
	WinExec("cmd /c %windir%\SysWOW64\dxdiag.exe /t info.txt",0);
#else
	WinExec("dxdiag /t info.txt",0);
	while (access("info.txt", 0) != 0)Sleep(50);
#endif
	SetFileAttributesW(L"info.txt", FILE_ATTRIBUTE_HIDDEN);
	auto pLine3 = new CListContainerElementUI;
	auto text3 = new CLabelUI; //text2->SetTextColor(0xff00ff00);
	pLine3->SetTag(3);
	pLine3->SetFixedHeight(24);
	pLine3->Add(text3);
	ifstream f; string li;
	f.open("info.txt");
	double cl=0.0;
	for (int i = 0; i < CountvAdapersMounts(); i++) {
		do {
			if (!getline(f, li))break;
		} while (li.find("DDI Version:") == string::npos);
		li = replace_all(li, " ", "");
		li = replace_all(li, "DDIVersion:", "");
		auto t3=atof(li.c_str());
		if (t3 > cl)cl = t3;
	}
	f.close();
	remove("info.txt");
	if (cl < 12) {
		text3->SetText(L"W:GPU不受支持");
		char strsize[250];
		sprintf(strsize, "W:您的GPU不满足Adobe对于DirectX12(GPU至少一个)的需求,可能会出现兼容问题.当前GPU的DirectX版本:%0.2f", cl);
		text3->SetToolTip(CString(strsize));
		pLine3->Add(warningflag());
	}
	else {
		text3->SetText(L"I:GPU兼容");
		text3->SetToolTip(L"I:GPU满足(至少一个)兼容DirectX12.");
		pLine3->Add(acceptflag());
	}
	list->Add(pLine3);
#endif // !_WIN64
}
void CMainWnd::checkp()
{
#ifndef DEBUG
	if (downLoad(1, purl, "file.ini"))
	{
		SetFileAttributesA("file.ini", FILE_ATTRIBUTE_HIDDEN);
		_parserjsonfile(i_, "\\file.ini");
		t0 = i_;
	}
	else {
		MessageBoxA(GetHWND(), "获取云端版本失败!", "Error:", MB_ICONERROR);
		return;
	}
#else 
	_parserjsonfile(i_, "\\file.ini");
#endif // DEBUG
	CString s = i_->ver.c_str();
	pver->SetText(s);
	MessageBoxW(NULL, L"最新版本:\n" + s, L"操作完成", 0);
	char t5[30];
	sprintf(t5, "%0.2f GB", float(i_->needspace / static_cast<double>(1024) / 1024));
	needspace->SetText(CString(t5));
}
void CMainWnd::downloadpack()
{
	ProgressHelper->SetProgressState(TBPF_NORMAL);
	next3->SetVisible(false);
	downLoad(32, downloadurl, t0->savename);
	downloadfinishflag = true;
	pthread_t th; int* thread_ret = NULL;
	//funcflag = 0;
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
			pthread_t th; int* thread_ret = NULL;
			funcflag = 3;
			pthread_create(&th, NULL, CMainWnd::ThreadFunc3, this);
		}
		if (msg.pSender->GetName() == _T("next2"))//下一步安装
		{
			first = false;
			m_pStepTabLayout->SelectItem(2);
			pthread_t th; int* thread_ret = NULL;
			funcflag = 0;
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
					funcflag = 0;
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
	list = static_cast<CListUI*>(m_pm.FindControl(_T("enr")));
	speed = static_cast<CLabelUI*>(m_pm.FindControl(_T("speed")));
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
	pthread_t th; int* thread_ret = NULL;
	funcflag = 2;
	pthread_create(&th, NULL, CMainWnd::ThreadFunc2, this);
	thread = new pthread_t;
	//if(first)
	pthread_create(thread, NULL, GetInternetSpeed, NULL);
	ProgressHelper = new progress;
	ProgressHelper->Attach(GetHWND());
	//funcflag = -1;
}
void CMainWnd::InstallStart()
{
	ProgressHelper->SetProgressState(TBPF_INDETERMINATE);
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
		ProgressHelper->SetProgressState(TBPF_ERROR);
		needdowload = true;
		DeleteFileW(resfilename);
		pthread_t th; int* thread_ret = nullptr;
		funcflag = 1;
		pthread_create(&th, NULL, CMainWnd::ThreadFunc1, this);
		next3->SetVisible(true);
		return;
		//pthread_join(th, (void**)&thread_ret);
		//while (!downloadfinishflag);
		//goto check;
	}
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
	Sleep(5500);
	InstallFinished();
}

void CMainWnd::InstallFinished()
{
allowclose = 1;
needdowload = false;
next3->SetVisible(true);
first = true;
ProgressHelper->SetProgressState(TBPF_NOPROGRESS);
}
