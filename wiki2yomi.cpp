
#include "common.h"
#include "XLWStringUtil.h"
#include "wiki2yomi.h"
#include "wiki2util.h"
#include "option.h"



//分岐点
static bool isKarano(WCHAR p)
{
	if (p == L'-' || p == L'－'  || p == L'ー' || p == L'～')
	{
		return true;
	}
	return false;
}

//かな漢字の分岐点
//山本太郎           kana:-1   kanji:0
//やまもと太郎           kana:0   kanji:4
//山本たろう           kana:3   kanji:0
//やまもとたろう           kana:0   kanji:-1
static void checkTitleKanaKanji(const std::wstring& titleW,int* outStartKana,int* outStartKanji)
{
	const WCHAR * p = titleW.c_str();
	if (!XLWStringUtil::isKanji(*p))
	{
		*outStartKana = 0;
		for(p=p+1 ; *p ; p++ )
		{
			if ( XLWStringUtil::isKanji(*p)  )
			{
				*outStartKanji = (int)( p- titleW.c_str() );
				return ;
			}
		}
		*outStartKanji = -1;
		return ;
	}
	else
	{
		*outStartKanji = 0;
		for(p=p+1 ; *p ; p++ )
		{
			if ( !XLWStringUtil::isKanji(*p) )
			{
				*outStartKana = (int)( p- titleW.c_str() );
				return ;
			}
		}
		*outStartKana = -1;
		return ;
	}
}


static std::wstring cleaningYomiForWord(const std::wstring& yomi,const std::wstring& word)
{
	int pos = yomi.find(word);
	if (pos != std::wstring::npos)
	{
		if (pos == 0)
		{
			return L"";
		}
		return yomi.substr(0,pos);
	}
	return yomi;
}

//wordで分割される最も長い部分を取る
static std::wstring cleaningYomiForLongWord(const std::wstring& yomi,const std::wstring& word)
{
	std::vector<std::wstring> vec = XLWStringUtil::split_vector(word,yomi);
	if (vec.empty())
	{
		return yomi;
	}

	std::vector<std::wstring>::iterator maxIT = vec.begin();
	size_t maxValue = maxIT->size();
	for(auto it = maxIT+1  ; it != vec.end() ; it++ )
	{
		if ( it->size() > maxValue)
		{
			maxValue = it->size();
			maxIT = it;
		}
	}
	return *maxIT;
}


//（ フェルミめん Fermi surface）と スペースで区切る人がいる
//ただし、すべてのスペースを区切りとすると （ - たろう） がダメになるので 省略記号に配慮した分割をする
static std::wstring cleaningYomiForSpaceSepalater(const std::wstring& yomi)
{
	const WCHAR* p;
	const WCHAR* pp;
	//あたまだし
	for( p = yomi.c_str() ; *p ; p++ )
	{
		if (! XLWStringUtil::isSpace(*p) )
		{
			break;
		}
	}
	const WCHAR* bodystart = p;

	if ( isKarano(*p) )
	{// -たろう パティーン
		for(p = p + 1  ; *p ; p++ )
		{
			if ( XLWStringUtil::isSpace(*p) )
			{//このスペースは終わりなのか・・・？ それとも、 やまだ たろう みたいに名前の途中なのか？ 調べる
				for(pp =  p + 1  ; *pp ; pp++ )
				{
					if ( XLWStringUtil::isSpace(*pp) || XLWStringUtil::isKana(*pp) || XLWStringUtil::isKata(*pp) || isKarano(*pp) )
					{
						continue;
					}
					else 
					{
						break;
					}
				}
				return XLWStringUtil::chop( std::wstring(bodystart,0,pp-bodystart) );
			}
		}
		return XLWStringUtil::chop( std::wstring(bodystart,0,p-bodystart) );
	}
	else
	{// やまだ-  or やまだたろう or やまだ たろう のパテーィン
		for(p = p + 1  ; *p ; p++ )
		{
			if ( XLWStringUtil::isSpace(*p) )
			{//このスペースは終わりなのか・・・？ それとも、 やまだ たろう みたいに名前の途中なのか？ 調べる
				for(pp =  p + 1  ; *pp ; pp++ )
				{
					if ( XLWStringUtil::isSpace(*pp) || XLWStringUtil::isKana(*pp) || XLWStringUtil::isKata(*pp) || isKarano(*pp) )
					{
						continue;
					}
					else 
					{
						break;
					}
				}
				return XLWStringUtil::chop( std::wstring(bodystart,0,pp-bodystart) );
			}
		}
		return XLWStringUtil::chop( std::wstring(bodystart,0,p-bodystart) );
	}
}

//読みが汚れていることがあるので綺麗にします.
static std::wstring cleaningYomi(const std::wstring& titleW,const std::wstring& yomi)
{
	if (yomi.empty() ) return yomi;
	//{{}}が始まるゴミがあったら消します	yomi = "げんご {{lang-la-short|Lingua}}、{{lang-en-short|Language}}"
	std::wstring w = XLWStringUtil::strsnip(yomi,L"{{",L"}}");
//	w = XLWStringUtil::strsnip(w,L"[",L"]");

	w = cleaningYomiForWord(w,L"、");
	if (w.empty())		return L"";

	w = cleaningYomiForWord(w,L"。");
	if (w.empty())		return L"";

	w = cleaningYomiForWord(w,L"；");
	if (w.empty())		return L"";
	
	w = cleaningYomiForWord(w,L";");
	if (w.empty())		return L"";

	w = cleaningYomiForWord(w,L",");
	if (w.empty())		return L"";

	w = cleaningYomiForWord(w,L"，");
	if (w.empty())		return L"";

	w = cleaningYomiForWord(w,L"[");
	if (w.empty())		return L"";

	w = cleaningYomiForWord(w,L"]");
	if (w.empty())		return L"";

	w = cleaningYomiForWord(w,L"&");
	if (w.empty())		return L"";

	w = cleaningYomiForWord(w,L"(");
	if (w.empty())		return L"";

	w = cleaningYomiForWord(w,L"（");
	if (w.empty())		return L"";

	w = cleaningYomiForWord(w,L"&gt;");
	if (w.empty())		return L"";

	w = cleaningYomiForWord(w,L"<");
	if (w.empty())		return L"";

	if ( titleW.size() == 1 && isKanjiOnly(titleW) )
	{//漢字だけで構成されたもので、なぜか読みが ・ 区切りのものがある.
		w = cleaningYomiForLongWord(w,L"・");
		if (w.empty())		return L"";
		w = cleaningYomiForLongWord(w,L"/");
		if (w.empty())		return L"";
		w = cleaningYomiForLongWord(w,L"／");
		if (w.empty())		return L"";
	}
	w = cleaningYomiForWord(w,L"|");
	if (w.empty())		return L"";

	//（ フェルミめん Fermi surface）と スペースで区切る人がいる
	//ただし、すべてのスペースを区切りとすると （ - たろう） がダメになるので 省略記号に配慮した分割をする
	w = cleaningYomiForSpaceSepalater(w);
	if (w.empty())
	{
		return L"";
	}

	w = XLWStringUtil::replace(w,L" " ,L"");
	w = XLWStringUtil::replace(w,L"　" ,L"");
	w = XLWStringUtil::replace(w,L"'" ,L"");
	return w;
}



