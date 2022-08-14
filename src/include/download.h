#pragma once
#include <iostream>
int assetsManagerProgressFunc(void* ptr, double totalToDownload, double nowDownloaded, double totalToUpLoad, double nowUpLoaded);
bool downLoad(int threadNum, std::string _packageUrl, std::string _storagePath, bool show=False);
static size_t downLoadPackage(void* ptr, size_t size, size_t nmemb, void* userdata);
long getDownloadFileLenth(const char* url);
void* workThread(void* pData);