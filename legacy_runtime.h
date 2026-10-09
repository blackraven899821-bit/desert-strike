/* Minimal startup and legacy MSVCRT imports: no UCRT or VC redist needed.
   No TLS, C++ constructors or CRT-owned streams are used by this program. */
#ifndef DS_LEGACY_RUNTIME_H
#define DS_LEGACY_RUNTIME_H
#include <stdarg.h>
#include <math.h>
#include <wchar.h>
__declspec(dllimport) int __cdecl ds_vsnprintf(char *,size_t,const char *,va_list) __asm__("__vsnprintf");
__declspec(dllimport) int __cdecl ds_vsnwprintf(wchar_t *,size_t,const wchar_t *,va_list) __asm__("__vsnwprintf");
static int ds_snprintf(char *b,size_t n,const char *fmt,...){va_list ap;va_start(ap,fmt);int r=ds_vsnprintf(b,n,fmt,ap);va_end(ap);if(n)b[n-1]=0;return r;}
static int ds_snwprintf(wchar_t *b,size_t n,const wchar_t *fmt,...){va_list ap;va_start(ap,fmt);int r=ds_vsnwprintf(b,n,fmt,ap);va_end(ap);if(n)b[n-1]=0;return r;}
static int ds_printf(const char *fmt,...){char b[1024];va_list ap;va_start(ap,fmt);int r=ds_vsnprintf(b,sizeof(b),fmt,ap);va_end(ap);b[1023]=0;DWORD written;WriteFile(GetStdHandle(STD_OUTPUT_HANDLE),b,(DWORD)lstrlenA(b),&written,NULL);return r;}
#undef snprintf
#undef swprintf
#undef printf
#define snprintf ds_snprintf
#define swprintf ds_snwprintf
#define printf ds_printf
#define sinf(x) ((float)sin((double)(x)))
#define cosf(x) ((float)cos((double)(x)))
#define sqrtf(x) ((float)sqrt((double)(x)))
#define atan2f(x,y) ((float)atan2((double)(x),(double)(y)))
#define floorf(x) ((float)floor((double)(x)))
#define fmodf(x,y) ((float)fmod((double)(x),(double)(y)))
static float ds_max(float a,float b){return a>b?a:b;}
static float ds_min(float a,float b){return a<b?a:b;}
#define fmaxf ds_max
#define fminf ds_min
#define fmin ds_min
#undef isfinite
#define isfinite(x) __builtin_isfinite(x)
unsigned long _tls_index;
#ifdef DS_SETUP
int WINAPI wWinMain(HINSTANCE,HINSTANCE,LPWSTR,int);
void ds_entry(void){ExitProcess((UINT)wWinMain(GetModuleHandleW(NULL),NULL,GetCommandLineW(),SW_SHOWNORMAL));}
#else
int WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int);
void ds_entry(void){ExitProcess((UINT)WinMain(GetModuleHandleA(NULL),NULL,GetCommandLineA(),SW_SHOWNORMAL));}
#endif
#endif