//記事に書いてあるパティーン
static std::wstring findYomiForInner(const std::wstring&  titleW,const std::wstring&  innerW)
{
	int linkkakoCount = 0;

	int pos = innerW.find(titleW);
	if (pos == std::wstring::npos)
	{
		return L"";
	}
	for( const WCHAR* p = &innerW[pos] + titleW.size() ; *p ; p++ , pos++)
	{
		//[ ] は無条件に読み飛ばします.
		{
			if (*p == L'[')
			{
				linkkakoCount++;
				continue;
			}
			if (*p == L']')
			{
				linkkakoCount--;
				continue;
			}
			if (linkkakoCount >= 1)
			{
				continue;
			}
		}

		if (*p == L'(' || *p == L'（' ||*p == L'「' ||*p == L'【'  )
		{
			//括弧終端まで取得
			const WCHAR* start = p+1;
			for(p=p+1;*p; p++)
			{
				if (*p == L')' ||  *p == L'）' ||*p == L'」' ||*p == L'】'  )
				{
					int len = (int) (p-start);
					if (len <= 0)
					{
						return L"";
					}

					//これが読みらしい.
					return std::wstring(start,0,len);
				}
			}
			//閉じ括弧がない
			return std::wstring(start);
		}
	}
	return L"";
}

//最終手段 記事のとても前部にある部分
static std::wstring findYomiForFirstKako(const std::wstring&  start,const std::wstring&  end,const std::wstring&  innerW)
{
	int pos = innerW.find(start);
	if (pos == std::wstring::npos)
	{
		return L"";
	}

	if (pos >= 100)
	{
		return L"";
	}
	pos += start.size() ;

	int endpos = innerW.find(end,pos);
	if (endpos == std::wstring::npos)
	{
		return L"";
	}

	return innerW.substr(pos,endpos-pos);
}

// | ふりがな = 　　があるパティーン (ただし鉄道系では結構間違っているのでこれを使うのは最後のチャンスかも)
static std::wstring findYomiForFurigana(const std::wstring& titleW,const std::wstring&  innerHeadW)
{
	//ふりがな属性が有るか？
	std::wstring yomi =  getInfoboxStr(innerHeadW,L"ふりがな");
	if (yomi.empty())
	{
		yomi = getInfoboxStr(innerHeadW,L"よみがな");
		if (yomi.empty())
		{
			std::wstring yomi = getInfoboxStr(innerHeadW,L"nativename");
			if (yomi.empty())
			{
				yomi = getInfoboxStr(innerHeadW,L"Nativename");
				if (yomi.empty())
				{
					yomi = getInfoboxStr(innerHeadW,L"nativename");
					if (yomi.empty())
					{//無念
						return L"";
					}
				}
			}
		}
	}

	//汚れているかもなので綺麗にする
	yomi = XLWStringUtil::strsnip(yomi ,L"&lt;",L"&gt;");

	//複数の発音があり、、 ・ などがあったら、最初のみを取る
	int pos = yomi.find(L"、");
	if (pos != std::wstring::npos)
	{
		return yomi.substr(0,pos);
	}
	pos = yomi.find(L"・");
	if (pos != std::wstring::npos)
	{
		return yomi.substr(0,pos);
	}
	pos = yomi.find(L",");
	if (pos != std::wstring::npos)
	{
		return yomi.substr(0,pos);
	}
	pos = yomi.find(L"、");
	if (pos != std::wstring::npos)
	{
		return yomi.substr(0,pos);
	}
	pos = yomi.find(L"，");
	if (pos != std::wstring::npos)
	{
		return yomi.substr(0,pos);
	}
	return yomi;
}

//日本語の文字列だけ(漢字かなカタ)の長さを取ります.
static int NihongoCount(const std::wstring& yomi)
{
	int count = 0;
	if (yomi.empty()) return count;
	for( const WCHAR* p = yomi.c_str() ; *p ; p++ )
	{
		if ( XLWStringUtil::isKanji(*p) || XLWStringUtil::isKata(*p) || XLWStringUtil::isKana(*p))
		{
			count++;
		}
	}
	return count;
}

//カタカナの塊を取得する.
static void getKatahiraKatamari(const std::wstring& titleW,std::vector<std::wstring>* outVec)
{
	std::wstring katamari ;
	if (titleW.empty()) return ;
	for( const WCHAR* p = titleW.c_str() ; *p ; p++ )
	{
		if ( XLWStringUtil::isKata(*p) )
		{
			katamari += *p;
		}
		else
		{
			if (!katamari.empty())
			{
				outVec->push_back(katamari);
				katamari = L"";
			}
		}
	}

	if (!katamari.empty())
	{
		outVec->push_back(katamari);
	}
}
//読みのスキップークを探します.
// -たろう       0
// やまだ-       3
// やまだたろう -1
static int checkYomiSkipmark(const std::wstring& yomi)
{
	if(yomi.size()<=1)
	{//1文字以下なら意味ない.
		return -1;
	}

	for( const WCHAR* p = yomi.c_str() ; *p ; p++ )
	{
		if ( isKarano(*p))
		{
			int pos =  (int)( p - yomi.c_str() );
			if (pos <= 0)
			{//先頭マッチ
				return pos;
			}

			if (*(p+1) != 0)
			{// て-す  みたいに ただのゴミの可能性
				return -1;
			}
			return pos;
		}
	}

	//スキップなし
	return -1;
}

