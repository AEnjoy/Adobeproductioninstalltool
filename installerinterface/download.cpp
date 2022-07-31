#include "include/curl/curl.h"
#include "include/pthreads/pthread.h"
#include <iostream>
#include <string>
#include "CMainWnd.h"
#include <io.h>
#include <atlstr.h>
#include "winsock2.h" 
#include "windows.h"
#include "winsock.h"
#include "iphlpapi.h"
#include "framework.h"
#pragma comment(lib,"Iphlpapi.lib")
#pragma comment(lib,"lib/libcurl.lib")
#pragma comment(lib,"lib/libpthread.lib")
using namespace std;
int downloadprog = 0;//下载进度
static long fileLength;
#define forcehttpspass(x) curl_easy_setopt(x, CURLOPT_SSL_VERIFYPEER, FALSE);
struct tNode
{
	FILE* fp;
	int startidx;
	int maxidx;
	void* _curl;
	pthread_t _tid;
};
struct progress {
	char* p;
	size_t size;
};
bool bError = false;
int threadCnt = 0;
static pthread_mutex_t foo_mutex = PTHREAD_MUTEX_INITIALIZER;
static DWORD dwBandIn = 0;           //下载速度 kb/s
static DWORD dwBandOut = 0;          //上传速度 kb/s
void* GetInternetSpeed(void* arg) {
	PMIB_IFTABLE    m_pTable = NULL;
	DWORD    m_dwAdapters = 0;
	ULONG    uRetCode = GetIfTable(m_pTable, &m_dwAdapters, TRUE);
	if (uRetCode == ERROR_NOT_SUPPORTED)
		return nullptr;
	if (uRetCode == ERROR_INSUFFICIENT_BUFFER)
		m_pTable = (PMIB_IFTABLE)new BYTE[65535];
	DWORD   dwLastIn = 0;//上一秒钟的接收字节数
	DWORD   dwLastOut = 0;//上一秒钟的发送字节数
	extern CMainWnd* pFrame;
	while (true)
	{
		GetIfTable(m_pTable, &m_dwAdapters, TRUE);
		DWORD   dwInOctets = 0;
		DWORD   dwOutOctets = 0;
		for (size_t i = 0; i < m_pTable->dwNumEntries; i++)
		{
			MIB_IFROW   Row = m_pTable->table[i];
			dwInOctets += Row.dwInOctets;
			dwOutOctets += Row.dwOutOctets;
		}
		dwBandIn = dwInOctets - dwLastIn;       //下载速度
		dwBandOut = dwOutOctets - dwLastOut;    //上床速速
		if (dwLastIn <= 0)
			dwBandIn = 0;
		else
			dwBandIn = dwBandIn / 1024; //b转换成kb
		if (dwLastOut <= 0)
			dwBandOut = 0;
		else
			dwBandOut = dwBandOut / 1024;   //b转换成kb
		dwLastIn = dwInOctets;
		dwLastOut = dwOutOctets;
		Sleep(1000);
	}
	delete[] m_pTable;
	return nullptr;
}
static size_t downLoadPackage(void* ptr, size_t size, size_t nmemb, void* userdata)
{
	tNode* node = (tNode*)userdata;
	size_t written = 0;
	pthread_mutex_lock(&foo_mutex);
	if (node->startidx + size * nmemb <= node->maxidx)
	{
		fseek(node->fp, node->startidx, SEEK_SET);
		written = fwrite(ptr, size, nmemb, node->fp);
		node->startidx += size * nmemb;
	}
	else
	{
		fseek(node->fp, node->startidx, SEEK_SET);
		written = fwrite(ptr, 1, node->maxidx - node->startidx + 1, node->fp);
		node->startidx = node->maxidx;
	}
	pthread_mutex_unlock(&foo_mutex);
	return written;
}

int assetsManagerProgressFunc(void* ptr, double totalToDownload, double nowDownloaded, double totalToUpLoad, double nowUpLoaded)
{
	static int percent = 0; double speed=0;
	CURL* easy_handle = static_cast<CURL*>(ptr);
	if(easy_handle)curl_easy_getinfo(easy_handle, CURLINFO_SPEED_DOWNLOAD, &speed);
	if (totalToDownload > 0)
	{
		downloadprog = (int)(nowDownloaded / totalToDownload * 100);
	}
	extern CMainWnd* pFrame; extern int allowclose;
	pFrame->m_download->SetValue(downloadprog);
	if (/*speed != 0 && */allowclose == 0)
	{
		char downloadspeed[100];
		sprintf(downloadspeed, "下载速度:%d Kb/s 总下载大小:%0.2f MB 总进度:%d%%", dwBandIn/10, 1.0 * pFrame->t0->zipspace / 1024, downloadprog);
		pFrame->speed->SetText(CString(UTF8ToUnicode(downloadspeed).c_str()));
	}
	//printf("下载进度%0d%%\r", downloadprog); 
	return 0;
}

