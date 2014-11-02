
#include "common.h"
#include "XLWStringUtil.h"
#include "XLWStringUtil.h"
#include "wiki2util.h"


std::string ParseTitleOneLine(const std::string& line) 
{
	enum status_enum
	{
		 status_enum_none
		,status_enum_tag_start
		,status_enum_main
	};

	status_enum st = status_enum_none;
	std::vector<char> title;
	for(const char* p = line.c_str() ; *p ; p++ )
	{
		if (st == status_enum_none)
		{
			if (*p == '<' && *(p+1) == 't')
			{
				st = status_enum_tag_start;
			}
		}
		else if (st == status_enum_tag_start)
		{
			if (*p == '>')
			{
				st = status_enum_main;
			}
		}
		else if (st == status_enum_main)
		{
			if (*p == '<')
			{
				break;
			}
			else
			{
				title.push_back(*p);
			}
		}
	}

	if (title.empty())
	{
		return "";
	}
	title.push_back(0);

	std::string ret = &title[0];
	return ret; 
}

//カッコ内のパース (逆方向)
bool parseKakoR(const std::wstring &strW	//対象文字列
	,int pos                            //skipする位置
	,int endpos                         //終了する位置
	,bool isSearchStartKako             //最初の括弧を true->find するか、false->[0]にある
	,const std::wstring& startKako		//開始括弧
	,const std::wstring& endKako		//終了括弧
	,int* outStartKakoPos				//開始括弧位置
	,int* outStart2KakoPos				//開始括弧をすぎて本文が始まる位置
	,int* outEndKakoPos					//終了括弧位置
	,int* outEnd2KakoPos)				//終了括弧をすぎて本文が始まる位置
{
	int i = pos;
	i = strW.rfind(endKako,i);
	if (i == std::wstring::npos)
	{//終了括弧がない
		return false;
	}

	if (isSearchStartKako)
	{
		if (i < endpos )
		{//endposまでにない
			return false;
		}
	}
	else
	{
		if (i != pos )
		{//最初ではない
			return false;
		}
	}
	if (outEndKakoPos) *outEndKakoPos = i;
	if (outEnd2KakoPos)  *outEnd2KakoPos = i + endKako.size();
	i -= endKako.size();
	if (i < 0)
	{
		return false;
	}

	int e;
	int nest = 1;
	do
	{
		e = strW.rfind(startKako,i);
		if (e == std::wstring::npos || e < endpos )
		{//開始括弧がない
			return false;
		}
		else
		{
			nest --;
		}

		int i2 = strW.rfind(endKako,i);
		if (i2 != std::wstring::npos && e < i2)
		{//ネストする括弧がある
			i = i2 -  endKako.size();
			nest ++ ;
		}
		else
		{
			i = e;
		}
	}
	while(nest >= 1);

	if (outStartKakoPos)  *outStartKakoPos = i;
	if (outStart2KakoPos)  *outStart2KakoPos = i + startKako.size();
	return true;
}

//カッコ内のパース
bool parseKako(const std::wstring &strW	//対象文字列
	,int pos                            //skipする位置
	,int endpos                         //終了する位置
	,bool isSearchStartKako             //最初の括弧を true->find するか、false->[0]にある
	,const std::wstring& startKako		//開始括弧
	,const std::wstring& endKako		//終了括弧
	,int* outStartKakoPos				//開始括弧位置
	,int* outStart2KakoPos				//開始括弧をすぎて本文が始まる位置
	,int* outEndKakoPos					//終了括弧位置
	,int* outEnd2KakoPos)				//終了括弧をすぎて本文が始まる位置
{
	int i = pos;
	i = strW.find(startKako,i);
	if (i == std::wstring::npos)
	{//開始括弧がない
		return false;
	}

	if (isSearchStartKako)
	{
		if (i >= endpos )
		{//endposまでにない
			return false;
		}
	}
	else
	{
		if (i != pos )
		{//最初ではない
			return false;
		}
	}
	if (outStartKakoPos) *outStartKakoPos = i;
	if (outStart2KakoPos)  *outStart2KakoPos = i + startKako.size();
	i += startKako.size();

	int e;
	int nest = 1;
	do
	{
		e = strW.find(endKako,i);
		if (e == std::wstring::npos || e >= endpos )
		{//終了括弧がない
			return false;
		}
		else
		{
			nest --;
		}

		int i2 = strW.find(startKako,i);
		if (i2 != std::wstring::npos && e > i2)
		{//ネストする括弧がある
			i = i2 +  startKako.size();
			nest ++ ;
		}
		else
		{
			i = e;
		}
	}
	while(nest >= 1);

	if (outEndKakoPos)  *outEndKakoPos = i;
	if (outEnd2KakoPos)  *outEnd2KakoPos = i + endKako.size();
	return true;
}

//カッコ内のパース
bool parseKakoREx(const std::wstring &strW	//対象文字列
	,int pos                            //skipする位置
	,int endpos                         //終了する位置
	,bool isSearchStartKako             //最初の括弧を true->find するか、false->[0]にある
	,int* outStartKakoPos				//開始括弧位置
	,int* outStart2KakoPos				//開始括弧をすぎて本文が始まる位置
	,int* outEndKakoPos					//終了括弧位置
	,int* outEnd2KakoPos)				//終了括弧をすぎて本文が始まる位置
{
	int hitStartPos,hitEndPos;
	bool r = XLWStringUtil::firstrrfindPos(strW,pos,endpos,&hitStartPos,&hitEndPos
		,L"'''"
		,L"」"
		,L"』"
		,L"]]"
		,L"）"
		,L")"
		);
	if (!r)
	{
		return false;
	}
	if (strW[hitStartPos] == L'\'' && strW[hitStartPos+1] == L'\'' && strW[hitStartPos+2] == L'\'')
	{
		if ( parseKakoR(strW,pos,endpos,isSearchStartKako,L"'''",L"'''",outStartKakoPos,outStart2KakoPos,outEndKakoPos,outEnd2KakoPos) )
		{
			return true;
		}
	}
	if (strW[hitStartPos] == L'」')
	{
		if ( parseKakoR(strW,pos,endpos,isSearchStartKako,L"「",L"」",outStartKakoPos,outStart2KakoPos,outEndKakoPos,outEnd2KakoPos) )
		{
			return true;
		}
	}
	if (strW[hitStartPos] == L'』')
	{
		if ( parseKakoR(strW,pos,endpos,isSearchStartKako,L"『",L"』",outStartKakoPos,outStart2KakoPos,outEndKakoPos,outEnd2KakoPos) )
		{
			return true;
		}
	}
	if (strW[hitStartPos] == L']')
	{
		if ( parseKakoR(strW,pos,endpos,isSearchStartKako,L"[[",L"]]",outStartKakoPos,outStart2KakoPos,outEndKakoPos,outEnd2KakoPos) )
		{
			return true;
		}
	}
	if (strW[hitStartPos] == L'）')
	{
		if ( parseKakoR(strW,pos,endpos,isSearchStartKako,L"（",L"）",outStartKakoPos,outStart2KakoPos,outEndKakoPos,outEnd2KakoPos) )
		{
			return true;
		}
	}
	if (strW[hitStartPos] == L')')
	{
		if ( parseKakoR(strW,pos,endpos,isSearchStartKako,L"(",L")",outStartKakoPos,outStart2KakoPos,outEndKakoPos,outEnd2KakoPos) )
		{
			return true;
		}
	}
	return false;
}

