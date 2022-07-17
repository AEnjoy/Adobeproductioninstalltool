#include "releaseHelper.h"
#include <iostream>
#include <direct.h>
CReleaseDLL::CReleaseDLL()
{
	this->m_hModule = GetSelfModuleHandle();
	if (m_hModule == NULL)
	{
		throw std::exception("Error:获取基址失败");
	}
	//获取目录
	memset(this->m_filePath, 0, MAX_DLL_PATH);
	_getcwd(this->m_filePath, MAX_DLL_PATH);
}

bool CReleaseDLL::FreeResFile(unsigned long m_lResourceID, const char* m_strResourceType, const char* m_strFileName)
{
	char strFullPath[MAX_DLL_PATH] = { 0 };
	sprintf_s(strFullPath, "%s\\%s", this->m_filePath, m_strFileName);
	//查找资源
	HRSRC hResID = ::FindResourceA(this->m_hModule, MAKEINTRESOURCEA(m_lResourceID), m_strResourceType);
	//加载资源  
	HGLOBAL hRes = ::LoadResource(this->m_hModule, hResID);
	//锁定资源
	LPVOID pRes = ::LockResource(hRes);
	if (pRes == NULL)
	{
		return FALSE;
	}
	//得到待释放资源文件大小 
	unsigned long dwResSize = ::SizeofResource(this->m_hModule, hResID);
	//创建文件 

	HANDLE hResFile = CreateFileA(strFullPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hResFile)
	{
		return FALSE;
	}
	DWORD dwWrited = 0;
	if (FALSE == WriteFile(hResFile, pRes, dwResSize, &dwWrited, NULL))
	{

		return FALSE;
	}

	CloseHandle(hResFile);
	UnlockResource(hRes);
	FreeResource(hRes);
	return TRUE;
}

HMODULE CReleaseDLL::GetSelfModuleHandle()
{
	try
	{
#ifdef _USER_RELEASEDLL_
		MEMORY_BASIC_INFORMATION mbi;
		return ((::VirtualQuery((LPCVOID)&CReleaseDLL::GetSelfModuleHandle, &mbi, sizeof(mbi)) != 0)
			? (HMODULE)mbi.AllocationBase : NULL);
#else
		return ::GetModuleHandle(NULL);
#endif
	}
	catch (...)
	{
		return NULL;
	}

}