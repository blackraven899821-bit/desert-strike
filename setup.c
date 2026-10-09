#define UNICODE
#define _UNICODE
#define COBJMACROS
#define WIN32_LEAN_AND_MEAN
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#include <windows.h>
#include <initguid.h>
#include <shlobj.h>
#include <shellapi.h>
#include <shobjidl.h>
#include <objbase.h>
#include <stdio.h>
#include <wchar.h>
#define DS_SETUP
#include "legacy_runtime.h"
static int shortcut(const wchar_t *path,const wchar_t *exe,const wchar_t *dir,const wchar_t *args){IShellLinkW *link=NULL;IPersistFile *file=NULL;HRESULT hr=CoCreateInstance(&CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,&IID_IShellLinkW,(void**)&link);if(FAILED(hr))return 0;IShellLinkW_SetPath(link,exe);IShellLinkW_SetWorkingDirectory(link,dir);IShellLinkW_SetArguments(link,args);IShellLinkW_SetDescription(link,L"Desert Strike - original classic-style FPS");hr=IShellLinkW_QueryInterface(link,&IID_IPersistFile,(void**)&file);if(SUCCEEDED(hr)){hr=IPersistFile_Save(file,path,TRUE);IPersistFile_Release(file);}IShellLinkW_Release(link);return SUCCEEDED(hr);}
int WINAPI wWinMain(HINSTANCE i,HINSTANCE prev,LPWSTR cmd,int show){(void)i;(void)prev;(void)cmd;(void)show;wchar_t src[MAX_PATH],base[MAX_PATH],dest[MAX_PATH],from[MAX_PATH],to[MAX_PATH],desktop[MAX_PATH],link[MAX_PATH],exe[MAX_PATH],message[1000];GetModuleFileNameW(NULL,src,MAX_PATH);wchar_t *slash=wcsrchr(src,L'\\');if(!slash)return 1;*slash=0;if(FAILED(SHGetFolderPathW(NULL,CSIDL_LOCAL_APPDATA,NULL,SHGFP_TYPE_CURRENT,base))||wcslen(base)>MAX_PATH-40||wcslen(src)>MAX_PATH-40){MessageBoxW(NULL,L"The installation path is unavailable or too long.",L"Desert Strike Setup",MB_ICONERROR);return 1;}swprintf(dest,MAX_PATH,L"%ls\\DesertStrike",base);swprintf(message,1000,L"Install Desert Strike to:\n%ls\n\nCopies the game and guides; adds desktop shortcuts.\nNo administrator rights required.\nExisting files in this game folder will be replaced.",dest);if(MessageBoxW(NULL,message,L"Desert Strike Setup",MB_OKCANCEL|MB_ICONINFORMATION)!=IDOK)return 0;if(!CreateDirectoryW(dest,NULL)&&GetLastError()!=ERROR_ALREADY_EXISTS){MessageBoxW(NULL,L"Cannot create the game folder.",L"Setup",MB_ICONERROR);return 1;}const wchar_t *names[]={L"DesertStrike.exe",L"README-fa.html",L"README.txt"};for(int n=0;n<3;n++){swprintf(from,MAX_PATH,L"%ls\\%ls",src,names[n]);swprintf(to,MAX_PATH,L"%ls\\%ls",dest,names[n]);if(_wcsicmp(from,to)&&!CopyFileW(from,to,FALSE)){swprintf(message,1000,L"Could not copy %ls.\nExtract the entire ZIP before running Setup.exe.\nIf the game is open, close it and try again.\nWindows error: %lu",names[n],GetLastError());MessageBoxW(NULL,message,L"Setup",MB_ICONERROR);return 1;}}swprintf(exe,MAX_PATH,L"%ls\\DesertStrike.exe",dest);int ok=0;HRESULT hr=CoInitialize(NULL);if(SUCCEEDED(hr)){if(SUCCEEDED(SHGetFolderPathW(NULL,CSIDL_DESKTOPDIRECTORY,NULL,SHGFP_TYPE_CURRENT,desktop))&&wcslen(desktop)<MAX_PATH-45){swprintf(link,MAX_PATH,L"%ls\\Desert Strike.lnk",desktop);ok=shortcut(link,exe,dest,L"");swprintf(link,MAX_PATH,L"%ls\\Desert Strike Server.lnk",desktop);shortcut(link,exe,dest,L"--server");}CoUninitialize();}swprintf(message,1000,L"Installation complete.\n%ls\n\n%ls\n\nLaunch the game now?",dest,ok?L"Desktop shortcuts created.":L"Shortcut unavailable; launch DesertStrike.exe from the folder above.");if(MessageBoxW(NULL,message,L"Desert Strike Setup",MB_YESNO|MB_ICONINFORMATION)==IDYES)ShellExecuteW(NULL,L"open",exe,NULL,dest,SW_SHOWNORMAL);return 0;}