//カッコ内のパース
bool parseKakoEx(const std::wstring &strW	//対象文字列
	,int pos                            //skipする位置
	,int endpos                         //終了する位置
	,bool isSearchStartKako             //最初の括弧を true->find するか、false->[0]にある
	,int* outStartKakoPos				//開始括弧位置
	,int* outStart2KakoPos				//開始括弧をすぎて本文が始まる位置
	,int* outEndKakoPos					//終了括弧位置
	,int* outEnd2KakoPos)				//終了括弧をすぎて本文が始まる位置
{
	int hitStartPos,hitEndPos;
	bool r = XLWStringUtil::firstfindPos(strW,pos,endpos,&hitStartPos,&hitEndPos
		,L"'''"
		,L"''"
		,L"「"
		,L"『"
		,L"[["
		,L"（"
		,L"("
		);
	if (!r)
	{
		return false;
	}
	if (strW[hitStartPos] == L'\'' && strW[hitStartPos+1] == L'\'' && strW[hitStartPos+2] == L'\'')
	{
		if ( parseKako(strW,pos,endpos,isSearchStartKako,L"'''",L"'''",outStartKakoPos,outStart2KakoPos,outEndKakoPos,outEnd2KakoPos) )
		{
			return true;
		}
	}
	if (strW[hitStartPos] == L'\'' && strW[hitStartPos+1] == L'\'')
	{
		if ( parseKako(strW,pos,endpos,isSearchStartKako,L"''",L"''",outStartKakoPos,outStart2KakoPos,outEndKakoPos,outEnd2KakoPos) )
		{
			return true;
		}
	}
	if (strW[hitStartPos] == L'「')
	{
		if ( parseKako(strW,pos,endpos,isSearchStartKako,L"「",L"」",outStartKakoPos,outStart2KakoPos,outEndKakoPos,outEnd2KakoPos) )
		{
			return true;
		}
	}
	if (strW[hitStartPos] == L'『')
	{
		if ( parseKako(strW,pos,endpos,isSearchStartKako,L"『",L"』",outStartKakoPos,outStart2KakoPos,outEndKakoPos,outEnd2KakoPos) )
		{
			return true;
		}
	}
	if (strW[hitStartPos] == L'[')
	{
		if ( parseKako(strW,pos,endpos,isSearchStartKako,L"[[",L"]]",outStartKakoPos,outStart2KakoPos,outEndKakoPos,outEnd2KakoPos) )
		{
			return true;
		}
	}
	if (strW[hitStartPos] == L'（')
	{
		if ( parseKako(strW,pos,endpos,isSearchStartKako,L"（",L"）",outStartKakoPos,outStart2KakoPos,outEndKakoPos,outEnd2KakoPos) )
		{
			return true;
		}
	}
	if (strW[hitStartPos] == L'(')
	{
		if ( parseKako(strW,pos,endpos,isSearchStartKako,L"(",L")",outStartKakoPos,outStart2KakoPos,outEndKakoPos,outEnd2KakoPos) )
		{
			return true;
		}
	}
	return false;
}


bool isAsciiOnlyTitle(const WCHAR* str )
{
	for(const WCHAR* p = str ; *p ; p = XLWStringUtil::nextChar(p) )
	{
		if ( *p >= 0x20 && *p <= 0x7e )
		{
			continue;
		}
		return false;
	}
	return true;
}

bool checkIchiran(const std::wstring& title)
{
	if ( XLWStringUtil::strend(title.c_str() , L"一覧") ) return true; 

	return false;
}

