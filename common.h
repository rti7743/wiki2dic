
#pragma once

#ifdef _MSC_VER
//#include <Winsock2.h>
//#include <mswsock.h>
//#include <Ws2tcpip.h>
#include <windows.h>

#pragma warning (push)
#pragma warning (disable : 4005)
#include <intsafe.h>
#include <stdint.h>
#include <time.h>
#pragma warning (push)
#pragma warning (default : 4005)



#define snprintf _snprintf
#define SecSleep(sec) ::Sleep((sec)*1000)
#define MiriSleep(miri) ::Sleep((miri))
#define INT64 __int64
#define PATHSEP "\\"
#define STRICMP(x,y)	_stricmp((x),(y))
#define STRINCMP(x,y,n)	strncmpi((x),(y),(n))
#define STRINCMPNN(x,y)	strncmpi((x),(y),sizeof(y)-1)

#else
#include <stdint.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <wchar.h>
#include <string.h>
#include <limits.h>

#define SecSleep(sec) sleep((sec))
#define MiriSleep(miri) usleep((miri)*1000)
#define INT64 long long
#define PATHSEP "/"
typedef unsigned int  LRESULT;
typedef unsigned int  WPARAM;
typedef unsigned int  LPARAM;
typedef unsigned int* HINSTANCE;
typedef unsigned int* HWND;
typedef wchar_t WCHAR;
#define STRICMP(x,y)	strcasecmp((x),(y))
#define STRINCMP(x,y,n)	strncasecmp((x),(y),(n))
const int MAX_PATH = 255;

#endif

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <functional>
#include <assert.h>
#include <sstream>
#include <algorithm>


#ifdef _MSC_VER
	#ifdef _DEBUG
		#define _CRTDBG_MAP_ALLOC
		#include <crtdbg.h>
		#define new ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
		#define malloc(X) _malloc_dbg(X,_NORMAL_BLOCK,__FILE__,__LINE__)
	#endif
#endif

//なぜか標準にない 簡易 数字->文字列変化.

//数字を文字列に.
template<typename _INT> static const std::string num2str(_INT i)
{
	std::stringstream out;
	out << i;
	return out.str();
}
template<typename _INT> static const std::wstring num2wstr(_INT i)
{
	std::wstringstream out;
	out << i;
	return out.str();
}
template<typename _INT> static const std::string num2str(_INT i,const std::string& format)
{
	char buf[64]={0};
	snprintf(buf,64,format.c_str(),i);
	return buf;
}

//なぜか標準にない atoi の std::string.

//atoi
static int atoi(const std::string& str)
{
	return atoi(str.c_str());
}
static unsigned int atou(const std::string& str)
{
	char* e;
	return strtoul( str.c_str() ,&e , 10); 
}
static unsigned int atou(const char* str)
{
	char * e;
	return strtoul( str ,&e, 10); 
}
static float atof(const std::string& str)
{
	return (float) atof(str.c_str());
}

//strcmpみたいに数字も比較
#define INTCMP(x,y) ((x) > (y) ? 1 : ((x)==(y) ? 0 : -1))

//TRUE を true に
#define Btob(x)	((x) != FALSE)
//true を TRUE に
#define btoB(x)	((DWORD) x)

//linux系で定義されていないことがあるので、されていなければ定義する
#ifndef MAX
#define MAX(a,b)	((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#endif
#ifndef MINMAX
#define MINMAX(a,x,b)	()
#endif


#include "XLException.h"
#include "windows_encoding.h"

#define SEXYTEST_ASSERT_OVERRAIDE
#include "sexytest.h"

//なぜか標準にない std::map.findを iteratorではなくてsecondを返し使い勝手良くする関数.

static std::string mapfind(const std::map<std::string,std::string>& stringmap ,const std::string& name ,const std::string& def = "" )
{
	auto it = stringmap.find(name);
	if (it == stringmap.end()) return def;
	return it->second;
}
static std::string mapfind(const std::map<std::string,std::string>* stringmap ,const std::string& name ,const std::string& def = "" )
{
	auto it = stringmap->find(name);
	if (it == stringmap->end()) return def;
	return it->second;
}

//文字列での true  false とか 0 とかの bool変換. php.ini みたいなノリ.

static bool stringbool(const std::string& str )
{
	if ( str.empty() ) return false;
	if ( str == "0" || STRICMP(str.c_str() , "false") == 0 || STRICMP(str.c_str() , "no") == 0 || STRICMP(str.c_str() , "off") == 0 )
	{
		return false;
	}
	return true;
}

//多分標準にない。 サイズを min< x < max の範囲で調整する.

template<typename _INT> static _INT sizehosei(_INT x, _INT min,_INT max)
{
	if (x > max) return max;
	if (x < min) return min;
	return x;
}

//boostに似たようなのが有るけど、Cリソースの自動開放.
class AutoClear
{
	std::function<void (void) > func;
public:
	AutoClear(std::function<void (void) > func)
	{
		this->func = func;
	}
	virtual ~AutoClear()
	{
		this->func();
	}
};

//何故か標準にない std::vector<struct foo*> の自動開放.
template<typename _T> void vectorfree(_T &target)
{
	for(auto it = target.begin() ; it != target.end() ; it ++)
	{
		delete *it;
	}
	target.clear();
}

//何故か標準にない std::map<XX,struct foo*> の自動開放.
template<typename _T> void mapfree(_T &target)
{
	for(auto it = target.begin() ; it != target.end() ; it ++)
	{
		delete it->second;
	}
	target.clear();
}
