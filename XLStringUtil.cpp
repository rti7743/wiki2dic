﻿// XLStringUtil.cpp: XLStringUtil クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////


#include "common.h"
#include "XLStringUtil.h"
#include "XLFileUtil.h"

/*
　　 　 　 ﾊヽ／::::ヽ.ヘ===ｧ
　　 　 　 {::{/≧=＝=≦V:/
　　 　 　 >:´:::::::::::::::::::::::::｀ヽ、
　　　 γ:::::::::::::::::::::::::::::::::::::::::ヽ
　 _／/::::::::::::::::::::::::::::::::::::::::::::::ﾊ　　　　　　モッピー知ってるよ
.　| ll ! :::::::l::::::/|ハ::::::::∧::::i :::::::i　　　　　 なんだかんだ言って
　 ､ヾ|:::::::::|:::/｀ト-:::::/ _,X:j:::/:::l                      みんなPHPが大好きなことを。
　　 ヾ:::::::::|≧ｚ　!Ｖ　ｚ≦　/::::/
　　　 ∧::::ト “　 　 　 　 “ ﾉ:::/!
　 　 /::::（＼　　 ー'　 　/￣）　 |
　　　 　 |　｀`ー――‐''|　　ヽ、.|
　　　　　 ゝ　ノ　　　　 ヽ　 ノ　|
￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣￣ 　　　　　　　

*/

#if _MSC_VER
	#include <tchar.h>
	#include <mbstring.h>
//	#pragma comment(lib, "Rpcrt4.lib")			//for uuid

#else
	#include <string.h>
	#include <sys/types.h>
//	#include <uuid/uuid.h>
#endif
//#define __BBL_USING_STATIC_TABLE__
//#define __BBL_DISABLE_SELECTORS__
//#include "../babel/babel.h"


//みんな大好きPHPのstrtoupper
std::string XLStringUtil::strtoupper(const std::string & str)
{
	if (str.empty())
	{
		return str;
	}
	std::string r = str;
	
	char * p = &r[0];
	for(; *p ; )
	{
		if (isMultiByte(p))
		{
			p = nextChar(p);
		}
		else
		{
			*p = toupper(*p);
			p ++;
		}
	}
	return r;
}

SEXYTEST()
{
	{
		std::string a = XLStringUtil::strtoupper("aBcDefg");
		std::string b = "ABCDEFG";
		SEXYTEST_EQ(a ,b); 
	}
}


//みんな大好きPHPのstrtolower
std::string XLStringUtil::strtolower(const std::string & str)
{
	if (str.empty())
	{
		return str;
	}
	std::string r = str;
	
	char * p = &r[0];
	for(; *p ; )
	{
		if (isMultiByte(p))
		{
			p = nextChar(p);
		}
		else
		{
			*p = tolower(*p);
			p ++;
		}
	}
	return r;
}

SEXYTEST()
{
	{
		std::string a = XLStringUtil::strtolower("aBcDefg");
		std::string b = "abcdefg";
		SEXYTEST_EQ(a ,b); 
	}
}

bool XLStringUtil::isnumlic(const std::string & str)
{
	if (str.empty())
	{
		return false;
	}
	std::string r = str;
	
	const char * p = &r[0];
	for(; *p ; )
	{
		if (isMultiByte(p))
		{
			return false;
		}
		if (! (*p >= '0' && *p <= '9') )
		{
			if (! (*p == '-' || *p == '.') )
			{
				return false;
			}
		}
		p ++;
	}
	return true;
}

bool XLStringUtil::isnumlic16(const std::string & str)
{
	if (str.empty())
	{
		return false;
	}
	std::string r = str;
	
	const char * p = &r[0];
	for(; *p ; )
	{
		if (isMultiByte(p))
		{
			return false;
		}
		if (! ((*p >= '0' && *p <= '9') || ((*p >= 'a' && *p <= 'f') || (*p >= 'A' && *p <= 'F') )) )
		{
			return false;
		}
		p ++;
	}
	return true;
}