bool checkGomiPage(const std::wstring& title)
{
	if (title.empty() ) return true;

	if (title == L"愛称" || title == L"名称" || title == L"通称" || title == L"呼称" || title == L"なし" )  return true;

	if ( XLWStringUtil::strstr(title.c_str() , L"‐ノート") ) return true;
	if ( XLWStringUtil::strfirst(title.c_str() , L"Wikipedia:") ) return true;
	if ( XLWStringUtil::strfirst(title.c_str() , L"Category:") ) return true;
	if ( XLWStringUtil::strfirst(title.c_str() , L"File:") ) return true;
	if ( XLWStringUtil::strfirst(title.c_str() , L"Image:") ) return true;
	if ( XLWStringUtil::strfirst(title.c_str() , L"Help:") ) return true;
	if ( XLWStringUtil::strfirst(title.c_str() , L"User:") ) return true;
	if ( XLWStringUtil::strfirst(title.c_str() , L"Note:") ) return true;
	if ( XLWStringUtil::strfirst(title.c_str() , L"Template:") ) return true;
	if ( XLWStringUtil::strfirst(title.c_str() , L"Portal:") ) return true;
	if ( XLWStringUtil::strfirst(title.c_str() , L"カテゴリ:") ) return true;
	if ( XLWStringUtil::strfirst(title.c_str() , L"ファイル:") ) return true;
	if ( XLWStringUtil::strfirst(title.c_str() , L"画像:") ) return true;
	if ( XLWStringUtil::strfirst(title.c_str() , L"ノート:") ) return true;
	if ( XLWStringUtil::strfirst(title.c_str() , L"利用者:") ) return true;
	if ( XLWStringUtil::strfirst(title.c_str() , L"プロジェクト:") ) return true;
	if ( XLWStringUtil::strfirst(title.c_str() , L"年代") ) return true;
	if ( XLWStringUtil::strfirst(title.c_str() , L"世紀") ) return true;
	if ( XLWStringUtil::strfirst(title.c_str() , L"登場人物") ) return true;
	if ( XLWStringUtil::strfirst(title.c_str() , L"利用者") ) return true;
	if ( XLWStringUtil::strfirst(title.c_str() , L"スタブ") ) return true;
	if ( XLWStringUtil::strfirst(title.c_str() , L"ノート:") ) return true;
	if ( XLWStringUtil::strfirst(title.c_str() , L"ファイル:") ) return true;
	if ( XLWStringUtil::strstr(title.c_str() , L"○○") ) return true;
	if ( XLWStringUtil::strstr(title.c_str() , L"用語") ) return true;
	if ( XLWStringUtil::strfirst(title.c_str() , L"国道") ) return true;
	if ( XLWStringUtil::strend(title.c_str() , L"系電車") ) return true; //てっちゃん自重汁.
	if ( XLWStringUtil::strend(title.c_str() , L"形電車") ) return true; //てっちゃん自重汁.
	if ( XLWStringUtil::strend(title.c_str() , L"形貨車") ) return true; //てっちゃん自重汁.
	if ( XLWStringUtil::strstr(title.c_str() , L"/history") ) return true;
	if ( XLWStringUtil::strend(title.c_str() , L"の作品") ) return true;	
//	if ( XLWStringUtil::strstr(title.c_str() , L":") ) return true;
//	if ( XLWStringUtil::strstr(title.c_str() , L"：") ) return true;
	if ( XLWStringUtil::strstr(title.c_str() , L"最近の出来事") ) return true;
	if ( XLWStringUtil::strstr(title.c_str() , L"…") ) return true;

//	//先頭がシャープ
//	if ( (title[0] == '#')   )	return true;

	//XX年代はいらない
	if ( XLWStringUtil::strend(title.c_str() , L"年代") )
	{
		return true;
	}

	if (title[0] >= L'0' && title[0] <= L'9' )
	{
		if(title.size()==1)
		{//数字1つ
			return true;
		}
		if ( XLWStringUtil::strend(title.c_str() , L"年") )
		{//数字から始まり、年で終わるのは、年号だろう.
			return true;
		}

		int i = 1;
		if (title[i] >= L'0' && title[i] <= L'9' )
		{
			i++;
			if(title.size()==i)
			{//"11"とか
				return true;
			}
		}
		if (title[i] == L'月' )
		{
			i++;
			if(title.size()==i)
			{//"1月"とか
				return true;
			}
		}
		if (title[i] >= L'0' && title[i] <= L'9' )
		{
			i++;
			if(title.size()==i)
			{//"11月1"とか
				return true;
			}
		}
		if (title[i] >= L'0' && title[i] <= L'9' )
		{
			i++;
			if(title.size()==i)
			{//"11月11"とか
				return true;
			}
		}
		if (title[i] == L'日' )
		{
			i++;
			if(title.size()==i)
			{//"1日" or "1月1日" とか
				return true;
			}
		}
		if (title[i] == L'年' )
		{
			i++;
			if(title.size()==i)
			{//"1111年"とか
				return true;
			}
			if (title[i] >= L'0' && title[i] <= L'9' )
			{
				i++;
				if(title.size()==i)
				{//"1111年1"とか
					return true;
				}
			}
			if (title[i] >= L'0' && title[i] <= L'9' )
			{
				i++;
				if(title.size()==i)
				{//"1111年11"とか
					return true;
				}
				if (title[i] == L'月' )
				{
					i++;
					if(title.size()==i)
					{//"1111年11月"とか
						return true;
					}
					if (title[i] >= L'0' && title[i] <= L'9' )
					{
						i++;
						if(title.size()==i)
						{//"1111年1"とか
							return true;
						}
					}
					if (title[i] >= L'0' && title[i] <= L'9' )
					{
						i++;
						if(title.size()==i)
						{//"1111年11"とか
							return true;
						}
						if (title[i] == L'日' )
						{//1111年11月11日
							return true;
						}
					}
				}
			}
		}
	}

	if ( XLWStringUtil::strstr(title.c_str() , L"年") )
	{
		if ( XLWStringUtil::strstr(title.c_str() , L"紀元前") )	return true;
		if ( XLWStringUtil::strstr(title.c_str() , L"明治") )	return true;
		if ( XLWStringUtil::strstr(title.c_str() , L"大正") )	return true;
		if ( XLWStringUtil::strstr(title.c_str() , L"昭和") )	return true;
		if ( XLWStringUtil::strstr(title.c_str() , L"平成") )	return true;
	}
	if ( XLWStringUtil::strstr(title.c_str() , L"号") )
	{
		if ( XLWStringUtil::strstr(title.c_str() , L"県道") )	return true;
		if ( XLWStringUtil::strstr(title.c_str() , L"北海道道") )	return true;
		if ( XLWStringUtil::strstr(title.c_str() , L"府道") )	return true;
		if ( XLWStringUtil::strstr(title.c_str() , L"都道") )	return true;
	}
	if ( XLWStringUtil::strend(title.c_str() , L"日") )
	{
		if ( (title[0] >= '0' && title[0] <= '9')   )
		{//N日 or N月N日
			return true;
		}
		if ( XLWStringUtil::strfirst(title.c_str() , L"一月") 
			|| XLWStringUtil::strfirst(title.c_str() , L"二月") 
			|| XLWStringUtil::strfirst(title.c_str() , L"三月") 
			|| XLWStringUtil::strfirst(title.c_str() , L"四月") 
			|| XLWStringUtil::strfirst(title.c_str() , L"五月") 
			|| XLWStringUtil::strfirst(title.c_str() , L"六月") 
			|| XLWStringUtil::strfirst(title.c_str() , L"七月") 
			|| XLWStringUtil::strfirst(title.c_str() , L"八月") 
			|| XLWStringUtil::strfirst(title.c_str() , L"九月") 
			|| XLWStringUtil::strfirst(title.c_str() , L"十月")
			|| XLWStringUtil::strfirst(title.c_str() , L"十一月") 
			|| XLWStringUtil::strfirst(title.c_str() , L"十二月") 
			)
		{
			return true;
		}
	}
	if ( XLWStringUtil::strfirst(title.c_str() , L"日本の") )
	{
			return true;
	}
	if ( XLWStringUtil::strfirst(title.c_str() , L"あ行") 
		|| XLWStringUtil::strfirst(title.c_str() , L"か行") 
		|| XLWStringUtil::strfirst(title.c_str() , L"さ行") 
		|| XLWStringUtil::strfirst(title.c_str() , L"た行") 
		|| XLWStringUtil::strfirst(title.c_str() , L"な行") 
		|| XLWStringUtil::strfirst(title.c_str() , L"は行") 
		|| XLWStringUtil::strfirst(title.c_str() , L"ま行") 
		|| XLWStringUtil::strfirst(title.c_str() , L"や行") 
		|| XLWStringUtil::strfirst(title.c_str() , L"ら行") 
		|| XLWStringUtil::strfirst(title.c_str() , L"わ行") 
	 )
	{
		return true;
	}
	return false;
}

