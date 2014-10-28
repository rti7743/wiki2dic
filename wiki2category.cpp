
#include "common.h"
#include "XLWStringUtil.h"
#include "wiki2chara.h"
#include "wiki2util.h"
#include "XLFileUtil.h"
#include "wiki2yomi.h"
#include "option.h"

static bool isCategoryPage(const std::wstring& titleW)
{
	return titleW.find(L"Category:") != std::wstring::npos;
}
static bool isCategoryPage(const std::string& title)
{
	return title.find("Category:") != std::wstring::npos;
}


bool isWikiWordOnly(const std::wstring& textW,int pos)
{
#ifdef _DEBUG
	auto aaa = textW.substr(pos);
#endif
	if(!(textW[pos]==L'['&&textW[pos+1]==L'['))
	{
		return false;
	}
	int endpos = textW.find(L"]]",pos+2);
	if (endpos == std::wstring::npos)
	{
		return false;
	}
#ifdef _DEBUG
	auto bbb = textW.substr(endpos);
#endif
	if ( (int)textW.size() <= endpos+2)
	{
		return true;
	}
	else if ( textW[endpos+2]=='\n' )
	{
		return true;
	}
	return false;
}

std::wstring killCategory(const std::wstring& str)
{
	std::wstring w;
	int pos = str.find(L"[[Category:");
	if (pos!=std::wstring::npos)
	{
		pos+=sizeof("[[Category:")-1;
		int endpos = str.find(L"]]",pos);
		if (endpos==std::wstring::npos)
		{
			return L"";
		}
		w = str.substr(pos,endpos-pos);
	}
	else
	{
		int pos = str.find(L"Category:");
		if (pos==std::wstring::npos)
		{
			return L"";
		}
		pos+=sizeof("Category:")-1;
		w = str.substr(pos);
	}

	int spaceBarPos = w.find(L' ');
	if (spaceBarPos!=std::wstring::npos)
	{
		if (spaceBarPos+2 < (int)w.size() && ! XLWStringUtil::isAlphanum(w[spaceBarPos+2])  )
		{
			w = w.substr(0,spaceBarPos);
		}
	}

	int killOrBarPos = w.find(L'|');
	if (killOrBarPos!=std::wstring::npos)
	{
		w = w.substr(0,killOrBarPos);
	}
	return w;
}

//同姓同名の解決ページ
static bool isDouseidoumei(const std::wstring& innerW)
{
	return innerW.find(L"[[同姓同名]]") != std::wstring::npos;
}

static void findCategory(const std::wstring& innerW,std::vector<std::wstring>* outVec)
{
	int pos =innerW.find(L'>');
	if (pos == std::wstring::npos)
	{
		pos=0;
	}
	else
	{
		pos++;
	}

	bool loopflag = true;
	while(loopflag)
	{
		int endpos = innerW.find(L'\n',pos);
		if (endpos == std::wstring::npos)
		{
			endpos = innerW.size();
			loopflag = false;
		}

		if (isWikiWordOnly(innerW,pos))
		{
			const std::wstring w = killCategory(innerW.substr(pos,endpos-pos) );
			if (!w.empty())
			{
				outVec->push_back(w);
			}
		}
		pos=endpos+1;
	}
}

static void AnalizeCategory(const std::wstring& titleW,const std::wstring& innerW)
{
	if ( isCategoryPage(titleW) )
	{//カテゴリーのページは処理しない
		return;
	}

	if ( isRedirect(innerW) || isAimai(innerW) || isDouseidoumei(innerW))
	{//リダイレクトor曖昧項目なので無視
		return;
	}

	const std::wstring _TitleW = cleaningInnerText(titleW);
	//曖昧さの解決のための()があったら消します.
	const std::wstring clipTitleW = snipAimaiKako(titleW);
	if (clipTitleW.empty())
	{
		return;
	}

//	XLFileUtil::write("_234.txt",inner);

	std::vector<std::wstring> vec;
	findCategory(innerW,&vec);
	
	
	std::wstring categoryW ;

	//結果の表示.
	for(auto it = vec.begin() ; it != vec.end() ; it ++ )
	{
		categoryW += L"\t"+ (*it);
	}

	
	if ( Option::m()->getShow() == Option::TypeShow_NoEmpty)
	{//空ならば表示しない
		if (categoryW.empty() )
		{
			return;
		}
	}
	else if ( Option::m()->getShow() == Option::TypeShow_Empty)
	{//空だけ表示する
		if ( !categoryW.empty() )
		{
			return;
		}
	}

//	//カタカナひらがな変換.
//	switch ( Option::m()->getCase())
//	{
//	case Option::TypeCase_Kana:	yomiA = XLStringUtil::mb_convert_kana(yomiA,"Hc"); break;
//	case Option::TypeCase_Kata:	yomiA = XLStringUtil::mb_convert_kana(yomiA,"KC"); break;
//	}

	if ( Option::m()->getAimai() == Option::TypeAimai_Del)
	{
		wprintf(L"%ls%ls\n",clipTitleW.c_str() ,categoryW.c_str());
	}
	else
	{
		wprintf(L"%ls%ls\n",_TitleW.c_str() ,categoryW.c_str());
	}
}



static void ReadAllForCategory(FILE* fp)
{
	enum ParseStatusEnum
	{
		 ParseStatusEnum_None
		,ParseStatusEnum_Page
	};
	ParseStatusEnum St;
	St = ParseStatusEnum_None;

	std::string title;
	std::string inner;
	std::wstring titleW;
	std::wstring innerW;
	inner.reserve(65535);	//本文を保持するので多めに確保しておくわ.

	std::vector<char> buferVec(65535);
	char* buffer = &buferVec[0];
	while(!feof(fp))
	{
		if ( fgets(buffer,buferVec.size() - 1,fp) == NULL ) break;

		if (St == ParseStatusEnum_None)
		{
			if ( strstr(buffer , "<page>") )
			{
				title = "";
				inner = "";
				St = ParseStatusEnum_Page;
			}
		}
		else if (St == ParseStatusEnum_Page)
		{
			if ( strstr(buffer , "<title>") )
			{
				title = ParseTitleOneLine(buffer) ;
				titleW = _U2W(title);

				if ( checkGomiPage(titleW) 	
					|| checkIchiran(titleW)  )
				{//いらいないやつだと 次の<page>まで全て無視します.
					St = ParseStatusEnum_None;
				}
			}
			else if ( strstr(buffer , "<text ") || strstr(buffer , "<text>")  )
			{
				inner = buffer;
				while(!feof(fp))
				{
					if ( fgets(buffer,buferVec.size() - 1,fp) == NULL ) break;
					char* p;
					p = strstr(buffer, "</text>");
					if ( p )
					{
						*p = '\0';
						if (*buffer)	inner += buffer;
						break;
					}
					p = strstr(buffer, "</page>");
					if ( p )
					{
						*p = '\0';
						if (*buffer)	inner += buffer;
						break;
					}

					inner += buffer;
				}
			}
			if ( strstr(buffer, "</page>") )
			{
				St = ParseStatusEnum_None;

				innerW = _U2W(inner);
				AnalizeCategory(titleW,innerW);
			}
		}
	}
}

void wiki2categoryConvert(const std::string& filename)
{
	FILE * fp = NULL;

	fp = fopen(filename.c_str() , "rb");
	if (!fp)
	{
		fwprintf(stderr,L"can not open %ls file\n" , _A2W(filename).c_str() );
		return ;
	}
	AutoClear ac([&](){ fclose(fp); });
	
	ReadAllForCategory(fp);
}

SEXYTEST()
{      
}