bool XLStringUtil::isalpha(const std::string & str)
{
	if (str.empty())
	{
		return false;
	}
	std::string r = str;
	
	const char * p = &r[0];
	for(; *p ; )
	{
		if (isMultiByte(p))
		{
			return false;
		}
		if (! ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z')) )
		{
			return false;
		}
		p ++;
	}
	return true;
}

bool XLStringUtil::isalpnum(const std::string & str)
{
	if (str.empty())
	{
		return false;
	}
	std::string r = str;
	
	const char * p = &r[0];
	for(; *p ; )
	{
		if (isMultiByte(p))
		{
			return false;
		}
		if (! ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9')) )
		{
			return false;
		}
		p ++;
	}
	return true;
}

bool XLStringUtil::isAsciiString(const std::string & str)
{
	if (str.empty())
	{
		return false;
	}
	std::string r = str;
	
	const char * p = &r[0];
	for(; *p ; )
	{
		if (isMultiByte(p))
		{
			return false;
		}
		p ++;
	}
	return true;
}

int XLStringUtil::atoi(const std::string & str)
{
	return std::atoi(str.c_str() );
}


//HTTPヘッダのキャメル
std::string XLStringUtil::HeaderUpperCamel(const std::string & str)
{
	if (str.empty())
	{
		return str;
	}
	std::string r = str;
	
	char * p = &r[0];
	*p = toupper(*p);
	p++;

	for(; *p ; ++p)
	{
		//Content-Length みたいに - のあとはでっかくするよ。
		if (*p == '-')
		{
			++p;
			*p = toupper(*p);
		}
		else
		{
			*p = tolower(*p);
		}
	}
	return r;
}

SEXYTEST()
{
	{
		std::string a = XLStringUtil::HeaderUpperCamel("content-length");
		std::string b = "Content-Length";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::string a = XLStringUtil::HeaderUpperCamel("Content-length");
		std::string b = "Content-Length";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::string a = XLStringUtil::HeaderUpperCamel("content-Length");
		std::string b = "Content-Length";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::string a = XLStringUtil::HeaderUpperCamel("server");
		std::string b = "Server";
		SEXYTEST_EQ(a ,b); 
	}
}


std::string XLStringUtil::join(const std::string& glue , const std::list<std::string> & pieces )
{
	std::string r;
	r.reserve(pieces.size() * 10);

	std::list<std::string>::const_iterator it = pieces.begin();
	std::list<std::string>::const_iterator et = pieces.end();
	if (it == et)
	{
		return r;
	}
	//最初の一発目.
	r += *it;
	++it;

	for( ; it != et ; ++it )
	{
		r += glue;
		r += *it;
	}
	return r;
}

//template って知ってるけど、とりあずこれで。
std::string XLStringUtil::join(const std::string& glue , const std::vector<std::string> & pieces )
{
	std::string r;
	r.reserve(pieces.size() * 10);

	std::vector<std::string>::const_iterator it = pieces.begin();
	std::vector<std::string>::const_iterator et = pieces.end();
	if (it == et)
	{
		return r;
	}
	//最初の一発目.
	r += *it;
	++it;

	for( ; it != et ; ++it )
	{
		r += glue;
		r += *it;
	}
	return r;
}

//key=value& みたいな感じの join
std::string XLStringUtil::crossjoin(const std::string& glue1 ,const std::string& glue2 , const std::map<std::string,std::string> & pieces )
{
	std::string r;
	r.reserve(pieces.size() * 10);

	std::map<std::string,std::string>::const_iterator it = pieces.begin();
	std::map<std::string,std::string>::const_iterator et = pieces.end();
	if (it == et)
	{
		return r;
	}
	//最初の一発目.
	r += it->first + glue1 + it->second;
	++it;

	for( ; it != et ; ++it )
	{
		r += glue2;
		r += it->first + glue1 + it->second;
	}
	return r;
}
//strstrのマルチバイトセーフ 文字列検索
const char* XLStringUtil::strstr(const std::string& target, const std::string & need )
{
#ifdef _MSC_VER
	//SJISだとこんな感じかな・・・
	return (const char*) _mbsstr( (unsigned char*) target.c_str() ,(unsigned char*) need.c_str() );
#else
	//UTF-8だと仮定してやるよw
	return ::strstr( target.c_str() ,need.c_str() );
#endif
}

//strstrのマルチバイトセーフ 文字列検索
const char* XLStringUtil::strstr(const char* target, const char* need )
{
#ifdef _MSC_VER
	//SJISだとこんな感じかな・・・
	return (const char*) _mbsstr( (unsigned char*)target ,(unsigned char*) need );
#else
	//UTF-8だと仮定してやるよw
	return ::strstr(target ,need );
#endif
}

//strstrのマルチバイトセーフ 文字列検索 //若干非効率
const char* XLStringUtil::strrstr(const std::string& target, const std::string & need )
{
	const int size = need.size();
	const char* p = target.c_str();
	const char* lastmatch = NULL;
	while(p = XLStringUtil::strstr(p,need))
	{
		lastmatch = p;
		p += size;
	}
	
	return lastmatch;
}

//strstrのマルチバイトセーフ 文字列検索 //若干非効率
const char* XLStringUtil::strrstr(const char* target, const char* need )
{
	const int size = strlen(need);
	const char* p = target;
	const char* lastmatch = NULL;
	while(p = XLStringUtil::strstr(p,need))
	{
		lastmatch = p;
		p += size;
	}
	
	return lastmatch;
}

//stristrのマルチバイトセーフ 大文字小文字関係なしの検索
const char* XLStringUtil::stristr(const std::string& target, const std::string & need )
{	
	std::string _target = XLStringUtil::strtolower(target);
	std::string _need = XLStringUtil::strtolower(need);
	const char * find = XLStringUtil::strstr(_target.c_str(),_need.c_str());
	if (!find)
	{
		return NULL;
	}
	const int offset = (int)(find - _target.c_str());
	return target.c_str() + offset;
}

//strchrのマルチバイトセーフ 文字列の最初ら検索して最初見に使った文字の位置
const char* XLStringUtil::strchr(const std::string& target, char need )
{
#ifdef _MSC_VER
	//SJISだとこんな感じかな・・・
	return (const char*) _mbschr( (unsigned char*)target.c_str() ,(unsigned char) need );
#else
	//UTF-8だと仮定してやるよw
	return ::strchr( target.c_str() ,(int)need );
#endif
}

//strchrのマルチバイトセーフ 文字列の最初ら検索して最初見に使った文字の位置
const char* XLStringUtil::strchr(const char* target, char need )
{
#ifdef _MSC_VER
	//SJISだとこんな感じかな・・・
	return (const char*) _mbschr( (unsigned char*)target ,(unsigned char) need );
#else
	//UTF-8だと仮定してやるよw
	return ::strchr( target ,(int)need );
#endif
}


//strchrのマルチバイトセーフ 文字列の最初ら検索して最初見に使った文字の位置
const char* XLStringUtil::strchrV(const char* target, char n1 ,char n2,char n3,char n4,char n5,char n6,char n7)
{
	const char* p1 = strchr(target,n1);
	if (n2==0)
	{
		return p1;
	}
	const char* p2 = strchr(target,n2);
	if (n3==0)
	{
		return notnull_min(p1,p2);
	}
	const char* p3 = strchr(target,n3);
	if (n4==0)
	{
		return notnull_min(p1,p2,p3);
	}
	const char* p4 = strchr(target,n4);
	if (n5==0)
	{
		return notnull_min(p1,p2,p3,p4);
	}
	const char* p5 = strchr(target,n5);
	if (n6==0)
	{
		return notnull_min(p1,p2,p3,p4,p5);
	}
	const char* p6 = strchr(target,n6);
	if (n7==0)
	{
		return notnull_min(p1,p2,p3,p4,p5,p6);
	}
	const char* p7 = strchr(target,n7);
	return notnull_min(p1,p2,p3,p4,p5,p6,p7);
}

const char* XLStringUtil::notnull_min(const char* n1 ,const char* n2,const char* n3,const char* n4,const char* n5,const char* n6,const char* n7)
{
	const char * min = (const char *)-1;
	if (n1!=NULL && min < n1) min = n1;
	if (n2!=NULL && min < n2) min = n2;
	if (n3!=NULL && min < n3) min = n3;
	if (n4!=NULL && min < n4) min = n4;
	if (n5!=NULL && min < n5) min = n5;
	if (n6!=NULL && min < n6) min = n6;
	if (n7!=NULL && min < n7) min = n7;
	return min;
}

//strrchrのマルチバイトセーフ 文字列の後ろから検索して最初見に使った文字の位置
const char* XLStringUtil::strrchr(const std::string& target, char need )
{
#ifdef _MSC_VER
	//SJISだとこんな感じかな・・・
	return (const char*) _mbsrchr( (unsigned char*)target.c_str() ,(unsigned char) need );
#else
	//UTF-8だと仮定してやるよw
	return ::strrchr( target.c_str() ,(int)need );
#endif
}

//strrchrのマルチバイトセーフ 文字列の後ろから検索して最初見に使った文字の位置
const char* XLStringUtil::strrchr(const char* target, char need )
{
#ifdef _MSC_VER
	//SJISだとこんな感じかな・・・
	return (const char*) _mbsrchr( (unsigned char*)target ,(unsigned char) need );
#else
	//UTF-8だと仮定してやるよw
	return ::strrchr( target ,(int)need );
#endif
}



//strposのマルチバイトセーフ (strposはPHPにアルやつね)
//最初に見つかった場所。見つからなければ -1 を返します。
int XLStringUtil::strpos(const std::string& target, const std::string & need )
{
	const char * p = XLStringUtil::strstr(target.c_str(),need.c_str());
	if (!p) return -1;

	return p - target.c_str();
}

//strposのマルチバイトセーフ (strposはPHPにアルやつね)
//最初に見つかった場所。見つからなければ -1 を返します。
int XLStringUtil::strpos(const char* target, const char* need )
{
	const char * p = XLStringUtil::strstr(target,need);
	if (!p) return -1;

	return p - target;
}
//strrposのマルチバイトセーフ (strposはPHPにアルやつね)
//逆から検索して最初に見つかった場所。見つからなければ -1 を返します。
int XLStringUtil::strrpos(const std::string& target, const std::string & need )
{
	const char * p = strrstr(target.c_str(),need.c_str());
	if (!p) return -1;

	return p - target.c_str();
}

//strrposのマルチバイトセーフ (strposはPHPにアルやつね)
//逆から検索して最初に見つかった場所。見つからなければ -1 を返します。
int XLStringUtil::strrpos(const char* target, const char* need )
{
	const char * p = strrstr(target,need);
	if (!p) return -1;

	return p - target;
}

//striposのマルチバイトセーフ (striposはPHPにアルやつね)
//大文字小文字関係なしで検索して最初に見つかった場所。見つからなければ -1 を返します。
int XLStringUtil::stripos(const std::string& target, const std::string & need )
{
	const char * p = XLStringUtil::stristr(target,need);
	if (!p) return -1;

	return p - target.c_str();
}


//なぜか標準にない /^foo/ みたいに前方マッチ.
const char* XLStringUtil::strfirst(const char* s,const char* n)
{
	const char* p = XLStringUtil::strstr(s,n);
	if (p == NULL) return NULL;
	
	if ( p != s ) return NULL;
	return p;
}

//なぜか標準にない /foo$/ みたいに後方マッチ.
const char* XLStringUtil::strend(const char* s,const char* n)
{
	const char* p = XLStringUtil::strrstr(s,n);
	if (p == NULL) return NULL;

	if ( strlen(p) != strlen(n) )
	{
		return NULL;
	}
	return p;
}



//split
std::list<std::string> XLStringUtil::split(const std::string& glue, const std::string & inTarget )
{
	std::list<std::string> r;

	int oldpos = 0;
	int pos = 0;
	while( (pos = inTarget.find( glue , oldpos)) != std::string::npos )
	{
		std::string k = inTarget.substr(oldpos , pos - oldpos);

		r.push_back(k);

		oldpos = pos+glue.size();
	}
	//最後の残り
	{
		std::string k = inTarget.substr(oldpos , pos - oldpos);
		r.push_back(k);
	}
	return r;
}

//vector
std::vector<std::string> XLStringUtil::split_vector(const std::string& glue, const std::string & inTarget )
{
	std::vector<std::string> r;

	int oldpos = 0;
	int pos = 0;
	while( (pos = inTarget.find( glue , oldpos)) != std::string::npos )
	{
		std::string k = inTarget.substr(oldpos , pos - oldpos);

		r.push_back(k);

		oldpos = pos+glue.size();
	}
	//最後の残り
	{
		std::string k = inTarget.substr(oldpos , pos - oldpos);
		r.push_back(k);
	}
	return r;
}

std::pair<std::string,std::string> XLStringUtil::split_two(const std::string& glue, const std::string & inTarget )
{
	const char * str = inTarget.c_str();
	const char * p = ::strstr(str,glue.c_str());
	if (p == NULL) return std::pair<std::string,std::string>("","");

	const std::string first = std::string(str , 0 , p - str );
	const std::string second = p + glue.size() ;
	return std::pair<std::string,std::string>(first,second);
}

bool XLStringUtil::split_two(const std::string& glue, const std::string & inTarget , std::string* first,std::string* second )
{
	const char * str = inTarget.c_str();
	const char * p = ::strstr(str,glue.c_str());
	if (p == NULL)
	{
		*first = "";
		*second = "";

		return false;
	}

	*first = std::string(str , 0 , p - str );
	*second = p + glue.size() ;

	return true;
}

bool XLStringUtil::split_two(const std::string& glue, const std::string & inTarget , std::pair<std::string,std::string>* ppp )
{
	const char * str = inTarget.c_str();
	const char * p = ::strstr(str,glue.c_str());
	if (p == NULL)
	{
		*ppp = std::pair<std::string,std::string>("","");
		return false;
	}

	const std::string first = std::string(str , 0 , p - str );
	const std::string second = p + glue.size() ;
	*ppp = std::pair<std::string,std::string>(first,second);

	return true;
}


//key=value& みたいな感じの split
std::map<std::string,std::string> XLStringUtil::crosssplit(const std::string& glue1 ,const std::string& glue2 , const std::string & inTarget )
{
	std::map<std::string,std::string> r;

	int oldpos = 0;
	int pos = 0;
	while( (pos = inTarget.find( glue1 , oldpos)) != -1 )
	{
		std::string k = inTarget.substr(oldpos , pos - oldpos);

		oldpos = pos + glue1.size();

		int vpos = k.find( glue2 );
		if (vpos < 0)
		{
			r.insert( std::pair<std::string,std::string>(k,"") );
			continue;
		}

		std::string v = k.substr( vpos + glue2.size() );
		k = k.substr(0 , vpos);
		r.insert( std::pair<std::string,std::string>(k,v) );

	}

	//最後の残り
	{
		std::string k = inTarget.substr(oldpos);
		int vpos = k.find( glue2 );
		if (vpos < 0)
		{
			r.insert( std::pair<std::string,std::string>(k,"") );
			return r;
		}

		std::string v = k.substr( vpos + glue2.size() );
		k = k.substr(0 , vpos);
		r.insert( std::pair<std::string,std::string>(k,v) );
	}
	return r;
}

SEXYTEST()
{
	{
		std::map<std::string,std::string> a = XLStringUtil::crosssplit("&","=","room=&menuエアコン&actionだんぼうMAX");
		std::map<std::string,std::string> b ; b["room"]=""; b["menuエアコン"] = ""; b["actionだんぼうMAX"]="";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::map<std::string,std::string> a = XLStringUtil::crosssplit("&","=","a=1&bb=22&ccc=333");
		std::map<std::string,std::string> b ; b["a"]="1"; b["bb"]="22"; b["ccc"]="333";
		SEXYTEST_EQ(a ,b); 
	}
}


//key=value& みたいな感じの split キーに対するchopを実行する
std::map<std::string,std::string> XLStringUtil::crosssplitChop(const std::string& glue1 ,const std::string& glue2 , const std::string & inTarget )
{
	std::map<std::string,std::string> r;

	int oldpos = 0;
	int pos = 0;
	while( (pos = inTarget.find( glue1 , oldpos)) != -1 )
	{
		std::string k = inTarget.substr(oldpos , pos - oldpos);

		oldpos = pos + glue1.size();

		int vpos = k.find( glue2 );
		if (vpos < 0)
		{
			r.insert( std::pair<std::string,std::string>(k,"") );
			continue;
		}

		std::string v = k.substr( vpos + glue2.size() );
		k = k.substr(0 , vpos);
		r.insert( std::pair<std::string,std::string>( chop( k) ,v) );

	}

	//最後の残り
	{
		std::string k = inTarget.substr(oldpos);
		int vpos = k.find( glue2 );
		if (vpos < 0)
		{
			r.insert( std::pair<std::string,std::string>(k,"") );
			return r;
		}

		std::string v = k.substr( vpos + glue2.size() );
		k = k.substr(0 , vpos);
		r.insert( std::pair<std::string,std::string>(chop( k),v) );
	}
	return r;
}

std::map<std::string,std::string> XLStringUtil::merge(const std::map<std::string,std::string>& a ,const std::map<std::string,std::string>& b , bool overideB = true )
{
	if (overideB)
	{
		std::map<std::string,std::string> r = b;
		r.insert(a.begin(),a.end());
		return r;
	}

	std::map<std::string,std::string> r = a;
	r.insert(b.begin(),b.end());
	return r;
}


std::list<std::string> XLStringUtil::merge(const std::list<std::string>& a ,const std::list<std::string>& b  )
{
	std::list<std::string> r = a;
	for(auto it = b.begin() ; it != b.end() ; it++ )
	{
		r.push_back(*it);
	}
	return r;
}




//みんな大好きPHPのurldecode
std::string XLStringUtil::urldecode(const std::string & inUrl)
{
	static const char xcb[] = "0123456789ABCDEF";
	static const char xcs[] = "0123456789abcdef";

	const char* url = inUrl.c_str();
	std::vector<char> buffer(inUrl.size() + 1);

	char* start = &buffer[0];
	char* nomal = start;


	for( ; *url ; url++ , nomal++)
	{
		if (*url != '%')
		{
			if (*url == '+')	*nomal = ' ';
			else				*nomal = *url;
		}
		else
		{
			//% だけで終わっている文字列の排除
			if ( *(url+1) == 0 ){	*nomal = '%';	continue;	}

			//%Z とかのわけわかめの排除
			int firstnum;
			const char* first = strchr(xcb,*(url+1));
			if (first) firstnum = first - xcb;
			else
			{
				first = strchr(xcs,*(url+1));
				if (first) firstnum = first - xcs;
				else
				{
					*nomal = '%';	continue;
				}
			}

			//%A だけで終わっている文字列の排除
			if ( *(url+2) == 0 ){	*nomal = '%';	continue;	}

			//%AZ とかのわけわかめの排除
			int secondnum;
			const char* second = strchr(xcb,*(url+2));
			if (second) secondnum = second - xcb;
			else
			{
				second = strchr(xcs,*(url+2));
				if (second) secondnum = second - xcs;
				else
				{
					*nomal = '%';	continue;
				}
			}

			*nomal = (16 * firstnum) + secondnum;
			url ++;
			url ++;
		}
	}
	*nomal = '\0';

	return start;
}


SEXYTEST()
{
	{
		std::string a = XLStringUtil::urldecode("search=%41%42C%44");
		std::string b = "search=ABCD";
		SEXYTEST_EQ(a ,b); 
	}
}



//http://d.hatena.ne.jp/ytakano/20081016/urlencode より
std::string XLStringUtil::urlencode(const std::string &str) 
{
    std::ostringstream os;

    for (unsigned int i = 0; i < str.size(); i++) {
        char c = str[i];
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '-' || c == '_' || c == '.' || c == '~') {
            os << c;
        } else {
            char s[4];

            snprintf(s, sizeof(s), "%%%02x", c & 0xff);
			os << s;
        }
    }

    return os.str();
}

//base64エンコード
std::string XLStringUtil::base64encode(const std::string& src) 
{
	return base64encode(src.c_str() , src.size() );
}

SEXYTEST()
{
	{
		std::string a = XLStringUtil::base64encode("IE4+, NN4.06+, Gecko, Opera6+");
		std::string b = "SUU0KywgTk40LjA2KywgR2Vja28sIE9wZXJhNis=";
		SEXYTEST_EQ(a ,b); 

		std::string c = XLStringUtil::base64decode(b);
		SEXYTEST_EQ(c ,"IE4+, NN4.06+, Gecko, Opera6+"); 
	}
	{
		std::string a = XLStringUtil::base64encode("ABCDEFG");
		std::string b = "QUJDREVGRw==";
		SEXYTEST_EQ(a ,b); 

		std::string c = XLStringUtil::base64decode(b);
		SEXYTEST_EQ(c ,"ABCDEFG"); 
	}
	{
		std::string a = XLStringUtil::base64encode("http://user1.matsumoto.ne.jp/~goma/js/base64.html");
		std::string b = "aHR0cDovL3VzZXIxLm1hdHN1bW90by5uZS5qcC9+Z29tYS9qcy9iYXNlNjQuaHRtbA==";
		SEXYTEST_EQ(a ,b); 

		std::string c = XLStringUtil::base64decode(b);
		SEXYTEST_EQ(c ,"http://user1.matsumoto.ne.jp/~goma/js/base64.html"); 
	}
}


//base64エンコード
std::string XLStringUtil::base64encode(const char* src,int len) 
{
	static const char *base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

	std::vector<char> v;
	v.resize( len * 2 );
	unsigned char * d = (unsigned char *) (&v[0]);

	const unsigned char * p = (unsigned char*) (src);
	const unsigned char * end = (unsigned char*) (p + (len / 3 * 3) );
	for(; p < end ; p += 3 , d += 4)
	{
		*(d + 0)  = base64char[ ((*(p + 0) & 0xfc) >> 2) ];

		*(d + 1)  = base64char[ ((*(p + 0) & 0x03) << 4) |  ((*(p + 1) & 0xf0) >> 4) ];

		*(d + 2)  = base64char[ ((*(p + 1) & 0x0f) << 2) |  ((*(p + 2) & 0xc0) >> 6) ];

		*(d + 3)  = base64char[ ((*(p + 2) & 0x3f)) ];
	}
	//端数を解きます。
	switch ( len % 3 )
	{
	case 2: //1文字足りない
		*(d + 0)  = base64char[ ((*(p + 0) & 0xfc) >> 2) ];

		*(d + 1)  = base64char[ ((*(p + 0) & 0x03) << 4) |  ((*(p + 1) & 0xf0) >> 4) ];

		*(d + 2)  = base64char[ ((*(p + 1) & 0x0f) << 2) ];

		*(d + 3)  = '=';

		d += 4;
		break;
	case 1: //2文字足りない
		*(d + 0)  = base64char[ ((*(p + 0) & 0xfc) >> 2) ];

		*(d + 1)  = base64char[ ((*(p + 0) & 0x03) << 4) ];

		*(d + 2)  = '=';

		*(d + 3)  = '=';

		d += 4;
		break;
	case 0:
		break;
	}

	*d = '\0';

	return &v[0];
}

//base64デコード
std::string XLStringUtil::base64decode(const std::string& src) 
{
	std::vector<char> out;
	base64decode(src,&out);

	return std::string(&out[0] , 0 , out.size() );
}

//base64デコード
void XLStringUtil::base64decode(const std::string& src ,std::vector<char>* out) 
{
	auto base64int = [](unsigned char pp) -> int {
		if (pp >= 'A' && pp <= 'Z') return   pp - 'A';
		else if (pp >= 'a' && pp <= 'z') return   pp - 'a' + 26;
		else if (pp >= '0' && pp <= '9') return   pp - '0' + 52;
		else if (pp == '+') return   62;
		else if (pp == '/') return   63;
		else return 255;
	};

	out->resize( src.size() );
	unsigned char * d = (unsigned char *) (&out->operator[](0) );
	const unsigned char * start = d;
	unsigned int conv0 = 255;
	unsigned int conv1 = 255;
	unsigned int conv2 = 255;
	unsigned int conv3 = 255;

	unsigned char * p = (unsigned char *) src.c_str();
	while(*p)
	{
		conv0 = base64int(*(p + 0));
		if (conv0 == 255) break;

		conv1 = base64int(*(p + 1));
		if (conv1 == 255) break;

		conv2 = base64int(*(p + 2));
		if (conv2 == 255) break;

		conv3 = base64int(*(p + 3));
		if (conv3 == 255) break;

		*(d + 0) = ((conv0) << 2) + ((conv1 & 0x30) >> 4);
		*(d + 1) = ((conv1 & 0x0f) << 4) + ((conv2 & 0x3c) >> 2);
		*(d + 2) = ((conv2 & 0x03) << 6) + ((conv3));

		p += 4;
		d += 3;
	}
	if (conv0 == 255)
	{
	}
	else if (conv1 == 255)
	{
		*(d + 0) = ((conv0) << 2);

		d += 1;
	}
	else if (conv2 == 255)
	{
		*(d + 0) = ((conv0) << 2) + ((conv1 & 0x30) >> 4);
		*(d + 1) = ((conv1 & 0x0f) << 4) ;

		d += 2;
	}
	else if (conv3 == 255)
	{
		*(d + 0) = ((conv0) << 2) + ((conv1 & 0x30) >> 4);
		*(d + 1) = ((conv1 & 0x0f) << 4) + ((conv2 & 0x3c) >> 2);
		*(d + 2) = ((conv2 & 0x03) << 6);

		d += 3;
	}

	//サイズ調整
	out->resize((int) (d - start));
}

//みんな大好きPHPのchop 左右の空白の除去 strのみマルチバイト対応  replaceTableは半角のみ
std::string XLStringUtil::chop(const std::string & str,const char * replaceTable)
{
	if (replaceTable == NULL)
	{
		replaceTable = " \t\r\n";
	}

	const char * p = str.c_str();
	//頭出し
	while(*p)
	{
		if (isMultiByte(p))
		{
			break;
		}
		const char * rep = replaceTable;
		for(; *rep ; ++rep)
		{
			if ( *rep == *p )
			{
				break;
			}
		}
		if (*rep == 0)
		{//not match
			break;
		}
		p ++;
	}

	const char * start = p;
	const char * lastEffectvie = p;

	//終端削り
	while(*p)
	{
		if (isMultiByte(p))
		{
			p = nextChar(p);
			lastEffectvie = p;
			continue;
		}
		const char * rep = replaceTable;
		for(; *rep ; ++rep)
		{
			if ( *rep == *p )
			{
				break;
			}
		}
		if (*rep == 0)
		{//not match
			++p;
			lastEffectvie = p;
		}
		else
		{//match
			++p;
		}
	}
	return std::string(start , 0 , (lastEffectvie - start) );
}

SEXYTEST()
{
	{
		std::string a = XLStringUtil::chop("コンピュータ");
		std::string b = "コンピュータ";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::string a = XLStringUtil::chop("コンピュータ ");
		std::string b = "コンピュータ";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::string a = XLStringUtil::chop(" コンピュータ");
		std::string b = "コンピュータ";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::string a = XLStringUtil::chop(" コンピュータ ");
		std::string b = "コンピュータ";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::string a = XLStringUtil::chop(" コンピュータ A ");
		std::string b = "コンピュータ A";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::string a = XLStringUtil::chop("abc");
		std::string b = "abc";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::string a = XLStringUtil::chop(" abc");
		std::string b = "abc";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::string a = XLStringUtil::chop(" abc  ");
		std::string b = "abc";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::string a = XLStringUtil::chop(" abc \r\n");
		std::string b = "abc";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::string a = XLStringUtil::chop("			abc \r\n");
		std::string b = "abc";
		SEXYTEST_EQ(a ,b); 
	}
}


//巡回して、関数 func を適応。 true を返したものだけを結合する。
std::string XLStringUtil::string_filter(const std::list<std::string>& list,const std::function<bool(const std::string&)>& func)
{
	std::string ret;
	for(auto it = list.begin() ; it != list.end() ; ++it )
	{
		if ( func(*it) )
		{
			ret += *it;
		}
	}
	return ret;
}

//巡回して、関数 func を適応。 funcの戻り文字列を結合します。
std::string XLStringUtil::string_map(const std::list<std::string>& list,const std::function<std::string (const std::string&)>& func)
{
	std::string ret;
	for(auto it = list.begin() ; it != list.end() ; ++it )
	{
		ret += func(*it);
	}
	return ret;
}

//巡回して、関数 func を適応。 true を返したものだけを返す。
std::list<std::string> XLStringUtil::array_filter(const std::list<std::string>& list,const std::function<bool(const std::string&)>& func)
{
	std::list<std::string> ret;
	for(auto it = list.begin() ; it != list.end() ; ++it )
	{
		if ( func(*it) )
		{
			ret.push_back(*it);
		}
	}
	return ret;
}

//巡回して、関数 func を適応。 funcの戻りで配列作って返します。
std::list<std::string> XLStringUtil::array_map(const std::list<std::string>& list,const std::function<std::string (const std::string&)>& func)
{
	std::list<std::string> ret;
	for(auto it = list.begin() ; it != list.end() ; ++it )
	{
		ret.push_back(func(*it));
	}
	return ret;
}

//何個検索文字が含まれているかを返す
unsigned int XLStringUtil::strcount(const std::string &inTarget ,const std::string &inSearch)
{
	if (inSearch.empty()) return 0;
	unsigned int count = 0;
	const char * p = inTarget.c_str();
	const char * match;
	while( match = XLStringUtil::strstr( p , inSearch.c_str() ) )
	{
		count++;
		p = match + inSearch.size();
	}
	return count;
}
//マルチバイト対応 なぜか std::string に標準で用意されていない置換。ふぁっく。
std::string XLStringUtil::replace(const std::string &inTarget ,const std::string &inOld ,const std::string &inNew)
{
	if (inOld.empty()) return inTarget;

	std::string ret;
	ret.reserve( inTarget.size() );	//先読み.

	const char * p = inTarget.c_str();
	const char * match;
	while( match = XLStringUtil::strstr( p , inOld.c_str() ) )
	{
		//ret += std::string(p,0,(int)(match - p));
		ret.append(p,(int)(match - p));
		ret += inNew;

		p = match + inOld.size();
	}
	//残りを足しておしまい.
	return ret + p;
}


//マルチバイト非対応 の文字列置換
std::string XLStringUtil::replace_low(const std::string &inTarget ,const std::string &inOld ,const std::string &inNew)
{
	if (inOld.empty()) return inTarget;

	std::string ret;
	ret.reserve( inTarget.size() );	//先読み.

	const char * p = inTarget.c_str();
	const char * match;
	while( match = ::strstr( p , inOld.c_str() ) )
	{
//		ret += std::string(p,0,(int)(match - p));
		ret.append(p,(int)(match - p));
		ret += inNew;

		p = match + inOld.size();
	}
	//残りを足しておしまい.
	return ret + p;
}


SEXYTEST()
{
	{
		std::string a = XLStringUtil::replace("にょろーん","ろー" , "ぱーー");
		SEXYTEST_EQ(a ,"にょぱーーん");
	}
	{
		std::string a = XLStringUtil::replace("ABCDEFG","BCD" , "XYZ");
		SEXYTEST_EQ(a ,"AXYZEFG");
	}
}



//みんな大好きPHPのhtmlspecialchars
//タグをエスケープ 基本的に PHP の htmlspecialchars と同じ.
//http://search.net-newbie.com/php/function.htmlspecialchars.html
std::string XLStringUtil::htmlspecialchars(const std::string &inStr)
{
	return replace(replace(replace(replace(inStr , ">" , "&gt;") , "<" , "&lt;") , "\"", "&quot;"), "'","&apos;");
}

//マルチバイト非対応 タグをエスケープ
std::string XLStringUtil::htmlspecialchars_low(const std::string &inStr)
{
	return replace_low(replace_low(replace_low(replace_low(inStr , ">" , "&gt;") , "<" , "&lt;") , "\"", "&quot;"), "'","&apos;");
}

//みんな大好きPHPのnl2br
//\nを<br>に 基本的に PHP の nl2br と同じ.
std::string XLStringUtil::nl2br(const std::string &inStr)
{
	return replace(inStr , "\r\n" , "<br>");
}

//マルチバイト非対応 \nを<br>に 基本的に PHP の nl2br と同じ.
std::string XLStringUtil::nl2br_low(const std::string &inStr)
{
	return replace_low(inStr , "\r\n" , "<br>");
}

//jsonで利用できる文字列表記にする
std::string XLStringUtil::jsonescape(const std::string & str)
{
	static const char *replaceTable[] = {
		 "\\","\\\\"
		,"\r","\\r"
		,"\n","\\n"
		,"\t","\\t"
//		,"'","\\'"
		,"\"","\\\""
		,"/","\\/"
		,"\b","\\b"
		,"\f","\\f"
		,"\u2028","\\n"
		,"\u2029","\\n"
		,NULL,NULL
	};
	return "\""+XLStringUtil::replace(str ,replaceTable ) + "\"";
}

//jsonに追加できる文字列として返す
std::string XLStringUtil::jsonvalue(const std::string & name,const std::string & str)
{
	return jsonescape(name) + ": " + jsonescape(str) ;
}

//制御文字を飛ばす
std::string XLStringUtil::clearcontrollcode(const std::string & str)
{
	static const char *replaceTable[] = {
		 "\r",""
		,"\n",""
		,NULL,NULL
	};
	return XLStringUtil::replace(str ,replaceTable );
}




//inTarget の inStart ～ inEnd までを取得
std::string XLStringUtil::cut(const std::string &inTarget , const std::string & inStart , const std::string &inEnd , std::string * outNext )
{
	const char * p = NULL;
	if ( !inStart.empty() )
	{
		p = strstr(inTarget.c_str() , inStart.c_str() );
		if (p == NULL ) return "";
		p += inStart.size();
	}
	else
	{
		p = inTarget.c_str();
	}

	const char * p2 = NULL;
	if ( ! inEnd.empty() )
	{
		p2 = strstr(p , inEnd.c_str() );
		if (p2 == NULL ) return "";
	}
	else
	{
		p2 = p + strlen(p);
	}

	const std::string ret = std::string( p , 0 , p2 - p );
	if (outNext)
	{
		*outNext = std::string(p2 + inEnd.size());
	}

	return ret;
}

//拡張子を取得する. abc.cpp -> ".cpp" のような感じになるよ
std::string XLStringUtil::baseext(const std::string &fullpath)
{
	const char * p = XLStringUtil::strrchr(fullpath.c_str() , '.');
	if (!p) return "";
	return p;
}

SEXYTEST()
{
	{
		std::string a = XLStringUtil::baseext("c:\\aaa\\bbb\\ccc.exe");
		std::string b = ".exe";
		SEXYTEST_EQ(a ,b); 
	}
}

//拡張子を取得する. abc.cpp -> "cpp" のような感じになるよ . をつけない
std::string XLStringUtil::baseext_nodot(const std::string &fullpath)
{
	const char * p = XLStringUtil::strrchr(fullpath.c_str() , '.');
	if (!p) return "";
	return p + 1;
}

SEXYTEST()
{
	{
		std::string a = XLStringUtil::baseext_nodot("c:\\aaa\\bbb\\ccc.exe");
		std::string b = "exe";
		SEXYTEST_EQ(a ,b); 
	}
}


//拡張子を取得する. abc.Cpp -> "cpp" のような感じになるよ . をつけないで小文字
std::string XLStringUtil::baseext_nodotsmall(const std::string &fullpath)
{
	const char * p = XLStringUtil::strrchr(fullpath.c_str() , '.');
	if (!p) return "";
	return XLStringUtil::strtolower(p + 1);
}

//ベースディレクトリを取得する  c:\\hoge\\hoge.txt -> c:\\hoge にする  最後の\\ は消える。
std::string XLStringUtil::basedir(const std::string &fullpath)
{
#ifdef _MSC_VER
	//SJISだとこんな感じかな・・・
	const char * p = XLStringUtil::strrchr(fullpath.c_str() , '\\');
#else
	//UTF-8だと仮定してやるよw
	const char * p = XLStringUtil::strrchr(fullpath.c_str() , '/');
#endif
	if (!p) return "";
	return fullpath.substr(0, (unsigned int) (p - fullpath.c_str()) );
}

SEXYTEST()
{
	{
#ifdef _MSC_VER
		std::string a = XLStringUtil::basedir("c:\\aaa\\bbb\\ccc.exe");
		std::string b = "c:\\aaa\\bbb";
		SEXYTEST_EQ(a ,b); 
#endif
	}
}


//ファイル名を取得する  c:\\hoge\\hoge.txt -> hoge.txt
std::string XLStringUtil::basename(const std::string &fullpath)
{
#ifdef _MSC_VER
	//SJISだとこんな感じかな・・・
	const char * p = XLStringUtil::strrchr(fullpath, '\\');
#else
	//UTF-8だと仮定してやるよw
	const char * p = XLStringUtil::strrchr(fullpath, '/');
#endif
	if (!p) return fullpath;
	return p + 1;
}

SEXYTEST()
{
	{
#ifdef _MSC_VER
		std::string a = XLStringUtil::basename("c:\\aaa\\bbb\\ccc.exe");
		std::string b = "ccc.exe";
		SEXYTEST_EQ(a ,b); 
#endif
	}
}


//ファイル名だけ(拡張子なし)を取得する  c:\\hoge\\hoge.txt -> hoge
std::string XLStringUtil::basenameonly(const std::string &fullpath)
{
#ifdef _MSC_VER
	//SJISだとこんな感じかな・・・
	const char * p = XLStringUtil::strrchr(fullpath, '\\');
#else
	//UTF-8だと仮定してやるよw
	const char * p = XLStringUtil::strrchr(fullpath, '/');
#endif
	if (!p) p = fullpath.c_str();
	else p = p + 1;

	const char* ext = XLStringUtil::strrchr(p , '.');
	if (ext == NULL) return p;

	return std::string(p , 0 , ext - p );
}

SEXYTEST()
{
	{
#ifdef _MSC_VER
		std::string a = XLStringUtil::basenameonly("c:\\aaa\\bbb\\ccc.exe");
		std::string b = "ccc";
		SEXYTEST_EQ(a ,b); 
#endif
	}
}


//フルパスかどうか
bool XLStringUtil::isfullpath(const std::string& dir,const std::string& pathsep)
{
	const std::string _dir = pathseparator(dir,pathsep);

#ifdef _MSC_VER
	if ( XLStringUtil::strpos(_dir,pathsep + pathsep ) == 0 )
	{// \\\\hogehoge
		return true;
	}
	if ( XLStringUtil::strpos(_dir,":" + pathsep ) == 1 )
	{// c:\\ 
		return true;
	}
#else
	if ( XLStringUtil::strpos(_dir,pathsep) == 0 )
	{// /var/hog ...
		return true;
	}
#endif
	return false;
}

//先頭から a と b の同一部分の文字数を返す
int XLStringUtil::strmatchpos(const std::string& a,const std::string& b)
{
	const char * _x;
	const char * _a = a.c_str();
	const char * _b = b.c_str();
	_x = _a;
	while(*_a++ == *_b++ && *_a != 0 );

	return (int)(_x - _a);
}

std::string XLStringUtil::pathcombine(const std::string& base,const std::string& dir,const std::string& pathsep)
{
#ifdef _MSC_VER
	std::string _pathsep = pathsep.empty() ? "\\" : pathsep;
#else
	std::string _pathsep = pathsep.empty() ? "/" : pathsep;
#endif
	std::string _base = pathseparator(base,pathsep);
	std::string _dir = pathseparator(dir,pathsep);

	std::list<std::string> nodes = split(_pathsep,_base + _pathsep + _dir);

	std::list<std::string>::iterator i = nodes.begin();
	std::list<std::string> useNodes;
	for( ; i != nodes.end() ; ++ i )
	{
		if ( i->empty() )
		{
			continue;
		}
		if (*i == ".")
		{
			if ( i == nodes.begin() )
			{//先頭にある ./ だけは残す
				useNodes.push_back(*i);
			}
			else
			{//それ以外の . は無視
			}
		}
		else if (*i == "..")
		{
			if (useNodes.size() == 1)
			{
#ifdef _MSC_VER
				auto topnode = useNodes.begin();
				if ( topnode->size() >= 2 && (topnode->c_str())[1] == ':' )
				{//windowsのc: より上には上がれないので無視
					continue;
				}
#else
				
#endif
				useNodes.pop_back();
				useNodes.push_back("");	//  /を追加.
			}
			else
			{
				useNodes.pop_back();
			}
		}
		else
		{
			useNodes.push_back(*i);
		}
	}
	std::string retpath = join(_pathsep,useNodes);
#ifdef _MSC_VER
#else
	if ( XLStringUtil::isfullpath(_base,_pathsep) )
	{
		retpath = "/" + retpath;
	}
	else
	{
	}
#endif
	return retpath;
}


//パスの区切り文字を平らにする.
std::string XLStringUtil::pathseparator(const std::string& path,const std::string& pathsep)
{
	if ( pathsep.empty() )
	{
#ifdef _MSC_VER
		return XLStringUtil::replace(path , "/" , "\\");
#else
		return XLStringUtil::replace(path , "\\" , "/");
#endif
	}
	if ( pathsep == "/" )
	{
		return XLStringUtil::replace(path , "\\" , "/");
	}
	else if ( pathsep == "\\" )
	{
		return XLStringUtil::replace(path , "/" , "\\");
	}
	else
	{
		assert(0);
		return path;
	}
}


//URLパラメーターの追加.
std::string XLStringUtil::AppendURLParam(const std::string& url,const std::string& append)
{
	if (url.find("?") != -1 )
	{
		return url + "&" + append;
	}
	return url + "?" + append;
}

SEXYTEST()
{
	{
		std::string a = XLStringUtil::AppendURLParam("http://127.0.0.1:15550/media_start","accesstoken=kaede");
		std::string b = "http://127.0.0.1:15550/media_start?accesstoken=kaede";
		SEXYTEST_EQ(a ,b); 
	}
}


//コマンドライン引数パース
std::list<std::string> XLStringUtil::parse_command(const std::string & command)
{
	enum state
	{
		 NO
		,TARGET
		,QUOTE_SINGLE
		,QUOTE_DOUBLE
	};
	state s = NO;
	const char* targetStart = NULL;
	std::list<std::string> ret;

	const char * p = command.c_str();
	for( ; *p ; ++p)
	{
		switch (s)
		{
		case NO:
			{
				if (*p == '"')
				{
					s = QUOTE_DOUBLE;
				}
				else if (*p == '\'')
				{
					s = QUOTE_SINGLE;
					targetStart = p + 1;
				}
				else if (*p == ' ' || *p == '\t')
				{
					//nop
				}
				else
				{
					s = TARGET;
					targetStart = p;
				}
				break;
			}
		case QUOTE_DOUBLE:
			{
				if (*p == '"')
				{
					assert(targetStart != NULL);
					s = NO;
					ret.push_back( std::string(targetStart ,0 , (int) (p - targetStart - 1) ) );
					targetStart = NULL;
				}
				else if (*p == '\\' && *(p+1) == '"')
				{
					//クウォートスキップ
					p++;
				}
			}
		case QUOTE_SINGLE:
			{
				if (*p == '\'')
				{
					assert(targetStart != NULL);
					s = NO;
					ret.push_back( std::string(targetStart ,0 , (int) (p - targetStart - 1) ) );
					targetStart = NULL;
				}
				else if (*p == '\\' && *(p+1) == '\'')
				{
					//クウォートスキップ
					p++;
				}
			}
		case TARGET:
			{
				if (*p == ' ' || *p == '\t')
				{
					assert(targetStart != NULL);
					s = NO;
					ret.push_back( std::string(targetStart ,0 , (int) (p - targetStart) ) );
					targetStart = NULL;
				}
				else if (*p == '\\' && (*(p+1) == ' ' && *(p+1) == '\t') )
				{
					//クウォートスキップ
					p++;
				}
				break;
			}
		}
	}
	if (s == QUOTE_SINGLE || s == QUOTE_DOUBLE  || s == TARGET)
	{
		assert(targetStart != NULL);
		ret.push_back( std::string(targetStart ,0 , (int) (p - targetStart) ) );
	}
	return ret;
}

//bigramによる文字列分割
std::list<std::string> XLStringUtil::makebigram(const std::string & words)
{
	std::list<std::string> ret;
	const char * s = NULL;
	const char * n = NULL;
	for(const char * p = words.c_str() ; *p ; )
	{
		s = nextChar(p);
		if (*s == 0x00)
		{
			break;
		}
		n = nextChar(s);

		ret.push_back( std::string(p,0,(int)(n-p) ) );

		p = s;
	}
	return ret;
}

SEXYTEST()
{
	{
		std::list<std::string> a = XLStringUtil::makebigram("abc");
		std::list<std::string> b ; b.push_back("ab"); b.push_back("bc");
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::list<std::string> a = XLStringUtil::makebigram("愛飢え男");
		std::list<std::string> b ; b.push_back("愛飢"); b.push_back("飢え"); b.push_back("え男");
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::list<std::string> a = XLStringUtil::makebigram("ai飢え男");
		std::list<std::string> b ; b.push_back("ai"); b.push_back("i飢"); b.push_back("飢え"); b.push_back("え男");
		SEXYTEST_EQ(a ,b); 
	}
}

//指定した幅で丸める
std::string XLStringUtil::strimwidth(const std::string &  str , int startMoji , int widthMoji ,const std::string& trimmarker)
{
	int countMoji = 0;
	std::string ret;

	//先頭も省略する場合
	if (startMoji >= 1)
	{
		ret = trimmarker;
	}

	//開始位置を見つける
	const char * p;
	for(p = str.c_str() ; *p ;)
	{
		if (countMoji >= startMoji)
		{
			break;
		}
		p = nextChar(p);
		countMoji++;
	}
	const char * start = p;

	//省略位置を見つける
	for( ; *p ; )
	{
		if (countMoji >= startMoji + widthMoji)
		{
			break;
		}
		p = nextChar(p);
		countMoji++;
	}

	//メインとなる文字列を取得
	ret += std::string(start , 0 ,(int) (p - start));

	//後ろを省略している場合
	if (countMoji >= startMoji + widthMoji)
	{
		ret += trimmarker;
	}
	return ret;
}


SEXYTEST()
{
	{
		std::string a = XLStringUtil::strimwidth("あいうえおあお",2,3,"!!");
		std::string b = "!!うえお!!";
		SEXYTEST_EQ(a ,b); 
	}
}


//マルチバイト対応 ダブルクウォート
std::string XLStringUtil::doublequote(const std::string& str)
{
	return replace(str , "\"" , "\\\"" );
}

//非マルチバイトのダブルクウォート
std::string XLStringUtil::doublequote_low(const std::string& str)
{
	return replace_low(str , "\"" , "\\\"" );
}

//quoteをはがす
std::string XLStringUtil::dequote(const std::string& str)
{
	if (str.size() <= 1) return str;
	if ( str[0] == '\"' && str[str.size() - 1] == '\"') return str.substr(1,str.size() - 2);
	if ( str[0] == '\'' && str[str.size() - 1] == '\'') return str.substr(1,str.size() - 2);
	return str;
}


//重複削除
std::list<std::string> XLStringUtil::unique(const std::list<std::string>& list)
{
	//単純なアルゴリズムなので遅いです。 気にしない!
	std::list<std::string> ret;
	for(std::list<std::string>::const_iterator it = list.begin() ; it != list.end() ; ++it) 
	{
		if ( ret.empty() )
		{
			ret.push_back(*it);
			continue;
		}

		std::list<std::string>::const_iterator itf = ret.begin();
		for( ; itf != ret.end() ; ++itf) 
		{
			if (*it == *itf)
			{
				break;
			}
		}
		if (itf == ret.end())
		{
			ret.push_back(*it);
		}
	}
	return ret;
}
//マルチバイト対応 inOldにマッチしたものがあったら消します
std::string XLStringUtil::remove(const std::string &inTarget ,const std::string &inOld )
{
	std::string ret;
	ret.reserve( inTarget.size() );	//先読み.

	const char * p = inTarget.c_str();
	const char * match;
	while( match = XLStringUtil::strstr( p , inOld.c_str() ) )
	{
		ret += std::string(p,0,(int)(match - p));
		p = match + inOld.size();
	}
	//残りを足しておしまい.
	return ret + p;
}

//マルチバイト対応 複数の候補を一括置換 const char * replacetable[] = { "A","あ"  ,"I","い"  , "上","うえ" , NULL , NULL}  //必ず2つ揃えで書いてね
std::string XLStringUtil::replace(const std::string &inTarget ,const char** replacetable,bool isrev)
{
	std::string ret;
	ret.reserve( inTarget.size() );	//先読み.

	if (inTarget.empty())
	{
		return inTarget;
	}

	const char * p = inTarget.c_str();
	for(; *p ; )
	{
		const char * pp = p;
		p = nextChar(p);

		int compareIndex = isrev == false ? 0 : 1;
		int replaceIndex = isrev == false ? 1 : 0;
		const char ** r1 = replacetable;
		for( ; *r1 != NULL ; r1+=2)
		{
			const char * ppp = pp;
			const char * r2 = *(r1+compareIndex);
			for( ; 1 ; ++r2,++ppp )
			{
				if ( *r2 == 0 || *ppp != *r2)
				{
					break;
				}
			}
			if (*r2 == 0)  //無事比較文字列の方が終端にたどりついた
			{
				ret.append(*(r1+replaceIndex));
				p = ppp;
				break;
			}
		}
		if ( *r1 == 0 )
		{
			ret.append(pp,(int) (p - pp));
		}
	}
	return ret;
}

std::string XLStringUtil::replace_low(const std::string &inTarget ,const char** replacetable,bool isrev)
{
	std::string ret;
	ret.reserve( inTarget.size() );	//先読み.

	if (inTarget.empty())
	{
		return inTarget;
	}

	const char * p = inTarget.c_str();
	for(; *p ; )
	{
		const char * pp = p;
		p++;

		int compareIndex = isrev == false ? 0 : 1;
		int replaceIndex = isrev == false ? 1 : 0;
		const char ** r1 = replacetable;
		for( ; *r1 != NULL ; r1+=2)
		{
			const char * ppp = pp;
			const char * r2 = *(r1+compareIndex);
			for( ; 1 ; ++r2,++ppp )
			{
				if ( *r2 == 0 || *ppp != *r2)
				{
					break;
				}
			}
			if (*r2 == 0)  //無事比較文字列の方が終端にたどりついた
			{
				ret.append(*(r1+replaceIndex));
				p = ppp;
				break;
			}
		}
		if ( *r1 == 0 )
		{
			ret.append(pp,(int) (p - pp));
		}
	}
	return ret;
}


SEXYTEST()
{
	{
		const char *replaceTable[] = {
			 "アイ","あい"
			,"イ","い"
			,NULL,NULL
		};
		std::string a = XLStringUtil::replace("うアイう",replaceTable);
		std::string b = "うあいう";
		SEXYTEST_EQ(a ,b); 
	}
}


//remove 複数の候補を一括削除  const char * replacetable[] = {"A","B","あ","うえお" , NULL} 全部消します
std::string XLStringUtil::remove(const std::string &inTarget ,const char** replacetable)
{
	std::string ret;
	ret.reserve( inTarget.size() );	//先読み.

	if (inTarget.empty())
	{
		return inTarget;
	}

	const char * p = inTarget.c_str();
	for(; *p ; )
	{
		const char * pp = p;
		p = nextChar(p);

		const char ** r1 = replacetable;
		for( ; *r1 != NULL ; r1++)
		{
			const char * ppp = pp;
			const char * r2 = *(r1);
			for( ; 1 ; ++r2,++ppp )
			{
				if ( *r2 == 0 || *ppp != *r2)
				{
					break;
				}
			}
			if (*r2 == 0)  //無事比較文字列の方が終端にたどりついた
			{
				p = ppp;
				break;
			}
		}
		if ( *r1 == 0 )
		{
			ret.append(pp,(int) (p - pp));
		}
	}
	return ret;
}


//typo修正
//r	 「ローマ字」を「ひらがな」に変換します。
//R	 「ひらがな」を「ローマ字」に変換します。
//k	 「かな入力typo」を「ひらがな」に変換します。
//K	 「ひらがな」を「かな入力typo」に変換します。
std::string XLStringUtil::mb_convert_typo(const std::string &inTarget,const std::string& option)
{
	static const char *replaceTableRomaNobasu[] = {
		 "aa","aー"
		,"ii","iー"
		,"uu","uー"
		,"ee","eー"
		,"oo","oー"
		,"","ー"
		,NULL,NULL
	};
	static const char *replaceTableRoma[] = {
		 "tsu","っ"
		,"match","まっ"
		,"ltsu","っ"
		,"u","う"
		,"lyi","ぃ"
		,"xyi","ぃ"
		,"lye","ぇ"
		,"xye","ぇ"
		,"wha","あぁ"
		,"wha","うぁ"
		,"whi","うぃ"
		,"whe","うぇ"
		,"who","うぉ"
		,"kyi","きぃ"
		,"kye","きぇ"
		,"kya","きゃ"
		,"kyu","きゅ"
		,"kyo","きょ"
		,"kwa","くぁ"
		,"qwa","くぁ"
		,"qwi","くぃ"
		,"qyi","くぃ"
		,"qwu","くぅ"
		,"qwe","くぇ"
		,"qye","くぇ"
		,"qwo","くぉ"
		,"qya","くゃ"
		,"qyu","くゅ"
		,"qyo","くょ"
		,"syi","しぃ"
		,"swi","しぇ"
		,"sha","しゃ"
		,"shu","しゅ"
		,"sho","しょ"
		,"syi","しぇ"
		,"sya","しゃ"
		,"syu","しゅ"
		,"syo","しょ"
		,"si","し"
		,"shi","し"
		,"swa","すぁ"
		,"swi","すぃ"
		,"swu","すぅ"
		,"swe","すぇ"
		,"swo","すぉ"
		,"cyi","ちぃ"
		,"tyi","ちぃ"
		,"che","ちぇ"
		,"cye","ちぇ"
		,"tye","ちぇ"
		,"cha","ちゃ"
		,"cya","ちゃ"
		,"tya","ちゃ"
		,"chu","ちゅ"
		,"cyu","ちゅ"
		,"tyu","ちゅ"
		,"cho","ちょ"
		,"cyo","ちょ"
		,"tyo","ちょ"
		,"ci","ち"
		,"chi","ち"
		,"tsa","つぁ"
		,"tsi","つぃ"
		,"tse","つぇ"
		,"tso","つぉ"
		,"tu","つ"
		,"tsu","つ"
		,"ltu","っ"
		,"xtu","っ"
		,"thi","てぃ"
		,"the","てぇ"
		,"tha","てゃ"
		,"thu","てゅ"
		,"tho","てょ"
		,"twa","とぁ"
		,"twi","とぃ"
		,"twu","とぅ"
		,"twe","とぇ"
		,"two","とぉ"
		,"nyi","にぃ"
		,"nye","にぇ"
		,"nya","にゃ"
		,"nyu","にゅ"
		,"nyo","にょ"
		,"hyi","ひぃ"
		,"hye","ひぇ"
		,"hya","ひゃ"
		,"hyu","ひゅ"
		,"hyo","ひょ"
		,"fwa","ふぁ"
		,"fwi","ふぃ"
		,"fyi","ふぃ"
		,"fwu","ふぅ"
		,"few","ふぇ"
		,"fye","ふぇ"
		,"fwo","ふぉ"
		,"fya","ふゃ"
		,"fyu","ふゅ"
		,"fyo","ふょ"
		,"myi","みぃ"
		,"mye","みぇ"
		,"mya","みゃ"
		,"myu","みゅ"
		,"myo","みょ"
		,"lya","ゃ"
		,"xya","ゃ"
		,"lyu","ゅ"
		,"xyu","ゅ"
		,"lyo","ょ"
		,"xyo","ょ"
		,"ryi","りぃ"
		,"rye","りぇ"
		,"rya","りゃ"
		,"ryu","りゅ"
		,"ryo","りょ"
		,"gyi","ぎぃ"
		,"gye","ぎぇ"
		,"gya","ぎゃ"
		,"gyu","ぎゅ"
		,"gyo","ぎょ"
		,"gwa","ぐぁ"
		,"gwi","ぐぃ"
		,"gwu","ぐぅ"
		,"gwe","ぐぇ"
		,"gwo","ぐぉ"
		,"jyi","じぃ"
		,"zyi","じぃ"
		,"jye","じぇ"
		,"zye","じぇ"
		,"jya","じゃ"
		,"zya","じゃ"
		,"lwa","ゎ"
		,"xwa","ゎ"
		,"jyu","じゅ"
		,"zyu","じゅ"
		,"jyo","じょ"
		,"zyo","じょ"
		,"dyi","ぢぃ"
		,"dye","ぢぇ"
		,"dya","ぢゃ"
		,"dyu","ぢゅ"
		,"dyo","ぢょ"
		,"dhi","でぃ"
		,"dhe","でぇ"
		,"dha","でゃ"
		,"dhu","でゅ"
		,"dho","でょ"
		,"dwa","どぁ"
		,"dwi","どぃ"
		,"dwu","どぅ"
		,"dwe","どぇ"
		,"dwo","どぉ"
		,"byi","びぃ"
		,"bye","びぇ"
		,"bya","びょ"
		,"byu","びゅ"
		,"byo","びょ"
		,"pyi","ぴぃ"
		,"pye","ぴぇ"
		,"pya","ぴゃ"
		,"pyu","ぴゅ"
		,"pyo","ぴょ"
		,"vyi","う゛ぃ"
		,"vye","う゛ぇ"
		,"vya","う゛ゃ"
		,"vyu","う゛ゅ"
		,"vyo","う゛ょ"
		,"wu","う"
		,"la","ぁ"
		,"li","ぃ"
		,"xi","ぃ"
		,"lu","ぅ"
		,"xu","ぅ"
		,"le","ぇ"
		,"xe","ぇ"
		,"lo","ぉ"
		,"xo","ぉ"
		,"ye","いぇ"
		,"ka","か"
		,"ca","か"
		,"lka","か" //ヵ
		,"xka","か" //ヵ
		,"ki","き"
		,"qa","くぁ"
		,"qi","くぃ"
		,"qe","くぇ"
		,"qo","くぉ"
		,"ku","く"
		,"cu","く"
		,"qu","く"
		,"ke","け"
		,"lke","け" //ヶ
		,"xke","け" //ヶ
		,"ko","こ"
		,"co","こ"
		,"sa","さ"
		,"si","し"
		,"ci","し"
		,"su","す"
		,"se","せ"
		,"ce","せ"
		,"so","そ"
		,"ta","た"
		,"ti","ち"
		,"tu","つ"
		,"te","て"
		,"to","と"
		,"na","な"
		,"ni","に"
		,"nu","ぬ"
		,"ne","ね"
		,"no","の"
		,"ha","は"
		,"hi","ひ"
		,"fa","ふぁ"
		,"fa","ふぁ"
		,"fi","ふぃ"
		,"fe","ふぇ"
		,"fo","ふぉ"
		,"hu","ふ"
		,"fu","ふ"
		,"he","へ"
		,"ho","ほ"
		,"ma","ま"
		,"mi","み"
		,"mu","む"
		,"me","め"
		,"mo","も"
		,"ya","や"
		,"yu","ゆ"
		,"yo","よ"
		,"ra","ら"
		,"ri","り"
		,"ru","る"
		,"re","れ"
		,"ro","ろ"
		,"wa","わ"
		,"wo","を"
		,"nn","ん"
		,"xn","ん"
		,"ga","が"
		,"gi","ぎ"
		,"gu","ぐ"
		,"ge","げ"
		,"go","ご"
		,"za","ざ"
		,"je","じぇ"
		,"ja","じゃ"
		,"ju","じゅ"
		,"jo","じょ"
		,"zi","じ"
		,"ji","じ"
		,"zu","ず"
		,"ze","ぞ"
		,"zo","ぞ"
		,"da","だ"
		,"di","ぢ"
		,"ji","ぢ"
		,"du","づ"
		,"de","で"
		,"do","ど"
		,"ba","ば"
		,"bi","び"
		,"bu","ぶ"
		,"be","べ"
		,"bo","ぼ"
		,"pa","ぱ"
		,"pi","ぴ"
		,"pu","ぷ"
		,"pe","ぺ"
		,"po","ぽ"
		,"va","う゛ぁ"
		,"vi","う゛ぃ"
		,"ve","う゛ぇ"
		,"vo","う゛ぉ"
		,"vu","う゛"
		,"a","あ"
		,"i","い"
		,"u","う"
		,"e","え"
		,"o","お"
		,NULL,NULL
	};
	static const char *replaceTableKana[] = {
		 "4@","う゛"
		,"a","あ"
		,"e","い"
		,"4","う"
		,"5","え"
		,"6","お"
		,"t","か"
		,"g","き"
		,"h","く"
		,":","け"
		,"b","こ"
		,"x","さ"
		,"d","し"
		,"r","す"
		,"p","せ"
		,"c","そ"
		,"q","た"
		,"a","ち"
		,"z","つ"
		,"w","て"
		,"s","と"
		,"u","な"
		,"i","に"
		,"1","ぬ"
		,",","ね"
		,"k","の"
		,"f","は"
		,"v","ひ"
		,"2","ふ"
		,"^","へ"
		,"-","ほ"
		,"j","ま"
		,"n","み"
		,"]","む"
		,"/","め"
		,"m","も"
		,"7","や"
		,"8","ゆ"
		,"9","よ"
		,"o","ら"
		,"l","り"
		,".","る"
		,";","れ"
		,"\\","ろ"
		,"0","わ"
		//,"","を"
		,"y","ん"
		,"#","ぁ"
		,"E","ぃ"
		,"$","ぅ"
		,"%","ぇ"
		,"&","ぉ"
		,"t@","が"
		,"g@","ぎ"
		,"h@","ぐ"
		,":@","げ"
		,"b@","ご"
		,"x@","ざ"
		,"d@","じ"
		,"r@","ず"
		,"p@","ぜ"
		,"c@","ぞ"
		,"q@","だ"
		,"a@","ぢ"
		,"z@","づ"
		,"w@","で"
		,"s@","ど"
		,"f@","ば"
		,"v@","び"
		,"2@","ぶ"
		,"^@","べ"
		,"-@","ぼ"
		,"f[","ぱ"
		,"v[","ぴ"
		,"2[","ぷ"
		,"^[","ぺ"
		,"-[","ぽ"
		,"'","ゃ"
		,"(","ゅ"
		,")","ょ"
		,"Z","っ"
		//,"ヮ","ゎ"
		,NULL,NULL
	};
	std::string ret = inTarget;
	//r	 「ローマ字」を「ひらがな」に変換します。
	//R	 「ひらがな」を「ローマ字」に変換します。
	if ( option.find("r") != -1  )
	{
		ret = XLStringUtil::replace(ret ,replaceTableRoma,false );
	}
	if ( option.find("R") != -1  )
	{
		ret = XLStringUtil::replace(ret ,replaceTableRoma,true );
		ret = XLStringUtil::replace(ret ,replaceTableRomaNobasu,true );
	}

	//k	 「かな入力typo」を「ひらがな」に変換します。
	//K	 「ひらがな」を「かな入力typo」に変換します。
	if ( option.find("k") != -1  )
	{
		ret = XLStringUtil::replace(ret ,replaceTableKana,true );
	}
	if ( option.find("K") != -1  )
	{
		ret = XLStringUtil::replace(ret ,replaceTableKana,false );
	}
	return ret;
}



//みんな大好き PHPのmb_convert_kanaの移植
//n	 「全角」数字を「半角」に変換します。
//N	 「半角」数字を「全角」に変換します。
//a	 「全角」英数字を「半角」に変換します。
//A	 「半角」英数字を「全角」に変換します 
//s	 「全角」スペースを「半角」に変換します
//S	 「半角」スペースを「全角」に変換します（U+0020 -> U+3000）。
//k	 「全角カタカナ」を「半角カタカナ」に変換します。
//K	 「半角カタカナ」を「全角カタカナ」に変換します。
//h	 「全角ひらがな」を「半角カタカナ」に変換します。
//H	 「半角カタカナ」を「全角ひらがな」に変換します。
//c	 「全角カタカナ」を「全角ひらがな」に変換します。
//C	 「全角ひらがな」を「全角カタカナ」に変換します。
std::string XLStringUtil::mb_convert_kana(const std::string &inTarget,const std::string& option)
{
	std::string ret = inTarget;
	static const char *replaceTableAplha[] = {
		 "Ａ","A"
		,"Ｂ","B"
		,"Ｃ","C"
		,"Ｄ","D"
		,"Ｅ","E"
		,"Ｆ","F"
		,"Ｇ","G"
		,"Ｈ","H"
		,"Ｉ","I"
		,"Ｊ","J"
		,"Ｋ","K"
		,"Ｌ","L"
		,"Ｍ","M"
		,"Ｎ","N"
		,"Ｏ","O"
		,"Ｐ","P"
		,"Ｑ","Q"
		,"Ｒ","R"
		,"Ｓ","S"
		,"Ｔ","T"
		,"Ｕ","U"
		,"Ｖ","V"
		,"Ｗ","W"
		,"Ｘ","X"
		,"Ｙ","Y"
		,"Ｚ","Z"
		,"ａ","a"
		,"ｂ","b"
		,"ｃ","c"
		,"ｄ","d"
		,"ｅ","e"
		,"ｆ","f"
		,"ｇ","g"
		,"ｈ","h"
		,"ｉ","i"
		,"ｊ","j"
		,"ｋ","k"
		,"ｌ","l"
		,"ｍ","m"
		,"ｎ","n"
		,"ｏ","o"
		,"ｐ","p"
		,"ｑ","q"
		,"ｒ","r"
		,"ｓ","s"
		,"ｔ","t"
		,"ｕ","u"
		,"ｖ","v"
		,"ｗ","w"
		,"ｘ","x"
		,"ｙ","y"
		,"ｚ","z"
		,"ｰ","ー"
		,"‘","'"
		,"’","'"
		,"“","\""
		,"”","\""
		,"（","("
		,"）",")"
		,"〔","["
		,"〕","]"
		,"［","["
		,"］","]"
		,"｛","{"
		,"｝","}"
		,"〈","<"
		,"〉",">"
		,"《","<"
		,"》",">"
		,"「","{"
		,"」","}"
		,"『","{"
		,"』","}"
		,"【","["
		,"】","]"
		,"・","･"
		,"！","!"
		,"♯","#"
		,"＆","&"
		,"＄","$"
		,"？","?"
		,"：",":"
		,"；",";"
		,"／","/"
		,"＼","\\"
		,"＠","@"
		,"｜","|"
		,"－","-"
		,"＝","="
		,"≒","="
		,"％","%"
		,"＋","+"
		,"－","-"
		,"÷","/"
		,"＊","*"
		,"～","~" //UTF-8だと別の～もあるから判断が難しい・・・
		,NULL,NULL
	};
//r	 「全角」英字を「半角」に変換します。
//R	 「半角」英字を「全角」に変換します。
//a	 「全角」英数字を「半角」に変換します。
//A	 「半角」英数字を「全角」に変換します 
	if ( option.find("r") != -1 ||   option.find("a") != -1 )
	{
		ret = XLStringUtil::replace(ret ,replaceTableAplha,false );
	}
	else if ( option.find("R") != -1 ||  option.find("A") != -1 )
	{
		ret = XLStringUtil::replace(ret ,replaceTableAplha,true );
	}

	static const char *replaceTableNum[] = {
		 "１","1"
		,"２","2"
		,"３","3"
		,"４","4"
		,"５","5"
		,"６","6"
		,"７","7"
		,"８","8"
		,"９","9"
		,"０","0"
		,NULL,NULL
	};
//n	 「全角」数字を「半角」に変換します。
//N	 「半角」数字を「全角」に変換します。
//a	 「全角」英数字を「半角」に変換します。
//A	 「半角」英数字を「全角」に変換します 
	if ( option.find("n") != -1 ||  option.find("a") != -1 )
	{
		ret = XLStringUtil::replace(ret ,replaceTableNum,false );
	}
	else if ( option.find("N") != -1 ||  option.find("A") != -1)
	{
		ret = XLStringUtil::replace(ret ,replaceTableNum,true );
	}

	static const char *replaceTableSpace[] = {
		 "　"," "
		,NULL,NULL
	};
//s	 「全角」スペースを「半角」に変換します
//S	 「半角」スペースを「全角」に変換します
	if ( option.find("s") != -1 )
	{
		ret = XLStringUtil::replace(ret ,replaceTableSpace,false );
	}
	else if ( option.find("S") != -1)
	{
		ret = XLStringUtil::replace(ret ,replaceTableSpace,true );
	}

	static const char *replaceTableHankanaToHiragana[] = {
		 "ｳﾞ","う゛"
		,"ｶﾞ","が"
		,"ｷﾞ","ぎ"
		,"ｸﾞ","ぐ"
		,"ｹﾞ","げ"
		,"ｺﾞ","ご"
		,"ｻﾞ","ざ"
		,"ｼﾞ","じ"
		,"ｽﾞ","ず"
		,"ｾﾞ","ぜ"
		,"ｿﾞ","ぞ"
		,"ﾀﾞ","だ"
		,"ﾁﾞ","ぢ"
		,"ﾂﾞ","づ"
		,"ｾﾞ","ぜ"
		,"ｿﾞ","ぞ"
		,"ﾊﾞ","ば"
		,"ﾋﾞ","び"
		,"ﾌﾞ","ぶ"
		,"ﾍﾞ","べ"
		,"ﾎﾞ","ぼ"
		,"ﾊﾟ","ぱ"
		,"ﾋﾟ","ぴ"
		,"ﾌﾟ","ぷ"
		,"ﾍﾟ","ぺ"
		,"ﾎﾟ","ぽ"
		,"ｱ","あ"
		,"ｲ","い"
		,"ｳ","う"
		,"ｴ","え"
		,"ｵ","お"
		,"ｶ","か"
		,"ｷ","き"
		,"ｸ","く"
		,"ｹ","け"
		,"ｺ","こ"
		,"ｻ","さ"
		,"ｼ","し"
		,"ｽ","す"
		,"ｾ","せ"
		,"ｿ","そ"
		,"ﾀ","た"
		,"ﾁ","ち"
		,"ﾂ","つ"
		,"ﾃ","て"
		,"ﾄ","と"
		,"ﾅ","な"
		,"ﾆ","に"
		,"ﾇ","ぬ"
		,"ﾈ","ね"
		,"ﾉ","の"
		,"ﾊ","は"
		,"ﾋ","ひ"
		,"ﾌ","ふ"
		,"ﾍ","へ"
		,"ﾎ","ほ"
		,"ﾏ","ま"
		,"ﾐ","み"
		,"ﾑ","む"
		,"ﾒ","め"
		,"ﾓ","も"
		,"ﾔ","や"
		,"ﾕ","ゆ"
		,"ﾖ","よ"
		,"ﾗ","ら"
		,"ﾘ","り"
		,"ﾙ","る"
		,"ﾚ","れ"
		,"ﾛ","ろ"
		,"ｦ","を"
		,"ﾜ","わ"
		,"ﾝ","ん"
		,"ｧ","ぁ"
		,"ｨ","ぃ"
		,"ｩ","ぅ"
		,"ｪ","ぇ"
		,"ｫ","ぉ"
		,"ｬ","ゃ"
		,"ｭ","ゅ"
		,"ｮ","ょ"
		,"ｯ","っ"
		,"ｰ","ー"
		,NULL,NULL
	};
	static const char *replaceTableHankanaToKatakana[] = {
		 "ｳﾞ","ヴ"
		,"ｶﾞ","ガ"
		,"ｷﾞ","ギ"
		,"ｸﾞ","グ"
		,"ｹﾞ","ゲ"
		,"ｺﾞ","ゴ"
		,"ｻﾞ","ザ"
		,"ｼﾞ","ジ"
		,"ｽﾞ","ズ"
		,"ｾﾞ","ゼ"
		,"ｿﾞ","ゾ"
		,"ﾀﾞ","ダ"
		,"ﾁﾞ","ヂ"
		,"ﾂﾞ","ヅ"
		,"ｾﾞ","ゼ"
		,"ｿﾞ","ゾ"
		,"ﾊﾞ","バ"
		,"ﾋﾞ","ビ"
		,"ﾌﾞ","ブ"
		,"ﾍﾞ","ベ"
		,"ﾎﾞ","ボ"
		,"ﾊﾟ","パ"
		,"ﾋﾟ","ピ"
		,"ﾌﾟ","プ"
		,"ﾍﾟ","ペ"
		,"ﾎﾟ","ポ"
		,"ｱ","ア"
		,"ｲ","イ"
		,"ｳ","ウ"
		,"ｴ","エ"
		,"ｵ","オ"
		,"ｶ","カ"
		,"ｷ","キ"
		,"ｸ","ク"
		,"ｹ","ケ"
		,"ｺ","コ"
		,"ｻ","サ"
		,"ｼ","シ"
		,"ｽ","ス"
		,"ｾ","セ"
		,"ｿ","ソ"
		,"ﾀ","タ"
		,"ﾁ","チ"
		,"ﾂ","ツ"
		,"ﾃ","テ"
		,"ﾄ","ト"
		,"ﾅ","ナ"
		,"ﾆ","ニ"
		,"ﾇ","ヌ"
		,"ﾈ","ネ"
		,"ﾉ","ノ"
		,"ﾊ","ハ"
		,"ﾋ","ヒ"
		,"ﾌ","フ"
		,"ﾍ","ヘ"
		,"ﾎ","ホ"
		,"ﾏ","マ"
		,"ﾐ","ミ"
		,"ﾑ","ム"
		,"ﾒ","メ"
		,"ﾓ","モ"
		,"ﾔ","ヤ"
		,"ﾕ","ユ"
		,"ﾖ","ヨ"
		,"ﾗ","リ"
		,"ﾘ","リ"
		,"ﾙ","ル"
		,"ﾚ","レ"
		,"ﾛ","ロ"
		,"ｦ","ヲ"
		,"ﾜ","ワ"
		,"ﾝ","ン"
		,"ｧ","ァ"
		,"ｨ","ィ"
		,"ｩ","ゥ"
		,"ｪ","ェ"
		,"ｫ","ォ"
		,"ｬ","ャ"
		,"ｭ","ュ"
		,"ｮ","ョ"
		,"ｯ","ッ"
		,"ｰ","ー"
		,NULL,NULL
	};
	static const char *replaceTableKatakanaToHiragana[] = {
		 "ヴ","う゛"
		,"ア","あ"
		,"イ","い"
		,"ウ","う"
		,"エ","え"
		,"オ","お"
		,"カ","か"
		,"キ","き"
		,"ク","く"
		,"ケ","け"
		,"コ","こ"
		,"サ","さ"
		,"シ","し"
		,"ス","す"
		,"セ","せ"
		,"ソ","そ"
		,"タ","た"
		,"チ","ち"
		,"ツ","つ"
		,"テ","て"
		,"ト","と"
		,"ナ","な"
		,"ニ","に"
		,"ヌ","ぬ"
		,"ネ","ね"
		,"ノ","の"
		,"ハ","は"
		,"ヒ","ひ"
		,"フ","ふ"
		,"ヘ","へ"
		,"ホ","ほ"
		,"マ","ま"
		,"ミ","み"
		,"ム","む"
		,"メ","め"
		,"モ","も"
		,"ヤ","や"
		,"ユ","ゆ"
		,"ヨ","よ"
		,"ラ","ら"
		,"リ","り"
		,"ル","る"
		,"レ","れ"
		,"ロ","ろ"
		,"ワ","わ"
		,"ヲ","を"
		,"ン","ん"
		,"ァ","ぁ"
		,"ィ","ぃ"
		,"ゥ","ぅ"
		,"ェ","ぇ"
		,"ォ","ぉ"
		,"ガ","が"
		,"ギ","ぎ"
		,"グ","ぐ"
		,"ゲ","げ"
		,"ゴ","ご"
		,"ザ","ざ"
		,"ジ","じ"
		,"ズ","ず"
		,"ゼ","ぜ"
		,"ゾ","ぞ"
		,"ダ","だ"
		,"ヂ","ぢ"
		,"ヅ","づ"
		,"デ","で"
		,"ド","ど"
		,"バ","ば"
		,"ビ","び"
		,"ブ","ぶ"
		,"ベ","べ"
		,"ボ","ぼ"
		,"パ","ぱ"
		,"ピ","ぴ"
		,"プ","ぷ"
		,"ペ","ぺ"
		,"ポ","ぽ"
		,"ャ","ゃ"
		,"ュ","ゅ"
		,"ョ","ょ"
		,"ッ","っ"
		,"ヮ","ゎ"
		,NULL,NULL
	};
//k	 「全角カタカナ」を「半角カタカナ」に変換します。
//K	 「半角カタカナ」を「全角カタカナ」に変換します。
	if ( option.find("k") != -1 )
	{
		ret = XLStringUtil::replace(ret ,replaceTableHankanaToKatakana,true );
	}
	else if ( option.find("K") != -1)
	{
		ret = XLStringUtil::replace(ret ,replaceTableHankanaToKatakana,false );
	}

//c	 「全角カタカナ」を「全角ひらがな」に変換します。
//C	 「全角ひらがな」を「全角カタカナ」に変換します。
	if ( option.find("c") != -1 )
	{
		ret = XLStringUtil::replace(ret ,replaceTableKatakanaToHiragana,false );
	}
	else if ( option.find("C") != -1)
	{
		ret = XLStringUtil::replace(ret ,replaceTableKatakanaToHiragana,true );
	}

//h	 「全角ひらがな」を「半角カタカナ」に変換します。
//H	 「半角カタカナ」を「全角ひらがな」に変換します。
	if ( option.find("h") != -1 )
	{
		ret = XLStringUtil::replace(ret ,replaceTableHankanaToHiragana,true );
	}
	else if ( option.find("H") != -1)
	{
		ret = XLStringUtil::replace(ret ,replaceTableHankanaToHiragana,false );
	}

	return ret;
}


SEXYTEST()
{
	{
//n	 「全角」数字を「半角」に変換します。
//N	 「半角」数字を「全角」に変換します。
//a	 「全角」英数字を「半角」に変換します。
//A	 「半角」英数字を「全角」に変換します 
//s	 「全角」スペースを「半角」に変換します
//S	 「半角」スペースを「全角」に変換します
//k	 「全角カタカナ」を「半角カタカナ」に変換します。
//K	 「半角カタカナ」を「全角カタカナ」に変換します。
//h	 「全角ひらがな」を「半角カタカナ」に変換します。
//H	 「半角カタカナ」を「全角ひらがな」に変換します。
//c	 「全角カタカナ」を「全角ひらがな」に変換します。
//C	 「全角ひらがな」を「全角カタカナ」に変換します。
		std::string a = XLStringUtil::mb_convert_kana("あいうえおアイウエオｱｲｳｴｵ　123１２３ ABCＡＢＣ","h"); //「全角ひらがな」を「半角カタカナ」に変換します。
		std::string b = "ｱｲｳｴｵアイウエオｱｲｳｴｵ　123１２３ ABCＡＢＣ";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::string a = XLStringUtil::mb_convert_kana("あいうえおアイウエオｱｲｳｴｵ　123１２３ ABCＡＢＣ","H"); //「半角カタカナ」を「全角ひらがな」に変換します。
		std::string b = "あいうえおアイウエオあいうえお　123１２３ ABCＡＢＣ";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::string a = XLStringUtil::mb_convert_kana("あいうえおアイウエオｱｲｳｴｵ　123１２３ ABCＡＢＣ","c"); //「全角カタカナ」を「全角ひらがな」に変換します。
		std::string b = "あいうえおあいうえおｱｲｳｴｵ　123１２３ ABCＡＢＣ";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::string a = XLStringUtil::mb_convert_kana("あいうえおアイウエオｱｲｳｴｵ　123１２３ ABCＡＢＣ","C"); //「全角ひらがな」を「全角カタカナ」に変換します。
		std::string b = "アイウエオアイウエオｱｲｳｴｵ　123１２３ ABCＡＢＣ";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::string a = XLStringUtil::mb_convert_kana("あいうえおアイウエオｱｲｳｴｵ　123１２３ ABCＡＢＣ","k"); //「全角カタカナ」を「半角カタカナ」に変換します。
		std::string b = "あいうえおｱｲｳｴｵｱｲｳｴｵ　123１２３ ABCＡＢＣ";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::string a = XLStringUtil::mb_convert_kana("あいうえおアイウエオｱｲｳｴｵ　123１２３ ABCＡＢＣ","K"); //「半角カタカナ」を「全角カタカナ」に変換します。
		std::string b = "あいうえおアイウエオアイウエオ　123１２３ ABCＡＢＣ";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::string a = XLStringUtil::mb_convert_kana("あいうえおアイウエオｱｲｳｴｵ　123１２３ ABCＡＢＣ","n"); //n	 「全角」数字を「半角」に変換します。
		std::string b = "あいうえおアイウエオｱｲｳｴｵ　123123 ABCＡＢＣ";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::string a = XLStringUtil::mb_convert_kana("あいうえおアイウエオｱｲｳｴｵ　123１２３ ABCＡＢＣ","N"); //N	 「半角」数字を「全角」に変換します。
		std::string b = "あいうえおアイウエオｱｲｳｴｵ　１２３１２３ ABCＡＢＣ";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::string a = XLStringUtil::mb_convert_kana("あいうえおアイウエオｱｲｳｴｵ　123１２３ ABCＡＢＣ","a"); //a	 「全角」英数字を「半角」に変換します。
		std::string b = "あいうえおアイウエオｱｲｳｴｵ　123123 ABCABC";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::string a = XLStringUtil::mb_convert_kana("あいうえおアイウエオｱｲｳｴｵ　123１２３ ABCＡＢＣ","A"); //A	 「半角」英数字を「全角」に変換します。
		std::string b = "あいうえおアイウエオｱｲｳｴｵ　１２３１２３ ＡＢＣＡＢＣ";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::string a = XLStringUtil::mb_convert_kana("あいうえおアイウエオｱｲｳｴｵ　123１２３ ABCＡＢＣ","s"); //s	 「全角」スペースを「半角」に変換します
		std::string b = "あいうえおアイウエオｱｲｳｴｵ 123１２３ ABCＡＢＣ";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::string a = XLStringUtil::mb_convert_kana("あいうえおアイウエオｱｲｳｴｵ　123１２３ ABCＡＢＣ","S"); //S	 「半角」スペースを「全角」に変換します
		std::string b = "あいうえおアイウエオｱｲｳｴｵ　123１２３　ABCＡＢＣ";
		SEXYTEST_EQ(a ,b); 
	}
}



//みんな大好きPHPのescapeshellarg
std::string XLStringUtil::escapeshellarg(const std::string &inStr)
{
	return "\"" + replace(inStr , "\"" , "\\\"") + "\"";
}
//みんな大好きPHPのescapeshellarg
std::string XLStringUtil::escapeshellarg_single(const std::string &inStr)
{
	return "'" + replace(inStr , "'" , "\\'") + "'";
}

//数字の桁数を求める
int XLStringUtil::getScaler(unsigned int num)
{
	if (num < 10) return 1;
	else if (num < 100) return 2;
	else if (num < 1000) return 3;
	else if (num < 10000) return 4;
	else if (num < 100000) return 5;
	else if (num < 1000000) return 6;
	else if (num < 10000000) return 7;
	else if (num < 100000000) return 8;
	else if (num < 1000000000) return 9;
	else return 10;
}

bool XLStringUtil::findFilter(const std::string& base,const std::string& filter)
{
	if (filter.empty()) return true;
	const char * base_c = base.c_str();
	const char * filter_c = filter.c_str();
	while(*filter_c)
	{
		if (*filter_c == '*')
		{
			while( *filter_c )
			{
				if (*filter_c != '*' && *filter_c != '?') break;
				filter_c++; 
			}
			while( *base_c )
			{
				if ( *base_c == *filter_c ) break;
				base_c++;
			}
		}
		else if (*filter_c == '?')
		{
			base_c++;
			filter_c++;
		}
		else 
		{
			if (*base_c != *filter_c)
			{
				return false;
			}
			base_c++;
			filter_c++;
		}

		if (! *base_c)
		{
			break;
		}
	}
	if (*filter_c == '\0' && *base_c == '\0') return true;
	return false;
}


time_t XLStringUtil::strtotime(const std::string& time)
{
	struct tm tt = {0};
	sscanf(time.c_str() , "%d/%d/%d %d:%d:%d"
		,&tt.tm_year,&tt.tm_mon,&tt.tm_mday,&tt.tm_hour,&tt.tm_min,&tt.tm_sec);
	tt.tm_year -= 1900;
	tt.tm_mon  -= 1;
	return mktime( &tt );
}

SEXYTEST()
{
	{
		time_t now = time(NULL);
		std::string nowString = XLStringUtil::timetostr(now);
		time_t rev = XLStringUtil::strtotime(nowString);

		SEXYTEST_EQ(now , rev);
	}
}


std::string XLStringUtil::timetostr(const time_t& time)
{
#ifdef _MSC_VER
	struct tm *date = localtime(&time);
#else
	struct tm dateT;
	localtime_r(&time,&dateT);

	struct tm *date = &dateT;
#endif
	return num2str(date->tm_year + 1900) + "/" + num2str(date->tm_mon + 1) + "/" + num2str(date->tm_mday)
		+ " " + num2str(date->tm_hour) + ":" + num2str(date->tm_min) + ":" + num2str(date->tm_sec);
}


std::string XLStringUtil::timetostr(const time_t& time,const std::string & format)
{
	char buffer[256];

#ifdef _MSC_VER
	struct tm *date = localtime(&time);
	strftime(buffer, 255, format.c_str() , date);
#else
	struct tm date;
	localtime_r(&time,&date);
	strftime(buffer, 255, format.c_str() , &date);
#endif

	return buffer;
}

std::string XLStringUtil::md5(const std::string & str)
{
	//http://www.geocities.co.jp/SiliconValley-Oakland/8878/lab17/lab17.html の改変.

	//---------------------------------------------------------------------------
	// Stirng Table
	//---------------------------------------------------------------------------
	const unsigned int T[] = {
		0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, //0
		0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501, //4
		0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, //8
		0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821, //12
		0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, //16
		0xd62f105d,  0x2441453, 0xd8a1e681, 0xe7d3fbc8, //20
		0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, //24
		0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a, //28
		0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, //32
		0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70, //36
		0x289b7ec6, 0xeaa127fa, 0xd4ef3085,  0x4881d05, //40
		0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665, //44
		0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, //48
		0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1, //52
		0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, //56
		0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391  //60
	};


	class md5_inner
	{
	public:
		//---------------------------------------------------------------------------
		// Public Varient
		//---------------------------------------------------------------------------
		unsigned int *pX;

		const unsigned int *T;

		//---------------------------------------------------------------------------
		// Support Functions
		//---------------------------------------------------------------------------
		// ROTATE_LEFTは x を左にnビット回転させる。これはRFCからそのまま流用
		unsigned int ROTATE_LEFT(unsigned int x, unsigned int n)
		{
			return (((x) << (n)) | ((x) >> (32-(n))));
		}
		//---------------------------------------------------------------------------
		unsigned int F(unsigned int X, unsigned int Y, unsigned int Z)
		{
		   return (X & Y) | (~X & Z);
		}
		unsigned int G(unsigned int X, unsigned int Y, unsigned int Z)
		{
		   return (X & Z) | (Y & ~Z);
		}
		unsigned int H(unsigned int X, unsigned int Y, unsigned int Z)
		{
		   return X ^ Y ^ Z;
		}
		unsigned int I(unsigned int X, unsigned int Y, unsigned int Z)
		{
		   return Y ^ (X | ~Z);
		}
		//---------------------------------------------------------------------------
		unsigned int Round(unsigned int a, unsigned int b, unsigned int FGHI,
							 unsigned int k, unsigned int s, unsigned int i)
		{
		   return b + ROTATE_LEFT(a + FGHI + pX[k] + T[i], s);
		}
		//---------------------
		void Round1(unsigned int &a, unsigned int b, unsigned int c, unsigned int d,
						  unsigned int k, unsigned int s, unsigned int i)
		{
		   a = Round(a, b, F(b,c,d), k, s, i);
		}
		void Round2(unsigned int &a, unsigned int b, unsigned int c, unsigned int d,
						  unsigned int k, unsigned int s, unsigned int i)
		{
		a = Round(a, b, G(b,c,d), k, s, i);
		}
		void Round3(unsigned int &a, unsigned int b, unsigned int c, unsigned int d,
						  unsigned int k, unsigned int s, unsigned int i)
		{
		   a = Round(a, b, H(b,c,d), k, s, i);
		}
		void Round4(unsigned int &a, unsigned int b, unsigned int c, unsigned int d,
						  unsigned int k, unsigned int s, unsigned int i)
		{
		   a = Round(a, b, I(b,c,d), k, s, i);
		}
		//---------------------------------------------------------------------------
		void MD5_Round_Calculate(const unsigned char *block,
											 unsigned int &A, unsigned int &B, unsigned int &C, unsigned int &D)
		{
		   //create X 必要なので
		   unsigned int X[16]; //512bit 64byte

		   //ラウンドの計算の為に仕方なく大域変数を。。。 for Round1...4
		   pX = X;

		   //Copy block(padding_message) i into X
		   for (int j=0,k=0; j<64; j+=4,k++)
				X[k] = ( (unsigned int )block[j] )         // 8byte*4 -> 32byte 変換
					| ( ((unsigned int )block[j+1]) << 8 ) // RFCでいうDecodeという関数
					| ( ((unsigned int )block[j+2]) << 16 )
					| ( ((unsigned int )block[j+3]) << 24 );

		   //Save A as AA, B as BB, C as CC, and D as DD (A,B,C,Dの保存)
		   unsigned int AA = A,
						   BB = B,
						   CC = C,
						   DD = D;

		   //Round 1
		   Round1(A,B,C,D,  0, 7,  0); Round1(D,A,B,C,  1, 12,  1); Round1(C,D,A,B,  2, 17,  2); Round1(B,C,D,A,  3, 22,  3);
		   Round1(A,B,C,D,  4, 7,  4); Round1(D,A,B,C,  5, 12,  5); Round1(C,D,A,B,  6, 17,  6); Round1(B,C,D,A,  7, 22,  7);
		   Round1(A,B,C,D,  8, 7,  8); Round1(D,A,B,C,  9, 12,  9); Round1(C,D,A,B, 10, 17, 10); Round1(B,C,D,A, 11, 22, 11);
		   Round1(A,B,C,D, 12, 7, 12); Round1(D,A,B,C, 13, 12, 13); Round1(C,D,A,B, 14, 17, 14); Round1(B,C,D,A, 15, 22, 15);

			//Round 2
		   Round2(A,B,C,D,  1, 5, 16); Round2(D,A,B,C,  6, 9, 17); Round2(C,D,A,B, 11, 14, 18); Round2(B,C,D,A,  0, 20, 19);
		   Round2(A,B,C,D,  5, 5, 20); Round2(D,A,B,C, 10, 9, 21); Round2(C,D,A,B, 15, 14, 22); Round2(B,C,D,A,  4, 20, 23);
		   Round2(A,B,C,D,  9, 5, 24); Round2(D,A,B,C, 14, 9, 25); Round2(C,D,A,B,  3, 14, 26); Round2(B,C,D,A,  8, 20, 27);
		   Round2(A,B,C,D, 13, 5, 28); Round2(D,A,B,C,  2, 9, 29); Round2(C,D,A,B,  7, 14, 30); Round2(B,C,D,A, 12, 20, 31);

		   //Round 3
		   Round3(A,B,C,D,  5, 4, 32); Round3(D,A,B,C,  8, 11, 33); Round3(C,D,A,B, 11, 16, 34); Round3(B,C,D,A, 14, 23, 35);
		   Round3(A,B,C,D,  1, 4, 36); Round3(D,A,B,C,  4, 11, 37); Round3(C,D,A,B,  7, 16, 38); Round3(B,C,D,A, 10, 23, 39);
		   Round3(A,B,C,D, 13, 4, 40); Round3(D,A,B,C,  0, 11, 41); Round3(C,D,A,B,  3, 16, 42); Round3(B,C,D,A,  6, 23, 43);
		   Round3(A,B,C,D,  9, 4, 44); Round3(D,A,B,C, 12, 11, 45); Round3(C,D,A,B, 15, 16, 46); Round3(B,C,D,A,  2, 23, 47);

		   //Round 4
		   Round4(A,B,C,D,  0, 6, 48); Round4(D,A,B,C,  7, 10, 49); Round4(C,D,A,B, 14, 15, 50); Round4(B,C,D,A,  5, 21, 51);
		   Round4(A,B,C,D, 12, 6, 52); Round4(D,A,B,C,  3, 10, 53); Round4(C,D,A,B, 10, 15, 54); Round4(B,C,D,A,  1, 21, 55);
		   Round4(A,B,C,D,  8, 6, 56); Round4(D,A,B,C, 15, 10, 57); Round4(C,D,A,B,  6, 15, 58); Round4(B,C,D,A, 13, 21, 59);
		   Round4(A,B,C,D,  4, 6, 60); Round4(D,A,B,C, 11, 10, 61); Round4(C,D,A,B,  2, 15, 62); Round4(B,C,D,A,  9, 21, 63);

		   // Then perform the following additions. (加算しましょう)
		   A = A + AA;
		   B = B + BB;
		   C = C + CC;
		   D = D + DD;

		   //機密情報のクリア
		   memset(pX, 0, sizeof(X));
		}
	};

	//var
	/*8bit*/
	unsigned char padding_message[64], //拡張メッセージ 512bit 64byte
		digest[16];

	/*32bit*/
	unsigned int msg_digest[4];      //メッセージダイジェスト 128bit 4byte

	unsigned int &A = msg_digest[0], //RFCに則ったメッセージダイジェスト（のリファレンス）
	&B = msg_digest[1],
	&C = msg_digest[2],
	&D = msg_digest[3];

	//prog
	//Step 3. Initialize MD Buffer (A,B,C,Dの初期化;ステップ３ですが仕方なく先頭に)
	A = 0x67452301;
	B = 0xefcdab89;
	C = 0x98badcfe;
	D = 0x10325476;

	//Step 1. Append Padding Bits (符号ビットの拡張)
	//1-1
	const unsigned int string_byte_len = str.size();    //文字列のバイト長を取得
	const unsigned char* pstring = (const unsigned char*)str.c_str(); //現在の文字列の位置をセット

	md5_inner md5_inn;
	md5_inn.T = T;

	//1-2 長さが６４バイト未満になるまで計算を繰り返す
	for (int i=string_byte_len; 64<=i; i-=64,pstring+=64)
	md5_inn.MD5_Round_Calculate(pstring, A,B,C,D);

	//1-3
	const unsigned int copy_len = string_byte_len % 64;                 //残ったバイト数を算出
	strncpy((char *)padding_message, (char *)pstring, copy_len);  //拡張ビット列へメッセージをコピー
	memset(padding_message+copy_len, 0, 64 - copy_len);           //拡張ビット長になるまで0で埋める
	padding_message[copy_len] |= 0x80;                            //メッセージの次は1

	//1-4 
	//残りが56バイト以上（６４バイト未満）ならば６４バイトに拡張して計算
	if (56 <= copy_len) {
		md5_inn.MD5_Round_Calculate(padding_message, A,B,C,D);
		memset(padding_message, 0, 56); //新たに５６バイトを０で埋める
	}
	
	//Step 2. Append Length (長さの情報を追加)
	unsigned int string_bit_len = string_byte_len * 8;             //バイト長からビット長へ（下位３２バイト）
	memcpy(&padding_message[56], &string_bit_len, 4); //下位３２バイトをセット

	//下位３２バイトだけではビット長を表現できないときは上位に桁上げ
	if (UINT_MAX / 8 < string_byte_len) {
		unsigned int high = (string_byte_len - UINT_MAX / 8) * 8;
		memcpy(&padding_message[60], &high, 4);
	} else {
		memset(&padding_message[60], 0, 4); //この場合は上位には０でよい
	}
	//Step 4. Process Message in 16-Word Blocks (MD5の計算)
	md5_inn.MD5_Round_Calculate(padding_message, A,B,C,D);

	//Step 5. Output (出力)
	memcpy(digest, msg_digest, 16); //8byte*4 <- 32byte 変換 RFCでいうEncodeという関数
	char output[33];
	sprintf(output,"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
		digest[ 0], digest[ 1], digest[ 2], digest[ 3],
		digest[ 4], digest[ 5], digest[ 6], digest[ 7],
		digest[ 8], digest[ 9], digest[10], digest[11],
		digest[12], digest[13], digest[14], digest[15]);

	return output;
}

std::string XLStringUtil::sha1(const std::string & str)
{
	//http://www.ietf.org/rfc/rfc3174.txt より改変.

	/*
	 *  Define the SHA1 circular left shift macro
	 */
	#define SHA1CircularShift(bits,word) \
					(((word) << (bits)) | ((word) >> (32-(bits))))
	class sha1_inner
	{
	public:
		enum
		{
			shaSuccess = 0,
			shaNull,            /* Null pointer parameter */
			shaInputTooLong,    /* input data too long */
			shaStateError       /* called Input after Result */
		};
		enum 
		{
			SHA1HashSize = 20
		};

		/*
		 *  This structure will hold context information for the SHA-1
		 *  hashing operation
		 */
		uint32_t Intermediate_Hash[SHA1HashSize/4]; /* Message Digest  */

		uint32_t Length_Low;            /* Message length in bits      */
		uint32_t Length_High;           /* Message length in bits      */

									/* Index into message block array   */
		int_least16_t Message_Block_Index;
		uint8_t Message_Block[64];      /* 512-bit message blocks      */

		int Computed;               /* Is the digest computed?         */
		int Corrupted;             /* Is the message digest corrupted? */

		int SHA1Reset()
		{
			this->Length_Low             = 0;
			this->Length_High            = 0;
			this->Message_Block_Index    = 0;

			this->Intermediate_Hash[0]   = 0x67452301;
			this->Intermediate_Hash[1]   = 0xEFCDAB89;
			this->Intermediate_Hash[2]   = 0x98BADCFE;
			this->Intermediate_Hash[3]   = 0x10325476;
			this->Intermediate_Hash[4]   = 0xC3D2E1F0;

			this->Computed   = 0;
			this->Corrupted  = 0;
	
			return shaSuccess;
		}

		int SHA1Result( uint8_t Message_Digest[SHA1HashSize])
		{
			int i;

//			if (!Message_Digest)
//			{
//				return shaNull;
//			}

			if (this->Corrupted)
			{
				return this->Corrupted;
			}

			if (!this->Computed)
			{
				SHA1PadMessage();
				for(i=0; i<64; ++i)
				{
					/* message may be sensitive, clear it out */
					this->Message_Block[i] = 0;
				}
				this->Length_Low = 0;    /* and clear length */
				this->Length_High = 0;
				this->Computed = 1;
			}

			for(i = 0; i < SHA1HashSize; ++i)
			{
				Message_Digest[i] = this->Intermediate_Hash[i>>2]
									>> 8 * ( 3 - ( i & 0x03 ) );
			}

			return shaSuccess;
		}

		int SHA1Input(	  const uint8_t  *message_array,
						  unsigned       length)
		{
			if (!length)
			{
				return shaSuccess;
			}

//			if ( !message_array)
//			{
//				return shaNull;
//			}

			if (this->Computed)
			{
				this->Corrupted = shaStateError;

				return shaStateError;
			}

			if (this->Corrupted)
			{
				 return this->Corrupted;
			}
			while(length-- && !this->Corrupted)
			{
				this->Message_Block[this->Message_Block_Index++] =
								(*message_array & 0xFF);

				this->Length_Low += 8;
				if (this->Length_Low == 0)
				{
					this->Length_High++;
					if (this->Length_High == 0)
					{
						/* Message is too long */
						this->Corrupted = 1;
					}
				}

				if (this->Message_Block_Index == 64)
				{
					SHA1ProcessMessageBlock();
				}

				message_array++;
			}

			return shaSuccess;
		}

		void SHA1ProcessMessageBlock()
		{
			const uint32_t K[] =    {       /* Constants defined in SHA-1   */
									0x5A827999,
									0x6ED9EBA1,
									0x8F1BBCDC,
									0xCA62C1D6
									};
			int           t;                 /* Loop counter                */
			uint32_t      temp;              /* Temporary word value        */
			uint32_t      W[80];             /* Word sequence               */
			uint32_t      A, B, C, D, E;     /* Word buffers                */

			/*
			 *  Initialize the first 16 words in the array W
			 */
			for(t = 0; t < 16; t++)
			{
				W[t] = this->Message_Block[t * 4] << 24;
				W[t] |= this->Message_Block[t * 4 + 1] << 16;
				W[t] |= this->Message_Block[t * 4 + 2] << 8;
				W[t] |= this->Message_Block[t * 4 + 3];
			}

			for(t = 16; t < 80; t++)
			{
			   W[t] = SHA1CircularShift(1,W[t-3] ^ W[t-8] ^ W[t-14] ^ W[t-16]);
			}

			A = this->Intermediate_Hash[0];
			B = this->Intermediate_Hash[1];
			C = this->Intermediate_Hash[2];
			D = this->Intermediate_Hash[3];
			E = this->Intermediate_Hash[4];

			for(t = 0; t < 20; t++)
			{
				temp =  SHA1CircularShift(5,A) +
						((B & C) | ((~B) & D)) + E + W[t] + K[0];
				E = D;
				D = C;
				C = SHA1CircularShift(30,B);

				B = A;
				A = temp;
			}

			for(t = 20; t < 40; t++)
			{
				temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[1];
				E = D;
				D = C;
				C = SHA1CircularShift(30,B);
				B = A;
				A = temp;
			}

			for(t = 40; t < 60; t++)
			{
				temp = SHA1CircularShift(5,A) +
					   ((B & C) | (B & D) | (C & D)) + E + W[t] + K[2];
				E = D;
				D = C;
				C = SHA1CircularShift(30,B);
				B = A;
				A = temp;
			}

			for(t = 60; t < 80; t++)
			{
				temp = SHA1CircularShift(5,A) + (B ^ C ^ D) + E + W[t] + K[3];
				E = D;
				D = C;
				C = SHA1CircularShift(30,B);
				B = A;
				A = temp;
			}

			this->Intermediate_Hash[0] += A;
			this->Intermediate_Hash[1] += B;
			this->Intermediate_Hash[2] += C;
			this->Intermediate_Hash[3] += D;
			this->Intermediate_Hash[4] += E;

			this->Message_Block_Index = 0;
		}

		void SHA1PadMessage()
		{
			/*
			 *  Check to see if the current message block is too small to hold
			 *  the initial padding bits and length.  If so, we will pad the
			 *  block, process it, and then continue padding into a second
			 *  block.
			 */
			if (this->Message_Block_Index > 55)
			{
				this->Message_Block[this->Message_Block_Index++] = 0x80;
				while(this->Message_Block_Index < 64)
				{
					this->Message_Block[this->Message_Block_Index++] = 0;
				}

				SHA1ProcessMessageBlock();

				while(this->Message_Block_Index < 56)
				{
					this->Message_Block[this->Message_Block_Index++] = 0;
				}
			}
			else
			{
				this->Message_Block[this->Message_Block_Index++] = 0x80;
				while(this->Message_Block_Index < 56)
				{
					this->Message_Block[this->Message_Block_Index++] = 0;
				}
			}

			/*
			 *  Store the message length as the last 8 octets
			 */
			this->Message_Block[56] = this->Length_High >> 24;
			this->Message_Block[57] = this->Length_High >> 16;
			this->Message_Block[58] = this->Length_High >> 8;
			this->Message_Block[59] = this->Length_High;
			this->Message_Block[60] = this->Length_Low >> 24;
			this->Message_Block[61] = this->Length_Low >> 16;
			this->Message_Block[62] = this->Length_Low >> 8;
			this->Message_Block[63] = this->Length_Low;

			SHA1ProcessMessageBlock();
		}
	};
    uint8_t digest[20];

	sha1_inner sha1_inn;
	sha1_inn.SHA1Reset();
	sha1_inn.SHA1Input((const unsigned char *) str.c_str(), str.size() );
    sha1_inn.SHA1Result(digest);

	char output[41];
	sprintf(output,"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
		digest[ 0], digest[ 1], digest[ 2], digest[ 3],
		digest[ 4], digest[ 5], digest[ 6], digest[ 7],
		digest[ 8], digest[ 9], digest[10], digest[11],
		digest[12], digest[13], digest[14], digest[15],
		digest[16], digest[17], digest[18], digest[19]);
	return output;
}

/*
std::string XLStringUtil::uuid()
{
#ifdef _MSC_VER
	unsigned char *p;
	UUID uuid;

	::UuidCreate(&uuid );
	::UuidToString(&uuid, &p );

	std::string ret = (char*)p;
	::RpcStringFree(&p );

	return ret;
#else
	uuid_t u;
	char buf[37];

	uuid_generate(u);
	uuid_unparse(u, buf);
	return buf;
#endif
}
*/
	
std::string XLStringUtil::randsting(unsigned int size)
{
	std::string ret;
	static const char *randomchar = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-";

	for(unsigned int i = 0 ; i < size; i++ )
	{
		unsigned int c = rand() % 64;
		ret += randomchar[c];
	}
	return ret;
}

//16進数dumpされた長い文字列を再びバイナリにする
bool XLStringUtil::X02HexsStringToBinary(const std::string & str,std::vector<char>* retVec)
{
	retVec->clear();

	const char *p= str.c_str();
	while(*p)
	{
		unsigned char c1 = *p;
		unsigned char c2 = *p+1;
		if ( ! ((c1 >= '0' && c1 <= '9') || (c1 >= 'a' && c1 <= 'f')) )
		{
			return false;
		}
		if ( ! ((c2 >= '0' && c2 <= '9') || (c2 >= 'a' && c2 <= 'f')) )
		{
			return false;
		}
		retVec->push_back ((c1 << 4) | c2);
		p+=2;
	}
	return true;
}

//ファイル名にイカれた文字が入っていないか確認する.
bool XLStringUtil::checkSafePath(const std::string& filename) 
{
	if (filename.empty())
	{
		return true;
	}

	const char * p = &filename[0];
	for(; *p ; )
	{
		if (isMultiByte(p))
		{
			p = nextChar(p);
		}
		else
		{
			if ( *p == '\n' ||  *p == '\r' || *p == '"' || *p == '\'' || *p == '/' || *p == '\\' || *p == '\t' || *p == ':' || *p == '*' || *p == '?' || *p == '|' || *p == '>' || *p == '<' ) return false;
			p ++;
		}
	}
	return true;
}

//改行が入っていないこと
bool XLStringUtil::IsNotReturnCode(const std::string& name) 
{
	if (name.empty())
	{
		return true;
	}

	const char * p = &name[0];
	for(; *p ; )
	{
		if (isMultiByte(p))
		{
			p = nextChar(p);
		}
		else
		{
			if ( *p == '\n' ||  *p == '\r' ) return false;
			p ++;
		}
	}
	return true;
}

//改行と=が入っていないこと
bool XLStringUtil::IsNotReturnAndEqualCode(const std::string& name) 
{
	if (name.empty())
	{
		return true;
	}

	const char * p = &name[0];
	for(; *p ; )
	{
		if (isMultiByte(p))
		{
			p = nextChar(p);
		}
		else
		{
			if ( *p == '\n' ||  *p == '\r'||  *p == '=' ) return false;
			p ++;
		}
	}
	return true;
}

//Http時間を Unix時間に変換.
time_t XLStringUtil::HttpTimeToUnixTime(const std::string& date)
{
	struct date_inner
	{
		static int parseMonth(const char* p)
		{
			//月のパース
			if (*p == 'J' || *p == 'j')
			{
				if ( (*(p+1) == 'a' || *(p+1) == 'A') && (*(p+2) == 'n' || *(p+2) == 'N') )
				{//Jan
					return 0;
				}
				else if ( (*(p+1) == 'u' || *(p+1) == 'U') && (*(p+2) == 'n' || *(p+2) == 'N') )
				{//Jun
					return 5;
				}
				else if ( (*(p+1) == 'u' || *(p+1) == 'U') && (*(p+2) == 'l' || *(p+2) == 'L') )
				{//Jul
					return 6;
				}
				else return -1;
			}
			else if (*p == 'M' || *p == 'm')
			{
				if ( (*(p+1) == 'a' || *(p+1) == 'A') && (*(p+2) == 'r' || *(p+2) == 'R') )
				{//Mar
					return 2;
				}
				else if ( (*(p+1) == 'a' || *(p+1) == 'A') && (*(p+2) == 'y' || *(p+2) == 'Y') )
				{//May
					return 4;
				}
				else return -1;
			}
			else if (*p == 'A' || *p == 'a')
			{
				if ( (*(p+1) == 'p' || *(p+1) == 'P') && (*(p+2) == 'r' || *(p+2) == 'R') )
				{//Apr
					return 3;
				}
				else if ( (*(p+1) == 'u' || *(p+1) == 'U') && (*(p+2) == 'g' || *(p+2) == 'G') )
				{//Aug
					return 7;
				}
				else return -1;
			}
			else if ( (*p == 'D' || *p == 'd') && (*(p+1) == 'e' || *(p+1) == 'E') && (*(p+2) == 'c' || *(p+2) == 'C') )
			{//Dec
					return 11;
			}
			else if ( (*p == 'F' || *p == 'f') && (*(p+1) == 'e' || *(p+1) == 'E') && (*(p+2) == 'b' || *(p+2) == 'B') )
			{//Feb
					return 1;
			}
			else if ( (*p == 'S' || *p == 's') && (*(p+1) == 'e' || *(p+1) == 'E') && (*(p+2) == 'p' || *(p+2) == 'P') )
			{//Sep
					return 8;
			}
			else if ( (*p == 'O' || *p == 'o') && (*(p+1) == 'c' || *(p+1) == 'C') && (*(p+2) == 't' || *(p+2) == 'T') )
			{//Oct
					return 9;
			}
			else if ( (*p == 'N' || *p == 'n') && (*(p+1) == 'o' || *(p+1) == 'O') && (*(p+2) == 'v' || *(p+2) == 'V') )
			{//Nov
					return 10;
			}
			else return -1;
		}
		static int parseWeek(const char* p)
		{
			//曜日のパース
			if (*p == 'S' || *p == 's')
			{
				if ( (*(p+1) == 'a' || *(p+1) == 'A') && (*(p+2) == 't' || *(p+2) == 'T') )
				{//sat
					return 6;
				}
				else if ( (*(p+1) == 'u' || *(p+1) == 'U') && (*(p+2) == 'n' || *(p+2) == 'N') )
				{//sun
					return 0;
				}
				else return -1;
			}
			else if ( (*p == 'M' || *p == 'm') && (*(p+1) == 'o' || *(p+1) == 'O') && (*(p+2) == 'n' || *(p+2) == 'N') )
			{//mon
					return 1;
			}
			else if ( (*p == 'W' || *p == 'w') && (*(p+1) == 'e' || *(p+1) == 'E') && (*(p+2) == 'd' || *(p+2) == 'D') )
			{//wed
					return 3;
			}
			else if ( (*p == 'F' || *p == 'f') && (*(p+1) == 'r' || *(p+1) == 'R') && (*(p+2) == 'i' || *(p+2) == 'I') )
			{//fri
					return 5;
			}
			else if (*p == 'T' || *p == 't')
			{
				if ( (*(p+1) == 'u' || *(p+1) == 'U') && (*(p+2) == 'e' || *(p+2) == 'E') )
				{//tue
					return 2;
				}
				else if ( (*(p+1) == 'h' || *(p+1) == 'H') && (*(p+2) == 'u' || *(p+2) == 'U') )
				{//thu
					return 4;
				}
				else return -1;
			}
			else return -1;
		}
		static int parse2num(const char* date,const char** next )
		{
			//2桁の数字のパース
			if ( *date < '0' ||*date > '9' ) return -1;
			if ( *(date+1) < '0' ||*(date+1) > '9' )
			{
				*next = date + 1;
				return (int)(*date - '0');
			}
			*next = date + 2;
			return ((int)(*date - '0') * 10) + ((int)(*(date+1) - '0'));
		}
		static int parse4num(const char* date,const char** next )
		{
			//4桁の数字のパース
			if ( *date < '0' ||*date > '9' ) return -1;
			if ( *(date+1) < '0' ||*(date+1) > '9' )
			{
				*next = date + 1;
				return (int)(*date - '0');
			}
			if ( *(date+2) < '0' ||*(date+2) > '9' )
			{
				*next = date + 2;
				return ((int)(*date - '0') * 10) + ((int)(*(date+1) - '0'));
			}
			if ( *(date+3) < '0' ||*(date+3) > '9' )
			{
				*next = date + 3;
				return ((int)(*date - '0') * 100) + ((int)(*(date+1) - '0')*10) + ((int)(*(date+2) - '0'));
			}
			*next = date + 4;
			return ((int)(*date - '0') * 1000) + ((int)(*(date+1) - '0')*100) + ((int)(*(date+2) - '0') * 10) + ((int)(*(date+3) - '0') );
		}
		static int checkGMT(const char* p )
		{
			return ( (*p == 'G' || *p == 'g') && (*(p+1) == 'M' || *(p+1) == 'm') && (*(p+1) == 'T' || *(p+1) == 't') );
		}
	};

	struct tm tt = {0};
	//Tue, 02 Feb 2010 14:39:52 GMT
	const char * p = date.c_str();
	
	tt.tm_wday = date_inner::parseWeek(p);
	if (tt.tm_wday == -1) 
	{
		return -1;
	}
	p += 3; //Tue
	if (*p != ',')
	{//Tue, 02 Feb 2010 14:39:52 GMT   //RFC 822, updated by RFC 1123
		return -1;
	}
	p ++; //skip ','
	p ++; //skip ' '
	tt.tm_mday = date_inner::parse2num(p,&p);
	if (tt.tm_mday == -1) 
	{
		return -1;
	}
	p ++; //skip ' '
	tt.tm_mon = date_inner::parseMonth(p);
	if (tt.tm_mon == -1) 
	{
		return -1;
	}
	p += 3; //Feb
	p ++; //skip ' '
	tt.tm_year = date_inner::parse4num(p,&p);
	if (tt.tm_year == -1) 
	{
		return -1;
	}
	tt.tm_year -= 1900;
	p ++; //skip ' '
	tt.tm_hour = date_inner::parse2num(p,&p);
	if (tt.tm_hour == -1) 
	{
		return -1;
	}
	p ++; //skip ' '
	tt.tm_min = date_inner::parse2num(p,&p);
	if (tt.tm_min == -1) 
	{
		return -1;
	}
	p ++; //skip ' '
	tt.tm_sec = date_inner::parse2num(p,&p);
	if (tt.tm_sec == -1) 
	{
		return -1;
	}
	p ++; //skip ' '
	date_inner::checkGMT(p);
	
	//0x0123ab84 ("if-modified-since", "Thu, 23 Aug 2012 02:22:48 GMT")
	return GMTtoLocalTime( mktime( &tt ) );
}

std::string XLStringUtil::UnixTimeToHttpTime(const time_t& t )
{
	static const char*	day[] = {	"Sun" , "Mon" , "Tue" , "Wed" , "Thu" , "Fri" , "Sat" };
	static const char*	month[] = {	"Jan" , "Feb" , "Mar" , "Apr" , "May" , "Jun" , "Jul" , "Aug" , "Sep" , "Oct" , "Nov" , "Dec" };

	//Wed, 04 Dec 2002 15:35:06 GMT

	char buf [30 + 1];
	struct tm *today = gmtime( &t );
	snprintf(buf , 30 , "%s, %02d %s %04d %02d:%02d:%02d GMT" , day[today->tm_wday] , today->tm_mday , month[today->tm_mon] , 1900 + today->tm_year , today->tm_hour , today->tm_min , today->tm_sec);

	return buf;
}

std::string XLStringUtil::between(const std::string& str ,const std::string& start , const std::string& end )
{
	const char * p = str.c_str();
	const char * s = strstr(p , start.c_str());
	if (!s)
	{
		return "";
	}
	s += start.size();
	const char * e = strstr(s , end.c_str());
	if (!e)
	{
		return "";
	}

	return str.substr( (int)(s - p)  , (int) (e - s) );
}

std::string XLStringUtil::between_replace(const std::string& str ,const std::string& start , const std::string& end ,const std::string& newString)
{
	const char * p = str.c_str();
	const char * s = strstr(p , start.c_str());
	if (!s)
	{
		return str;
	}
	const char * e = strstr(s , end.c_str());
	if (!e)
	{
		return str;
	}

	std::string r = str.substr(0,  (int)(s - p) ) ;
	r += newString;
	r += e + end.size();

	return r;
}

std::string XLStringUtil::template_if(const std::string& str ,const std::string& usestart , const std::string& useend ,const std::string& killstart , const std::string& killend)
{
	//不要な部分を消す
	std::string r = between_replace(str,killstart,killend,"");

	r = replace_low(r,usestart,"");
	r = replace_low(r,useend,"");

	return r;
}
unsigned int XLStringUtil::mb_strlen(const std::string& str)
{
	unsigned int countMoji = 0;
	const char * p;
	for(p = str.c_str() ; *p ;)
	{
		p = nextChar(p);
		countMoji++;
	}
	return countMoji;
}

std::string XLStringUtil::mb_substr(const std::string& str,unsigned int start,unsigned int len)
{
	const char* startP = NULL;
	const char* endP = NULL;
	
	unsigned int countMoji = 0;
	const char * p;
	for(p = str.c_str() ; *p ;)
	{
		if (start == countMoji)
		{
			startP = p;
		}
		if (start+len == countMoji)
		{
			endP = p;
		}
		p = nextChar(p);
		countMoji++;
	}
	
	if (startP == NULL)
	{
		return "";
	}
	if (endP == NULL)
	{
		endP = p;
	}
	return std::string(startP , 0 , (unsigned int)(endP - startP) );
}

void XLStringUtil::StringtoVector(std::vector<char>* vec,const std::string& str)
{
	vec->assign(str.c_str() , str.c_str()+str.size());
}


bool XLStringUtil::checkMailAddress(const std::string& mail)
{
	//とりあえず @ が入っているか。 @のあとに . があるかなどを見ます。
	const char * p = strchr(mail.c_str() , '@');
	if (!p) return false;
	p = strchr(p+1,'.');
	if (!p) return false;

	return true;
}

bool XLStringUtil::checkNumlic(const std::string& str)
{
	const char * p = &str[0];
	for(; *p ; )
	{
		if (isMultiByte(p))
		{
			p = nextChar(p);
		}
		else
		{
			if ( !( *p >= '0' && *p <= '9') ) return false;
			p ++;
		}
	}
	return true;
}

bool XLStringUtil::checkSingleBytes(const std::string& str)
{
	const char * p = &str[0];
	for(; *p ; )
	{
		if (isMultiByte(p))
		{
			return false;
		}
		else
		{
			p ++;
		}
	}
	return true;
}

time_t XLStringUtil::LocalTimetoGMT(const time_t& t)
{
	static time_t diff = 24*60*60;
	if ( diff == 24*60*60 )
	{
		time_t now = time(NULL);
		struct tm *gmtTM = gmtime(&now);
		time_t gmttime = mktime(gmtTM);
		diff = now - gmttime;
	}
	return t - diff ;
}

time_t XLStringUtil::GMTtoLocalTime(const time_t& t)
{
	static time_t diff = 24*60*60;
	if ( diff == 24*60*60 )
	{
		time_t now = time(NULL);
		struct tm *gmtTM = gmtime(&now);
		time_t gmttime = mktime(gmtTM);
		diff = now - gmttime;
	}
	return t + diff ;
}

std::string XLStringUtil::mb_mime_header(const std::string& str )
{
	return "=?utf-8?B?" + XLStringUtil::base64encode(str) + "?=";
}

std::string XLStringUtil::mailaddress_to_name(const std::string& mailaddress )
{
	const char * p  = mailaddress.c_str();
	const char * at = strstr(p,"@");
	if (at == NULL)
	{
		return "";
	}
	return std::string(p , 0 , (int) (at - p) );
}
	
std::string XLStringUtil::mailaddress_to_domain(const std::string& mailaddress )
{
	const char * p  = mailaddress.c_str();
	const char * at = strstr(p,"@");
	if (at == NULL)
	{
		return "";
	}
	return at + 1;
}




//const char** 配列(終端はNULL)から文字を探す。
const char* XLStringUtil::findConstCharTable(const char** table , const char* search)
{
	for(; *table ; table++)
	{
		if ( strcmp(search, *table) == 0)
		{
			return *table;
		}
	}
	return NULL;
}

// http://hoghoge.com:123/dir/hoghgoe?aa=1 --> http://hoghoge.com:123 に変換する
std::string XLStringUtil::hosturl(const std::string& url)
{
	const char * start = url.c_str();
	const char * p = start;
	p = strstr(p,"//");
	if (!p)
	{// "http://" とかで始まらない？
		p = strchr(p,'/');
		if (!p)
		{//謎のプロトコル
			return "";
		}
		return std::string(start,0,p - start );
	}
	p = strchr(p+2,'/');
	if (!p)
	{//   http://hoghoge.com:123 というデータだったのかもしれぬ
		return url;
	}
	return std::string(start,0,p - start );
}

// http://hoghoge.com:123/dir/hoghgoe?aa=1 --> http://hoghoge.com:123/dir/ に変換する
std::string XLStringUtil::baseurl(const std::string& url)
{
	const char * start = url.c_str();
	const char * p = start;

	std::string tempurl;
	p = strchr(p,'?');
	if (!p)
	{//
		tempurl = url;
	}
	else
	{//はてなの部分までを一度取得する
		tempurl = std::string(start,0 , p - start);
	}

	start = tempurl.c_str();
	p = strrchr(start,'/');
	if (!p)
	{// / がない不思議なデータ
		return tempurl;
	}
	if (start == p)
	{// /hogehoge とかいう / から始まるデータを渡したということだよなぁ・・
		return std::string(start,0, p - start + 1);
	}

	if ( *(p-1) == '/' )
	{// //と連続している。 http:// とかかな・・？
		if ( p - start < 6)
		{//たぶん http://を誤爆している
			return tempurl;
		}
	}

	return std::string(start,0, p - start + 1);
}

// http://hoghoge.com:123/dir/hoghgoe?aa=1 --> https://hoghoge.com:123/dir/hoghgoe?aa=1 に変換する
std::string XLStringUtil::changeprotocol(const std::string& url,const std::string newProtocol)
{
	const char * start = url.c_str();
	const char * p = start;

	p = strstr(p,"//");
	if (!p)
	{//プロトコルの区切りがない!
		return "";
	}

	return newProtocol + std::string(p);
}

// http://hoghoge.com:123/dir/hoghgoe?aa=1 --> http://hoghoge.com:123/dir/hoghgoe?aa=1&bb=2 に変換する
std::string XLStringUtil::appendparam(const std::string& url,const std::string param)
{
	const char * start = url.c_str();
	const char * p = start;

	p = strchr(p,'?');
	if (!p)
	{//
		return url + "?" + param;
	}
	return url + "&" + param;
}

//std::mapどうしの比較
bool XLStringUtil::CompareStringMap(const std::map<std::string,std::string>& a ,const std::map<std::string,std::string>& b)
{
	//サイズが違うならば更新されている.
	if (a.size() != b.size() ) return true;

	for(auto ita = a.begin() ; ita != a.end() ; ++ita)
	{
		//aのキーがbになければ更新されている.
		auto itb = b.find(ita->first);
		if ( itb == b.end() ) return true;

		//aのキーの中身がbと違うならば更新されている.
		if ( ita->second != itb->second ) return true;
	}
	//更新されていない
	return false;
}

//祝日判定
bool XLStringUtil::is_japan_holiday(const time_t& now,std::string *outName)
{
	/*
	_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	_/
	_/　CopyRight(C) K.Tsunoda(AddinBox) 2001 All Rights Reserved.
	_/　( http://www.h3.dion.ne.jp/~sakatsu/index.htm )
	_/
	_/　　この祝日マクロは『kt関数アドイン』で使用しているものです。
	_/　　このロジックは、レスポンスを第一義として、可能な限り少ない
	_/　  【条件判定の実行】で結果を出せるように設計してあります。
	_/　　この関数では、２００７年施行の改正祝日法(昭和の日)までを
	_/　  サポートしています(９月の国民の休日を含む)。
	_/
	_/　(*1)このマクロを引用するに当たっては、必ずこのコメントも
	_/　　　一緒に引用する事とします。
	_/　(*2)他サイト上で本マクロを直接引用する事は、ご遠慮願います。
	_/　　　【 http://www.h3.dion.ne.jp/~sakatsu/holiday_logic.htm 】
	_/　　　へのリンクによる紹介で対応して下さい。
	_/　(*3)[ktHolidayName]という関数名そのものは、各自の環境に
	_/　　　おける命名規則に沿って変更しても構いません。
	_/　
	_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
	*/
	struct _inner
	{
		static  int Holiday(const time_t& t,std::string* outName){
		  int yy;
		  int mm;
		  int dd;
		  int ww;
		  int r;
		  std::string name;
		  const time_t SYUKUJITSU=-676976400; /*1948年7月20日*/
		  const time_t FURIKAE=103388400; /*1973年04月12日*/
		  #ifdef _MSC_VER
			struct tm *Hizuke = localtime(&t);
		  #else
			struct tm datetmp;
			struct tm *Hizuke = &datetmp;
			localtime_r(&t,&datetmp);
		  #endif
		  yy=Hizuke->tm_year+1900;
		  mm=Hizuke->tm_mon+1;
		  dd=Hizuke->tm_mday;
		  ww=Hizuke->tm_wday;

		  r=0;
		  if (ww==6){
			r=1;
		  } else if (ww==0){
			r=2;
		  }

		  if (t<SYUKUJITSU){
			return r;
		  }

		  switch (mm) {
		  case 1:
			if (dd==1){
			  r=5; /*元日*/
			  name = "元旦";
			} else {
			  if (yy>=2000){
				if (((int)((dd-1)/7)==1)&&(ww==1)){
				  r=5; /*成人の日*/
				  name = "成人の日";
				}
			  } else {
				if (dd==15){
				  r=5; /*成人の日*/
				  name = "成人の日";
				}
			  }
			}
			break;
		  case 2:
			if (dd==11){
			  if (yy>=1967){
				r=5; /*建国記念の日*/
				name = "建国記念の日";
			  }
			} else if ((yy==1989)&&(dd==24)){
			  r=5; /*昭和天皇の大喪の礼*/
			  name = "昭和天皇の大喪の礼";
			}
			break;
		  case 3:
			if (dd==Syunbun(yy)){
			  r=5; /*春分の日*/
			  name = "春分の日";
			}
			break;
		  case 4:
			if (dd==29){
			  if (yy>=2007){
				r=5; /*昭和の日*/
				name = "昭和の日";
			  } else if (yy>=1989){
				r=5; /*みどりの日*/
				name = "みどりの日";
			  } else {
				r=5; /*天皇誕生日*/
				name = "天皇誕生日";
			  }
			} else if ((yy==1959)&&(dd==10)){
			  r=5; /*皇太子明仁親王の結婚の儀*/
			  name = "皇太子明仁親王の結婚の儀";
			}
			break;
		  case 5:
			if (dd==3){
			  r=5; /*憲法記念日*/
			  name = "憲法記念日";
			} else if (dd==4){
			  if (yy>=2007) {
				r=5; /*みどりの日*/
				name = "みどりの日";
			  } else if (yy>=1986) {
				  /* 5/4が日曜日は『只の日曜』､月曜日は『憲法記念日の振替休日』(～2006年)*/
				  if (ww>1) {
					r=3; /*国民の休日*/
					name = "国民の休日";
				  }
			  }
			} else if (dd==5) {
			  r=5; /*こどもの日*/
			  name = "こどもの日";
			} else if (dd==6) {
			  /* [5/3,5/4が日曜]ケースのみ、ここで判定 */
			  if ((yy>=2007)&&((ww==2)||(ww==3))){
				r=4; /*振替休日*/
			    name = "振替休日";
			  }
			}
			break;
		  case 6:
			if ((yy==1993)&&(dd==9)){
			  r=5; /*皇太子徳仁親王の結婚の儀*/
			  name = "皇太子徳仁親王の結婚の儀";
			}
			break;
		  case 7:
			if (yy>=2003){
			  if (((int)((dd-1)/7)==2)&&(ww==1)){
				r=5; /*海の日*/
				name = "海の日";
			  }
			} else if (yy>=1996){
			  if (dd==20) {
				r=5; /*海の日*/
				name = "海の日";
			  }
			}
			break;
		  case 8:
			break;
		  case 9:
			if (dd==Syubun(yy)){
			  r=5; /*秋分の日*/
			  name = "秋分の日";
			} else {
			  if (yy>=2003) {
				if (((int)((dd-1)/7)==2)&&(ww==1)){
				  r=5; /*敬老の日*/
				  name = "敬老の日";
				} else if (ww==2){
				  if (dd==Syubun(yy)-1){
					r=3; /*国民の休日*/
					name = "国民の休日";
				  }
				}
			  } else if (yy>=1966){
				if (dd==15) {
				  r=5; /*敬老の日*/
				  name = "敬老の日";
				}
			  }
			}
			break;
		  case 10:
			if (yy>=2000){
			  if (((int)((dd-1)/7)==1)&&(ww==1)){
				r=5; /*体育の日*/
				name = "体育の日";
			  }
			} else if (yy>=1966){
			  if (dd==10){
				r=5; /*体育の日*/
				name = "体育の日";
			  }
			}
			break;
		  case 11:
			if (dd==3){
			  r=5; /*文化の日*/
			  name = "文化の日";
			} else if (dd==23) {
			  r=5; /*勤労感謝の日*/
			  name = "労感謝の日";
			} else if ((yy==1990)&&(dd==12)){
			  r=5; /*即位礼正殿の儀*/
			  name = "即位礼正殿の儀";
			}
			break;
		  case 12:
			if (dd==23){
			  if (yy>=1989){
				r=5; /*天皇誕生日*/
				name = "天皇誕生日";
			  }
			}
		  }

		  if ((r<=3)&&(ww==1)){
			 /*月曜以外は振替休日判定不要
				5/6(火,水)の判定は上記ステップで処理済
				5/6(月)はここで判定する  */
			if (t>=FURIKAE) {
			  if (Holiday(t-86400,outName)==5){    /* 再帰呼出 */
				r=4;
			  }
			}
		  }

		  if (outName != NULL)
		  { 
			  if ( r == 5 )  *outName = name;
			  else           outName->clear();
		  }
		  return r;
		}

		/*  春分/秋分日の略算式は
			『海上保安庁水路部 暦計算研究会編 新こよみ便利帳』
		  で紹介されている式です。 */

		/*春分の日を返す関数*/
		static int Syunbun(int yy){
		  int dd;
		  if (yy<=1947){
			dd=99;
		  } else if (yy<=1979){
			dd=(int)(20.8357+(0.242194*(yy-1980))-(int)((yy-1983)/4));
		  } else if (yy<=2099){
			dd=(int)(20.8431+(0.242194*(yy-1980))-(int)((yy-1980)/4));
		  } else if (yy<=2150){
			dd=(int)(21.851+(0.242194*(yy-1980))-(int)((yy-1980)/4));
		  } else {
			dd=99;
		  }
		  return dd;
		}

		/*秋分の日を返す関数*/
		static int Syubun(int yy){
		  int dd;
		  if (yy<=1947){
			dd=99;
		  } else if (yy<=1979){
			dd=(int)(23.2588+(0.242194*(yy-1980))-(int)((yy-1983)/4));
		  } else if (yy<=2099){
			dd=(int)(23.2488+(0.242194*(yy-1980))-(int)((yy-1980)/4));
		  } else if (yy<=2150){
			dd=(int)(24.2488+(0.242194*(yy-1980))-(int)((yy-1980)/4));
		  } else {
			dd=99;
		  }
		  return dd;
		}
	};

	return ( _inner::Holiday(now,outName) == 5 );
}

std::string XLStringUtil::DayofweekToJapanese(int dayofweek)
{
	switch(dayofweek)
	{
	case 0:	return "日";
	case 1:	return "月";
	case 2:	return "火";
	case 3:	return "水";
	case 4:	return "木";
	case 5:	return "金";
	case 6:	return "土";
	}
	return "";
}
std::string XLStringUtil::DayofweekToEnglish(int dayofweek)
{
	switch(dayofweek)
	{
	case 0:	return "Sun";
	case 1:	return "Mon";
	case 2:	return "Tue";
	case 3:	return "Wed";
	case 4:	return "Thu";
	case 5:	return "Fri";
	case 6:	return "Sat";
	}
	return "";
}

std::string XLStringUtil::MonthToJapanese(int month)
{
	switch(month)
	{
	case 0: 	return "1月";
	case 1: 	return "2月";
	case 2: 	return "3月";
	case 3: 	return "4月";
	case 4: 	return "5月";
	case 5: 	return "6月";
	case 6: 	return "7月";
	case 7: 	return "8月";
	case 8: 	return "9月";
	case 9: 	return "10月";
	case 10:	return "11月";
	case 11:	return "12月";
	}
	return "";
}
std::string XLStringUtil::MonthToEnglish(int month)
{
	switch(month)
	{
	case 0: 	return "Jan";
	case 1: 	return "Feb";
	case 2: 	return "Mar";
	case 3: 	return "Apr";
	case 4: 	return "May";
	case 5: 	return "Jun";
	case 6: 	return "Jul";
	case 7: 	return "Aug";
	case 8: 	return "Sep";
	case 9: 	return "Oct";
	case 10:	return "Nov";
	case 11:	return "Dec";
	}
	return "";
}
/*

//自作HTMLパーサーで今いるタグの中を取得する.
std::string XLStringUtil::InnerHTML(const std::string& html)
{
	return XLHTMLScrapper::InnerHTML(html);
}

//今いるタグの中を取得する.
std::string XLStringUtil::InnerText(const std::string& html)
{
	return XLHTMLScrapper::InnerText(html);
}

//属性を取得する
std::string XLStringUtil::HTMLAttr(const std::string& html,const std::string& attributeName)
{
	return XLHTMLScrapper::HTMLAttr(html,attributeName);
}

//すーぱーHTMLセレクター
std::vector<std::string> XLStringUtil::HTMLSelector(const std::string& html,const std::string& selector)
{
	return XLHTMLScrapper::HTMLSelector(html,selector);
}


//みんな大好き文字コード変換. babelを利用してます.
std::string XLStringUtil::mb_convert_encoding(const std::string& str , const std::string& to , const std::string& from)
{
	struct _inner {
		static int NameToBabelCode(const char* enc)
		{
			if ( STRICMP( enc ,"utf8")==0) return 7; //"UTF-8";
			else if ( STRICMP( enc ,"utf-8")==0) return 7; //"UTF-8";

			if ( STRICMP( enc ,"sjis")==0) return 3; //"SHIFT-JIS";
			else if ( STRICMP( enc ,"shift-jis")==0) return 3; //"SHIFT-JIS";
			else if ( STRICMP( enc ,"shiftjis")==0) return 3; //"SHIFT-JIS";
			if ( STRICMP( enc ,"sjiswin")==0) return 3; //"SHIFT-JIS";
			if ( STRICMP( enc ,"sjis-win")==0) return 3; //"SHIFT-JIS";

			if ( STRICMP( enc ,"euc")==0) return 5; //"EUC-JP";
			else if ( STRICMP( enc ,"eucjp")==0) return 5; //"EUC-JP";
			else if ( STRICMP( enc ,"euc-jp")==0) return 5; //"EUC-JP";

			if ( STRICMP( enc ,"jis")==0) return 4; //"JIS";

			if ( STRICMP( enc ,"iso-2022-jp")==0) return 6; //"ISO-2022-JP";
			if ( STRICMP( enc ,"iso2022jp")==0) return 6;   //"ISO-2022-JP";

			if ( STRICMP( enc ,"utf16be")==0) return 8; //"UTF-16be";
			else if ( STRICMP( enc ,"utf-16be")==0) return 8; //"UTF-16be";
			if ( STRICMP( enc ,"utf16le")==0) return 9; //"UTF-16le";
			else if ( STRICMP( enc ,"utf-16le")==0) return 9; //"UTF-16le";
			if ( STRICMP( enc ,"utf32be")==0) return 10; //"UTF-32be";
			else if ( STRICMP( enc ,"utf-32be")==0) return 10; //"UTF-32be";
			if ( STRICMP( enc ,"utf32le")==0) return 11; //"UTF-32le";
			else if ( STRICMP( enc ,"utf-32le")==0) return 11; //"UTF-32le";
			if ( STRICMP( enc ,"utf16")==0) return 12; //"UTF-16";
			else if ( STRICMP( enc ,"utf-16")==0) return 12; //"UTF-16";
			if ( STRICMP( enc ,"utf32le")==0) return 13; //"UTF-32";
			else if ( STRICMP( enc ,"utf-32le")==0) return 13; //"UTF-32";

			return 0;
		}
	};
	if (str.empty() ) return "";

	babel::init_babel();
	int f;
	int t;

	if (to.empty())
	{
#ifdef _WINDOWS
		t = 3; //"SJIS";
#else
		t = 7; //"UTF-8";
#endif
	}
	else
	{
		t = _inner::NameToBabelCode(to.c_str() );
		if (t <= 0)
		{
#ifdef _WINDOWS
			t = 3; //"SJIS";
#else
			t = 7; //"UTF-8";
#endif
		}
	}
	if (from.empty())
	{
		babel::analyze_result r(babel::analyze_base_encoding(str ));
		f = r;
	}
	else
	{
		f = _inner::NameToBabelCode(from.c_str() );
		if (f <= 0)
		{
			babel::analyze_result r(babel::analyze_base_encoding(str ));
			f = r;
		}
	}

	//from と to が同じなので何もしなくてよ
	if (f == t) return str;

	babel::manual_translate<std::string,std::string> r(str ,f,t);
	return r;
}

SEXYTEST(XLStringUtil__mb_convert_encoding)
{
	{
		std::string r;
		std::string r_utf8 = XLFileUtil::cat("./config/testdata/test_utf8.txt") ;
		std::string r_sjis = XLFileUtil::cat("./config/testdata/test_sjis.txt") ;
		std::string r_euc  = XLFileUtil::cat("./config/testdata/test_eucjp.txt") ;
		std::string r_jis  = XLFileUtil::cat("./config/testdata/test_jis.txt") ;

		r = XLStringUtil::mb_convert_encoding(r_utf8,"sjis","utf-8");
		SEXYTEST_EQ(r , r_sjis);

		r = XLStringUtil::mb_convert_encoding(r_utf8,"euc","utf-8");
		SEXYTEST_EQ(r , r_euc);

		r = XLStringUtil::mb_convert_encoding(r_utf8,"jis","utf-8");
		SEXYTEST_EQ(r , r_jis);


		r = XLStringUtil::mb_convert_encoding(r_sjis,"utf-8","sjis");
		SEXYTEST_EQ(r , r_utf8);

		r = XLStringUtil::mb_convert_encoding(r_sjis,"euc","sjis");
		SEXYTEST_EQ(r , r_euc);

		r = XLStringUtil::mb_convert_encoding(r_sjis,"jis","sjis");
		SEXYTEST_EQ(r , r_jis);

		r = XLStringUtil::mb_convert_encoding(r_sjis,"utf-8","sjis");
		SEXYTEST_EQ(r , r_utf8);

		r = XLStringUtil::mb_convert_encoding(r_euc,"sjis","euc");
		SEXYTEST_EQ(r , r_sjis);

		r = XLStringUtil::mb_convert_encoding(r_jis,"euc","jis");
		SEXYTEST_EQ(r , r_euc);
	}
}

std::string XLStringUtil::mb_detect_encoding(const std::string& str,const std::string& def)
{
	babel::init_babel();
	babel::analyze_result result(babel::analyze_base_encoding(str.c_str() ));
	const char *encodingMap[] = {
		"ASCII",	//0
		"ASCII",	//1
		"ASCII",	//2
		"SJIS",		//3
		"JIS",		//4
		"EUC-JP",	//5
		"ISO-2022-JP",	//6
		"UTF-8",	//7
		"UTF-16be",	//8
		"UTF-16le",	//9
		"UTF-32be",	//10
		"UTF-32le",	//11
		"UTF-16",	//12
		"UTF-32",	//13
	};
	return encodingMap[result];
}

SEXYTEST(XLStringUtil__mb_detect_encoding)
{
	{
		std::string r;
		r = XLStringUtil::mb_detect_encoding(XLFileUtil::cat("./config/testdata/test_utf8.txt") );
		SEXYTEST_EQ(r,"UTF-8");

		r = XLStringUtil::mb_detect_encoding(XLFileUtil::cat("./config/testdata/test_sjis.txt") );
		SEXYTEST_EQ(r,"SJIS");

		r = XLStringUtil::mb_detect_encoding(XLFileUtil::cat("./config/testdata/test_eucjp.txt") );
		SEXYTEST_EQ(r,"EUC-JP");

		r = XLStringUtil::mb_detect_encoding(XLFileUtil::cat("./config/testdata/test_jis.txt") );
		SEXYTEST_EQ(r,"JIS");
	}
}


std::string XLStringUtil::mb_convert_utf8(const std::string& str , const std::string& from)
{
	return mb_convert_encoding( str , "UTF-8" ,from );
}
*/


bool XLStringUtil::IsTimeOver(const struct tm *date,int hour ,int minute)
{
	if ( date->tm_hour > hour ) return true;
	if ( date->tm_hour == hour && date->tm_min >= minute ) return true;
	return false;
}

SEXYTEST()
{
	{
		struct tm date = {0};
		date.tm_hour = 0;
		date.tm_min = 0;
		bool r = XLStringUtil::IsTimeOver(&date,0,0);	// 0:0 vs 0:0  --> true
		SEXYTEST_EQ(r,true);
	}
	{
		struct tm date = {0};
		date.tm_hour = 0;
		date.tm_min = 0;
		bool r = XLStringUtil::IsTimeOver(&date,1,0);	// 0:0 vs 1:0  --> false
		SEXYTEST_EQ(r,false);
	}
	{
		struct tm date = {0};
		date.tm_hour = 0;
		date.tm_min = 0;
		bool r = XLStringUtil::IsTimeOver(&date,23,0);	// 0:0 vs 23:0  --> false
		SEXYTEST_EQ(r,false);
	}
	{
		struct tm date = {0};
		date.tm_hour = 23;
		date.tm_min = 0;
		bool r = XLStringUtil::IsTimeOver(&date,23,0);	// 23:0 vs 23:0  --> true
		SEXYTEST_EQ(r,true);
	}
	{
		struct tm date = {0};
		date.tm_hour = 23;
		date.tm_min = 0;
		bool r = XLStringUtil::IsTimeOver(&date,23,1);	// 23:0 vs 23:1  --> false
		SEXYTEST_EQ(r,false);
	}
	{
		struct tm date = {0};
		date.tm_hour = 23;
		date.tm_min = 0;
		bool r = XLStringUtil::IsTimeOver(&date,23,1);	// 23:0 vs 22:59  --> false
		SEXYTEST_EQ(r,false);
	}
	{
		struct tm date = {0};
		date.tm_hour = 23;
		date.tm_min = 0;
		bool r = XLStringUtil::IsTimeOver(&date,15,50);	// 23:0 vs 15:0  --> true
		SEXYTEST_EQ(r,true);
	}
	{
		struct tm date = {0};
		date.tm_hour = 15;
		date.tm_min = 49;
		bool r = XLStringUtil::IsTimeOver(&date,15,50);	// 15:49 vs 15:50  --> false
		SEXYTEST_EQ(r,false);
	}
	{
		struct tm date = {0};
		date.tm_hour = 15;
		date.tm_min = 49;
		bool r = XLStringUtil::IsTimeOver(&date,15,48);	// 15:49 vs 15:48  --> true
		SEXYTEST_EQ(r,true);
	}
}

bool XLStringUtil::IsTimeOver(const struct tm *date,int start_hour ,int start_minute,int end_hour ,int end_minute)
{
	if (start_hour > end_hour || (start_hour == end_hour && start_minute > end_minute ) )
	{//開始時刻(23:00) > 終了時刻(7:00)
		if ( IsTimeOver(date,start_hour,start_minute) ) 
		{
			return true;
		}
		else
		{
			if ( IsTimeOver(date,end_hour,end_minute) ) 
			{//終了時間を超えているのでダメ
				if (date->tm_hour == end_hour && date->tm_min == end_minute ) return true;	//ぴったしなら入る
				return false;
			}
			else
			{//開始時間(23:00) < 終了時間(6:00)のように一周している場合で、 開始時間より前だが、終了時間よりも前なので成立する.
				return true;
			}
		}
	}
	else
	{//開始時刻(19:00) < 終了時刻(23:00)
		if ( IsTimeOver(date,start_hour,start_minute) ) 
		{
			if ( IsTimeOver(date,end_hour,end_minute) ) 
			{//終了時間を超えているのでダメ
				if (date->tm_hour == end_hour && date->tm_min == end_minute ) return true;	//ぴったしなら入る
				return false;
			}
			else
			{
				return true;
			}
		}
		else
		{
			return false;
		}
	}

}

SEXYTEST()
{
	{
		struct tm date = {0};
		date.tm_hour = 0;
		date.tm_min = 0;
		bool r = XLStringUtil::IsTimeOver(&date,0,0,0,0);	// 0:0 vs 0:0-0:0  --> true
		SEXYTEST_EQ(r,true);
	}
	{
		struct tm date = {0};
		date.tm_hour = 0;
		date.tm_min = 1;
		bool r = XLStringUtil::IsTimeOver(&date,0,0,0,0);	// 0:1 vs 0:0-0:0  --> false
		SEXYTEST_EQ(r,false);
	}
	{
		struct tm date = {0};
		date.tm_hour = 23;
		date.tm_min = 59;
		bool r = XLStringUtil::IsTimeOver(&date,0,0,0,0);	// 23:59 vs 0:0-0:0  --> false
		SEXYTEST_EQ(r,false);
	}
	{
		struct tm date = {0};
		date.tm_hour = 23;
		date.tm_min = 0;
		bool r = XLStringUtil::IsTimeOver(&date,0,0,0,0);	// 23:0 vs 0:0-0:0  --> false
		SEXYTEST_EQ(r,false);
	}
	{
		struct tm date = {0};
		date.tm_hour = 15;
		date.tm_min = 0;
		bool r = XLStringUtil::IsTimeOver(&date,15,0,19,0);	// 15:0 vs 15:0-19:0  --> true
		SEXYTEST_EQ(r,true);
	}
	{
		struct tm date = {0};
		date.tm_hour = 19;
		date.tm_min = 0;
		bool r = XLStringUtil::IsTimeOver(&date,15,0,19,0);	// 19:0 vs 15:0-19:0  --> true
		SEXYTEST_EQ(r,true);
	}
	{
		struct tm date = {0};
		date.tm_hour = 19;
		date.tm_min = 1;
		bool r = XLStringUtil::IsTimeOver(&date,15,0,19,0);	// 19:1 vs 15:0-19:0  --> false
		SEXYTEST_EQ(r,false);
	}
	{
		struct tm date = {0};
		date.tm_hour = 14;
		date.tm_min = 0;
		bool r = XLStringUtil::IsTimeOver(&date,15,0,19,0);	// 14:0 vs 15:0-19:0  --> false
		SEXYTEST_EQ(r,false);
	}
	{
		struct tm date = {0};
		date.tm_hour = 16;
		date.tm_min = 0;
		bool r = XLStringUtil::IsTimeOver(&date,15,0,19,0);	// 16:0 vs 15:0-19:0  --> true
		SEXYTEST_EQ(r,true);
	}
	{
		struct tm date = {0};
		date.tm_hour = 23;
		date.tm_min = 0;
		bool r = XLStringUtil::IsTimeOver(&date,15,0,19,0);	// 23:0 vs 15:0-19:0  --> false
		SEXYTEST_EQ(r,false);
	}

	{
		struct tm date = {0};
		date.tm_hour = 23;
		date.tm_min = 0;
		bool r = XLStringUtil::IsTimeOver(&date,23,0,19,0);	// 23:0 vs 23:0-19:0  --> true
		SEXYTEST_EQ(r,true);
	}
	{
		struct tm date = {0};
		date.tm_hour = 22;
		date.tm_min = 0;
		bool r = XLStringUtil::IsTimeOver(&date,23,0,19,0);	// 22:0 vs 23:0-19:0  --> false
		SEXYTEST_EQ(r,false);
	}
	{
		struct tm date = {0};
		date.tm_hour = 0;
		date.tm_min = 0;
		bool r = XLStringUtil::IsTimeOver(&date,23,0,19,0);	// 0:0 vs 23:0-19:0  --> true
		SEXYTEST_EQ(r,true);
	}
	{
		struct tm date = {0};
		date.tm_hour = 10;
		date.tm_min = 0;
		bool r = XLStringUtil::IsTimeOver(&date,23,0,19,0);	// 10:0 vs 23:0-19:0  --> true
		SEXYTEST_EQ(r,true);
	}
	{
		struct tm date = {0};
		date.tm_hour = 19;
		date.tm_min = 1;
		bool r = XLStringUtil::IsTimeOver(&date,23,0,19,0);	// 19:1 vs 23:0-19:0  --> false
		SEXYTEST_EQ(r,false);
	}
	{
		struct tm date = {0};
		date.tm_hour = 20;
		date.tm_min = 0;
		bool r = XLStringUtil::IsTimeOver(&date,23,0,19,0);	// 20:0 vs 23:0-19:0  --> false
		SEXYTEST_EQ(r,false);
	}
}

unsigned int parseKeyValue(const char* line,const char* needkey,char parse1,char parse2)
{
	long ret = 0;
	const unsigned int keylen = strlen(needkey);
	const char* p  = line;

	while( 1 )
	{
		p = strstr(p,needkey);
		if (p == NULL)
		{
			break;
		}

		if ( p <= line )
		{//行頭
			
		}
		else 
		{
			if ( *(p-1) != parse1 )
			{//hogeを探して、 AAhoge にマッチしているっぽい.
				p = p + keylen;
				continue;
			}
		}

		if ( *(p+keylen) != parse2 )
		{//hogeを探して hogeAA にマッチしているっぽい
			p = p + keylen;
			continue;
		}

		return atoi(p+keylen+1);
	}
	return 0;
}

static unsigned int myatou(const char* line)
{
	unsigned int ret = 0;
	int found = 0;
	const char* p = line;
	while(*p)
	{
		if (*p < '0' || *p > '9')
		{
			if (found == 0)
			{
				if (*p == ' ' || *p=='\t')
				{
					p++;
					continue;
				}
			}
			break;
		}
		ret = ret * 10 + (*p - '0');
		found = 1;
		p++;
	}
	return ret;
}

SEXYTEST()
{
	{
		const char * line = "@	onavg:531	offavg:530	bigwave:70	src:1634	period:23342	duty:11682	Frequency:42841hz	DutyCycle:50%	checkerror:0	message:1101 0010 0010 1101 0100 1000 1011 0111 11";
		unsigned int a = parseKeyValue(line,"onavg",'\t',':');
		unsigned int b = parseKeyValue(line,"offavg",'\t',':');
		unsigned int c = parseKeyValue(line,"bigwave",'\t',':');

		a = myatou("	123	4567");
		a = myatou("	abc	4567");
	}
}


std::string XLStringUtil::ByteToHumanReadable(const size_t & size)
{
	if (size <= 1024) return num2str(size);
	if (size <= 1024*1024) return num2str(size/1024) + "K";
	if (size <= 1024 * 1024 * 1024) return num2str(size / 1024 / 1024) + "M";
	return num2str(size / 1024 / 1024 / 1024) + "G";
}