SEXYTEST()
{
	bool r;
	{
		r = checkGomiPage(L"国鉄ダイヤ改正");
		assert(!r);

		r = checkGomiPage(L"1942年11月15日国鉄ダイヤ改正");
		assert(r);

		r = checkGomiPage(L"1月");
		assert(r);

		r = checkGomiPage(L"1月1日");
		assert(r);

		r = checkGomiPage(L"大田広域市都市鉄道公社100系電車");
		assert(r);

		r = checkGomiPage(L"日本の歴史");
		assert(r);

		r = checkGomiPage(L"日本の警察官");
		assert(r);

		r = checkGomiPage(L"日本の政治家");
		assert(r);

		r = checkGomiPage(L"日本の漫画家");
		assert(r);
	}
}

//[[]] を消します.
std::wstring snipWikiKeywordSyntax(const std::wstring& r)
{
	std::wstring w = XLWStringUtil::replace(r,L"[[",L"");
	w = XLWStringUtil::replace(w,L"]]",L"");
	return w;
}

//不要な項目を消す
std::wstring cleaningInnerText(const std::wstring& str)
{
	std::wstring w = str;
	w = XLWStringUtil::replace(w,L"&quot;",L"");
	w = XLWStringUtil::replace(w,L"&amp;",L"&");
	w = killHTMLTagBR(w,L"br");
	w = killHTMLTagBR(w,L"small");

	w = XLWStringUtil::strsnip(w ,L"&lt;!--",L"--&gt;");//コメント
	w = XLWStringUtil::strsnip(w,L"\n{{独自研究",L"}}");
	w = XLWStringUtil::strsnip(w,L"\n{{大言壮語",L"}}");
	w = XLWStringUtil::strsnip(w,L"\n{{出典の明記",L"}}");
	w = XLWStringUtil::strsnip(w,L"\n[[Image:",L"]]");
	w = XLWStringUtil::strsnip(w,L"\n[[image:",L"]]");
	w = XLWStringUtil::strsnip(w,L"\n[[ファイル:",L"]]");
	w = killHTMLTag(w,L"ref");
	w = killHTMLTag(w,L"span");
	w = XLWStringUtil::strsnip(w, L"&lt;",L"&gt;");	//タグ
	w = XLWStringUtil::replace(w,L"  ",L" "); //連続するスペースを1つに
	return XLWStringUtil::chop( w );
}



//飛び先に曖昧さ解決の()が入っていたら消します.
std::wstring snipAimaiKako(const std::wstring& r)
{
	//曖昧さ解決が入っていたら底で切る
//	const std::wstring w = XLWStringUtil::chop(XLWStringUtil::strsnip(r, L"(",L")"));
	std::wstring w = XLWStringUtil::strsnip(r, L"(",L")");
	w = cleaningInnerText(w);
	w = XLWStringUtil::strsnip(w, L"（",L"）");
	return XLWStringUtil::chop(w, L" 　\t\r\n'");
}
		
//曖昧さの解決のページかどうか
bool isAimai(const std::wstring& innerW)
{
	if ( innerW.find(L"{{aimai}}") != std::wstring::npos)
	{
		return true;
	}
	if ( innerW.find(L"{{Aimai}}") != std::wstring::npos)
	{
		return true;
	}
	if ( innerW.find(L"{{AIMAI}}") != std::wstring::npos)
	{
		return true;
	}
	return false;
}
//曖昧さの解決のページかどうか
bool isAimai(const std::string& inner)
{
	return inner.find("{{aimai}}") != std::string::npos;
}

