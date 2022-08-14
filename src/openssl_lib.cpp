#include <fstream>
#include <openssl\md5.h>
#include <string>
//#include <string.h>
using namespace std;
#pragma warning(disable:4996)
extern "C" int get_file_md5(const string & file_name, string & md5_value)
{
    md5_value.clear();
    ifstream file(file_name.c_str(), ifstream::binary);
    if (!file)
    {
        return -1;
    }
    MD5_CTX md5Context;
    MD5_Init(&md5Context);
    char* buf= (char*)malloc(16384); 
    while (file.good()) {
        file.read(buf, 16384);
        MD5_Update(&md5Context, buf, file.gcount());
    }
    unsigned char result[MD5_DIGEST_LENGTH];
    MD5_Final(result, &md5Context);
    char hex[35];
    memset(hex, 0, sizeof(hex));
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i)
    {
        sprintf(hex + i * 2, "%02x", result[i]);
    }
    hex[32] = '\0';
    md5_value = string(hex);
    return 0;
}
