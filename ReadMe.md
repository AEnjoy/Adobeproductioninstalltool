# What's this?

这个版本库收录了Adobe Photoshop 和 Premiere Pro的绿色版安装包,资源来自[zd423](https://www.423down.com/).

除此之外,该版本库还包含了在线安装的相关配置文件

这个版本库还将作为InstallTool的源代码库存在,并长期更新.



目前已经上传了PS2022 23.0.1版本,其它绿色版本稍后上传

项目使用宽松MIT协议分发,鼓励用于学习等用途

由于本人拙劣的代码水平,各位大神看官笑笑就好=.=

# Third-party libraries referenced by the project

1.[OpenSSL](https://github.com/openssl/openssl)

2.[libcurl](https://github.com/curl/curl)

3.[LZMASDK](https://www.7-zip.org/sdk.html)

4.[DuiLib_Ultimate](https://github.com/qdtroy/DuiLib_Ultimate)

5.[pthreads](https://sourceware.org/pthreads-win32/)

# 编译源代码

下载git客户端并安装及配置→[传送门](https://www.runoob.com/manual/git-guide/)

在终端中拉取源代码至本地↓

```
git clone https://github.com/AEnjoy/Adobeproductioninstalltool
```

进入Adobeproductioninstalltool目录,拉取DuiLib_Ultimate至本地

```
git clone https://github.com/qdtroy/DuiLib_Ultimate
rename DuiLib_Ultimate duilib
```

使用VS打开duilib\DuiLib.vcxproj,选择release,x86

菜单选择项目→属性→配置属性→常规→配置类型→选择动态库(dll)

编译,然后找到Bin下的duilib.dll和Lib下的duilib.lib,copy至上一级目录

配置openssl lib及libcurl:

```bash
git clone https://github.com/openssl/openssl
git clone https://github.com/curl/curl
```

建立deps文件夹:

然后copy openssl依赖用于编译SSL支持的curl

```bash
copy openssl/include deps
```

同时,copy openssl的include到msvc的include下

编译openssl→[传送门](https://blog.csdn.net/wzfgd/article/details/109745506)

编译libcurl→[传送门](https://blog.csdn.net/Sakuya__/article/details/113246127)

编译lzmasdk→[传送门](https://www.cnblogs.com/greenerycn/archive/2011/10/26/7zSDK_cpp_compile.html)

两个文件:7za.exe 7zxr.dll

下载libpthread.dll依赖

编译项目,然后copy res

```bash
copy res release
```

copy依赖

```
libcurl.dll
DuiLib.dll
lzma sdk:extracting.dll
libcrypto-3.dll
libpthread.dll
libssl-3.dll
```

7-zip打包至..\package.zip

然后编译unpack

# 鸣谢

zd423

热心网友们