bool isRedirect(const std::wstring& innerW)
{
	int pos ;
	pos = innerW.find(L">#転送");
	if (pos == std::wstring::npos)
	{
		pos = innerW.find(L">#REDIRECT");
		if (pos == std::wstring::npos)
		{
			pos = innerW.find(L">#redirect");
			if (pos == std::wstring::npos)
			{
				pos = innerW.find(L">#Redirect");
				if (pos == std::wstring::npos)
				{
					return false;
				}
			}
		}
	}
	for(pos += 4 ; innerW[pos] ; pos ++ )
	{
		if (innerW[pos] == L'[' )
		{
			if (innerW[pos+1] == L'[' )
			{
				break;
			}
		}
	}
	if (innerW[pos] != L'[')
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool isRedirect(const std::string& inner)
{
	int pos ;
	pos = inner.find(">#転送");
	if (pos == std::string::npos)
	{
		pos = inner.find(">#REDIRECT");
		if (pos == std::string::npos)
		{
			pos = inner.find(">#redirect");
			if (pos == std::string::npos)
			{
				pos = inner.find(">#Redirect");
				if (pos == std::string::npos)
				{
					return false;
				}
			}
		}
	}
	for(pos += 4 ; inner[pos] ; pos ++ )
	{
		if (inner[pos] == '[' )
		{
			if (inner[pos+1] == '[' )
			{
				break;
			}
		}
	}
	if (inner[pos] != L'[')
	{
		return false;
	}
	else
	{
		return true;
	}
}

//タグを消す. (この関数は完璧ではない)
std::wstring killHTMLTagImpl(const std::wstring& innerW,const std::wstring& starttag,const std::wstring& end1tag,const std::wstring& end2tag)
{
	std::wstring cap;
	int nest = 0;
	int p = 0;
	int capP = 0;

	while(1)
	{
		int s = innerW.find(starttag,p);
		if (s == std::wstring::npos)
		{
			if (nest <= 0)
			{
				cap += innerW.substr(capP);
			}
			break;
		}

		auto aaa = innerW.substr(s + starttag.size());
		WCHAR tagendC = innerW[s + starttag.size()];
		if ( XLWStringUtil::isAlphanum(tagendC) )
		{//偽のタグ終わり
			p += starttag.size();
			continue;
		}

		if (nest <= 0)
		{
			cap += innerW.substr(p,s-p);
		}

		s += starttag.size();
		int s1 = innerW.find(starttag,s);
		int e1 = innerW.find(end1tag,s);
		int e2 = innerW.find(end2tag,s);

		if (e1 != std::wstring::npos && e2 != std::wstring::npos)
		{
			if (s1 != std::wstring::npos && (s1 < e1 && s1 < e2) )
			{//<ref> nest
				nest ++;
				p = s1;
				capP = p;
				continue;
			}

			if (e2 < e1)
			{// <ref /> 即閉じ
				p = e2 + end2tag.size();
				capP = p;
			}
			else
			{//e2 > e1 </ref>
				nest --;
				p = e1 + end1tag.size();
				capP = p;
			}
			continue;
		}
		else if (e1 != std::wstring::npos && e2 == std::wstring::npos)
		{//</ref>
			if (s1 != std::wstring::npos && (s1 < e1 ) )
			{//<ref> nest
				nest ++;
				p = s1 + starttag.size();
				capP = p;
				continue;
			}

			nest --;
			p = e1 + end1tag.size();
			capP = p;
			continue;
		}
		else if (e1 == std::wstring::npos && e2 != std::wstring::npos)
		{// <ref /> 即閉じ
			if (s1 != std::wstring::npos && (s1 < e2 ) )
			{//<ref> nest
				nest ++;
				p = s1 + starttag.size();
				capP = p;
				continue;
			}

			p = e2 + end2tag.size();
			capP = p;
			continue;
		}
		else
		{//閉じタグも何もない
			cap += innerW.substr(s-starttag.size());
			break;
		}
	}
	return cap;
}



std::wstring killHTMLTagBR(const std::wstring& innerW,const std::wstring& name)
{
	std::wstring w = innerW;
	{
		w = XLWStringUtil::replace(w,L"&lt;"+name+L" /&gt;",L"");
		w = XLWStringUtil::replace(w,L"&lt;"+name+L"/&gt;",L"");
		w = XLWStringUtil::replace(w,L"&lt;"+name+L"&gt;",L"");
		w = XLWStringUtil::replace(w,L"&lt;/"+name+L"&gt;",L"");
	}
#if _DEBUG
	{//テストデータを &lt; エスケープするのがめんどいので
		w = XLWStringUtil::replace(w,L"<"+name+L" />",L"");
		w = XLWStringUtil::replace(w,L"<"+name+L"/>",L"");
		w = XLWStringUtil::replace(w,L"<"+name+L">",L"");
		w = XLWStringUtil::replace(w,L"</"+name+L">",L"");
	}
#endif

	return w;
}


std::wstring killHTMLTag(const std::wstring& innerW,const std::wstring& name)
{
	std::wstring w = innerW;
	{
		const std::wstring starttag = L"&lt;" + name;
		const std::wstring end1tag = L"&lt;/" + name + L"&gt;";
		const std::wstring end2tag = L"/&gt;";
		w= killHTMLTagImpl(w,starttag,end1tag,end2tag);
	}
#if _DEBUG
	{//テストデータを &lt; エスケープするのがめんどいので
		const std::wstring starttag = L"<" + name;
		const std::wstring end1tag = L"</" + name + L">";
		const std::wstring end2tag = L"/>";
		w= killHTMLTagImpl(w,starttag,end1tag,end2tag);
	}
#endif

	return w;
}

SEXYTEST()
{
	{
		const std::wstring a= killHTMLTag(L">a<refef>vb",L"ref");
		assert(a==L">a<refef>vb");
	}
	{
		const std::wstring a= killHTMLTag(L">a<ref group=\"note\">vb",L"ref");
		assert(a==L">a<ref group=\"note\">vb");
	}
	{
		const std::wstring a= killHTMLTag(L">a<ref group=\"note\">v</ref>b",L"ref");
		assert(a==L">ab");
	}

	{
		const std::wstring a= killHTMLTag(L">（えいでん）<ref name=\"ryakushou\" >。本社は</ref>",L"ref");
		assert(a==L">（えいでん）");

	}
	{
		const std::wstring a= killHTMLTag(L">（えいでん）<ref name=\"ryakushou\" />。本社は",L"ref");
		assert(a==L">（えいでん）。本社は");

	}
	{
		const std::wstring a= killHTMLTag(L"BNGI、&lt;ref&gt;バンナムTwitterプロフィールより。&lt;/ref&gt;",L"ref");
		assert(a==L"BNGI、");
	}
}

std::wstring killHTMLTagOnly(const std::wstring& innerW,const std::wstring& name)
{
	std::wstring w = innerW;
	{
		w = XLWStringUtil::replace(w,L"&lt;"+name+L" /&gt;",L"");
		w = XLWStringUtil::replace(w,L"&lt;"+name+L"/&gt;",L"");
		w = XLWStringUtil::replace(w,L"&lt;"+name+L"&gt;",L"");
		w = XLWStringUtil::replace(w,L"&lt;/"+name+L"&gt;",L"");
	}
#if _DEBUG
	{//テストデータを &lt; エスケープするのがめんどいので
		w = XLWStringUtil::replace(w,L"<"+name+L" />",L"");
		w = XLWStringUtil::replace(w,L"<"+name+L"/>",L"");
		w = XLWStringUtil::replace(w,L"<"+name+L">",L"");
		w = XLWStringUtil::replace(w,L"</"+name+L">",L"");
	}
#endif
	return w;
}

SEXYTEST()
{
	{
		std::wstring w = L"ab<br>c</br>d<br />e<br/>d";
		w = killHTMLTagOnly(w,L"br");
		assert(w == L"abcded");
	}
}

//頭出し
std::wstring makeInnerHead(const std::wstring& innerW)
{
	int pos = innerW.find(L"\n==",0);
	if (pos == std::wstring::npos)
	{
		return cleaningInnerText(innerW);
	}
	std::wstring w = innerW.substr(0,pos);

	return cleaningInnerText(w);
}

//など と最後に書かれていたら削る
std::wstring snipNado(const std::wstring& w)
{
	if (w.size() >= 2)
	{
		if ( w.find(L"など",w.size() -2 ) != std::wstring::npos)
		{
			return w.substr(0,w.size() -2 );
		}
	}
	return w;
}

std::wstring getInfoboxStr(const std::wstring& innerHeadW,const std::wstring& key )
{
	int i = 0;
	int endpos = innerHeadW.size();
	int hitStartPos,hitEndPos;
	bool r;
	r = XLWStringUtil::firstfindPos(innerHeadW,i,endpos,&hitStartPos,&hitEndPos,L">{{Infobox",L"\n{{Infobox",L">{|",L"\n{|",L">{{",L"\n{{",L"{{");
	if (r)
	{
		i = hitEndPos;
	}
	else
	{//エラーにしたのだけども、 {{ でスタートすることがある。 そして、それはトップに有るとは限らない.
		i = 0;
	}

	//略称の表記が　大学の略称 などと揺れるので吸収する.
	//| *愛称*=  となっている項目がほしい.

	while(1)
	{
#ifdef _DEBUG
		auto aaa = innerHeadW.substr(i);
#endif
		if (i >= endpos)
		{
			return L"";
		}
		if ( innerHeadW[i] == L'(' || innerHeadW[i] == L'[' || innerHeadW[i] == L'{')
		{//現在地に括弧があったら終端まで無視.
			int startKakoPos;				//開始括弧位置
			int start2KakoPos;				//開始括弧をすぎて本文が始まる位置
			int endKakoPos;					//終了括弧位置
			int end2KakoPos;				//終了括弧をすぎて本文が始まる位置
			r = parseKakoREx(innerHeadW,hitStartPos,endpos,false,&startKakoPos,&start2KakoPos,&endKakoPos,&end2KakoPos);
			if (r)
			{//
				i = end2KakoPos;
				continue;
			}
			else
			{//閉じ括弧がないだと・・
				//せめて \n	まで見つけてあげる.
				i = innerHeadW.find(L"\n",hitStartPos);
				if (i == std::wstring::npos)
				{//\n もないなら判別不能
					return L"";
				}
#ifdef _DEBUG
				aaa = innerHeadW.substr(i);
#endif
				continue;
			}
		}

		//開始キーワード | を見つける.
		r = XLWStringUtil::firstfindPos(innerHeadW,i,endpos,&hitStartPos,&hitEndPos
			,L"|"	//開始キーワード
			,L"[["	//ゴミ
			,L"{{"	//ゴミ
			);
		if (!r)
		{
			return L"";
		}
		i = hitEndPos;
#ifdef _DEBUG
		auto bbb = innerHeadW.substr(i);
#endif
		if ( innerHeadW[hitStartPos] != L'|' )
		{
			continue;
		}
		//|があることは確定
		r = XLWStringUtil::firstfindPos(innerHeadW,i,endpos,&hitStartPos,&hitEndPos
			,key	//開始キーワード
			,L"="	//ゴミ
			,L"|"	//ゴミ
			,L"[["	//ゴミ
			,L"\n"	//ゴミ
			);
		if (!r)
		{
			return L"";
		}
		i = hitEndPos;
#ifdef _DEBUG
		auto ccc = innerHeadW.substr(i);
#endif
		if (innerHeadW[hitStartPos] == L'=' || innerHeadW[hitStartPos] == L'|' || innerHeadW[hitStartPos] == L'['|| innerHeadW[hitStartPos] == L'\n' )
		{
			continue;
		}
		//|*KEYがあることは確定

		//|*KEY[:SPACE:]*=
		for( ; i < endpos ; i++ )
		{
			if (XLWStringUtil::isSpace( innerHeadW[i] ))
			{
				continue;
			}
			else
			{
				break;
			}
		}
#ifdef _DEBUG
		auto ddd = innerHeadW.substr(i);
#endif
		if (innerHeadW[i] != L'=')
		{
			continue;
		}
		i++;

		//ここまでで、キーワード部が確定。次は終端を探す.
		break;
	}

	//終端を探す.
	const int keywordStart = i;
	while(1)
	{
#ifdef _DEBUG
		auto aaa = innerHeadW.substr(i);
#endif
		if (i >= endpos)
		{
			return L"";
		}
		if ( innerHeadW[i] == L'(' || innerHeadW[i] == L'[')
		{//現在地に括弧があったら終端まで無視.
			int startKakoPos;				//開始括弧位置
			int start2KakoPos;				//開始括弧をすぎて本文が始まる位置
			int endKakoPos;					//終了括弧位置
			int end2KakoPos;				//終了括弧をすぎて本文が始まる位置
			r = parseKakoREx(innerHeadW,hitStartPos,endpos,false,&startKakoPos,&start2KakoPos,&endKakoPos,&end2KakoPos);
			if (r)
			{//
				i = end2KakoPos;
				continue;
			}
			else
			{//閉じ括弧がないだと・・
				//せめて \n	まで見つけてあげる.
				i = innerHeadW.find(L"\n",hitStartPos);
				if (i == std::wstring::npos)
				{//\n もないなら判別不能
					return L"";
				}
				continue;
			}
		}

		r = XLWStringUtil::firstfindPos(innerHeadW,i,endpos,&hitStartPos,&hitEndPos
			,L"|"	//開始キーワード
			,L"\n"	//これも期待
			,L"[["	//ゴミ
			);
		if (!r)
		{
			return L"";
		}
		i = hitEndPos;
#ifdef _DEBUG
		auto bbb = innerHeadW.substr(i);
#endif
		if ( ! (innerHeadW[hitStartPos] == L'|' || innerHeadW[hitStartPos] == L'\n' ) )
		{
			continue;
		}

		//キーワード確定
		break;
	}
	const int keywordEnd = i-1;

	std::wstring w = innerHeadW.substr(keywordStart,keywordEnd - keywordStart);

	//クーリニング
	w = XLWStringUtil::strsnip(w ,L"(",L")");
	w = XLWStringUtil::strsnip(w ,L"（",L"）");
	w = XLWStringUtil::strsnip(w ,L"{{",L"}}");
	w = XLWStringUtil::strsnip(w ,L"{|",L"|}");
	
	w = XLWStringUtil::replace(w,L"&amp;",L"&");
	w = killHTMLTag(w,L"ref");
	w = killHTMLTag(w,L"span");
	w = killHTMLTagOnly(w,L"sup");
	w = killHTMLTagOnly(w,L"br");
	w = killHTMLTagOnly(w,L"quot");
	w = XLWStringUtil::strsnip(w ,L"&lt;!--",L"--&gt;");//コメント
	w = XLWStringUtil::chop(w,L" 　\t'");
	w = snipNado(w);

	return w;
}

SEXYTEST()
{
	{
		std::wstring innerHeadW = L"{{基礎情報 スペインの自治体2\n|名前={{lang|gl|Boimorto}}\n|市旗= |市旗幅=130px\n|市章=Boimorto.svg |市章幅=90px\n|画像= |画像幅=300px |画像説明=\n|州=ガリシア |県=ア・コルーニャ |コマルカ=[[コマルカ・デ・アルスーア|アルスーア]] |司法管轄区=[[アルスーア]]\n|面積=[[1 E8 m&sup2;|82.3km&sup2;]] <ref name=IGE>{{cite web|url=http://www.ige.eu/web/index.jsp?paxina=001&idioma=gl|title=IGE（ガリシア統計局）|publisher=ガリシア自治州政府|language=ガリシア語|accessdate=2012-09-05}}</ref>\n|教区数=13 |居住地区数=115 |標高=500\n|人口=2,211人（2011年） |人口密度=26.9人/km&sup2; <ref name=IGE/>\n|住民の呼称=boimortense |ガリシア語率=99.92 |自治体首長=ホセ・イグナシオ・ポルトス・バスケス<br />（[[ガリシア社会党|PSdeG-PSOE]]）\n|latd=43 |latm=0 |lats=27 |lond=8 |lonm=07 |lons=37 |EW=W\n}}";
		std::wstring r = getInfoboxStr(innerHeadW,L"呼称");
		assert(r == L"boimortense");
	}
}
std::wstring killWikiKakoImpl( const std::wstring& strW,const std::wstring& exStartKako,const std::wstring& startKako, const std::wstring& endKako)
{
	int pos = strW.find(exStartKako);
	if (pos == std::wstring::npos) return strW;

	pos += exStartKako.size() - startKako.size();
	assert(pos >= 0);

	const std::wstring newStrW = strW.substr(pos);
	std::wstring r = XLWStringUtil::strsnipOne(newStrW,startKako,endKako);

	r = strW.substr(0,pos) + r;
	return r;
}

SEXYTEST()
{
	{
		std::wstring w = L">\n[[ファイル:P2P-network.svg|thumb|200px|P2P型ネットワーク（図はピュアP2P型）。コンピューター同士が対等に通信を行うのが特徴である。]]\n'''ピアトゥピア'''または'''ピアツーピア'''（）とは、多数の端末間で通信を行う際の[[アーキテクチャ]]のひとつで、対等の者（Peer、ピア）同士が通信をすることを特徴とする通信方式、通信モデル、あるいは、通信技術の一分野を指す。'''P2P'''と[[Leet|略記]]することが多く、以下本記事においてもP2Pとする。";
		std::wstring r = killWikiKakoImpl(w,L"\n[[",L"[[",L"]]");

		assert(r == L">\n\n'''ピアトゥピア'''または'''ピアツーピア'''（）とは、多数の端末間で通信を行う際の[[アーキテクチャ]]のひとつで、対等の者（Peer、ピア）同士が通信をすることを特徴とする通信方式、通信モデル、あるいは、通信技術の一分野を指す。'''P2P'''と[[Leet|略記]]することが多く、以下本記事においてもP2Pとする。");

	}
}


//wiki表記で、パースの邪魔になる \n{{ ....  }}   [[ファイル  ]]  などの広域な構文のみを消します. 全部消すと問題あるので、この2つに限り消す.
std::wstring killWikiKako( const std::wstring& strW)
{
	std::wstring w = strW;
	std::wstring temp ;
	while(1)
	{
		temp = w;
		w = killWikiKakoImpl(w,L">{{",L"{{",L"}}");
		w = killWikiKakoImpl(w,L"\n{{",L"{{",L"}}");
		w = killWikiKakoImpl(w,L">[[",L"[[",L"]]");
		w = killWikiKakoImpl(w,L"\n[[",L"[[",L"]]");
		w = killWikiKakoImpl(w,L">{|",L"{|",L"|}");
		w = killWikiKakoImpl(w,L"\n{|",L"{|",L"|}");
		if (temp.size() == w.size() || w.empty() )
		{
			break;
		}
	}

	return w;
}



std::wstring makeInnerText(const std::wstring& innerW)
{
	//余計な記号を消した本文を作成します.
	std::wstring w = innerW;
	w = killWikiKako(w);
	w = cleaningInnerText(w);

	//ただし、あんまり下の方まで行くと登場人物などが書いてある場合があるので適当なところで切ります.
	int partCount = 0;
	int pos = 0;
	while(w[pos])
	{
		pos = w.find(L"\n==",pos);
		if (pos == std::wstring::npos)
		{
			break;
		}
		pos += sizeof("\n==")-1;

//		const std::wstring parts = w.substr(pos ,10);
//		if (std::wstring::npos != parts.find(L"概要 ==\n"))
		{
//			continue;
		}

//		partCount++;
		//とりあえず1段落目で切ってみるか.
//		if (partCount >= 1)
		{
			break;
		}
	}
	w = w.substr(0,pos);
	w = XLWStringUtil::replace(w,L"  ",L" "); //連続するスペースを1つに
	return XLWStringUtil::chop( w );
}

bool IsTofuOnly(const std::wstring& str)
{
	if (str.empty())
	{
		return false;
	}
	if (str == L"?") 
	{//windowsの文字コード変換でとーふ.
		return true;
	}

	const WCHAR* p = str.c_str();
	for( ; *p ; p++ )
	{
		if ( XLWStringUtil::isKana(*p))
		{
			return false;
		}
		if ( XLWStringUtil::isKata(*p))
		{
			return false;
		}
		if ( XLWStringUtil::isSpace(*p))
		{//倍角空白があるのでそのまま
			return false;
		}
		if ( XLWStringUtil::isKanji(*p))
		{
			return false;
		}
//		if ( XLWStringUtil::isAlphanum(*p))
//		{
//			return false;
//		}
		if ( *p >= 0x0020 && *p <= 0x007e)
		{//ASCII範囲
			return false;
		}
	}
	return true;
}

bool isKanjiOnly(const std::wstring& str)
{
	const WCHAR* p = str.c_str();
	for( ; *p ; p++ )
	{
		if ( XLWStringUtil::isKanji(*p))
		{
			continue;
		}
		return false;
	}
	return true;
}

bool isKakoStart(WCHAR c)
{
	if (c==L'「'
		|| c==L'『'
		|| c==L'（'
		|| c==L'('
		|| c==L'['
		|| c==L'\''
		)
	{
		return true;
	}
	return false;
}





bool isKatakanaOnly(const std::wstring& str)
{
	for(unsigned int i = 0 ; i < str.size() ; i++)
	{
		if ( XLWStringUtil::isKata(str[i]))
		{
			continue;
		}
		if ( XLWStringUtil::isSpace(str[i]))
		{
			continue;
		}
		if ( str[i] == L'_' )
		{
			continue;
		}
		return false;
	}
	return true;
}

bool isAplhaOnly(const std::wstring& str)
{
	for(unsigned int i = 0 ; i < str.size() ; i++)
	{
		if ( XLWStringUtil::isAlpha(str[i]))
		{
			continue;
		}
		if ( XLWStringUtil::isSpace(str[i]))
		{
			continue;
		}
		if ( str[i] == L'_' || str[i] == L'.')
		{
			continue;
		}
		return false;
	}
	return true;
}

bool isAplhaSmallOnly(const std::wstring& str)
{
	for(unsigned int i = 0 ; i < str.size() ; i++)
	{
		if (str[i] >= L'a' && str[i] <= L'z')
		{
			continue;
		}
		if ( XLWStringUtil::isSpace(str[i]))
		{
			continue;
		}
		if ( str[i] == L'_' || str[i] == L'.')
		{
			continue;
		}
		return false;
	}
	return true;
}

bool isAlphanumOnly(const std::wstring& str)
{
	for(unsigned int i = 0 ; i < str.size() ; i++)
	{
		if ( XLWStringUtil::isAlphanum(str[i]))
		{
			continue;
		}
		if ( XLWStringUtil::isSpace(str[i]))
		{
			continue;
		}
		if ( str[i] == L'_' || str[i] == L'.' )
		{
			continue;
		}
		return false;
	}
	return true;
}

bool isHiraKataOnly(const std::wstring& str)
{
	for(unsigned int i = 0 ; i < str.size() ; i++)
	{
		if ( XLWStringUtil::isKana(str[i]))
		{
			continue;
		}
		if ( XLWStringUtil::isKata(str[i]))
		{
			continue;
		}
		if ( XLWStringUtil::isSpace(str[i]))
		{
			continue;
		}
		if ( str[i] == L'_' )
		{
			continue;
		}
		return false;
	}
	return true;
}


//よみがなから余計な文字を取り払います
std::wstring convertPlainYomi(const std::wstring & str)
{
	std::wstring w = XLWStringUtil::replace(str,L" ",L"");
	w = XLWStringUtil::replace(w,L"-",L"");
	w = XLWStringUtil::replace(w,L"_",L"");
	w = XLWStringUtil::replace(w,L"・",L"");
	w = XLWStringUtil::replace(w,L"=",L"");
	w = XLWStringUtil::replace(w,L"＝",L"");

	return w;
}

//nameのwikiブロックだけとりだします
std::wstring getWikiBlock(const std::wstring& innerW,const std::wstring& name)
{
	int start;
	const std::wstring endKako = L"\n==";
	start = innerW.find(L"\n== " + name + L" ==\n");
	if (start == std::wstring::npos)
	{
		start = innerW.find(L"\n==" + name + L"==\n");
		if (start == std::wstring::npos)
		{
			start = innerW.find(L"\n=== " + name + L" ===\n");
			if (start == std::wstring::npos)
			{
				start = innerW.find(L"\n===" + name + L"===\n");
				if (start == std::wstring::npos)
				{//ないことにする.
					return L"";
				}
			}
		}
	}

	int pos = start+endKako.size()+name.size();
	int endpos;
	while(1)
	{
		endpos = innerW.find(endKako, pos );
		if (endpos  == std::wstring::npos )
		{
			break;
		}
		const int checkpos = endpos+endKako.size();
		if ( (int)innerW.size() < checkpos )
		{
			break;
		}
#ifdef _DEBUG
		auto aaa = innerW.substr(checkpos);
#endif
		if (innerW[checkpos] != L'=')
		{
			break;
		}
		pos = checkpos;
	}

	if (endpos  == std::wstring::npos )
	{
		return L"";
	}
	return  innerW.substr(start,endpos-start);
}

//nameのwikiブロックを消します
std::wstring cutWikiBlock(const std::wstring& innerW,const std::wstring& name)
{
	int start;
	const std::wstring endKako = L"\n==";
	start = innerW.find(L"\n== " + name + L" ==\n");
	if (start == std::wstring::npos)
	{
		start = innerW.find(L"\n==" + name + L"==\n");
		if (start == std::wstring::npos)
		{
			start = innerW.find(L"\n=== " + name + L" ===\n");
			if (start == std::wstring::npos)
			{
				start = innerW.find(L"\n===" + name + L"===\n");
				if (start == std::wstring::npos)
				{//ないことにする.
					return innerW;
				}
			}
		}
	}

	int pos = start+endKako.size()+name.size();
	int endpos;
	while(1)
	{
		endpos = innerW.find(endKako, pos );
		if (endpos  == std::wstring::npos )
		{
			break;
		}
		const int checkpos = endpos+endKako.size();
		if ( (int)innerW.size() < checkpos )
		{
			break;
		}
#ifdef _DEBUG
		auto aaa = innerW.substr(checkpos);
#endif
		if (innerW[checkpos] != L'=')
		{
			break;
		}
		pos = checkpos;
	}

	if (endpos  == std::wstring::npos )
	{
		return innerW.substr(0,start);
	}
	return  innerW.substr(0,start) + innerW.substr(endpos);
}

SEXYTEST()
{
	{
		auto a = cutWikiBlock(L"\n== abc ==\nfoo\ngoo\n===goo===\ngoogle\n==bbb==\na",L"goo");
		assert(a == L"\n== abc ==\nfoo\ngoo\n==bbb==\na");
	}
	{
		auto a = cutWikiBlock(L"\n==abc==\nfoo\ngoo\n==bbb==\na",L"bbb");
		assert(a == L"\n==abc==\nfoo\ngoo");
	}
	{
		auto a = cutWikiBlock(L"\n== abc ==\nfoo\ngoo\n===goo===\ngoogle\n==bbb==\na",L"abc");
		assert(a == L"\n==bbb==\na");
	}
	{
		auto a = cutWikiBlock(L"\n==abc==\nfoo\ngoo\n==bbb==\na",L"abc");
		assert(a == L"\n==bbb==\na");
	}
	{
		auto a = cutWikiBlock(L"\n==abc==\nfoo\ngoo\n==bbb==\na",L"a");
		assert(a == L"\n==abc==\nfoo\ngoo\n==bbb==\na");
	}
	{
		auto a = cutWikiBlock(L"\n== abc ==\nfoo\ngoo\n==bbb==\na",L"abc");
		assert(a == L"\n==bbb==\na");
	}
}