/************************************************************************/
/* 获取要下载的远程文件的大小 											*/
/************************************************************************/
long getDownloadFileLenth(const char* url) {
	double downloadFileLenth = 0;
	CURL* handle = curl_easy_init();
	curl_easy_setopt(handle, CURLOPT_URL, url);
	curl_easy_setopt(handle, CURLOPT_HEADER, 1);    //只需要header头
	curl_easy_setopt(handle, CURLOPT_NOBODY, 1);    //不需要body
	//forcehttpspass(handle)
	curl_easy_setopt(handle, CURLOPT_CAINFO, "curl-ca-bundle.crt");
	if (curl_easy_perform(handle) == CURLE_OK)
	{
		curl_easy_getinfo(handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &downloadFileLenth);
	}
	else
	{
		downloadFileLenth = -1;
	}
	return downloadFileLenth;
}

void* workThread(void* pData)
{
	tNode* pNode = (tNode*)pData;

	int res = curl_easy_perform(pNode->_curl);

	if (res != 0)
	{

	}

	curl_easy_cleanup(pNode->_curl);

	pthread_mutex_lock(&foo_mutex);
	threadCnt--;
	pthread_mutex_unlock(&foo_mutex);
	delete pNode;
	pthread_exit(0);

	return NULL;
}
bool downLoad(int threadNum, std::string _packageUrl, std::string _storagePath, bool show)
{
	fileLength = getDownloadFileLenth(_packageUrl.c_str());
	if (fileLength <= 0)
	{
		fprintf(stderr,"Error:文件头获取错误...");
		return false;
	}


	// Create a file to save package.
	const string outFileName = _storagePath;
	if (_access(outFileName.c_str(), 0) != -1) remove(outFileName.c_str());
	FILE* fp = fopen(outFileName.c_str(), "wb");
	if (!fp)
	{
		return false;
	}

	//_chsize((int)fp, fileLength);

	int gap = fileLength / threadNum;

	for (int i = 0; i <= threadNum; i++)
	{
		tNode* pNode = new tNode();

		if (i < threadNum)
		{
			pNode->startidx = i * gap;
			pNode->maxidx = pNode->startidx - 1;
		}
		else
		{
			if (fileLength % threadNum != 0)
			{
				pNode->startidx = i * gap;
				pNode->maxidx = fileLength - 1;
			}
		}

		CURL* _curl = curl_easy_init();

		pNode->_curl = _curl;
		pNode->fp = fp;

		curl_easy_setopt(_curl, CURLOPT_URL, _packageUrl.c_str());
		//forcehttpspass(_curl)
		curl_easy_setopt(_curl, CURLOPT_CAINFO, "curl-ca-bundle.crt");
		curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, downLoadPackage);
		curl_easy_setopt(_curl, CURLOPT_WRITEDATA, pNode);
		curl_easy_setopt(_curl, CURLOPT_PROGRESSFUNCTION, assetsManagerProgressFunc);//进度处理函数
		/*************/
		curl_easy_setopt(_curl, CURLOPT_NOPROGRESS, 0L);
		struct progress data;
		curl_easy_setopt(_curl, CURLOPT_XFERINFODATA, &data);
		curl_easy_setopt(_curl, CURLOPT_XFERINFOFUNCTION, assetsManagerProgressFunc);
		/************/
		curl_easy_setopt(_curl, CURLOPT_NOSIGNAL, 1L);
		curl_easy_setopt(_curl, CURLOPT_LOW_SPEED_LIMIT, 1L);
		curl_easy_setopt(_curl, CURLOPT_LOW_SPEED_TIME, 5L);
		if (show == false)curl_easy_setopt(_curl, CURLOPT_VERBOSE, 0);
		pthread_mutex_lock(&foo_mutex);
		threadCnt++;
		pthread_mutex_unlock(&foo_mutex);
		int rc = pthread_create(&pNode->_tid, NULL, workThread, pNode);
	}
	if (bError)
	{
		fclose(fp);
		return false;
	}

	while (threadCnt > 0)
	{
		Sleep(1000);
	}

	fclose(fp);

	//printf("下载完成......\n");
	return true;
}