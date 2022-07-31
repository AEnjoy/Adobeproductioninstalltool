#pragma once
#include "..\duilib\Utils\WinImplBase.h"
#include "installerinterface.h"
#include "include/pthreads/pthread.h"
#define TIMER_ID_PROGRESS (1000)
class CWndUI : public CControlUI
{
public:
    CWndUI() : m_hWnd(NULL) {}

    virtual void SetInternVisible(bool bVisible = true)
    {
        __super::SetInternVisible(bVisible);
        ::ShowWindow(m_hWnd, bVisible);
    }

    virtual void SetPos(RECT rc)
    {
        __super::SetPos(rc);
        ::SetWindowPos(m_hWnd, NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_NOZORDER | SWP_NOACTIVATE);
    }

    BOOL Attach(HWND hWndNew)
    {
        if (!::IsWindow(hWndNew))
        {
            return FALSE;
        }

        m_hWnd = hWndNew;
        return TRUE;
    }

    HWND Detach()
    {
        HWND hWnd = m_hWnd;
        m_hWnd = NULL;
        return hWnd;
    }

protected:
    HWND m_hWnd;
};

class CMainWnd : public WindowImplBase
{
    
public:

	CMainWnd();
	CDuiString GetSkinFile();
	LPCTSTR GetWindowClassName() const;
    virtual CControlUI* CreateControl(LPCTSTR pstrClassName)
    {
        //MessageBox(NULL, pstrClassName, pstrClassName,0);
        if (_tcsicmp(pstrClassName, _T("WndE")) == 0)
        {
            CWndUI* pUI = new CWndUI;
            HWND    hWnd = CreateWindow(_T("EDIT"), _T("win32"), WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 0, 0, 0, 0, m_pm.GetPaintWindow(), NULL, NULL, NULL);
            pUI->Attach(hWnd);         
            return pUI;
        }

        return NULL;
    }
	void InitWindow();
	void Notify(TNotifyUI& msg);
	void OnFinalMessage(HWND hWnd);
    void progress_();
    void environmental_inspection();
    void checkp();
    void downloadpack();
    void dirchanged(bool once);
    //HRESULT ShowProgressInTaskbar(HWND hwnd, __in ITaskbarList3* pTL);
    CProgressUI* m_pProgressBar, * m_download;
    CLabelUI* speed;
    iinfo* t0;
    bool first=true;//点击开始安装会变为false
private:
	void Init();
    void InstallStart();
	void InstallFinished();
    static void* ThreadFunc(void*);
    static void* ThreadFunc1(void* arg);
    static void* ThreadFunc2(void* arg);
    static void* ThreadFunc3(void* arg);
    float t;//当前安装位置剩余空间
private:
    //控件指针
	CAnimationTabLayoutUI* m_pStepTabLayout;//页面
    CCheckBoxUI* nowver;//第二页选择版本
    CEditUI* dir; //安装目录
    CLabelUI* needspace, *extspace;//所需空间,剩余空间
    CLabelUI *pver,*nver;
    CButtonUI *next3,*next2;
    CListUI* list;
    string downloadurl;
    bool downloadfinishflag = false;
    bool needdowload = false;
    pthread_t* thread;
    //wstring appname = L"Adobe Install Tool";
    //wstring info;
};