static std::wstring complateYomi(const std::wstring& titleW,int startKana,int startKanji,const std::wstring& yomi,int yomiSkipMark)
{
	assert(yomiSkipMark != -1);
	assert(startKana != -1);

	assert(startKanji != -1);


	if (yomiSkipMark == 0)
	{// やまだ太郎(-たろう) パティーン 
		if (startKanji == 0)
		{//おかしい 漢字が先頭なのだが.
		}
		return titleW.substr(0,startKanji) + XLWStringUtil::chop(yomi.substr(yomiSkipMark+1));
	}
	else
	{// 山田たろう(やまだ-) パティーン
		if (startKanji != 0)
		{//おかしい 漢字が先頭ではないのだが.
		}
		return yomi.substr(0,yomiSkipMark) + XLWStringUtil::chop(titleW.substr(startKana));
	}
}




//正しいそうな読みですか？
static bool checkYomiOrGomi(const std::wstring& titleW,const std::wstring& yomi)
{
	//読みの中に　漢字やアルファベットがあると変だよね.
	if (yomi.empty()) return false;
	for( const WCHAR* p = yomi.c_str() ; *p ; p++ )
	{
		if ( XLWStringUtil::isKanji(*p) )
		{
			return false;
		}
		if ( XLWStringUtil::isAlphanum(*p) )
		{
			return false;
		}
	}

	//スキップマーク入っていますか？
	if( checkYomiSkipmark(yomi)==-1)
	{//スキップマークが入っていない。フルワードが有るはず.

		//元々のタイトルにカタカナ/ひらがなが入っているならば、それが読みにも入っているはず.

		std::wstring titleKA= XLWStringUtil::mb_convert_kana(titleW,L"KC");
		std::wstring yomiKA = XLWStringUtil::mb_convert_kana(yomi,L"KC");

		bool found = false;
		std::vector<std::wstring> vec;
		getKatahiraKatamari(titleW,&vec);
		if (!vec.empty())
		{
			for(auto it = vec.begin() ; it != vec.end() ; it++)
			{
				if (yomiKA.find(*it) != std::wstring::npos)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{//タイトルに入っている　カタカナ/ひらがな部分が、よみにはいっていない.
				return false;
			}
		}
	}

//	const int titleCount = NihongoCount(titleW);
//	const int  yomiCount = NihongoCount(yomi);
//	if (titleCount < yomiCount)
//	{//読みがあまりにも長い場合おかしいよね。
//		return false;
//	}

	//たぶん正しい.
	return true;
}

// 山田太郎 -> '''山田 太郎''' のように 姓名の間に スペースを入れるパテーィンがある
static std::vector<std::wstring> makeSpaceAddData(const std::wstring& titleW)
{
	std::vector<std::wstring> ret;
	
	for( int i = 1; titleW[i] ; i++ )
	{
		ret.push_back( titleW.substr(0,i) + L" " + titleW.substr(i)  );
		ret.push_back( titleW.substr(0,i) + L"　" + titleW.substr(i)  );
	}
	return ret;
}



// | 名前 = 　　が特殊であるパティーン (| 名前 =　藤子 不二雄{{Unicode|&amp;#9398;}} みたいなケース)
static std::wstring findAliasName(const std::wstring& titleHeadW,const std::wstring&  innerW)
{
	std::wstring name = getInfoboxStr(titleHeadW,L"名前");
	if (!name.empty())
	{
		return name;
	}
	name = getInfoboxStr(titleHeadW,L"NAME");
	if (!name.empty())
	{
		return name;
	}
	name = getInfoboxStr(titleHeadW,L"name");
	if (!name.empty())
	{
		return name;
	}
	name = getInfoboxStr(titleHeadW,L"Name");
	if (!name.empty())
	{
		return name;
	}
	return L"";
}



static std::wstring findYomiImpl(const std::wstring&  titleW,const std::wstring&  innerW,const std::wstring&  innerTextW,const std::wstring& innerHeadW)
{
	std::wstring yomi;

	yomi = cleaningYomi(titleW,findYomiForInner(L"'''" + titleW + L"'''", innerTextW));
	if ( checkYomiOrGomi(titleW,yomi) ) return yomi;

	yomi = cleaningYomi(titleW,findYomiForInner(L"''" + titleW + L"''", innerTextW));
	if ( checkYomiOrGomi(titleW,yomi) ) return yomi;

	// 山田太郎 -> '''山田 太郎''' のように 姓名の間に スペースを入れるパテーィンがある
	std::vector<std::wstring> vec = makeSpaceAddData(titleW);
	for(auto it = vec.begin() ; it != vec.end() ; it ++)
	{
		yomi = cleaningYomi(titleW,findYomiForInner(L"'''" + *it + L"'''", innerTextW));
		if ( checkYomiOrGomi(titleW,yomi) ) return yomi;

		yomi = cleaningYomi(titleW,findYomiForInner(L"''" + *it + L"''", innerTextW));
		if ( checkYomiOrGomi(titleW,yomi) ) return yomi;
	}

	yomi = cleaningYomi(titleW,findYomiForFurigana(titleW, innerHeadW));
	if ( checkYomiOrGomi(titleW,yomi) ) return yomi;
	
	return L"";
}
std::wstring findYomi(const std::wstring&  titleW,const std::wstring&  innerW)
{
	//余計な記号を消した本文を作成します.
	std::wstring innerTextW = makeInnerText(innerW);
	//infobox用に頭出しデータを作る
	std::wstring innerHeadW = makeInnerHead(innerW);

	//本文冒頭に読みが書いてあることを期待して探す
	std::wstring yomi = findYomiImpl(titleW,innerW,innerTextW,innerHeadW);
	if(!yomi.empty())
	{
		return yomi;
	}

	// | 名前 = 　　が特殊であるパティーン (| 名前 =　藤子 不二雄{{Unicode|&amp;#9398;}} みたいなケース)
	std::wstring aliasNameW = findAliasName(innerHeadW,innerW);
	if (!(aliasNameW.empty() || aliasNameW == titleW ))
	{
		//別名でもう一度読みを撮れるか挑戦.
		yomi = findYomiImpl(aliasNameW,innerW,innerTextW,innerHeadW);
		if(!yomi.empty())
		{
			return yomi;
		}
	}

	{
		//もうわけわからんので、 '''(YOMI) があれば調べてみます
		yomi = cleaningYomi(titleW,findYomiForInner(L"'''", innerTextW));
		if ( checkYomiOrGomi(titleW,yomi) ) return yomi;
	}

	{
		//それでもだめなら、先頭に来る特徴的な言葉を探す
		yomi = cleaningYomi(titleW,findYomiForFirstKako(L"'''",L"'''", innerTextW));
		if ( checkYomiOrGomi(titleW,yomi) ) return yomi;
	}

	{
		//{{音声ルビ|}}という表現もあるのか (ごろー風)
		yomi = cleaningYomi(titleW,findYomiForFirstKako(L"{{音声ルビ|",L"}}", innerTextW));
		if ( checkYomiOrGomi(titleW,yomi) ) return yomi;
	}
	return L"";
}



static std::wstring AnalizeImpl(const std::wstring& titleW,const std::wstring& innerW)
{
	int startKana,startKanji;
	checkTitleKanaKanji(titleW,&startKana,&startKanji);

	if (startKanji == -1 )
	{
		if(isHiraKataOnly(titleW) )
		{
			//すべてひらがなとカタカナなら、何もしない
			return convertPlainYomi(titleW);
		}
		else
		{
			//本文冒頭に読みが書いてあることを期待して探す
			const std::wstring yomiA = findYomi(titleW,innerW);
			return convertPlainYomi(yomiA);
		}
	}

	//本文冒頭に読みが書いてあることを期待して探す
	std::wstring yomi = findYomi(titleW,innerW);
	if(yomi.empty())
	{//読みが取れなかった
		return L"";
	}

	if ( startKana == -1)
	{//すべて漢字なら読みのとおりだろう.
		return convertPlainYomi(yomi);
	}

	//読みのスキップマークを調べます.
	int yomiSkipMark;
	yomiSkipMark = checkYomiSkipmark(yomi);
	if (yomiSkipMark == -1)
	{//読みのスキップはありません.
		return convertPlainYomi(yomi);
	}

	//読みが -たろう  やまだ-  などと省力してあることがあるので、保管する.
	std::wstring trueYomi = complateYomi(titleW,startKana,startKanji,yomi,yomiSkipMark);
	return convertPlainYomi(trueYomi);
}

static void Analize(const std::wstring& titleW,const std::wstring& innerW)
{
	const std::wstring _TitleW = cleaningInnerText(titleW);
	//曖昧さの解決のための()があったら消します.
	const std::wstring clipTitleW = snipAimaiKako(titleW);
	if (clipTitleW.empty())
	{
		return;
	}
	if (isRedirect(innerW))
	{
		return;
	}

	std::wstring yomiW = AnalizeImpl(clipTitleW,innerW);


	if ( Option::m()->getShow() == Option::TypeShow_NoEmpty)
	{//空ならば表示しない?
		if ( yomiW.empty() )
		{
			return ;
		}
	}
	else if ( Option::m()->getShow() == Option::TypeShow_Empty)
	{//空だけ表示する
		if (! yomiW.empty() )
		{
			return ;
		}
	}

	//カタカナひらがな変換.
	switch ( Option::m()->getCase())
	{
	case Option::TypeCase_Kana:	yomiW = XLWStringUtil::mb_convert_kana(yomiW,L"Hc"); break;
	case Option::TypeCase_Kata:	yomiW = XLWStringUtil::mb_convert_kana(yomiW,L"KC"); break;
	}

	if ( Option::m()->getAimai() == Option::TypeAimai_Del)
	{
		wprintf(L"%ls	%ls\n",yomiW.c_str(),clipTitleW.c_str() );
	}
	else
	{//keep
		wprintf(L"%ls	%ls\n",yomiW.c_str(),_TitleW.c_str() );
	}

}



	
static void ReadAllForYomi(FILE* fp )
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
				title = ParseTitleOneLine( buffer ) ;
				titleW = _U2W(title);

				if ( checkGomiPage(titleW)  
					|| checkIchiran(titleW) 
					|| isAsciiOnlyTitle(titleW.c_str() ) 
					)
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
						p = '\0';
						inner += buffer;
						break;
					}
					p = strstr(buffer, "</page>");
					if ( p )
					{
						p = '\0';
						inner += buffer;
						break;
					}

					inner += buffer;
				}
			}
			if ( strstr(buffer, "</page>") )
			{
				St = ParseStatusEnum_None;

				innerW = _U2W(inner);
				
				if( ! (isRedirect(innerW) ) )
				{
					Analize(titleW,innerW);
				}
			}
		}
	}
}

