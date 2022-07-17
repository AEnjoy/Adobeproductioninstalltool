#pragma once
#include "..\duilib\Utils\WinImplBase.h"
#include "installerinterface.h"
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
    static void* ThreadFunc(void*);
    static void* ThreadFunc1(void* arg);
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

    void downloadpack();
    void dirchanged(bool once);
    CProgressUI* m_pProgressBar, * m_download;
    iinfo* t0;
private:
	void Init();
    void InstallStart();
	void InstallFinished();
    float t;//��ǰ��װλ��ʣ��ռ�
private:
    //�ؼ�ָ��
	CAnimationTabLayoutUI* m_pStepTabLayout;//ҳ��
    CCheckBoxUI* nowver;//�ڶ�ҳѡ��汾
    CEditUI* dir; //��װĿ¼
    CLabelUI* needspace, *extspace;//����ռ�,ʣ��ռ�
    CLabelUI *pver,*nver;
    CButtonUI *next3,*next2;

    string downloadurl;
    bool downloadfinishflag = false;
    bool needdowload = false;
};