void wiki2yomiConvert(const std::string& filename)
{
	FILE * fp = fopen(filename.c_str() , "rb");
	if (!fp)
	{
		fwprintf(stderr,L"can not open %ls file\n" , _A2W(filename).c_str() );
		return ;
	}
	AutoClear ac([&](){ fclose(fp); });

	fwprintf(stderr,L"@DEBUG:結果を出力しています\n");
	ReadAllForYomi( fp );
}





SEXYTEST()
{
	std::wstring r;

	{//  /で分けられているパティーン
		r = AnalizeImpl(L"普",L">\n'''普'''（ふ/ぷ）\n\n*[[プロイセン]]とりわけ[[プロイセン王国]]を指す場合が大きい。\n**例：[[普仏戦争]]\n*[[普通]]の略。\n**例：[[普通選挙]]の略である「普選」。[[普通列車]]・[[各駅停車]]を単に「普」と省略する場合もある。\n\n{{aimai}}\n{{デフォルトソート:ふ}}\n");
		assert(r == L"ふ");
	}
	{//間違ったよみ 取れないのが正しい
		r = AnalizeImpl(L"110メートルハードル",L"\n'''110メートルハードル'''は、[[陸上競技]]の[[障害走]]の一種で、10台の[[ハードル]]を跳び越えながら110[[メートル]]を走るタイムを競う競技。主に男子の競技であり、女子では[[100メートルハードル]]が行われる。略して'''トッパ（ー）'''とも呼ばれる。\n");
		assert(r == L"");
	}
	{//取れないのが正しい
		r = AnalizeImpl(L"東京メトロ南北線",L">\n{| {{Railway line header}}\n{{UKrail-header2|[[File:Tokyo Metro logo.svg|17px|東京地下鉄|link=東京地下鉄]] 南北線|#00ada9}}\n|}\n|}\n'''南北線'''（なんぼくせん）は、[[東京都]][[品川区]]の[[目黒駅]]から[[北区 (東京都)|北区]]の[[赤羽岩淵駅]]を結ぶ[[東京地下鉄]]（東京メトロ）が運営する[[鉄道路線]]。[[鉄道要覧]]における名称は'''7号線南北線'''である。\n\n路線名の由来は、東京を南北に貫くことから。車体および路線図や乗り換え案内で使用される[[日本の鉄道ラインカラー一覧|ラインカラー]]は「エメラルドグリーン」、路線記号は'''N'''。[[ファイル:Subway TokyoNamboku.png|21px|南北線]]\n");
		assert(r == L""); 
	}
	{//-ではなくーで省略表記されているパティーン
		r = AnalizeImpl(L"キットカーソン郡",L">\n\n[[Image:Map of Colorado highlighting Kit Carson County.svg|300px|right|Colorado with Kit Carson highlighted]]\n\n'''キットカーソン郡''' (''ーぐん''、'''''Kit Carson County''''') は[[アメリカ合衆国]][[コロラド州]]に位置する[[郡 (アメリカ合衆国)|郡]]である。[[2000年]]現在、人口は8,011人である。ここの[[郡庁所在地]]は[[バーリントン (コロラド州)|バーリントン]]である。\n");
		assert(r == L"キットカーソンぐん");
	}
	{//よみがなで ・　を使って、複数表記している珍しいパティーン
		r = AnalizeImpl(L"乳",L">\n日本語の'''乳'''（ち・ちち）には、次のような用法がある。\n* [[乳房]]のこと<ref name=KouJi1246>{{Cite book|和書|year=1989|title=日本語大辞典|edition=第一刷|publisher=講談社|pages=1246|chapter=【乳】|isbn=4-06-121057-2}}</ref>。\n* [[梵鐘]]の突起状装飾のこと<ref>{{cite web|title=梵鐘 対馬佐護観音堂（長崎）伝来\n|publisher=[[文化庁]]、文化遺産オンライン|url= http://bunka.nii.ac.jp/SearchDetail.do?heritageId=214803 |accessdate=2012-05-30}}</ref>。\n* [[旗]]、[[幕]]、[[暖簾]]、[[草鞋]]などの縁に紐や棒を通すためにつけた小さい輪のこと。[[犬]]の[[乳首]]のように等間隔にあることからこのように呼ばれる。\n* 乳汁のこと。本項で詳述する。\n[[File:Human Breastmilk - Foremilk and Hindmilk.png|thumb|人間の母乳をサンプルとした、初乳（左）と後期乳（右）の比較。]]\n[[File:Milk.jpg|thumb|[[パスチャライゼーション]]を施された[[牛乳]]。]]\n'''乳汁'''（にゅうじゅう、ちちしる）とは、'''乳'''（ちち、にゅう）、'''ミルク'''（{{lang-en-short|milk}}）とも言われる、[[動物]]のうち[[哺乳類]]が[[幼児]]に栄養を与えて育てるために母体が作りだす[[分泌液]]である。特に'''母乳'''（ぼにゅう）と呼ぶ場合は、[[ヒト]]の[[女性]]が出す乳汁を指すのが、慣例である。[[誕生]]後の哺乳類が他の[[食物]]を摂取できるようになるまでの間、[[子供]]の成長に見合った[[栄養]]を獲得できる最初の源となる<ref name=pub>{{cite web|title=牛乳、乳製品の知識|publisher=社団法人日本酪農乳業協会|url= http://www.j-milk.jp/publicities/8d863s0000063ng7-att/8d863s0000063nl9.pdf |format=PDF |accessdate=2012-05-30}}</ref>。");
		assert(r == L"ちち");
	}
	{
		r = AnalizeImpl(L"CentOS",L">{{Infobox OS\n|name = CentOS\n|logo = [[File:Centos_full.svg|250px]]\n|screenshot = [[Image:CentOS 7.0 GNOME.png|300px]]\n|caption = CentOS 7.0の[[GNOME]] デスクトップ環境\n|website = [http://www.centos.org/ www.centos.org]\n|developer = The CentOS Project（[[レッドハット]]後援）\n|family = [[Linux]]\n|source_model = [[FLOSS]]\n|released  = {{start date and age|df=y|2004|05|14|03}}<ref name=\"CentOS2Announcement\">{{cite web | url=http://lists.centos.org/pipermail/centos/2004-May/000153.html | title=CentOS-2 Final finally released | author=John Newbigin | publisher=centos.org | date=2004-05-14 | accessdate=2014-09-30 }}</ref>\n|frequently_updated = yes\n|kernel_type = [[モノリシックカーネル]]\n|ui = [[GNOME]]または[[KDE]]\n|license = [[GNU General Public License|GPL]]\n|working_state = 開発中\n|supported_platforms = [[IA-32]], [[x64]]\n|updatemodel = \n|package_manager = [[RPM Package Manager|RPM]]\n}}\n\n'''CentOS'''（セントオーエス<REF>{{cite web|url=http://itpro.nikkeibp.co.jp/article/COLUMN/20120223/382669/|title=最新OS＆ソフト わくわくインストール - 第1回 サーバー向け無償OSの定番「CentOS 6.2」：ITpro|date=2012/02/28 (JST)|accessdate=2013-04-20 (JST)|author=斉藤 栄太郎|publisher=Nikkei Business Publications}}</REF><REF>{{cite web|url=http://openstandia.jp/oss_info/centos/|title=CentOS 最新情報 | OpenStandia&trade; （オープンスタンディア）|date=|accessdate=2013-04-20 (JST)|author=OpenStandia|publisher=http://openstandia.jp/}}</REF>, <REF group=\"注釈\">[http://www.centos.org/modules/newbb/viewtopic.php?viewmode=flat&topic_id=925&forum=18 公式フォーラム]等を中心に sent-oss（セントス）と発音する例も。</REF>）は、[[Red Hat Enterprise Linux]]（以下「RHEL」と呼ぶ）との完全互換を目指したフリーの[[Linuxディストリビューション]]である。");
		assert(r == L"セントオーエス");
	}
	{
		r = AnalizeImpl(L"地球外生命",L">'''地球外生命'''（{{音声ルビ|ちきゅうがいせいめい}}、{{lang-en-short|extraterrestrial life}}、略称:ET）は、[[地球]][[大気圏]]の外の[[生命]]の総称である。");
		assert(r == L"ちきゅうがいせいめい");
	}
	{
		r = AnalizeImpl(L"金日成",L">{{大統領\n| 人名 = 金日成\n| 各国語表記 = {{Lang|ko|・・ｼ・ｱ}} \n| 画像 = Kim Il Song Portrait-2.jpg\n| 画像サイズ = 200px\n| キャプション = 金日成の公式肖像画\n| 国名 = {{PRK2}}\n| 代数 = 初\n| 職名 = 最高指導者\n| 就任日 = [[1948年]][[9月9日]]\n| 退任日 = [[1994年]][[7月8日]]\n| 副大統領 = \n| 元首 = \n<!-- ↓省略可↓ -->\n| 国名2 = {{PRK2}}\n| 代数2 = 初\n| 職名2 = [[朝鮮民主主義人民共和国主席|国家主席]]\n| 就任日2 = [[1972年]][[12月28日]]\n| 退任日2 = [[1994年]][[7月8日]]\n| 副大統領2 =\n| 元首2 = \n| 国名3 = {{PRK2}}\n| 代数3 = 初\n| 職名3 = [[朝鮮民主主義人民共和国の首相|内閣首相]]\n| 就任日3 = [[1948年]][[9月9日]]\n| 退任日3 = [[1972年]][[12月28日]]\n| 副大統領3 = \n| 元首3 = \n| 国名4 = {{PRK2}}\n| 代数4 = 初\n| 職名4 = [[朝鮮人民軍]]最高司令官\n| 就任日4 = [[1950年]][[7月4日]]\n| 退任日4 = [[1991年]][[12月24日]]\n| 副大統領4 = \n| 元首4 = \n| 国名5 = [[朝鮮労働党]]\n| 代数5 = 初\n| 職名5 = [[朝鮮労働党中央委員会総書記|中央委員会総書記]]\n| 就任日5 = [[1966年]][[10月12日]]\n| 退任日5 = [[1994年]][[7月8日]]\n| 副大統領5 = \n| 元首5 = \n<!-- ↑省略可↑ -->\n| 出生日 = [[1912年]][[4月15日]]\n| 生地 = {{JPN}} [[平安南道 (日本統治時代)|平安南道]][[平壌|平壌府]][[万景台]]\n| 死亡日 = {{死亡年月日と没年齢|1912|4|15|1994|7|8}}\n| 没地 = {{PRK2}}、[[平壌]]\n| 配偶者 = [[金正淑]]<br />[[金聖愛]]\n| 政党 = {{Flagicon|KWP}} [[朝鮮労働党]]\n| サイン = Kim Il Sung Signature.svg\n}}\n{{北朝鮮の人物\n|title=金日成\n|picture-type=\n|picture=\n|caption=\n|hangeul=・・ｼ・ｱ\n|hanja=金日成\n|hiragana=きん・にっせい\n|katakana=キム・イルソン\n|latin=Kim Il-sﾅ熟g\n|alphabet-type=\n|alphabet=Kim Il-sung\n}}\n'''金 日成'''（キム・イルソン、{{Lang|ko|・・ｼ・ｱ}}、[[1912年]][[4月15日]] - [[1994年]][[7月8日]]）は、[[日本統治下の朝鮮|朝鮮]]の[[革命家]]・独立運動家で、[[朝鮮民主主義人民共和国]]（北朝鮮）の[[政治家]]<!--ノート参照-->、[[軍人]]。[[満州]]において[[抗日パルチザン]]活動に部隊指揮官として参加し、[[第二次世界大戦]]後は[[ソビエト連邦]]の支持の下、北朝鮮に朝鮮民主主義人民共和国を建国した。以後、死去するまで同国の最高指導者の地位にあり、[[1948年]]から[[1972年]]までは[[朝鮮民主主義人民共和国の首相|首相]]を、[[1972年]]から死去するまで[[朝鮮民主主義人民共和国主席|国家主席]]を務めた。また、同国の支配政党である[[朝鮮労働党]]の党首（[[1949年]]から[[1966年]]までは中央委員会委員長、1966年以降は[[朝鮮労働党中央委員会総書記|中央委員会総書記]]）の地位に、結党以来一貫して就いていた。\n\n[[称号]]は朝鮮民主主義人民共和国[[大元帥]]・[[英雄称号|朝鮮民主主義人民共和国共和国英雄]]（三回受章しており「三重英雄」と称される）。\n\n北朝鮮においては「偉大なる[[リーダー|首領様]]」などの尊称の下に神格化され、崇拝されている。彼の死後[[1998年]]に改定された[[朝鮮民主主義人民共和国社会主義憲法]]では「永遠の主席」とされ、主席制度は事実上廃止された。現在、遺体は[[平壌]]近郊の[[錦繍山太陽宮殿]]に安置・保存されている。");
		assert(r == L"キムイルソン");
	}
	{
		r = AnalizeImpl(L"京都大学",L"><!--この記事は[[プロジェクト:大学/大学テンプレート (日本国内)]]にしたがって作成されています。-->\n{{大学 \n|国 = 日本\n|大学名 = 京都大学\n|ふりがな = きょうとだいがく\n|英称 = Kyoto University\n|画像 = Kyoto University.jpg\n|大学設置年 = 1897年\n|創立年 = 1869年\n|学校種別 = 国立\n|設置者 = 国立大学法人京都大学\n|本部所在地 = [[京都府]][[京都市]][[左京区]]吉田本町36番地1\n|緯度度 = 35|緯度分 = 1|緯度秒 = 34\n|経度度 = 135|経度分 = 46|経度秒 = 51\n|キャンパス = 吉田（京都府京都市左京区）<br>宇治（京都府[[宇治市]]）<br>桂（京都府京都市[[西京区]]）\n|学部 = 総合人間学部<br>文学部<br>教育学部<br>法学部<br>経済学部<br>理学部<br>医学部<br>薬学部<br>工学部<br>農学部\n|研究科= 文学研究科<br>教育学研究科<br>法学研究科<br>経済学研究科<br>理学研究科<br>医学研究科<br>薬学研究科<br>工学研究科<br>農学研究科<br>人間・環境学研究科<br>エネルギー科学研究科<br>アジア・アフリカ地域研究研究科<br>情報学研究科<br>生命科学研究科<br>地球環境学大学院<br>公共政策大学院<br>経営管理大学院<br>総合生存学館\n|全国共同利用施設 = 学術情報メディアセンター<br>放射線生物研究センター<br>生態学研究センター<br>地域研究統合情報センター<br>野生動物研究センター\n|大学の略称 = 京大（きょうだい）\n|ウェブサイト = http://www.kyoto-u.ac.jp/\n}}");
		assert(r == L"きょうとだいがく");
	}
	{
		r = AnalizeImpl(L"Peer to Peer",L">{{Otheruses|端末間通信技術|P2P技術を利用したファイル共有システム|ファイル共有ソフトウェア}}\n[[ファイル:P2P-network.svg|thumb|200px|P2P型ネットワーク（図はピュアP2P型）。コンピューター同士が対等に通信を行うのが特徴である。]]\n'''ピアトゥピア'''または'''ピアツーピア'''（{{lang-en-short|peer to peer}}）とは、多数の端末間で通信を行う際の[[アーキテクチャ]]のひとつで、対等の者（Peer、ピア）同士が通信をすることを特徴とする通信方式、通信モデル、あるいは、通信技術の一分野を指す。'''P2P'''と[[Leet|略記]]することが多く、以下本記事においてもP2Pとする。");
		assert(r == L"ピアトゥピア");
	}
	{
		r = AnalizeImpl(L"高等師範学校",L">'''高等師範学校'''（こうとうしはんがっこう）:\n#近代の[[日本]]に存在した[[中等学校]]教員養成機関。\n##上記教育機関の総称。本項目で詳述する。\n##[[1886年]]（明治19年）に東京に設置された最初の高等師範学校の校名。[[1890年]]（明治23年）に女子部が[[東京女子高等師範学校|女子高等師範学校]]として独立。[[1902年]]（明治35年）には広島高等師範学校の設置にともない、校名を'''東京高等師範学校'''と改称した。現在の[[筑波大学]]の前身にあたる。　⇒　'''[[東京高等師範学校]]'''\n#[[フランス]]の国立高等教員養成機関'''エコール・ノルマル・シュペリウール'''（Ecole normale superieure）の訳語。　⇒　'''[[高等師範学校 (フランス)]]'''\n-----\n'''高等師範学校'''（こうとうしはんがっこう）は、[[近代]]の[[日本]]に存在した教員養成機関の種別の一つで、[[中等学校]]教員の養成にあたったものをいい、一般には「'''高師'''」（こうし）と略される。");
		assert(r == L"こうとうしはんがっこう");
	}
	{
		r = AnalizeImpl(L"ハト派",L">{{出典の明記|date=2012年8月}}\n'''ハト派'''（ハトは。英語：'''dove, pacifist'''）とは、[[鳩]]が持つ[[平和]]のイメージを政治的傾向の分類に用いたものである。用例によっては{{ルビ|'''穏健派'''|おんけんは}}、もしくは{{ルビ|'''慎重派'''|しんちょうは}}ともいう。[[旧約聖書]]の[[ノアの方舟]]の伝説に基づく。対義語は[[タカ派]]。");
		assert(r == L"ハトは");
	}
	{
		r = AnalizeImpl(L"相原コージ",L">{{Infobox 漫画家|\n| 名前 = 相原　コージ\n| 画像 = \n| 画像サイズ = \n| 脚注 = \n| 本名 = 相原　弘治\n| 生地 = {{JPN}}　[[北海道]][[登別市]]\n| 国籍 = {{JPN}}\n| 生年 = {{生年月日と年齢|1963|5|3}}\n| 没年 = \n| 職業 = [[漫画家]]\n| 活動期間 = [[1983年]] - \n| ジャンル = [[ギャグ漫画]]<br/>[[ストーリー漫画]]\n| 代表作 = 『[[コージ苑]]』<br/>『[[かってにシロクマ]]』\n| 受賞 = \n| 公式サイト = \n}}\n{{漫画}}\n'''相原 コージ'''（あいはら コージ、本名：'''相原 弘治'''<ref name=\"aihara\">日外アソシエーツ発行『漫画家人名事典』（2003年2月、ISBN 9784816917608）P4</ref>、[[1963年]][[5月3日]]<ref name=\"aihara\" /> - ）は、[[日本]]の[[漫画家]]。[[北海道]][[登別市]]出身<ref name=\"aihara\" />。妻の[[両角ともえ]]も[[漫画家]]で、現在は相原の[[アシスタント (漫画)|アシスタント]]（元、[[いしかわじゅん]]のアシスタント）。兄はミュージシャンの相原ピリカ。");
		assert(r == L"あいはらコージ");
	}
	{
		r = AnalizeImpl(L"静岡県",L">{{基礎情報 都道府県\n|都道府県名 = 静岡県\n|画像 = [[ファイル:20091228富士山.jpg|220px|富士山と三保半島]]\n|画像の説明 = [[富士山]]と[[三保の松原|三保半島]]\n|都道府県旗 = [[ファイル:Flag of Shizuoka Prefecture.svg|100px|boder|静岡県旗]]\n|都道府県旗の説明 = [[静岡県旗]]<ref>[[1968年]]（昭和43年）[[8月26日]]制定</ref><ref>{{Cite web|url=https://www.pref.shizuoka.jp/a_content/info/profile/kensei/trademark.html|title=県の概要-県章・県旗|work=静岡県ホームページ|date=|accessdate=2014/07/19}}</ref>\n|区分 = 県\n|コード = 22000-1\n|ISO 3166-2 = JP-22\n|隣接都道府県 = [[神奈川県]]、[[山梨県]]、[[長野県]]、[[愛知県]]\n|木 = [[モクセイ]]（[[キンモクセイ]]）<ref name=\"sym\">{{Cite web|url=https://www.pref.shizuoka.jp/a_content/info/profile/kensei/flowerbird.html|title=県の概要-県の鳥・花・木work=静岡県ホームページ|date=|accessdate=2014/07/19}}</ref>\n|花 = [[ツツジ]]<ref>[[1965年]]（昭和40年）[[9月21日]]選定</ref><ref name=\"sym\" />\n|鳥 = [[サンコウチョウ]]<ref>[[1964年]]（昭和39年）[[10月2日]]制定</ref><ref name=\"sym\" />\n|シンボル名 = 県の歌<hr>県愛唱歌<hr>県民の日\n|歌など = [[静岡県歌]]<hr>[[富士よ夢よ友よ|しずおか賛歌～富士よ夢よ友よ]]<hr>[[8月21日]]\n|知事 = [[川勝平太]]\n|郵便番号 = 420-8601\n|所在地 = 静岡市葵区追手町9番6号<br /><small>{{ウィキ座標度分秒|34|58|36.9|N|138|22|58.8|E|region:JP-22_type:adm1st|display=inline,title}}</small><br />[[ファイル:静岡県庁本館.JPG|220px|静岡県庁]]\n|外部リンク = [http://www.pref.shizuoka.jp/ 静岡県庁]\n|位置画像 = [[ファイル:Map of Japan with highlight on 22 Shizuoka prefecture.svg|320px|静岡県の位置]]{{基礎自治体位置図|22|000}}\n|特記事項 =\n}}\n\n'''静岡県'''（しずおかけん、''{{lang-en-short|Shizuoka Prefecture}}''、''{{lang-fr-short|La prefecture de Shizuoka}}''、''{{lang-es-short|Prefectura de Shizuoka}}''）は、[[太平洋]]に面する、[[日本]]の[[県]]の一つ。[[県庁所在地]]は[[静岡市]]。[[中部地方]]及び[[東海地方]]に含まれる。[[2012年]]（平成24年）現在、人口は約374万人で、[[都道府県の人口一覧|都道府県別]]で、第10位であり、[[静岡市]]と[[浜松市]]の2つの[[政令指定都市]]を有する。");
		assert(r == L"しずおかけん");
	}
	{
		r = AnalizeImpl(L"阿澄佳奈",L">{{声優\n| 名前 = 阿澄 佳奈\n| ふりがな = あすみ かな\n| 画像ファイル =\n| 画像サイズ =\n| 画像コメント =\n| 本名 =\n| 愛称 = あすみん、アスミス\n| 性別 = [[女性]]\n| 出生地 = {{JPN}}・[[福岡県]]\n| 死没地 =\n| 生年 = 1983\n| 生月 = 8\n| 生日 = 12\n| 没年 =\n| 没月 =\n| 没日 =\n| 血液型 = [[ABO式血液型|A型]]\n| 身長 = 161 [[センチメートル|cm]]\n| 職業 = [[声優]]、[[歌手]]\n| 事務所 = [[81プロデュース]]\n| 配偶者 = あり\n| 著名な家族 =\n| 公式サイト =\n| 活動 = {{声優/活動\n  | 職種 = 声優\n  | 活動名義 = \n  | 活動期間 = [[2005年]] - \n  | ジャンル = [[アニメ]]、[[ラジオ]]\n  | デビュー作 = ウェイトレス<ref name=\"声優データファイル2009\">{{Cite journal|和書 |title= 人気声優データファイル2009 |journal= アニメディア |issue= 2009年7月号第1付録 |pages= p.37 |publisher= 学習研究社 |accessdate= 2009-7-20 }}</ref><br/><small>（『[[Canvas2|Canvas2 縲恣Fのスケッチ縲彎]』）</small>\n}}{{声優/音楽活動\n  | 活動名義 = \n  | 活動期間 = [[1999年]] - \n  | ジャンル = [[J-POP]]、[[アニメソング]]\n  | 職種 = [[歌手]]\n  | 担当楽器 = \n  | レーベル = \n  | 共同作業者 = [[小梅伍]]、[[Friends (声優ユニット)|Friends]]、[[LISP (声優ユニット)|LISP]]\n  | 影響 = \n}}}}\n'''阿澄 佳奈'''（あすみ かな、[[1983年]][[8月12日]]<ref>{{Cite web|publisher=TSUTAYA online|url=http://www.tsutaya.co.jp/artist/00516254.html|title=阿澄佳奈 アーティストページ|accessdate=2014-06-09}}</ref> - ）は、[[日本]]の[[声優]]、[[歌手]]。\n\n\n[[福岡県]]出身。[[81プロデュース]]所属。{{VOICE Notice Hidden|冒頭部分に記載する代表作は、編集合戦誘発の原因となりますので、多数の出典で確認できるものに限ってください。[[プロジェクト:芸能人#記事の書き方]]にてガイドラインが制定されていますので、そちらも参照して下さい。}}");
		assert(r == L"あすみかな");
	}


}
