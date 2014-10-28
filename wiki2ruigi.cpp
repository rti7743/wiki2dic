
#include "common.h"
#include "XLWStringUtil.h"
#include "wiki2ruigi.h"
#include "wiki2util.h"
#include "option.h"
#include "XLFileUtil.h"



static bool isKugiriMoji(WCHAR c)
{
	if (c==L'、'
		|| c==L'　'
		|| c==L'，'
		|| c==L','
		|| c==L'；'
		|| c==L';'
		|| c==L'。'
		)
	{
		return true;
	}
	return false;
}

static bool isKugiriMojiForWikiWord(WCHAR c)
{
	if (c==L'|'
		|| c==L'｜'
		)
	{
		return true;
	}
	return false;
}


//区切り文字で文字列パース
static int findKugiriMoji(const std::wstring& str,int pos,int endpos,int* outSplitPos)
{
	int end = endpos;
	if (end == std::wstring::npos)
	{
		end = str.size();
	}
	for(int i = pos ; i < end ; i++)
	{
		if (isKugiriMoji(str[i]))
		{
			if (outSplitPos) *outSplitPos = i;
			return i;
		}
		if (isKugiriMojiForWikiWord(str[i]))
		{
			if (outSplitPos) *outSplitPos = i;
			return i;
		}
		if ( XLWStringUtil::isSpace(str[i]) && str[i+1] == L'/'  && XLWStringUtil::isSpace(str[i+2]) )
		{
			if (outSplitPos) *outSplitPos = i;
			return i+2;
		}
		if ( XLWStringUtil::isSpace(str[i]) && str[i+1] == L'／' && XLWStringUtil::isSpace(str[i+2]) )
		{
			if (outSplitPos) *outSplitPos = i;
			return i+2;
		}
		if (str[i] == L'。') break;	// 。がかかれているとそれ以上取得すると次の文章を巻き込んで不幸なことになるので止める.
	}
	if (outSplitPos) *outSplitPos = (int)std::wstring::npos;
	return (int)std::wstring::npos;
}



// [[ ]] 表記の中での区切り限定
static int findKugiriMojiForWikiWord(const std::wstring& str,int pos,int endpos)
{
	int end = endpos;
	if (end == std::wstring::npos)
	{
		end = str.size();
	}
	for(int i = pos ; i < end ; i++)
	{
		if (isKugiriMojiForWikiWord(str[i])) return i;
		if (str[i] == L'。') break;	// 。がかかれているとそれ以上取得すると次の文章を巻き込んで不幸なことになるので止める.
	}
	return (int)std::wstring::npos;
}


static std::wstring cleanWord(const std::wstring & str)
{
	std::wstring w = str;
	w = XLWStringUtil::replace(w,L"[[",L"");
	w = XLWStringUtil::replace(w,L"]]",L"");
	w = XLWStringUtil::replace(w,L"'''",L"");
	w = XLWStringUtil::strsnip(w,L"(",L")");
	w = XLWStringUtil::strsnip(w,L"（",L"）");
	w = XLWStringUtil::strsnip(w,L"{{",L"}}");

	w = XLWStringUtil::chop(w);

	return w;
}

//ワードとしてふさわしいかどうかチェックする.
static bool checkRuigi(const std::wstring& titleW,const std::wstring& w)
{
	if (w.empty())
	{
		return false;
	}

	//タイトルと同一
	if (titleW == w) 
	{
		return false;
	}

	//ゴミワードと同一
	if ( checkGomiPage(w) || checkIchiran(w)  )
	{
		return false;
	}
					
	//タイトルが英語じゃないのに、全部小文字の英語 (ただの英訳)
	if (!isAplhaOnly(titleW))
	{
		if (isAplhaSmallOnly(w))
		{//英語だけなダメ.
			return false;
		}
	}
	//ただし、文字以上のアルファベットはほしい NHKみたいな略字
	if (isAplhaOnly(w))
	{
		if (w.size() <= 1)
		{//1文字英語はいらぬ.
			return false;
		}
	}
	if ( w.find(L"#") != std::wstring::npos)
	{//#が入っている場合、どっかの項目の一部分を指すので不要.
		return false;
	}
	if ( w.find(L"カテゴリ") != std::wstring::npos)
	{//カテゴリが入っているのも、カテゴリの中での意味しかないので不要.
		return false;
	}

	const std::wstring clipTitleW = snipAimaiKako(titleW);
	const std::wstring clipW = snipAimaiKako(w);
	if (clipTitleW==clipW)
	{
		return false;
	}


	return true;
}


static bool tryInsert(const std::wstring&r,std::vector<std::wstring>* outRuigiVec)
{
	if (r.empty()) return false;

	//すでに愛称を取得できているならば追加しない
	auto it2 = std::find(outRuigiVec->begin(),outRuigiVec->end() , r);
	if ( it2 != outRuigiVec->end())
	{
		return false;
	}
	outRuigiVec->push_back(r);
	return true;
}

void SplitSentence(const std::wstring& str,std::vector<std::wstring>* outVec)
{
	int start = 0;
	for(unsigned int i = 0; i < str.size() ; i ++)
	{
		if ( str[i] == L'。' || str[i] == L'.' )
		{
			const std::wstring m = L" " + XLWStringUtil::chop(str.substr(start,i-start+1)) + L" ";
			if (!m.empty())
			{
				outVec->push_back(m);
			}
			start = i+1;
		}
	}
	{
		const std::wstring m = XLWStringUtil::chop(str.substr(start));
		if (!m.empty())
		{
			outVec->push_back(m);
		}
	}
}


//[[ ]] をパースして、outRuigiVec に格納する.
static bool parseWikiRuigi(const std::wstring &titleW,const std::wstring& str,std::vector<std::wstring>* outRuigiVec)
{
	int pos = 0;
	int endpos = str.size();
	int startKakoPos,start2KakoPos,endKakoPos,end2KakoPos;

	if (! parseKako(str,pos,endpos,true,L"[[",L"]]",&startKakoPos,&start2KakoPos,&endKakoPos,&end2KakoPos) )
	{
		return false;
	}
	const std::wstring match = cleanWord(std::wstring(str,start2KakoPos,endKakoPos - start2KakoPos));

	// | があれば、右側だけを採用する
	int splitPos = match.find(L"|");
	if (splitPos == std::wstring::npos)
	{
		if ( checkRuigi(titleW,match) )
		{
			tryInsert( match ,outRuigiVec);
		}
	}
	else
	{
		const std::wstring w = match.substr(splitPos+1);
		if ( checkRuigi(titleW,w) )
		{
			tryInsert( w ,outRuigiVec);
		}
	}
/*
	//A|Bと正規表現の可能性があるので、 | で切ってみる
	auto stringList = XLWStringUtil::split(L"|",match);
	for(auto it = stringList.begin() ; it != stringList.end() ; it ++ )
	{
		if ( checkRuigi(titleW,it) )
		{
			tryInsert( *it ,outRuigiVec);
		}
	}
*/
	return true;
}


//愛称が複数あることがあるので、調べて、outRuigiVec に格納する.
static bool parseMultipleRuigi(const std::wstring &titleW,const std::wstring& str,std::vector<std::wstring>* outRuigiVec)
{
	std::wstring w;
	int pos = 0;
	int endpos = str.size();
	int startKakoPos,start2KakoPos,endKakoPos,end2KakoPos;
	bool isNewData = false;

	while(pos < endpos)
	{
		if ( parseKakoEx(str,pos,endpos,false,&startKakoPos,&start2KakoPos,&endKakoPos,&end2KakoPos) )
		{
			isNewData = true;
			const std::wstring match = cleanWord(std::wstring(str,start2KakoPos,endKakoPos - start2KakoPos));

			int n;
			if (str[startKakoPos] == L'[' && str[startKakoPos+1] == L'[' )
			{//wiki
				//次の区切り文字へ
				n = findKugiriMojiForWikiWord(str,end2KakoPos , endpos);

				//さらにネストする可能性を調べる
				//parseWikiRuigi(titleW,L"[["+match+L"]]",outRuigiVec);
				parseMultipleRuigi(titleW,match,outRuigiVec);
			}
			else if (str[startKakoPos] == L'\'' )
			{//wiki
				//次の区切り文字へ
				n = findKugiriMoji(str,end2KakoPos , endpos,NULL);

				if ( isKakoStart(str[startKakoPos+3]) )
				{//さらにネストする可能性を調べる
					parseMultipleRuigi(titleW,match,outRuigiVec);
				}
				else
				{
					if ( checkRuigi(titleW,match) )
					{
						tryInsert( match ,outRuigiVec);
					}
				}
			}
			else
			{
				//次の区切り文字へ
				n = findKugiriMoji(str,end2KakoPos , endpos,NULL);
				//さらにネストする可能性を調べる
				parseMultipleRuigi(titleW,match,outRuigiVec);
			}
			if (n == std::wstring::npos)
			{//もう候補はない
				return isNewData;
			}
			//まだ候補がある
			pos = n+1;
		}
		else
		{
			//区切り文字があれば、区切り文字まで取得する.
			int spitPos = endpos;
			int n = findKugiriMoji(str,pos,endpos,&spitPos);
			if (n == std::wstring::npos)
			{//もう候補はない
				isNewData = true;
				const std::wstring match = cleanWord(std::wstring(str,pos,endpos-pos));
				if ( checkRuigi(titleW,match) )
				{
					tryInsert( match ,outRuigiVec);
				}
				return isNewData;
			}

			isNewData = true;
			const std::wstring match = std::wstring(str,pos,spitPos-pos);
			if ( checkRuigi(titleW,match) )
			{
				tryInsert( match ,outRuigiVec);
			}
			//まだ候補がある
			pos = n+1;
		}
	}
	return isNewData;
}

//括弧に囲まれてはいないけど略語っぽい部分
static bool ryakugoFinder(const std::wstring& str,int pos,int endpos,int* outHitStartPos,int* outHitEndPos)
{
#ifdef _DEBUG
	std::wstring aaa = str.substr(pos);
#endif
	if (endpos < pos+3)
	{
		return false;
	}

	int i = pos;
	//略語としては、
	//略語は、
	//略語として、
	if (str[i] == L'と' && str[i+1] == L'し' )
	{
		i+=2;
	}
	if (str[i] == L'は')
	{
		i++;
	}
	if (str[i] == L'、')
	{
		i++;
	}

	//skipword
	int hitStartPos,hitEndPos;
	bool r = XLWStringUtil::firstfindPos(str,pos,endpos,&hitStartPos,&hitEndPos
		,L"語の"
		,L"型の"
		);
	if (r)
	{
		i = hitEndPos + 1;
	}

#ifdef _DEBUG
	std::wstring bbb = str.substr(i);
#endif

	int start = i;
	if (XLWStringUtil::isKata(str[i] ))
	{//カタカナの連続
		for( ; i < endpos ; i ++)
		{
			if (! XLWStringUtil::isKata(str[i] ) )
			{
				break;
			}
		}
	}
	else if (XLWStringUtil::isKanji(str[i] ))
	{//漢字の連続
		for( ; i < endpos ; i ++)
		{
			if (! XLWStringUtil::isKanji(str[i] ) )
			{
				break;
			}
		}
	}
	else if (XLWStringUtil::isAlpha(str[i] ))
	{//アルファベットの連続
		for( ; i < endpos ; i ++)
		{
			if (! XLWStringUtil::isAlpha(str[i] ) )
			{
				break;
			}
		}
	}
	else
	{//略称はない
		return false;
	}

	if ( i - start <= 2 )
	{//2文字以下
		return false;
	}
	if (outHitStartPos) *outHitStartPos = start;
	if (outHitEndPos) *outHitEndPos = i;

	return true;
}

//括弧に囲まれてはいないけど略語っぽい部分 逆方向
static bool ryakugoFinderR(const std::wstring& str,int pos,int endpos,int* outHitStartPos,int* outHitEndPos)
{
#if _DEBUG
	auto aaa = str.substr(pos);
#endif

	int start = pos;
	if ( str[start] == L'、')
	{
		start --;
	}
	if (start < endpos)
	{
		return false;
	}

	int i = start;
	if (XLWStringUtil::isKata(str[i] ))
	{//カタカナの連続
		for( ; i > endpos ; i --)
		{
			if (! XLWStringUtil::isKata(str[i] ) )
			{
				break;
			}
		}
	}
	else if (XLWStringUtil::isKanji(str[i] ))
	{//漢字の連続
		for( ; i > endpos ; i --)
		{
			if (! XLWStringUtil::isKanji(str[i] ) )
			{
				break;
			}
		}
	}
	else
	{//略称はない
		return false;
	}

	if (  start - i <= 2 )
	{//2文字以下
		return false;
	}
	if (outHitStartPos) *outHitStartPos = i+1;
	if (outHitEndPos) *outHitEndPos = start+1;

	return true;
}

// 英略称：SLRI） などなっている例外ケースをここで拾う.
static bool parseRuigiForInner2Koron(const std::wstring& titleW,const std::wstring& str,std::vector<std::wstring>* outRuigiVec,int pos,int endpos)
{
	int i = pos;
	if ( XLWStringUtil::isSpace(str[i] ) )
	{
		i++;
	}
	if(i >= endpos)
	{
		return false;
	}

	bool r;
	if (str[i] == L':' || str[i] == L'：' )
	{
		i++;
		if ( XLWStringUtil::isSpace(str[i] ) )
		{
			i++;
		}

		//括弧が降られていない可能性.
		int hitStartPos,hitEndPos;
		r = ryakugoFinder(str	//対象文字列
			,i				//skipする位置
			,endpos				//終了する位置
			,&hitStartPos		//開始括弧位置
			,&hitEndPos);		//終了括弧をすぎて本文が始まる位置
		if (!r)
		{
			return false;
		}

		const std::wstring match = str.substr(hitStartPos,hitEndPos-hitStartPos);
		r = parseMultipleRuigi(titleW,match,outRuigiVec);
		if (!r)
		{
			return false;
		}

		return true;
	}


	return false;
}

//前方向に現れる略称を拾う
static void parseRuigiForInner2(const std::wstring& titleW,const std::wstring& str,std::vector<std::wstring>* outRuigiVec)
{
	int pos = 0;
	int endpos = str.size();
	int hitStartPos;
	int hitEndPos;

	bool r = XLWStringUtil::firstfindPos(str,pos,endpos,&hitStartPos,&hitEndPos
		,L"略称"
		,L"愛称"
		,L"通称"
		,L"呼称"
		,L"略語"
		,L"隠語"
		,L"英略称"
		);
	if (!r)
	{
		return;
	}

	pos = hitEndPos;
#if _DEBUG
	auto aaa = str.substr(pos);
#endif
	// 英略称：SLRI） などなっている例外ケースをここで拾う.
	if ( parseRuigiForInner2Koron(titleW,str,outRuigiVec,pos,endpos) )
	{
		return ;
	}

	//スキップワード
	r = XLWStringUtil::firstfindPos(str,pos,endpos,&hitStartPos,&hitEndPos
		,L"複数形の"
		,L"男女同形の"
		,L"複数形である"
		,L"男女同形である"
		);
	if (r)
	{
		pos = hitEndPos;
	}

	while(pos < endpos)
	{
#if _DEBUG
		aaa = str.substr(pos);
#endif
		int badHitPos = endpos;
		r = XLWStringUtil::firstfindPos(str,pos,endpos,&badHitPos,NULL
			,L"のひとつであり"
			,L"するため"
			,L"]]などによって"
			);
		if(!r)
		{
			badHitPos = endpos;
		}

		//括弧ぽいのを探す
		r = parseKakoEx(str		//対象文字列
			,pos				//skipする位置
			,endpos				//終了する位置
			,true				//最初の括弧を true->find するか、false->[0]にある
			,&hitStartPos		//開始括弧位置
			,NULL				//開始括弧をすぎて本文が始まる位置
			,NULL				//終了括弧位置
			,&hitEndPos);		//終了括弧をすぎて本文が始まる位置
		if (!r)
		{
			//括弧が降られていない可能性.
			r = ryakugoFinder(str	//対象文字列
				,pos				//skipする位置
				,endpos				//終了する位置
				,&hitStartPos		//開始括弧位置
				,&hitEndPos);		//終了括弧をすぎて本文が始まる位置
			if (!r)
			{
				return;
			}
		}
		pos = hitEndPos;

		if (badHitPos < pos)
		{//悪い単語が、ヒット場所より前にあるなら 再考
			return;
		}


		
		//愛称が複数あることがあるので、調べて、outRuigiVec に格納する.
		std::wstring match = str.substr(hitStartPos,hitEndPos-hitStartPos);
		r = parseMultipleRuigi(titleW,match,outRuigiVec);
		if (!r)
		{
			return;
		}

		//愛称が複数あることがあるので、調べて、outRuigiVec に格納する.
		r = XLWStringUtil::firstfindPos(str,pos,endpos,&hitStartPos,&hitEndPos
			,L"および"
			,L"または"
			,L"、また"
			,L"や"
			,L"、"
			,L"'"
			,L"「"
			,L"『"
			,L"[["
			,L"（"
			,L"("
			);
		if (!r)
		{
			return;
		}
#if _DEBUG
		aaa = str.substr(pos);
#endif
		//ダメなワード
		r = XLWStringUtil::firstfindPos(str,pos,endpos,&badHitPos,NULL
			,L"の"
			,L"を"
			,L"で"
			,L"は"
			);
		if (r)
		{
#if _DEBUG
			auto aaa1 = str.substr(hitEndPos);
			auto aaa2 = str.substr(badHitPos);
#endif
			if (hitEndPos > badHitPos  && hitEndPos - badHitPos >= 2  )
			{//AのBはCというみたいな、そのものを指していないのは無視.
				return;
			}
		}
		r = XLWStringUtil::firstfindPos(str,pos,endpos,&badHitPos,NULL
			,L"の[["
			,L"を[["
			,L"で[["
			,L"は[["
			,L"の「"
			,L"を「"
			,L"で「"
			,L"は「"
			,L"の（"
			,L"を（"
			,L"で（"
			,L"は（"
			,L"の'''"
			,L"を'''"
			,L"で'''"
			,L"は'''"
			);
		if (r)
		{
			if (hitEndPos > badHitPos )
			{//AのBはCというみたいな、そのものを指していないのは無視.
				return;
			}
		}
		r = XLWStringUtil::firstfindPos(str,pos,endpos,&badHitPos,NULL
			,L"とする"
			,L"は、"
			,L"頭文字を"
			,L"頭文字。"
			,L"一般的に"
			,L"とも呼び"
			);
		if (r)
		{
			if (hitEndPos > badHitPos )
			{//AのBはCというみたいな、そのものを指していないのは無視.
				return;
			}
		}


		pos = hitEndPos + 1;
	}
}


static void parseRuigiForInnerR2(const std::wstring& titleW,const std::wstring& str,std::vector<std::wstring>* outRuigiVec)
{
	int pos = str.size();
	int endpos = 0;
	int hitStartPos;
	int hitEndPos;

	bool r = XLWStringUtil::firstrrfindPos(str,pos,endpos,&hitStartPos,&hitEndPos
		,L"略称"
		,L"愛称"
		,L"通称"
		,L"呼称"
		,L"略語"
		,L"隠語"
		,L"と略さ"
		,L"呼ばれる。"
		,L"呼ばれ、"
		,L"呼ばれて"
		,L"呼ばれることもある"
		,L"呼ばれることがある"
		,L"ともいう"
		,L"略記"
		,L"としても知られている。"
		,L"を意味する"
		,L"英略称"
		,L"と名前を変えて"
		);
	if (!r)
	{
		return;
	}
	pos = hitStartPos-1;
#if _DEBUG
	auto aaa = str.substr(pos);
	auto bbb = str.substr(hitEndPos);
#endif

	if (pos-1 < 0 || ( str[pos] == L'、' ) )
	{
		return ;
	}
	if (str[hitEndPos] == L':' || str[hitEndPos] == L'：' )
	{
		return ;
	}
//	if (str[hitStartPos] == L'と')
//	{
//		pos++;
//	}

	while(pos >= endpos)
	{
#if _DEBUG
		aaa = str.substr(pos);
#endif
		//愛称が複数あることがあるので、調べて、outRuigiVec に格納する.

		int badHitPos = pos;
		r = XLWStringUtil::firstrrfindPos(str,pos,endpos,&hitStartPos,&hitEndPos
			,L"および"
			,L"または"
			,L"や"
			,L"という"
			,L"、"
			,L"'"
			,L"」"
			,L"』"
			,L"]]"
			,L"）"
			,L")"
			);
		if (!r)
		{
			return;
		}
		
		//ダメなワード
		r = XLWStringUtil::firstrrfindPos(str,pos,endpos,&badHitPos,NULL
			,L"の"
			,L"を"
			,L"で"
			,L"は"
			,L"："
			);
		if (r)
		{
#if _DEBUG
			aaa = str.substr(badHitPos);
#endif
			if (hitStartPos < badHitPos && badHitPos - hitStartPos >= 3 )
			{//AのBはCというみたいな、そのものを指していないのは無視.
				return;
			}
		}
		r = XLWStringUtil::firstrrfindPos(str,pos,endpos,&badHitPos,NULL
			,L"]]の"
			,L"]]を"
			,L"]]で"
			,L"]]は"
			,L"]]ら"
			,L"'''の"
			,L"'''を"
			,L"'''で"
			,L"'''は"
			,L"」の"
			,L"」を"
			,L"」で"
			,L"」は"
			,L"）の"
			,L"）を"
			,L"）で"
			,L"）は"
			);
		if (r)
		{
#if _DEBUG
			aaa = str.substr(badHitPos);
#endif
			if (hitStartPos < badHitPos )
			{//AのBはCというみたいな、そのものを指していないのは無視.
				return;
			}
		}

		r = XLWStringUtil::firstrrfindPos(str,pos,endpos,&badHitPos,NULL
			,L"とする"
			,L"は、"
			,L"頭文字を"
			,L"一般的に"
			,L"にした"
			,L"とは、"
			,L"を取得しており、"
			,L"作用があり"
			,L"するため"
			);
		if (r)
		{
#if _DEBUG
			aaa = str.substr(badHitPos);
#endif
			if (hitStartPos < badHitPos )
			{//AのBはCというみたいな、そのものを指していないのは無視.
				return;
			}
		}

		pos = hitStartPos;


		//括弧ぽいのを探す
		r = parseKakoREx(str	//対象文字列
			,pos				//skipする位置
			,endpos				//終了する位置
			,true				//最初の括弧を true->find するか、false->[0]にある
			,&hitStartPos		//開始括弧位置
			,NULL				//開始括弧をすぎて本文が始まる位置
			,NULL				//終了括弧位置
			,&hitEndPos);		//終了括弧をすぎて本文が始まる位置
		if (!r)
		{
			//括弧が降られていない可能性.
			r = ryakugoFinderR(str	//対象文字列
				,pos				//skipする位置
				,endpos				//終了する位置
				,&hitStartPos		//開始括弧位置
				,&hitEndPos);		//終了括弧をすぎて本文が始まる位置
			if (!r)
			{
				return;
			}
		}
		if (pos - hitEndPos >= 11)
		{//あまりに離れすぎていないか.
			pos = hitEndPos-5; //再考せよ.
			continue;
		}
		if (hitStartPos < badHitPos)
		{//悪いのの後に[[]]かあるらしい
			pos = badHitPos; //再考せよ.
			continue;
		}

		//愛称が複数あることがあるので、調べて、outRuigiVec に格納する.
		std::wstring match = str.substr(hitStartPos,hitEndPos-hitStartPos);
		parseMultipleRuigi(titleW,match,outRuigiVec);

		pos = hitStartPos-1;
	}
}

//この曖昧さの解決の候補を採用していいか判別する.(true->ゴミ)
static bool checkGomiAimai(const std::wstring& str)
{
	std::wstring w = str;
	//() [] 内に誤爆しないように
	w = XLWStringUtil::strsnip(w,L"(",L")");
	w = XLWStringUtil::strsnip(w,L"[",L"]");

	int n = w.find(L"登場人物");
	if (n != std::wstring::npos)
	{
		return true;
	}
	n = w.find(L"架空の");
	if (n != std::wstring::npos)
	{
		return true;
	}
	n = w.find(L"シングル");
	if (n != std::wstring::npos)
	{
		return true;
	}
	n = w.find(L"アルバム");
	if (n != std::wstring::npos)
	{
		return true;
	}
	n = w.find(L"収録曲");
	if (n != std::wstring::npos)
	{
		return true;
	}
	n = w.find(L"機能。");
	if (n != std::wstring::npos)
	{
		return true;
	}
	n = w.find(L"における");
	if (n != std::wstring::npos)
	{
		return true;
	}
	n = w.find(L"登場する");
	if (n != std::wstring::npos)
	{
		return true;
	}
	n = w.find(L"内の");
	if (n != std::wstring::npos)
	{
		return true;
	}
	n = w.find(L"小説。");  //小説家はOKだけど、 小説内のキャラはダメ.
	if (n != std::wstring::npos)
	{
		return true;
	}
	n = w.find(L"記事。");
	if (n != std::wstring::npos)
	{
		return true;
	}
	n = w.find(L"曲。"); //楽曲を消したい。 
	if (n != std::wstring::npos)
	{
		return true;
	}
	n = w.find(L"話の");
	if (n != std::wstring::npos)
	{
		return true;
	}
	n = w.find(L"各地にある");
	if (n != std::wstring::npos)
	{
		return true;
	}
	/*
	//の曲　とか の名前 とかの の○○ を消す
	n = w.find(L"の");
	if (n != std::wstring::npos)
	{
		if ( XLWStringUtil::isKanji( w[n+1] ) )
		{
			//


			return true;
		}
	}
	*/
	n = w.find(L"の曲");
	if (n != std::wstring::npos)
	{
		return true;
	}
	n = w.find(L"の名前");
	if (n != std::wstring::npos)
	{
		return true;
	}

	return false;
}


//喩え話、一般論などの紛らわしい文章は消す。
//間違うぐらいならヒットするな。というポリシーで。
static bool Tatoebanashi(const std::wstring& titleW,const std::wstring& str)
{
	const std::wstring strW[] = {
		 titleW+L"の"
		,titleW+L"を"
		,titleW+L"と、"
		,titleW+L"と[["
		,titleW+L"ではない"
		,titleW+L"と違"
		,L""	//番兵
	};
	for(const std::wstring* p = strW ; !p->empty() ; p++)
	{
		if ( str.find(*p) != std::wstring::npos)
		{
			return true;
		}
	}

	const WCHAR* strW2[] = {
		 L"を利用した"
		,L"らと"
		,L"呼ばれ、"
		,L"呼ばれており"
		,L"呼んでおり"
		,L"たとえば"
		,L"例えば"
		,L"のうち、"
		,L"のなかで"
		,L"を指すが"
		,L"全般を指す"
		,L"広義"
		,L"一般的には"
		,L"正確には"
		,L"]]に対する"
		,L"の値を意味"
		,L"これらの場合"
		,L"機構の名称"
		,L"]]の犠牲者"
		,L"]]に関する"
		,L"]]の関係者"
		,L"]]の被害者"
		,L"などのこと"
		,L"などの事"
		,L"などでは"
		,L"では、"
		,L"こともある"
		,L"こちらは"
		,L"ばあい"
		,L"]]内"
		,L"]]における"
		,L"]]での"
		,L"]]の[["
		,L"]]などでは"
		,L"]]では"
		,L"]]にも"
		,L"]]が"
		,L"</math>"			//数学の記事では  という。 という単語が山のように出てくるので相手にしない。
		,L"&lt;/math&gt;"   //内容もわけわかんないこと書いているしw 日本語でok
		,L"住民は"
		,L"市民は"
		,L"住民呼称は"
		,L"読者側の"
		,L"した[["
		,L"からなる"
		,L"を指すのが通例"
		,L"始まりである"
		,L"関数部分"
		,L"愛称区間"	//駅で路線の愛称区間を取れてもね
		,L"こう呼ぶケースがある"
		,L"性差を含まない呼称"	//フェミニストバトルは他所でやれ.
		,L"このことからしばしば"
		,L"]]の用語で"
		,L"]]用語で"
		,L"語化した形であり"
		,L"圏]]で"	//翻訳がほしいわけではない
		,L"圏]]の"	//翻訳がほしいわけではない
		,L"国]]の"	//翻訳がほしいわけではない
		,L"国]]で"	//翻訳がほしいわけではない
		,L"語]]で"	//翻訳がほしいわけではない
//		,L"語]]："	//翻訳がほしいわけではない
//		,L"語]]:"	//翻訳がほしいわけではない
		,L"語略称"	//翻訳がほしいわけではない
		,L"]]での"	//翻訳がほしいわけではない
		,L"海外では"//翻訳がほしいわけではない
		,L"語で[["	//翻訳がほしいわけではない
		,L"なくなり"	//過去
		,L"以前"		//過去
		,L"いた。"		//過去
		,L"を使った"	//過去
		,L"のように"	//過去
		,L"過去"		//過去
		,L"なった"		//過去
		,L"略称であり、"	//AはBの略称でありCではない のような、面倒な文章は消す.
		,L"愛称であり、"
		,L"通称であり、"
		,L"呼称であり、"
		,L"略語であり、"
		,L"隠語であり、"
		,L"略であり、"
		,L"これに対し"	//ご丁寧に対義語が書かれているケース
		,L"反対に"
		,L"対義語"
		,L"逆に"
		,L"自伝"		//旅人といかいう紛らわしいタイトルの自伝を出した人を恨め
		,L"]]に表示される"	//すり で刷で本を参照とかで飛ばさないでほしい。
		,L"との区別のため"
		,L"と区別のため"
		,NULL
	};
	for(const WCHAR** p = strW2 ; *p ; p++)
	{
		if ( str.find(*p) != std::wstring::npos)
		{
			return true;
		}
	}
	
	return false;
}

//喩え話、一般論などの紛らわしい文章は消す。
//間違うぐらいならヒットするな。というポリシーで。
static bool TatoebanashiForAimai(const std::wstring& titleW,const std::wstring& str)
{
	const WCHAR* strW2[] = {
		 L"これに対し"	//ご丁寧に対義語が書かれているケース
		,L"反対に"
		,L"対義語"
		,L"逆に"
		,L"自伝"		//旅人といかいう紛らわしいタイトルの自伝を出した人を恨め
		,L"]]に表示される"	//すり で刷で本を参照とかで飛ばさないでほしい。
		,NULL
	};
	for(const WCHAR** p = strW2 ; *p ; p++)
	{
		if ( str.find(*p) != std::wstring::npos)
		{
			return true;
		}
	}
	
	return false;
}


//[[ ]]を参照などと書かれていれば、それを取得する.
static bool parseAimaiForReferenceBefore(const std::wstring& titleW,const std::wstring& str,std::vector<std::wstring>* outAimaiVec)
{
	//を参照 の直前の項目を取得する.
	//↓日常でこーゆーのがあるんだよ・・・
	//* [[鎌倉時代]]の豪族・[[日蓮宗]]の僧。[[富木常忍]]を参照。
	int pos,matchend;
	bool r =XLWStringUtil::firstfindPos(str,0,str.size(),&pos,&matchend,
			 L"]]のこと"
			,L"]]の事"
			,L"]]の愛称"
			,L"]]の略称"
			,L"]]の名称"
			,L"]]を参照のこと"
			,L"]]を参照"
			,L"]]の意"
			,L"]]を示す"
			,L"]]の呼称"
			,L"]]の別名"
			,L"]]の名前"
			,L"]]と呼称"
			);
	if (!r)
	{//参照ワードが入っていない.
		return false;
	}
#if _DEBUG
	auto aaa = str.substr(matchend);
#endif
	if (! (str[matchend] == L'\0' || str[matchend] == L'。' || str[matchend] == L'.' || str[matchend] == L'\n' || XLWStringUtil::isSpace(str[matchend])) )
	{//終端していない
		return false;
	}

	//カッコ内のパース
	int startKakoPos,endKakoPos;
	r = parseKakoREx(str	//対象文字列
		,pos                            //skipする位置
		,0                         //終了する位置
		,false             //最初の括弧を true->find するか、false->[0]にある
		,&startKakoPos				//開始括弧位置
		,NULL				//開始括弧をすぎて本文が始まる位置
		,NULL					//終了括弧位置
		,&endKakoPos);				//終了括弧をすぎて本文が始まる位置
	if (!r)
	{
		return false;
	}
	std::wstring match = str.substr(startKakoPos,endKakoPos-startKakoPos);
	parseWikiRuigi(titleW,match,outAimaiVec);
	return true;
}

//[[ ]]などを参照  などと書かれていれば、それを取得する.
static bool parseAimaiForReferenceBeforeMulti(const std::wstring& titleW,const std::wstring& str,std::vector<std::wstring>* outAimaiVec)
{
	//こういうものを取得。誤爆に注意.
	//[[計画]]、[[予定]]、[[スキーム]]、[[スケジュール]]、[[デザイン]]、[[プラン]]、[[プロジェクト]]、[[プロット]]なども参照
	int pos,matchend,matchstart;
	bool r =XLWStringUtil::firstfindPos(str,0,str.size(),&matchstart,&matchend,
			 L"]]なども参照"
			);
	if (!r)
	{//参照ワードが入っていない.
		return false;
	}
#if _DEBUG
	auto aaa = str.substr(matchend);
#endif
	bool captured= false;
	pos = matchstart;//+2は]]

	// [[]]、[[]]、
	while(1)
	{
#if _DEBUG
		auto bbb = str.substr(pos);
#endif
		r = parseKakoR(str	//対象文字列
			,pos                            //skipする位置
			,0                              //終了する位置
			,false             //最初の括弧を true->find するか、false->[0]にある
			,L"[["		//開始括弧
			,L"]]"		//終了括弧
			,&matchstart				//開始括弧位置
			,NULL				//開始括弧をすぎて本文が始まる位置
			,NULL					//終了括弧位置
			,&matchend);				//終了括弧をすぎて本文が始まる位置
		if (!r)
		{
			break;
		}

		std::wstring match = str.substr(matchstart,matchend-matchstart);
		parseWikiRuigi(titleW,match,outAimaiVec);

		captured = true;

		if (matchstart <= 0)
		{
			break;
		}
		if (str[matchstart-1]!= L'、')
		{
			break;
		}
		if ( ! (str[matchstart-2]== L']'&&str[matchstart-3]== L']'))
		{
			break;
		}
		pos =matchstart-3;
	}
	return captured;
}

//[[ ]]を参照などと書かれていれば、それを取得する.
static bool parseAimaiForReferenceAfter(const std::wstring& titleW,const std::wstring& str,std::vector<std::wstring>* outAimaiVec)
{
	//を参照。[[ の直後の項目を取得する.
	//* 営業に関連した人脈やネットワークビジネスのこと。[[連鎖販売取引]]。
	int pos;
	bool r =XLWStringUtil::firstfindPos(str,0,str.size(),&pos,NULL,
			 L"のこと。[["
			,L"の事。[["
			,L"の愛称。[["
			,L"の略称。[["
			,L"の名称。[["
			,L"を参照。[["
			,L"の意。[["
			,L"を示す。[["
			,L"の呼称。[["
			,L"の別名。[["
			,L"の名前。[["
			,L"と呼称。[["
			);
	if (!r)
	{//参照ワードが入っていない.
		return false;
	}

	//カッコ内のパース
	int endKakoPos;
	r = parseKakoEx(str	//対象文字列
		,pos                            //skipする位置
		,str.size()                         //終了する位置
		,false             //最初の括弧を true->find するか、false->[0]にある
		,NULL				//開始括弧位置
		,NULL				//開始括弧をすぎて本文が始まる位置
		,NULL					//終了括弧位置
		,&endKakoPos);				//終了括弧をすぎて本文が始まる位置
	if (!r)
	{
		return false;
	}
	std::wstring match = str.substr(pos,endKakoPos-pos);
	parseWikiRuigi(titleW,match,outAimaiVec);
	return true;
}

//* [[ ]] としか書かれていない内容の取得
static bool parseAimaiForReferenceOne(const std::wstring& titleW,const std::wstring& str,std::vector<std::wstring>* outAimaiVec,const std::wstring& innerW,int endpos)
{
	//を参照 の直前の項目を取得する.
	//こんな風にしか書かれていないものをパース
	//* [[鎌倉時代]]
	int pos;
	int asterCount=1;//\n*で切っているので*がひとつ隠れています.
	for(pos=0;pos< (int)str.size() ; pos++)
	{
		if (XLWStringUtil::isSpace(str[pos]))
		{
			continue;
		}
		if (str[pos] == L'*')
		{
			asterCount++;
			continue;
		}
		break;
	}

	if (pos>=(int)str.size())
	{
		return false;
	}

	if (str[pos] != L'[')
	{
		return false;
	}
	bool r;

	//カッコ内のパース
	int endKakoPos;
	r = parseKakoEx(str	//対象文字列
		,pos                            //skipする位置
		,str.size()                         //終了する位置
		,false             //最初の括弧を true->find するか、false->[0]にある
		,NULL				//開始括弧位置
		,NULL				//開始括弧をすぎて本文が始まる位置
		,NULL					//終了括弧位置
		,&endKakoPos);				//終了括弧をすぎて本文が始まる位置
	if (!r)
	{
		return false;
	}
	//endKakoPosのあとにはなにもないことを確認
	int i;
	for(i=endKakoPos+1;i< (int)str.size() ; i++)
	{
		if (XLWStringUtil::isSpace(str[pos]))
		{
			continue;
		}
		if (str[pos] == L'\n')
		{
			continue;
		}
		break;
	}
	if (i < (int)str.size())
	{// ]]のあとに文章が続くのでダメ

		//	ただし、こういう行はセーフにします. ]]
		//* [[アメリカ合衆国商務省産業安全保障局]] (Bureau of Industry and Security)\n

		if (! (isKakoStart( str[i] ) && str[i] != L'['))
		{
			return false;
		}

		//直後括弧でその括弧が\nまで続いているならセーフ
		int endNestKakoPos;
		r = parseKakoEx(str	//対象文字列
			,i                            //skipする位置
			,str.size()                         //終了する位置
			,false             //最初の括弧を true->find するか、false->[0]にある
			,NULL				//開始括弧位置
			,NULL				//開始括弧をすぎて本文が始まる位置
			,NULL					//終了括弧位置
			,&endNestKakoPos);				//終了括弧をすぎて本文が始まる位置
		if (!r)
		{
			return false;
		}
		//今度こそ終端にいってる？
		for(i=endNestKakoPos+1;i< (int)str.size() ; i++)
		{
			if (XLWStringUtil::isSpace(str[pos]))
			{
				continue;
			}
			if (str[pos] == L'\n')
			{
				continue;
			}
			break;
		}
		if (i < (int)str.size())
		{// ダメでした。 （）のあと文章が続くらしい
			return false;
		}
		//セーフ.
	}

	//ただし、こんな風に、箇条書きにするやつがいる。かんべんしてくれよ
	//* [[千葉県]]
	//** [[千葉市立高浜中学校]]
	//* [[神奈川県]]
	//** [[藤沢市立高浜中学校]]
	{
		//次の秒には何個アスター(*)があるのか調べます。
		int nextLineAster=0;
		for(i = endpos+1 ; i < (int)innerW.size() ; i++)
		{
#ifdef _DEBUG
			auto aaa = innerW.substr(i);
#endif
			if (XLWStringUtil::isSpace(innerW[i]) || innerW[i] == L'\n')
			{
				continue;
			}
			if (innerW[i] == L'*')
			{
				nextLineAster++;
				continue;
			}
			break;
		}

		//現在の行よりアスターが増えているならば、いまの行は無視します。
		if (nextLineAster>asterCount)
		{
			return false;
		}
	}


	std::wstring match = str.substr(pos,endKakoPos-pos);
	parseWikiRuigi(titleW,match,outAimaiVec);
	return true;
}



static bool parseAimaiForString(const std::wstring& titleW,const std::wstring& str,std::vector<std::wstring>* outAimaiVec)
{
	bool r;
	int pos =0;
	int matchstart,matchend;
	bool captured = false;

	while(1)
	{
		if(captured == false)
		{//最初なので、 [[]] の位置を探す
			r = parseKako(str	//対象文字列
				,pos                            //skipする位置
				,str.size()                              //終了する位置
				,true             //最初の括弧を true->find するか、false->[0]にある
				,L"[["		//開始括弧
				,L"]]"		//終了括弧
				,&matchstart				//開始括弧位置
				,NULL				//開始括弧をすぎて本文が始まる位置
				,NULL					//終了括弧位置
				,&matchend);				//終了括弧をすぎて本文が始まる位置
		}
		else
		{//2度以降なので、[[]]の場所は探さない
			r = parseKako(str	//対象文字列
				,pos                            //skipする位置
				,str.size()                              //終了する位置
				,false             //最初の括弧を true->find するか、false->[0]にある
				,L"[["		//開始括弧
				,L"]]"		//終了括弧
				,&matchstart				//開始括弧位置
				,NULL				//開始括弧をすぎて本文が始まる位置
				,NULL					//終了括弧位置
				,&matchend);				//終了括弧をすぎて本文が始まる位置
		}
		if (!r)
		{
			break;
		}
		if (captured == false)
		{
			//次も続かないといけない
			if (str[matchend]!= L'、')
			{
				break;
			}
			if ( ! (str[matchend+1]== L'['&&str[matchend+2]== L'['))
			{
				break ;
			}
		}
		std::wstring match = str.substr(matchstart,matchend-matchstart);
		parseWikiRuigi(titleW,match,outAimaiVec);

		//次も続くかな？
		if (str[matchend]!= L'、')
		{
			break;
		}
		if ( ! (str[matchend+1]== L'['&&str[matchend+2]== L'['))
		{
			break ;
		}
		pos =matchend+1;

		captured  =true;
	}
	return captured;
}

//↓ - で、項目と説明をくくれるなら、ハイフンまでにする.
//* 高山マリア - ライトノベル・テレビアニメ『[[僕は友達が少ない]]』の登場人物。
static int findAimaiFirstSeparate(const std::wstring& innerW,int pos,int endpos)
{
	//  / (-|－|：|:)/ がある最初の位置を探します. 

	int minmatch = INT_MAX;
	int a1 = innerW.find(L"-",pos);
	if (a1 != std::wstring::npos && a1 < endpos && a1 < minmatch )
	{
		if (a1>=0 && XLWStringUtil::isSpace(innerW[a1-1]) )
		{
			minmatch = a1;
		}
	}

	int a2 = innerW.find(L"－",pos);
	if (a2 != std::wstring::npos && a2 < endpos && a2 < minmatch )
	{
		if (a2>=0 && XLWStringUtil::isSpace(innerW[a2-1]) )
		{
			minmatch = a2;
		}
	}

	int a3 = innerW.find(L":",pos);
	if (a3 != std::wstring::npos && a3 < endpos && a3 < minmatch )
	{
		if (a3>=0 && XLWStringUtil::isSpace(innerW[a3-1]) )
		{
			minmatch = a3;
		}
	}

	int a4 = innerW.find(L"：",pos);
	if (a4 != std::wstring::npos && a4 < endpos && a4 < minmatch )
	{
		if (a4>=0 && XLWStringUtil::isSpace(innerW[a4-1]) )
		{
			minmatch = a4;
		}
	}

	if (minmatch == INT_MAX)
	{//どれにもヒットしなかったら、ねーよを返す.
		return (int)std::wstring::npos;
	}
	return minmatch;
}

//↓ ⇒ で、項目を後ろに書く奴がいるので、それを調べる.
//**[[国家]]の学習計画のこと。 ⇒ [[カリキュラム]]を参照のこと。
static int findAimaiBackSeparate(const std::wstring& innerAimaiW,int pos,int endpos)
{
	//  / (⇒|→|->)/ がある最初の位置を探します. 

	int minmatch = INT_MAX;
	int a1 = innerAimaiW.find(L"⇒",pos);
	if (a1 != std::wstring::npos && a1 < endpos && a1 < minmatch )
	{
		minmatch = a1 + 1;
	}

	int a2 = innerAimaiW.find(L"→",pos);
	if (a2 != std::wstring::npos && a1 < endpos && a2 < minmatch )
	{
		minmatch = a2 + 1;
	}

	int a3 = innerAimaiW.find(L"->",pos);
	if (a3 != std::wstring::npos && a1 < endpos && a3 < minmatch )
	{
		minmatch = a3 + 2;
	}

	if (minmatch == INT_MAX)
	{//どれにもヒットしなかったら、ねーよを返す.
		return (int)std::wstring::npos;
	}
	return minmatch;
}

// * [[A]] という単純なキーワード構造ですか？
static bool isAimaiSimpleKeyWord(const std::wstring& innerAimaiW,int pos,int startpos)
{
	int i = pos;
	for( ; i >= startpos ; i-- )
	{
		if ( ! XLWStringUtil::isSpace(innerAimaiW[i]) )
		{
			break;
		}
	}

	if (i - 2 < startpos) return false;
#if _DEBUG
	auto aaa= innerAimaiW.substr(pos);
	auto bbb= innerAimaiW.substr(i);
	auto ccc= innerAimaiW.substr(i-2);
#endif
	// [foo] - aa
	if (innerAimaiW[i] == L']' && innerAimaiW[i-1] == L']')
	{// [foo] - aa みたいなパティーンを見る
	 //ただし、↓こういう書き方をする人がいるので、 "*[[ ]] -" であることを確実に確かめないとダメだ!
     //*[[熊本県]][[熊本市]] - [[熊本市水の科学館]]を参照

		//最初の * を探す. これまでに ] が現れなければ成功.
		for( i = i - 2; i >= startpos ; i-- )
		{
			if(innerAimaiW[i] == L']')
			{//2重に ] が現れた.  * [[A]][[B]] - foo のパティーン
				return false; //死んでしまえ。二度と来んな.
			}
			if (innerAimaiW[i] == L'*')
			{
				break;
			}
		}
		//とりあえず、普通の * [[A]] -  形式っぽい.
		return true;
	}
	return false;
}

//- を後ろに入れるパティーンが有る。やめてくれよ。そんなパターンかどうかを調べる.
//*[[茨城県]][[久慈郡]][[大子町]]にある[[天台宗]]の寺院。 - [[日輪寺 (大子町)]]
static bool isAimaiBackWord(const std::wstring& titleW,const std::wstring& innerAimaiW,int pos,int endpos,int hyphenpos)
{
#if _DEBUG
	auto snipW = innerAimaiW.substr(pos);
	auto snip1W = innerAimaiW.substr(endpos);
	auto snip2W = innerAimaiW.substr(hyphenpos);
#endif
	assert(hyphenpos >= pos);
	assert(endpos >= pos);

	//こういう例があるのでちゃんと調べる
	//* [[アレクサンダー・ヒュー・ホームズ・スチュアート]] - アメリカの政治家。
	if (hyphenpos <= 1)
	{
		return false;
	}
	//連続するスペースを飛ばす.
	int i = hyphenpos - 1;

#ifdef _DEBUG
	auto snip3W = innerAimaiW.substr(i);
#endif
	//タイトルの項目が入っている方はどっち？
	int titleInsidePos = innerAimaiW.find(titleW,pos);
	if (titleInsidePos  == std::wstring::npos)
	{//ヒットしない場合は、念のため曖昧さの解決を場してテストする.
		const std::wstring clipTitleW = snipAimaiKako(titleW);
		titleInsidePos = innerAimaiW.find(clipTitleW,pos);
	}
	if (titleInsidePos  != std::wstring::npos && titleInsidePos < endpos)
	{
		//↓こういうのがあるので、 * [[ ]] - が正義とは限らないらしい。 
		//  タイトルが入っている方にある分割地点が正しいと判別する.
		//* [[エジプト]] - [[2012年エジプト大統領選挙|2012年]]

		if (titleInsidePos > hyphenpos)
		{//どうやら後ろのワードの方にタイトルがあるようだ
			return true;
		}
		else
		{
			return false;
		}
	}


	// * [[A]] という単純なキーワード構造ですか？
	if (isAimaiSimpleKeyWord(innerAimaiW,i,pos))
	{
		return false;
	}


	//それ以外は、キーワードが後ろにあるとみなす
	return true;
}

//キーワードは分離されているか
static bool parseAimaiOneLineIsSplitKeyWord(const std::wstring& snipW)
{
	const bool r =
			XLWStringUtil::firstfindPos(snipW,0,snipW.size(),NULL,NULL
			,L"]][["
			,L"]]'''[["
			,L"]]『[["
			,L"]]『'''[["
			,L"]]「[["
			,L"]]「'''[["
			,L"]]（[["
			,L"]]（'''[["
			,L"]]([["
			,L"]]('''[["
			,L"]]の"
			,L"]]にある"
			,L"]]等"
			,L"]]など"
		);
	return !r;

}

//曖昧さの解決項目の候補を解析する.
//↓ - で、項目と説明をくくれるなら、ハイフンまでにする.  ->> * 高山マリア
//* 高山マリア - ライトノベル・テレビアニメ『[[僕は友達が少ない]]』の登場人物。
//また、
//↓ ⇒ で、項目を後ろに書く奴がいるので、それを調べる.  --> [[カリキュラム]]を参照のこと。
//**[[国家]]の学習計画のこと。 ⇒ [[カリキュラム]]を参照のこと。
static std::wstring parseAimaiOneLine(const std::wstring& titleW,const std::wstring& innerAimaiW,int pos,int endpos,bool* outIsSpliter)
{
	std::wstring snipW;
#if _DEBUG
	snipW = innerAimaiW.substr(pos);
#endif
	//↓ - で、項目と説明をくくれるなら、ハイフンまでにする.
	//* 高山マリア - ライトノベル・テレビアニメ『[[僕は友達が少ない]]』の登場人物。
	const int hyphenpos = findAimaiFirstSeparate(innerAimaiW,pos,endpos);
	if (hyphenpos != std::wstring::npos)
	{
		if ( isAimaiBackWord(titleW,innerAimaiW,pos,endpos,hyphenpos)  )
		{	//- を後ろに入れるパティーンが有る。やめてくれよ。
			//*[[茨城県]][[久慈郡]][[大子町]]にある[[天台宗]]の寺院。 - [[日輪寺 (大子町)]]
			*outIsSpliter = false; //誤爆の可能性があるので.
			snipW = innerAimaiW.substr(hyphenpos,endpos - hyphenpos );
			return snipW;
		}
		else
		{
			snipW = innerAimaiW.substr(pos,hyphenpos - pos );
			*outIsSpliter = parseAimaiOneLineIsSplitKeyWord(snipW);
			return snipW;
		}
	}

	//↓ ⇒ で、項目を後ろに書く奴がいるので、それを調べる.  --> [[カリキュラム]]を参照のこと。
	//**[[国家]]の学習計画のこと。 ⇒ [[カリキュラム]]を参照のこと。
	const int beforeSeparatePos = findAimaiBackSeparate(innerAimaiW,pos,endpos);
	if (beforeSeparatePos != std::wstring::npos )
	{
		//こういうのは分離できていないことにする.
		//* 子午線公園<!-- (京都府)--> → [[京都府]][[京丹後市]][[網野町]]にある公園。
		snipW = innerAimaiW.substr(beforeSeparatePos,endpos - beforeSeparatePos );
		*outIsSpliter = parseAimaiOneLineIsSplitKeyWord(snipW);

		if (*outIsSpliter == false)
		{
			//実は前は前のほうがきれいなキーワードだった場合は入れ替え
			//* [[日本へそ公園]] → 兵庫県[[西脇市]]にある公園。
			if (isAimaiSimpleKeyWord(innerAimaiW,beforeSeparatePos-2,pos))
			{
				snipW = innerAimaiW.substr(pos,beforeSeparatePos - 2 - pos);
				*outIsSpliter = parseAimaiOneLineIsSplitKeyWord(snipW);
			}
		}

		return snipW;
	}

	//分岐点なし.
	*outIsSpliter = false;
	snipW = innerAimaiW.substr(pos,endpos - pos );
	return snipW;
}

std::wstring makeAimai(const std::wstring& innerW)
{
	std::wstring w = cleaningInnerText(innerW);
	w = XLWStringUtil::replace(w,L"'''",L"");	//まぎらわしい ''' を飛ばす. '''に依存できなくなるけど気にしない.

	//もし、関連項目の項があったら消します。 関連であって、そのことを指しているわけではない。
	int pos = w.find(L"\n== 関連項目",0);
	if (pos == std::wstring::npos)
	{
		return w;
	}

	std::wstring w2 = w.substr(0,pos+1);	//+1は\n

	int nokori = w.find(L"\n== ",pos+8);
	if (nokori == std::wstring::npos)
	{
		return w2;
	}

	w2 += w.substr(nokori);
	return w2;
}

//曖昧さの解決のページから候補を取る.
static void parseAimai(const std::wstring& titleW,const std::wstring& innerW,std::vector<std::wstring>* outAimaiVec)
{
	if ( ! isAimai(innerW) ) return;

	std::wstring innerAimaiW = makeAimai(innerW);

	int pos = 0;
	while(1)
	{
		pos = innerAimaiW.find(L"\n*",pos);
		if (pos == std::wstring::npos)
		{
			return ;
		}
		pos += sizeof("\n*")-1;

		int endpos = innerAimaiW.find(L"\n",pos);
		if (endpos == std::wstring::npos)
		{
			endpos = innerAimaiW.size();
		}

		const std::wstring line = innerAimaiW.substr(pos,endpos-pos);
		if ( TatoebanashiForAimai(titleW,line) )
		{//タイトルのことについて書いてなさそうなら飛ばす. 間違うぐらいなら取得するな
			continue;
		}


		//↓ - で、項目と説明をくくれるなら、ハイフンまでにする. --> * 高山マリア
		//* 高山マリア - ライトノベル・テレビアニメ『[[僕は友達が少ない]]』の登場人物。
		//また、
		//↓ ⇒ で、項目を後ろに書く奴がいるので、それを調べる.  --> [[カリキュラム]]を参照のこと。
		//**[[国家]]の学習計画のこと。 ⇒ [[カリキュラム]]を参照のこと。
		bool isSpliter = false;
		const std::wstring snipW = parseAimaiOneLine(titleW,innerAimaiW,pos,endpos,&isSpliter);

		//次のループの準備をしておく.
		pos = endpos;

		//この候補を採用していいか判別する.
		if ( checkGomiAimai(snipW) )
		{//この候補を採用してはいけない.
			continue;
		}

		//[[ ]]を参照 と書かれているならば、その項目のみを見る.
		if (! parseAimaiForReferenceAfter(titleW,snipW,outAimaiVec) )
		{
			if (! parseAimaiForReferenceBefore(titleW,snipW,outAimaiVec) )
			{
				if (! parseAimaiForReferenceOne(titleW,snipW,outAimaiVec,innerAimaiW,endpos) )
				{
					if (! parseAimaiForReferenceBeforeMulti(titleW,snipW,outAimaiVec) )
					{
					}
				}
			}
		}

		if (isSpliter)
		{//スプリッターが有効なときのみ文字列パースする. あまりに誤爆が多いので諦める.
			//文字列をパースして調べる.
			if ( parseAimaiForString(titleW,snipW,outAimaiVec) )
			{
				continue;
			}
		}
		//次の項目へ
		continue;
	}
}

static bool isGomiRedirect(const std::wstring& titleW,const std::wstring& target)
{//変なリダイレクトの消去

	if( target.find(L'#') != std::wstring::npos )
	{//ページの一部に飛ぶものは無視.
		return true;
	}

//	if( target.find(L"[[Category:") != std::wstring::npos )
//	{
//		return true;
//	}

	//完全マッチ ==word
	const WCHAR* strW0[] = {
		 L"てんてけマーチ"
		,L"モンモン山が泣いてるよ"
		,L"ヤジとボク"
		,L"山太郎かえる"
		,L"いないいないばあ"
		,L"松早ファミリーマート"	//なぜ・・・
		,NULL
	};
	for(const WCHAR** p = strW0; *p ; p++)
	{
		if ( titleW == *p )
		{
			return true;
		}
	}

	return false;
}

static bool parseRedirect(const std::wstring& titleW,const std::wstring& innerW,std::vector<std::wstring>* outAimaiVec)
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
					//無念
					return false;
				}
			}
		}
	}
	for(pos += 4 ;pos < (int)innerW.size() ; pos ++ )
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
	
	const std::wstring target = innerW.substr(pos);
	if (isGomiRedirect(titleW,target))
	{
		return false;
	}

	//リダイレクトは基本一つなのでここでふさわしいかを洗っておきます。
	if (!checkRuigi(titleW,target))
	{
		return false;
	}
	return parseWikiRuigi(titleW,target,outAimaiVec);
}

//こういう略語とも書かれていない略語を探す
//'''スマートフォン'''（{{lang-en-short|[[w:Smartphone|Smartphone]]}}、'''スマホ'''）とは、[[携帯電話]]機の一形態を指す用語である。
static bool	parseRuigiForRyakugo(const std::wstring& titleW,const std::wstring& str,std::vector<std::wstring>* outRuigiVec)
{

	int hitStartPos;
	int hitEndPos;
	bool r = 
		XLWStringUtil::firstfindPos(str,0,str.size(),&hitStartPos,&hitEndPos
			,L"'''"+titleW+L"'''"
			,titleW+L"（"
		);
	if (!r)
	{
		return false;
	}
	int pos = hitEndPos;
#ifdef _DEBUG
	auto aaa = str.substr(pos);
#endif

//カッコ内のパース
	int startKakoPos,start2KakoPos,endKakoPos,end2KakoPos;
	r =
		parseKakoEx(str	//対象文字列
		,pos	                            //skipする位置
		,str.size()						    //終了する位置
		,false						        //最初の括弧を true->find するか、false->[0]にある
		,&startKakoPos						//開始括弧位置
		,&start2KakoPos						//開始括弧をすぎて本文が始まる位置
		,&endKakoPos						//終了括弧位置
		,&end2KakoPos						//終了括弧をすぎて本文が始まる位置
		);
	if(!r)
	{
		return false;
	}

	//ここが略語っぽい
	std::wstring match = str.substr(start2KakoPos,endKakoPos-start2KakoPos);

	//ここで扱う略語は誤爆を避けるために、カタカナだけのものしか相手にしない
	for(unsigned int i = 0 ; i < match.size() ; i++)
	{
		if ( XLWStringUtil::isKata(match[i]))
		{
			continue;
		}
		if ( XLWStringUtil::isSpace(match[i]))
		{
			continue;
		}
		if ( match[i] == L'_' || match[i] == L'、' || match[i] == L'\'' )
		{
			continue;
		}
		return false;
	}

	//格納
	r = parseMultipleRuigi(titleW,match,outRuigiVec);
	if (!r)
	{
		return false;
	}
	return true;
}

//テキスト本文のパース
static void parseInnerText(const std::wstring& titleW,const std::wstring& innerTextW,std::vector<std::wstring>* outRuigiVec)
{
	std::vector<std::wstring> sentenceVec;
	SplitSentence(innerTextW,&sentenceVec);
	for(auto it = sentenceVec.begin() ; it != sentenceVec.end() ; it++ )
	{
		std::wstring str = *it;
		str = XLWStringUtil::strsnip(str,L"（",L"）");

		if (Tatoebanashi(titleW,str) )
		{//ただのたとえ話や昔話なら無視する.
			continue;
		}

		parseRuigiForInner2(titleW,str,outRuigiVec);
		parseRuigiForInnerR2(titleW,str,outRuigiVec);
	}

	//暗黙の略語を探す
	for(auto it = sentenceVec.begin() ; it != sentenceVec.end() ; it++ )
	{
		std::wstring str = *it;
		str = XLWStringUtil::strsnip(str,L"{{",L"}}");

		if (parseRuigiForRyakugo(titleW,str,outRuigiVec) )
		{
			break;
		}
	}
}

//なぜか大学の略称には、文章で説明を入れたがる人達がたくさんいる	箇条書きのやり方って小学校で習わなかったっけ？
//これらをすべて綺麗に整地する.
static std::wstring cleanupDaigakuRyakuSyo(const std::wstring& titleW,const std::wstring& innerHeadW,const std::wstring& line)
{
	std::wstring w  = line;
	//|大学の略称 = 特にないが、筑波（つくば）、筑波大（つくばだい）、筑大（つくだい）などと呼ばれている
	//|大学の略称=主に理科大。他に東京理科、東京理大、東理大、理大、TUS（以前はSUT）も使用されることがある
	//| 大学の略称=金大（きんだい）。[[旧官立大学]]で医学部は[[新潟大学]]、[[千葉大学]]、[[岡山大学]]、[[長崎大学]]、[[熊本大学]]とともに旧制六医科大学（[[旧六]]）をルーツとする
	//| 大学の略称=福島県内を中心に東北地方では福大が使用されることが多い。
	//| 大学の略称=徳島文理大、文理大、徳文大など。ただし四国地方内では単に文理と略称することが多い
	//| 大学の略称=主に国際医療。他に国際医療福祉、国際医福大、医療福祉大、医療大、福祉大、こっぷく、IUHW
	//| 大学の略称=長総大。地元では、総科大とも呼ばれている	
	//| 大学の略称=東北工大。また東北地方の一部地域では東工大も用いられる

	w = XLWStringUtil::replace(w,L"。他に",L"、");	//パースしやすいようにつなげる

	w = XLWStringUtil::replace(w,L"特にないが",L"");
	w = XLWStringUtil::replace(w,L"特に無いが",L"");
	w = XLWStringUtil::replace(w,L"などと呼ばれている",L"");
	w = XLWStringUtil::replace(w,L"定着したものはないが",L"");
	w = XLWStringUtil::replace(w,L"も使用されることがある",L"");
	w = XLWStringUtil::replace(w,L"主に",L"");
	w = XLWStringUtil::replace(w,L"など",L"");
	w = XLWStringUtil::replace(w,L"学生の間では",L"");
	w = XLWStringUtil::replace(w,L"である",L"");
	w = XLWStringUtil::replace(w,L"この他に",L"");
	
	w = XLWStringUtil::replace(w,L"「",L"");
	w = XLWStringUtil::replace(w,L"」",L"、");
	w = XLWStringUtil::replace(w,L"｣",L"、");
	w = XLWStringUtil::replace(w,L"や",L"、");
	w = XLWStringUtil::replace(w,L"'''",L"、");
	w = XLWStringUtil::replace(w,L"または",L"、");

	w = XLWStringUtil::replace(w,L"[[旧六]]",L"。");	

	w = XLWStringUtil::strsnip(w,L"（",L"）");	//よみは不要
	w = XLWStringUtil::strsnip(w,L"[[",L"]]");	//wikiワードも不要
	w = XLWStringUtil::strsnip(w,L"{{",L"}}");	//略もいらない

	w = XLWStringUtil::replace(w,L"等と",L"。");	//等と、で文章を続けたがる人を強制的に止める.
	w = XLWStringUtil::replace(w,L"だが",L"。");	//だが、で文章を続けたがる人を強制的に止める.
	w = XLWStringUtil::replace(w,L"が",L"。");	//が、で文章を続けたがる人を強制的に止める.
	w = XLWStringUtil::replace(w,L"※",L"。");	//※で説明を続けたがる人を強制的に止める.
	w = XLWStringUtil::replace(w,L"とも呼ば",L"。");	//とも呼ば で説明を続けたがる人を強制的に止める.

	//最初の文だけ採用して、後ろは消す。
	//地元の呼び名とかは取れなくなるけど、気にしない方向で。
	std::vector<std::wstring> sentenceVec;
	SplitSentence(w,&sentenceVec);
	if (sentenceVec.empty())
	{
		return L"";
	}
	w = sentenceVec[0];

	if (w.find(L"県内") != std::wstring::npos 
		|| w.find(L"地元") != std::wstring::npos 
		|| w.find(L"地方では") != std::wstring::npos 
		)
	{//県内の読み方はいらない。間違って変な名称にするぐらいなら略称はなしにする。
		return L"";
	}
	w = XLWStringUtil::replace(w,L"。",L"");
	return XLWStringUtil::chop(w);
}

static std::wstring parseInfoLine(const std::wstring& titleW,const std::wstring& innerHeadW,const std::wstring& line)
{
	std::wstring w = line;

	if ( titleW.find(L"大学") != std::wstring::npos )
	{	//なぜか大学の略称には、文章で説明を入れたがる人達がたくさんいる	箇条書きのやり方って小学校で習わなかったっけ？
		w = cleanupDaigakuRyakuSyo(titleW,innerHeadW,w);
	}

	w = XLWStringUtil::replace(w,L"および",L"、");
	w = XLWStringUtil::replace(w,L"もしくは",L"、");
	

	//|大学の略称 = 特になし  じゃあ書かないでよ・・・
	w = XLWStringUtil::replace(w,L"特になし",L"");
	w = XLWStringUtil::replace(w,L"特に無い",L"");

	//愛称:-  とかひどい・・・
	if (w == L"-")
	{
		w = L"";
	}

	return w;
}
static void parseRuigiForInfoBox(const std::wstring& titleW,const std::wstring& innerHeadW,const std::wstring& key,std::vector<std::wstring>* outRuigiVec)
{
	std::wstring r = getInfoboxStr(innerHeadW,key);
	if (!r.empty())
	{
		r = parseInfoLine(titleW,innerHeadW,r);
		if (!r.empty())
		{
			parseMultipleRuigi(titleW,r,outRuigiVec);
		}
	}
}

static void parseRuigiForInfoBox(const std::wstring& titleW,const std::wstring& innerW,std::vector<std::wstring>* outRuigiVec)
{
	const std::wstring innerHeadW = makeInnerHead(innerW);
	
//例
//| 愛称 = あすみん、アスミス
//|大学の略称 = 京大（きょうだい）
	parseRuigiForInfoBox(titleW,innerHeadW,L"略称",outRuigiVec);
	parseRuigiForInfoBox(titleW,innerHeadW,L"愛称",outRuigiVec);
	parseRuigiForInfoBox(titleW,innerHeadW,L"通称",outRuigiVec);
		 
	//中国歴で、呼称=皇帝 が山のようにいるので、そいつらは消す. 代わりに諡号を取る
	if (innerHeadW.find(L"{{基礎情報 中国君主") != std::wstring::npos)
	{
		parseRuigiForInfoBox(titleW,innerHeadW,L"諡号",outRuigiVec);
	}
	else
	{
		parseRuigiForInfoBox(titleW,innerHeadW,L"呼称",outRuigiVec);
	}
}






static void parseRuigi(const std::wstring& titleW,const std::wstring& innerW,std::vector<std::wstring>* outRuigiVec)
{
//	if (titleW==L"スラッシュドット効果")
	{
//		puts("!");
	}

	parseRuigiForInfoBox(titleW,innerW,outRuigiVec);

	std::wstring innerTextW = makeInnerText(innerW);

	parseInnerText(titleW,innerTextW,outRuigiVec);
}

static void AnalizeRedirect(const std::wstring& titleW,const std::wstring& innerW,std::map<std::wstring,std::vector<std::wstring>* >* redirectMap)
{
	if (! (isRedirect(innerW) || isAimai(innerW) ) )
	{//リダイレクトor曖昧項目だけを処理する. それ以外は没.
		return;
	}

	//曖昧さの解決
	fwprintf(stderr,L"@DEBUG: %ls ->", titleW.c_str() );

	std::vector<std::wstring> aimaiVec;

	//リダイレクトのパース
	bool r = parseRedirect(titleW,innerW,&aimaiVec);
	if (r)
	{
		if (! aimaiVec.empty())
		{
			fwprintf(stderr,L"	redirect:%ls", aimaiVec[0].c_str() );
		}
	}
	else
	{
		//曖昧さの解決のパース 結構間違うかも・・・
		parseAimai(titleW,innerW,&aimaiVec);
	}


	for(auto aimaiVecIT = aimaiVec.begin() ; aimaiVecIT != aimaiVec.end(); aimaiVecIT++ )
	{
		auto redirectMapIT = redirectMap->find(*aimaiVecIT);
		if (redirectMapIT == redirectMap->end() )
		{
			std::vector<std::wstring>* vec = new std::vector<std::wstring>;
			vec->push_back(titleW);
			(*redirectMap)[*aimaiVecIT] = vec;
		}
		else
		{
			redirectMapIT->second->push_back(titleW);
		}
		fwprintf(stderr,L"	%ls", (*aimaiVecIT).c_str() );
	}
	fwprintf(stderr,L"\n" );
}

//同姓同名の解決ページ
static bool isDouseidoumei(const std::wstring& innerW)
{
	return innerW.find(L"[[同姓同名]]") != std::wstring::npos;
}

static void AnalizeRuigi(const std::wstring& titleW,const std::wstring& innerW,const std::map<std::wstring,std::vector<std::wstring>* >& redirectMap)
{
	if ( isRedirect(innerW) || isAimai(innerW) || isDouseidoumei(innerW))
	{//リダイレクトor曖昧項目なので無視
		return;
	}
	const std::wstring _TitleW = cleaningInnerText(titleW);
	const std::wstring clipTitleW = snipAimaiKako(titleW);

	std::vector<std::wstring> ruigiVec;
	parseRuigi(titleW,innerW,&ruigiVec);
	
	std::wstring ruigiW;
	for(auto ruigiVecIT = ruigiVec.begin() ; ruigiVecIT != ruigiVec.end() ; ruigiVecIT++)
	{
		if (clipTitleW == *ruigiVecIT || titleW == *ruigiVecIT)
		{
			continue;
		}
		ruigiW += L"\t" + *ruigiVecIT;
	}

	//転送元を加える.
	auto redirectMapIT = redirectMap.find(titleW);
	if (redirectMapIT != redirectMap.end() )
	{
		for(auto it = redirectMapIT->second->begin() ; it !=  redirectMapIT->second->end() ; it++ )
		{
			//すでに愛称を取得できているならば追加しない
			auto it2 = std::find(ruigiVec.begin(),ruigiVec.end() , *it);
			if ( it2 == ruigiVec.end())
			{//まだ愛称を取得できていないものなので出力する.
				if (clipTitleW == *it || titleW == *it)
				{
					continue;
				}
				ruigiW += L"\t" + *it;
			}
		}
	}



	if ( Option::m()->getShow() == Option::TypeShow_NoEmpty)
	{//空ならば表示しない?
		if ( titleW.empty() || ruigiW.empty() )
		{
			return ;
		}
	}
	else if ( Option::m()->getShow() == Option::TypeShow_Empty)
	{//空だけ表示する
		if (! (titleW.empty() || ruigiW.empty() ) )
		{
			return;
		}
	}
	//カタカナひらがな変換は存在しない.

	//tofuに注意しながらMultiByteに修正します.
	std::string ruigiA,titleA;
	if ( Option::m()->getAimai() == Option::TypeAimai_Del)
	{
		const std::wstring ruigiW2 = snipAimaiKako(ruigiW);
		wprintf(L"%ls	%ls\n",clipTitleW.c_str(),ruigiW2.c_str() );
	}
	else
	{
		const std::wstring ruigiW2 = XLWStringUtil::chop(ruigiW);
		wprintf(L"%ls	%ls\n",_TitleW.c_str(),ruigiW2.c_str() );
	}
}



static void ReadAllForRuigi(FILE* fp,std::map<std::wstring,std::vector<std::wstring>* >* redirectMap,int loopCount)
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
					|| checkIchiran(titleW) )
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

				innerW =  _U2W(inner);
				if (loopCount == 0)
				{//1回目
					AnalizeRedirect(titleW,innerW,redirectMap);
				}
				else
				{
					AnalizeRuigi(titleW,innerW,*redirectMap);
				}
			}
		}
	}
}

void wiki2ruigiConvert(const std::string& filename)
{
	//2回読み込みます.
	//1回目は、redirectと曖昧さの解決だけ収集します.
	//2回目は、本文中の略語を拾っていきす。 略語とredirect先を混ぜて表示します
	//
	//redirectを先にするのは、本文中の略語に比べて数が少ないためです。
	std::map<std::wstring,std::vector<std::wstring>* > redirectMap;
	FILE * fp = NULL;

	fp = fopen(filename.c_str() , "rb");
	if (!fp)
	{
		fwprintf(stderr,L"can not open %ls file\n" , _A2W(filename).c_str() );
		return ;
	}
	AutoClear ac([&](){ fclose(fp); mapfree(redirectMap); });
	
	//リダイレクトリの収集
	fwprintf(stderr,L"@DEBUG:redirectと曖昧さ解決を取得しています\n");
	ReadAllForRuigi(fp,&redirectMap,0);
	
	//先頭に戻して
	fseek(fp,0,SEEK_SET);
	
	//略語を拾う
	fwprintf(stderr,L"@DEBUG:結果を出力しています\n");
	ReadAllForRuigi(fp,&redirectMap,1);
}

SEXYTEST()
{
	{
		std::vector<std::wstring> ruigiVec;
		parseAimai(L"シーマン(曖昧さの解決)",L">\n'''シーマン''' (seaman)\n*[[コンピュータゲーム]] - [[シーマン]]を参照。\n*[[船員]]、船乗り、水夫\n*[[海軍]]などの[[軍隊の階級|階級]]\n**[[一等海士]] - [[海上自衛隊]]\n**[[上等水兵]] - [[アメリカ海軍]]・[[アメリカ沿岸警備隊]]\n**[[水兵]] - [[ソ連海軍]]など\n\n*人名\n**[[ダニエル・シーマン]] - イスラエルの高官\n**[[デビッド・シーマン]] - イギリスのサッカー選手\n\n{{aimai}}\n{{デフォルトソート:しいまん}}\n[[Category:英語の姓]]\n",&ruigiVec);
		//ruigiVec = [5]("一等海士","上等水兵","水兵","ダニエル・シーマン","デビッド・シーマン")
		assert(ruigiVec[0] == L"一等海士");
		assert(ruigiVec[1] == L"上等水兵");
		assert(ruigiVec[2] == L"水兵");
		assert(ruigiVec[3] == L"ダニエル・シーマン");
		assert(ruigiVec[4] == L"デビッド・シーマン");
		assert(ruigiVec.size()==5);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseAimai(L"コブラ11",L">\n'''コブラ11'''\n\n* [[アウトバーンコップ|アラーム・フォー・コブラ11]] - 海外ドラマ。\n* [[新谷かおる|コブラ11]] - 講談社の月刊少年マガジンの1983年7月号掲載の読み切り作品。ベトナム戦争中の日系攻撃ヘリパイロットの物語。戦場ロマン系。\n\n{{Aimai}}\n{{デフォルトソート:こふら11}}\n",&ruigiVec);
		assert(ruigiVec[0] == L"アラーム・フォー・コブラ11");
		assert(ruigiVec.size()==1);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseAimai(L"バーボン",L"'''バーボン (Bourbon)''' とは、[[ブルボン家]]を意味する[[英語]]の語。それにちなみ、以下のものにも使われている。\n\n\n* [[アメリカ合衆国]][[ケンタッキー州]]を中心に生産されている[[ウイスキー]]（[[蒸留酒]]）の1つ、その[[略語]]。[[バーボン・ウイスキー]]を参照。\n* アメリカ合衆国の[[郡 (アメリカ合衆国)|郡]]の名称、[[バーボン郡 (カンザス州)]]及び[[バーボン郡 (ケンタッキー州)]]。\n* 刑事ドラマ『[[華麗なる刑事]]』および『[[ケータイ刑事]]シリーズ』で、[[草刈正雄]]が演じた高村一平刑事の愛称。\n* 漫画『[[名探偵コナン]]』の登場人物。[[黒の組織#バーボン]]を参照。v* 「'''バーボンハウス'''」の略。\n** [[2ちゃんねる用語]]で、次の意味を持つ。\n*** 興味を引く[[嘘]]をタイトルとしたスレッドの「>>1」に書かれるテンプレート。このスレッドがいわゆる[[釣り#比喩的な用法|釣り]]であることを暗示するのに多用される。\n*** [[2ちゃんねる]]へのアクセスが多すぎて、[[アクセス規制]]を受けている回線からアクセスしたときに出るメッセージ<ref name=\"info.2ch-bourbonhouse\">[http://info.2ch.net/wiki/index.php?%A4%C8%A4%AB%A4%B2%A4%CE%BF%AC%C8%F8%C0%DA%A4%EA%A1%A6%A5%D0%A1%BC%A5%DC%A5%F3%A5%CF%A5%A6%A5%B9 とかげの尻尾切り・バーボンハウス - いきいき Wiki]</ref>。通称「'''BBON'''」（ボボン）。\n** 各地にある[[バー (酒場)|バー（酒場）]]の名称。\n** 各地にある[[飲食店]]の名称。[[ニューオーリンズ]]など。\n\n== 脚注 ==\n{{Reflist}}\n\n{{Aimai}}\n{{DEFAULTSORT:はあほん}}",&ruigiVec);
		assert(ruigiVec[0] == L"バーボン・ウイスキー");
		assert(ruigiVec.size()==1);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"邦楽",L">{{otheruses|日本の[[伝統音楽]]|日本の[[ポピュラー音楽]]|歌謡曲}}\n{{Redirect|和楽|小学館の雑誌『和'''樂'''』（わらく）|和樂}}\n'''邦楽'''（ほうがく）とは、主に[[日本]]の伝統的な[[民族音楽|伝統音楽]]、古典音楽などを指す呼称である。'''和楽'''、'''国楽'''とも呼ばれる。日本の[[ポピュラー音楽]]との区別のため、「純邦楽」とも呼ばれる。",&ruigiVec);
		//ruigiVec = [2]("国楽","和楽")
		assert(ruigiVec[0] == L"国楽");
		assert(ruigiVec[1] == L"和楽");
		assert(ruigiVec.size()==2);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseAimai(L"首都リーグ",L"\n'''首都リーグ'''（しゅと-）は、[[首都圏]]にある団体が加盟するリーグ・連盟の通称・略称で、正式名称としては以下のようなものがある。\n\n*[[首都大学野球連盟|首都大学野球リーグ]]\n*[[全日本大学軟式野球連盟|首都大学軟式野球リーグ]]\n*[[全日本学生軟式野球連盟|首都学生軟式野球リーグ]]\n*[[首都大学ソフトテニスリーグ]]\n*[[関東学生アメリカンフットボール連盟#首都六大学リーグ|首都六大学アメリカンフットボールリーグ]]\n*[[関東学生アメリカンフットボール連盟#首都七大学リーグ|首都七大学アメリカンフットボールリーグ]]\n*[[関東学生アメリカンフットボール連盟#首都八大学リーグ|首都八大学アメリカンフットボールリーグ]]\n*[[首都クラブラグビー連盟|首都クラブラグビーリーグ]]\n\n== 関連項目 ==\n*[[首都大学]]\n*[[首都]]\n\n{{aimai}}\n{{DEFAULTSORT:しゆとりく}}",&ruigiVec);
		//ruigiVec = [8]("首都大学野球リーグ","首都大学軟式野球リーグ","首都学生軟式野球リーグ","首都大学ソフトテニスリーグ","首都六大学アメリカンフットボールリーグ","首都七大学アメリカンフットボールリーグ","首都八大学アメリカンフットボールリーグ","首都クラブラグビーリーグ")
		assert(ruigiVec.size()==8);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseAimai(L"マリアージュ",L">'''マリアージュ'''（''mariage''）\n\n;一般的な意味\n*[[フランス語]]で「結婚」のこと。→'''[[結婚]]'''\n\n※　フランス人は、しばしば（しかも、とてもしばしば）、もともとふたつで別々だった存在があたかもひとつの存在のように調和した状態になることを、[[詩]]的に（[[メタファー]]的に）「mariage マリアージュ」と言う<ref>辞書で言えば、[[http://megalodon.jp/2014-0122-1612-08/www.cnrtl.fr/definition/mariage]]　のB.-1, -2などの用法。</ref>。\n例えば視覚芸術ならば、[[キャンバス]]上で二つの要素がひとつのまとまった調和した視覚的効果を生むことを「マリアージュ」と言うし、料理・食事関係ならば、たとえば、一緒に食べるfromage フロマージュ（[[チーズ]]）とvin ヴァン（[[ワイン]]）の味が絶妙に調和した状態を「マリアージュ」と言う<ref>たとえば、[http://www.produits-laitiers.com/2011/12/21/fromages-vins-dalsace-mariage-festif/]のような用法。</ref>し、また、例えば肉料理や魚料理などとフランス流ソースの味がうまくひとつになって調和した味が生まれている時も[[シェフ]]やお客は「マリアージュ」と表現する。おまけにフランス人は、それらの調和した組み合わせをまず「mariage de A et B（AとBのマリアージュ）」と表現しておいて、さらにそのあとにわざわざ「ふたつの vie いのちが、ひとつのいのちになるのです。」などと、詩のような文章まで添えることもしばしば。\n\n;人名、苗字\n*Mariage\n　（たとえば、あるマリアージュ家の、ある兄弟は、紅茶専門店、紅茶ブランドの「MARIAGE FRERES　[[マリアージュフレール]]」を立ち上げた）\n\n;作品名\n*日本のロックバンドDEENの2012年のアルバム。→[[マリアージュ (DEENのアルバム)]] \n== 脚注 ==\n<references/> \n\n{{aimai}}\n{{デフォルトソート:まりああしゆ}}",&ruigiVec);
		assert(ruigiVec[0] == L"結婚");
		assert(ruigiVec.size()==1);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseAimai(L"すり",L">'''すり'''\n*[[スリ]] - （掏摸）他人の懐から物を盗む犯罪行為。（掏児）その行為を生業とする人。\n**『[[スリ (映画)|スリ〈掏摸〉]]』 - 1959年製作のフランス映画（原題：[[:fr:Pickpocket (film)|Pickpocket]]）\n**『[[スリ (2000年の映画)|スリ]]』- 2000年公開の映画。[[黒木和雄]]監督。\n**『[[スリ (2008年の映画)|スリ]]』- 2008年の[[香港映画]]のDVDタイトル。（原題：文雀 Sparrow）。日本では東京フィルメックスで「文雀」として上映され、後にDVDリリース。[[ジョニー・トー]]監督。\n*刷り - 印刷をすること。または、印刷の具合。[[印刷]]、[[版画]]を参照。\n*刷 - [[奥付]]に表示される、同じ版から印刷された刷り数。さつ。[[本|書籍]]を参照。\n*刷り - 布に文様を染めつけることをいう古語。[[染色]]を参照。\n*[[すり合わせ]]の俗称。\n\n*[[スリ・クルーズ]] - [[トム・クルーズ]]と[[ケイティ・ホームズ]]の長女。\n{{aimai}}",&ruigiVec);
		assert(ruigiVec[0] == L"版画");
		assert(ruigiVec[1] == L"染色");
		assert(ruigiVec[2] == L"スリ・クルーズ");
		assert(ruigiVec.size()==3);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseAimai(L"会式",L">'''会式'''（えしき）\n* お会式（おえしき） - 各宗派の[[宗祖]]などの命日に行われる[[法要]]行事。→ [[お会式]]の項を参照。\n\n* 会式（かいしき） - 以下の場合は「[[臨時軍用気球研究会]]」式 の略。\n** [[会式イ号飛行船]] - 日本最初の[[飛行船]] （[[1911年]]（明治44年）10月）。\n** [[雄飛 (飛行船)|会式雄飛号飛行船]] - 初の国産（パルセヴァル式を改良した）飛行船 （[[1915年]]（大正4年）4月）。\n** 会式（1号～7号）[[飛行機]] - 軍用機としては初の国産となった飛行機 → [[会式一号機]]を参照。\n\n{{デフォルトソート:えしき}}\n{{aimai}}",&ruigiVec);
		//ruigiVec = [2]("会式イ号飛行船","会式雄飛号飛行船")
		assert(ruigiVec[0] == L"会式イ号飛行船");
		assert(ruigiVec[1] == L"会式雄飛号飛行船");
		assert(ruigiVec.size()==2);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"通貨",L">'''通貨'''（つうか）とは、'''流通貨幣'''の略称で、[[国家]]などによって[[価値]]を[[保証]]された、[[決済]]のための価値交換媒体。[[政府]]は[[租税]]の算定にあたって通貨を利用する（[[法定通貨]]⇔[[電子マネー#法的な位置づけ|仮想通貨]]、[[地域通貨]]）。\n\nモノやサービスとの交換に用いられる「[[お金]]」を、経済用語では[[貨幣]]、または通貨と呼ぶ<ref>岩田規久男 『国際金融入門』 岩波書店・新版〈岩波新書〉、2009年、8頁。</ref>。（「お金持ち」などのように[[資産]]全体を指す用法も存在する。）通貨は、現金通貨と預金通貨に大別され、前者は[[紙幣]]・[[硬貨]]（[[補助紙幣]]）であり、後者は[[普通預金]]・[[当座預金]]などの決済口座である<ref>野口旭 『「経済のしくみ」がすんなりわかる講座』 ナツメ社、2003年、123頁。</ref>。\n\n{{otheruses||[[琉球列島高等弁務官]]が制定した布令『通貨』|通貨 (高等弁務官布令)}}",&ruigiVec);
		//ruigiVec = [4]("City ","Citizens ","The Sky Blues ","The Blues")
		assert(ruigiVec[0] == L"流通貨幣");
		assert(ruigiVec.size()==1);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"有理数",L">'''有理数'''（ゆうりすう、{{lang-en-short|''rational number''}}) とは、二つの[[整数]] ''a'', ''b'' （ただし ''b'' は 0 でない）をもちいて ''a''/''b'' という[[分数]]で表せる[[数]]のことをいう。''b'' = 1 とすることにより、任意の整数は有理数として扱うことができる。\n\n有理数を[[十進法]]などの[[位取り記数法]]を用いて[[小数]]表示した場合、どの有理数も位取りの基数のとり方に関わらず[[有限小数]]または[[循環小数]]のいずれかとなる（もちろん、ある基数で表示したとき有限小数となる有理数が、別の基数では循環小数となったりすること、あるいはその逆になることはある）。同様に、有理数は必ず[[連分数|有限正則連分数展開]]を持つ。\n\n有理数全体のつくる集合はしばしば、英語で[[除法|商]]を意味する \"''quotient''\" の頭文字をとり、太字の '''Q''' で表す。手書きするときなどには中抜きの太字にするため、書籍等で[[黒板太字]]と言われる書体で &#x211a; を使うこともある。すなわち、\n: <math>\\mathbb{Q} = \\left\\{{a \\over b} \\mid a, b \\in \\mathbb{Z}, b\\ne 0\\right\\}</math>\nである（ただし、'''Z''' は全ての整数からなる集合を表す）。ここで、各個の有理数に対して、それをあらわす分数 ''a''/''b'' は一般に複数（しかも無数に）存在することは留意すべき事実である。通常は個々の文脈に適した形を選んで利用する。すなわち厳密に言えば、分数 ''a''/''b'' は整数 ''a'', ''b'' の組の属する[[同値類]]（の代表元）を表しているのであり（[[#形式的な構成|形式的な構成]]節参照）、有理数全体の成す集合 '''Q''' は[[同値関係#商集合|商集合]]の最も典型的で身近な例となっている。\n\n有理数の[[完備化]]（適当な距離に関する「無限小数」展開を考えることに相当）として、[[実数]]や[[p進数| ''p''-進数]]が得られる（後述。あるいは[[コーシー列]]・[[デデキント切断]]等を参照）。有理数ではない[[実数]]は[[無理数]]と呼ばれる。また、すべての有理数係数多項式の根の全体は[[体 (数学)|体]]を成し（'''Q''' の[[代数閉包]]）、その元を[[代数的数]]と呼ぶ。",&ruigiVec);
		//ruigiVec = [4]("City ","Citizens ","The Sky Blues ","The Blues")
		assert(ruigiVec.size()==0);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseAimai(L"高速",L">{{Wiktionary|高速}}\n'''高速'''（こうそく）は、一般的には速度が速いこと。対義語は低速。\n\n*列車種別の一つ ⇒ [[高速 (列車)]]、[[高速貨物列車]]\n*[[高速鉄道]]\n*[[都市高速鉄道]] - 地下鉄運営団体などの'''高速'''の語源はここからきており、現代の感性では違和感を覚えるが、それらが（市電・駅馬車などより）相対的に速度が速いことを示すにすぎない。同様の例に[[都市高速道路]]。\n*[[高速道路]]\n*[[高速バス]]\n*[[高速フーリエ変換]]\n*[[高速カーブ]]\n*食品向け軽包装資材の専門商社。⇒ [[高速 (企業)]]\n\n== 関連項目 ==\n* [[特別:Prefixindex/高速|高速で始まる記事の一覧]]\n\n{{aimai}}\n{{デフォルトソート:こうそく}}",&ruigiVec);
		//ruigiVec = [7]("高速貨物列車","高速鉄道","都市高速鉄道","高速道路","高速バス","高速フーリエ変換","高速カーブ")
		assert(ruigiVec.size()==7);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseAimai(L"プログラム",L"'''プログラム'''（{{lang-en-short|'''Program'''; '''Programme'''}}, [[ギリシア語|希]]: prographein（公示する）から）\n\n*ある物事の進行状態についての[[順序]]・[[組み合わせ]]・[[筋]]などのこと。及び、それを書いたもの。例：[[金融再生プログラム]]。\n*: ⇒ [[計画]]、[[予定]]、[[スキーム]]、[[スケジュール]]、[[デザイン]]、[[プラン]]、[[プロジェクト]]、[[プロット]]なども参照のこと。\n**[[国家]]の学習計画のこと。 ⇒ [[カリキュラム]]を参照のこと。\n**[[政党]]の[[綱領]]や[[選挙公約]]のこと。 ⇒ [[マニフェスト]]を参照のこと。\n**[[プログラムマネジメント]] - 複数のプロジェクトを統括的に[[管理]]すること。\n*各種の[[催し物]]や[[行事]]（[[イベント]]）の[[計画]]、[[予定]]、[[演目]]、[[番組]]などのこと。催し物そのものを指す場合もある。\n**[[演奏会]]、[[演劇]]、[[運動会]]、[[映画]]などでは、それが書かれた小[[冊子]]のこともさす。出演者の紹介や解説などが書かれる場合もある。予定表、演目表、[[番組表]]。\n**特に、[[公営競技]]における[[競走]]（レース）のこと。番組。\n*[[プログラム (コンピュータ)]]。[[コンピュータ]]への命令を、特定の[[プログラミング言語|言語]]での定められた形式に従って記述した筋書き。また、それを作成すること。コンピュータプログラム。\n*[[写真]]撮影における[[シャッター]]速度・絞り双方のコントロールを[[カメラ]]側において自動的に調節すること。⇒[[AEカメラ]]を参照のこと。\n*[[電子楽器]]への命令を、[[MIDI]]などでの定められた形式に従って記述した筋書き。また、それを作成すること。MIDIプログラム。音楽プログラムともいうが、この場合、催し物や演奏会の意味にも使われることが多い。\n*[[ニテンピラム]]を[[有効成分]]とし、[[ノミ]]・[[ダニ]]の[[駆除]]を目的とした[[ノバルティス]] アニマルヘルス社の[[動物用医薬品]]の商品名。\n\n== 関連項目 ==\n* [[番組]]\n\n{{aimai}}\n",&ruigiVec);
		//ruigiVec = [12]("プロット","プロジェクト","プラン","デザイン","スケジュール","スキーム","予定","計画","カリキュラム","マニフェスト","プログラムマネジメント","AEカメラ")
		assert(ruigiVec.size()==12);
	}

	{
		std::vector<std::wstring> ruigiVec;
		parseAimai(L"第五高等学校",L"      <text xml:space=\"preserve\">'''第五高等学校'''\n* [[第五高等学校 (旧制)]] - 新制[[熊本大学]]の前身となった[[旧制高等学校]]。\n\n* [[山形県]]\n** 山形県立山形第五高等学校 ⇒（統合） [[山形県立山形東高等学校]] ⇒ （分離）[[山形県立山形北高等学校]]\n* [[東京都]]\n** 東京都立第五高等学校 ⇒ [[東京都立小石川高等学校]]\n* [[愛媛県]]\n** [[帝京第五高等学校]]\n* [[福岡県]]\n** [[東海大学付属第五高等学校]]\n\n== 関連項目 ==\n* [[第一高等学校]]・[[第二高等学校]]・[[第三高等学校]]・[[第四高等学校]]\n\n{{aimai}}\n{{DEFAULTSORT:たい5こうとうかつこう}}\n[[Category:日本の高等学校]]",&ruigiVec);
		//ruigiVec = [3]("東京都立小石川高等学校","帝京第五高等学校","東海大学付属第五高等学校")
		assert(ruigiVec.size()==3);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseAimai(L"第四高等学校",L"      <text xml:space=\"preserve\">'''第四高等学校'''\n\n*[[第四高等学校 (旧制)]] - 新制[[金沢大学]]の前身となった[[旧制高等学校]]。\n\n\n*[[北海道]]\n**[[東海大学付属第四高等学校・中等部|東海大学付属第四高等学校]]\n*[[岩手県]]\n**[[岩手県立盛岡第四高等学校]]\n*[[山形県]]\n**山形県立山形第四高等学校　⇒（統合）　[[山形県立山形南高等学校]]　⇒（分離）[[山形県立山形西高等学校]]\n*[[東京都]]\n**東京都立第四高等学校　⇒　[[東京都立戸山高等学校]]\n\n== 関連項目 ==\n* [[第一高等学校]]・[[第二高等学校]]・[[第三高等学校]]・[[第五高等学校]]\n\n{{aimai}}\n{{DEFAULTSORT:たい4こうとうかつこう}}\n[[Category:日本の高等学校]]\n",&ruigiVec);
		assert(ruigiVec[0] == L"東海大学付属第四高等学校");
		assert(ruigiVec[1] == L"岩手県立盛岡第四高等学校");
		assert(ruigiVec[2] == L"東京都立戸山高等学校");
		assert(ruigiVec.size()==3);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseAimai(L"高浜中学校",L">'''高浜中学校'''（たかはまちゅうがっこう）\n\n* [[千葉県]]\n** [[千葉市立高浜中学校]]\n* [[神奈川県]]\n** [[藤沢市立高浜中学校]]\n* [[福井県]]\n** [[高浜町立高浜中学校]]\n* [[愛知県]]\n** [[高浜市立高浜中学校]]\n* [[愛媛県]]\n** [[松山市立高浜中学校]]\n\n以下は廃校\n* [[石川県]]\n** [[志賀町立高浜中学校]]\n* [[長崎県]]\n** [[野母崎町立高浜中学校]]\n\n{{aimai}}\n{{デフォルトソート:たかはまちゆうかつこう}}\n[[Category:日本の公立中学校]]",&ruigiVec);
		//ruigiVec = [7]("千葉市立高浜中学校","藤沢市立高浜中学校","高浜町立高浜中学校","高浜市立高浜中学校","松山市立高浜中学校","志賀町立高浜中学校","野母崎町立高浜中学校")
		assert(ruigiVec.size()==7);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseAimai(L"水の科学館",L"'''水の科学館'''（みずのかがくかん）は、水に関する日本の[[科学館]]の名称。所在地は以下のとおり。\n\n\n*[[兵庫県]][[神戸市]] - [[神戸市水の科学館]]を参照\n*[[東京都]][[江東区]] - [[東京都水の科学館]]を参照\n*[[熊本県]][[熊本市]] - [[熊本市水の科学館]]を参照\n\n{{aimai}}\n{{デフォルトソート:みすのかかくかん}}",&ruigiVec);

		assert(ruigiVec[0] == L"神戸市水の科学館");
		assert(ruigiVec[1] == L"東京都水の科学館");
		assert(ruigiVec[2] == L"熊本市水の科学館");
		assert(ruigiVec.size()==3);
	}
	{
		std::vector<std::wstring> ruigiVec;

		parseAimai(L"BIS",L">'''BIS'''\n* [[BlackBerry Internet Service|BlackBery Internet Service（BIS）]]ブラックベリーインターネットサービスの略\n* [[ビスマーク市営空港]]の[[空港コード|IATAコード]]。\n* [[国際決済銀行]] - 国際特殊銀行。\n* [[アメリカ合衆国商務省産業安全保障局]] (Bureau of Industry and Security)\n* Bureau of Indian Standards - インドの標準化機関。\n* [[NPB・BIS]] - [[日本野球機構]]のプロ野球データベースシステム。\n* [[BISレコード]] - [[スウェーデン]]の[[クラシック音楽]]の[[レコードレーベル]]。\n* [[Bohemia Interactive Studio]] - チェコの[[コンピュータゲーム]]開発会社。\n----\n{{Wiktionary|bis}}\n'''bis'''\n*[[bis (ISO 639)]]\n* 「二度」「第二の」等の意を持ち、戦車等の兵器に「改良型」「2型」という意でよく用いられる。また、フランス語で「[[アンコール]]」を表す。\n* [[言語]]の名称の[[国際標準化機構|国際標準]]、[[ISO 639]]において[[ビスラマ語]]を表すコード。\n* [[bis]] - かつて存在していた、日本の雑誌。\n* bis{{enlink|Bis (band)|a=on}} - [[スコットランド]]出身の[[ポップ・ミュージック|ポップ]]・[[バンド (音楽)|バンド]]。マンダ・リン、サイ・ファイ・スティーヴン、ジョン・ディスコの3名で構成。日本では、アルバム『ニュー・トランジスター・ヒーローズ{{enlink|The New Transistor Heroes|a=on}}』（[[1997年]]発売）収録の『kandy pop（キャンディ・ポップ）』がスマッシュ・ヒットとなった。[[2003年]]に解散。\n----\n'''BiS'''\n* [[BiS]] - 新生アイドル研究会。[[プー・ルイ]]が立ち上げた女性アイドルグループ。\n----\n{{aimai}}\n",&ruigiVec);
		assert(ruigiVec[0] == L"国際決済銀行");
		assert(ruigiVec[1] == L"アメリカ合衆国商務省産業安全保障局");
		assert(ruigiVec[2] == L"NPB・BIS");
		assert(ruigiVec[3] == L"BISレコード");
		assert(ruigiVec[4] == L"Bohemia Interactive Studio");
		assert(ruigiVec[5] == L"bis");
		assert(ruigiVec[6] == L"BiS");
		assert(ruigiVec.size()==7);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"マンチェスター・シティFC",L">{{サッカークラブ\n|font-color= #FFFFFF\n|background-color= #88BBFF\n|原語表記=Manchester City Football Club\n|愛称=City / Citizens / The Sky Blues / The Blues\n|カラー=水色\n|創設=1887\n|リーグ=[[プレミアリーグ]]\n|ディビジョン=\n|ホームタウン=[[マンチェスター]]\n|スタジアム=[[シティ・オブ・マンチェスター・スタジアム|エティハド・スタジアム]]\n|キャパ=48,000\n|代表={{Flagicon|UAE}} [[シャイフ]] {{仮リンク|マンスール・ビン・ザーイド・アール・ナヒヤーン|en|Mansour bin Zayed Al Nahyan}}\n|監督={{Flagicon|CHI}} [[マヌエル・ペジェグリーニ]]\n|HP=http://www.mcfc.co.uk/\n}}'''マンチェスター・シティ・フットボール・クラブ'''（{{En|'''Manchester City Football Club'''}}, <small>[[イギリス英語]]発音</small>：{{IPA-en|ﾋ・anﾊｧistﾉ鬢 ﾋ・iti ﾋ・utﾋ恵ﾉ藩人 klﾊ恵|}}）は、[[イングランド]]・[[マンチェスター]]に本拠地を置く[[サッカー]]クラブである。",&ruigiVec);
		//ruigiVec = [4]("City ","Citizens ","The Sky Blues ","The Blues")
		assert(ruigiVec[0] == L"City");
		assert(ruigiVec[1] == L"Citizens");
		assert(ruigiVec[2] == L"The Sky Blues");
		assert(ruigiVec[3] == L"The Blues");
		assert(ruigiVec.size()==4);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"マジックナンバー",L">'''マジックナンバー'''（日本では一般に'''マジック'''と呼ばれる）は[[プロ野球]]の用語で、「他のチームの試合結果に関わらず、自チームがあと何勝すれば優勝が決定する」と言える勝ち数を意味する。\n\n優勝までにあと何回勝たねばならないかにほぼ等しい。日本では他の全チームに自力優勝の可能性がなくなった状況でのみこの値を用い、この条件を満たすことをマジックナンバーが「点灯」したという。野球チームは優勝までに、マジック点灯⇒マジックナンバーを減らす⇒優勝という経過を通常たどるため、マジックナンバーはチームが優勝するまでの道筋として用いられる。\n\n米国では自力優勝の条件が満たされない場合でもマジックナンバーを用いるため、「点灯」の概念はない。また米国では野球以外のスポーツでもマジックナンバーを用いる。",&ruigiVec);
		assert(ruigiVec.size()==0);
	}
	{
		std::vector<std::wstring> ruigiVec;

		parseAimai(L"マジックナンバー",L">'''マジックナンバー'''、{{lang|en|magic number}}<!--は、「[[魔法]]の[[数字]]」「[[魔術]]に関わる数字」およびそれに派生する様々な言葉である。-->\n\n==スポーツ==\n*[[マジックナンバー (野球)]]\n\n==音楽==\n*[[ザ・マジック・ナンバーズ]] - ロック・バンド、イギリス\n*[[マジックナンバー (坂本真綾の曲)]]\n*[[magic number (KICK THE CAN CREWのアルバム)|{{lang|en| magic number}} ({{lang|en|KICK THE CAN CREW}}のアルバム)]]\n\n==ファッションブランド==\n*{{lang|en|[[MAGIC NUMBER]]}} - 日本、ストリート、サーフ系、[[中村竜]]監督\n\n==コンピュータ用語==\n<!--コンピュータ業界では、一見意味不明に見える情報を意味する言葉として、以下のような様々な場面で用いられている。-->\n*[[マジックナンバー (プログラム)]] - プログラム中に直接埋め込まれた数値\n*[[マジックナンバー (フォーマット識別子)]]\n*[[マジックナンバー (メッセージ)]] - 内部状態を利用者に通知するメッセージ\n\n==その他==\n<!--*「良く分からないけど、この数字を使うと物事がうまくいく」または「良く分からないけど、ある事項でこの数字が頻出する」、そのような数字。-->\n*[[魔法数]] - 原子核の安定性\n*[[マジカルナンバー (心理学)]] - 短期的記憶の上限数、[[ジョージ・ミラー]]<!--で、人間が短期的に記憶できる限界とされる数。7±2であるという。正しくは'''マジカルナンバー（magical number）'''。近い将来にある物事が起きるか起きないか注目されている時、特に一部の人達にとって起きて欲しい事柄である時、その言葉がまるで呪文（magic word）のように唱えられ、あるいは連呼される。このような言葉を「magic word」と言うことがある。それが数字である場合には「magic number」と呼ぶ。[[ビンゴ]]で使われたのが始まりとされる。日本で言う「リーチ」状態の時、ビンゴ完成のために必要な数字をマジックナンバーという。現在では様々なジャンルで使われている。たとえば[[アメリカ合衆国上院|米国上院]]（2年ごとに定数の3分の1を改選）の選挙において、ある政党が議会過半数を得るのに必要な、その選挙での最小獲得議席数をマジックナンバーと呼ぶ事がある。また、国家[[財政]]を[[原油]][[輸出]]に依存している国にとって、財政が黒字になるか赤字になるかの分岐点となる原油価格をマジックナンバーと呼ぶ事がある（[[:en:Magic number (oil)]]）。-->\n*[[マジックナンバー (漫画)]] - 漫画、[[荒井チェリー]]\n\n==関連項目==\n*[[マジック]]\n\n{{aimai}}\n{{デフォルトソート:ましつくなんはあ}}",&ruigiVec);
		assert(ruigiVec[0] == L"ザ・マジック・ナンバーズ");
		assert(ruigiVec[1] == L"魔法数");
		assert(ruigiVec[2] == L"マジック");
		assert(ruigiVec.size()==3);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"熊本大学",L">{{大学\n|国 = 日本\n|大学名 = 熊本大学\n|研究科 = 文学研究科<br />教育学研究科<br />法学研究科<br />社会文化科学研究科<br />自然科学研究科<br />医学薬学研究部<br />医学教育部<br />保健学教育部<br />薬学教育部<br />法曹養成研究科\n|大学の略称 = 熊大（くまだい） [[旧六]]の一つ\n}}",&ruigiVec);
		assert(ruigiVec[0] == L"熊大");
		assert(ruigiVec.size()==1);
	}
	{
		std::vector<std::wstring> ruigiVec;

		parseRuigi(L"コンピュータゲーム",L">{{Selfref|コンピュータゲーム（[[プロジェクト:コンピュータゲーム|プロジェクト]]/[[Portal:コンピュータゲーム|ポータル]]）へのリンクを貼るためのテンプレートに付いては[[Template:Cite video game]]を参照}}\n{{Pathnav|ゲーム|frame=1}}\n{{出典の明記|date=2014年10月}}\n{{コンピュータゲームのサイドバー}}\n{{コンピュータゲーム産業}}\n\n'''コンピュータゲーム'''（{{Lang-en-short|[[w:Video game|Video game]]}}）とは、[[コンピュータ]]によって[[コンピューティング|処理]]される[[ゲーム]]<ref>{{Cite web|url=http://kotobank.jp/word/%E3%82%B3%E3%83%B3%E3%83%94%E3%83%A5%E3%83%BC%E3%82%BF%E3%83%BC%E3%82%B2%E3%83%BC%E3%83%A0|title=コンピューターゲームとは|work=[[コトバンク]]|publisher=[[朝日新聞社]]|accessdate=2013-11-08}}</ref>。コンピュータゲームは[[和製英語]]であり、「Computer game」は英語で「パソコンゲーム」を指す。[[アーケードゲーム]]、[[コンシューマーゲーム]]（[[テレビゲーム]]/[[携帯型ゲーム]]）、[[パソコンゲーム|PCゲーム]]、[[携帯電話ゲーム|モバイルゲーム]]などがある。ゲーム画面を[[ディスプレイ (コンピュータ)|ビデオモニター]]に出力するため'''ビデオゲーム'''や'''デジタルゲーム'''とも呼ばれる。いわゆる[[LSIゲーム]]も含めて'''電子ゲーム'''と呼ばれる場合もある。'''ゲーム'''とシンプルに呼ばれることもある。",&ruigiVec);
		assert(ruigiVec[0] == L"デジタルゲーム");
		assert(ruigiVec[1] == L"ビデオゲーム");
		assert(ruigiVec.size()==2);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"エクジソン",L">{{chembox\n| ImageFile = Ecdysone.svg\n| ImageSize = 200px\n| IUPACName = <small>(2''S'',3''R'',5''R'',9''R'',10''R'',13''R'',14''S'',17''R'')-17- [(2''S'',3''R'')-3,6-ジヒドロキシ-6-メチルヘプタン-2-イル]-2,3,14-トリヒドロキシ-10,13-ジメチル- 2,3,4,5,9,11,12,15,16,17-デカヒドロ- 1''H''-シクロペンタ[a]ファナントレン-6-オン</small>\n| OtherNames = \n| Section1 = {{Chembox Identifiers\n| CASNo =  3604-87-3 \n| PubChem = 19212\n| SMILES = C[C@@H]([C@H]1CC[C@@]2([C@@]1 (CC[C@H]3C2=CC(=O)[C@H]4[C@@]3 (C[C@@H]([C@@H](C4)O)O)C)C)O) [C@@H](CCC(C)(C)O)O\n}}\n| Section2 = {{Chembox Properties\n| Formula = C<sub>27</sub>H<sub>44</sub>O<sub>6</sub>\n| MolarMass = 464.63 g/mol\n| Appearance = \n| Density = \n| MeltingPt = \n| BoilingPt = \n| Solubility = }}\n| Section3 = {{Chembox Hazards\n| MainHazards = \n| FlashPt = \n| Autoignition = }}\n}}\n'''エクジソン'''（''Ecdysone''; エクダイソン）は[[昆虫]]の[[ホルモン]]の一種。[[前胸腺]]から[[分泌]]される[[ステロイドホルモン]]で、[[脱皮]]（ecdysis）または[[変態]]を促進する作用があり、'''脱皮ホルモン'''（Molting hormone）とも呼ばれる。\n\n[[画像:構造式_Ecdysone.png|inline|thumb|300px|Ecdysone の構造式。下は立体表示]]\n\nエクジソンはホルモン前駆体であり、20-ヒドロキシエクジソン（20E）に代謝されて機能を発揮する。これらの類似構造を持つホルモンを総称して'''エクジステロイド'''（ecdysteroid）とも呼ぶ。これらは[[植物]]にも存在し、植物エクジソンと呼ぶ。昆虫以外の[[節足動物]]にも存在し同様の機能を有する。[[甲殻類]]では[[Y器官]]から分泌される。",&ruigiVec);
		assert(ruigiVec[0] == L"脱皮ホルモン");
		assert(ruigiVec.size()==1);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"Objective-C",L">{{Infobox プログラミング言語\n|名前 = Objective-C\n|パラダイム = [[オブジェクト指向プログラミング]]\n|設計者 = {{仮リンク|ブラッド・コックス|en|Brad Cox}}\n|型付け = [[静的型付け]]・[[動的型付け]]\n|処理系 = Apple版、GNU版\n|影響を受けた言語 = [[Smalltalk]]、[[C言語]]\n|影響を与えた言語 = [[Java]]、[[Swift (プログラミング言語)|Swift]]\n|プラットフォーム = [[OS X|Mac OS X]]、[[GNUstep]]他\n}}\n\n{{プログラミング言語}}\n'''Objective-C'''（オブジェクティブ シー）は、[[プログラミング言語]]の一種。[[C言語]]をベースに[[Smalltalk]]型の[[オブジェクト指向]]機能を持たせた上位互換言語である。\n\nObjective-Cは[[NeXT]]、[[OS X|Mac OS X]]の[[オペレーティングシステム|OS]]に標準付属する公式開発言語である。OS Xのパッケージ版に開発環境がDVDで付属するほか、ユーザ登録をすれば無償でダウンロードできる（[[Xcode]]の項目参照）。現在では主に[[アップル インコーポレイテッド|アップル]]のMac OS Xや[[iOS (アップル)|iOS]]上で動作するアプリケーションの開発で利用される。\n\n== 概要 ==\nObjective-CはCを拡張してオブジェクト指向を可能にしたというよりは、Cで書かれたオブジェクト指向システムを制御しやすいように[[マクロ (コンピュータ用語)|マクロ]]的な拡張を施した言語である。したがって、「better C」に進んだ[[C++]]とは異なり、「C & Object System」という考え方であり、ある意味2つの言語が混在した状態にある。\n\n関数（メソッド）の定義と呼び出し方が独特であるため、Objective-Cのコードは一見C++以上にCとはかけ離れた独特の記述となる。しかし、言語仕様はCの完全上位互換であり、[[if文|if]]/[[for文|for]]/[[while文|while]]などの制御文や、intなどのスカラー型、[[サブルーチン#関数|関数]]記法、宣言・[[変数 (プログラミング)|代入]]といった基本的な文法はCに準拠する。一方オブジェクトシステムはSmalltalkの概念をほぼそのまま借用したもので、動的型のクラス型オブジェクト指向ランタイムを持ち、メッセージパッシングにより動作する。このことからしばしば「インラインでCの書けるSmalltalk」または「インラインでSmalltalkの書けるC」などと呼ばれる。Cとは異なるObjective-Cに特有の部分は､@で始まる'''コンパイラディレクティブ'''で明示され、オブジェクトのメソッド呼び出しは[]で囲まれた'''メッセージ式'''で行われる。\n\n最大の特徴はオブジェクトシステムが完全に動的という点で、実行時のクラス拡張、オブジェクト汎用型idの導入により型によらない動的配列・辞書など、インタプリタに近い記述力をもつことである。実際にコードそのものはネイティブコンパイルされるものの、動作原理はほぼインタプリタに近く、コンパイラ型言語としてはまれな柔軟性を発揮する。\n\nしたがって、C側から見れば一種のスクリプトインタプリタが乗っているような状態であり、逆にオブジェクトシステムからはOS機能や膨大なC言語資源を直接利用可能なインターフェースが備わっているといえる。また仮想マシンを持たずに済むため、取り回しも良い。パフォーマンスはJavaのような中間コード型言語よりも良好で、CやC++のようなネイティブコンパイル言語には劣るとされる。Objective-C特有のこの形態は双方のメリット・デメリットが明確で、実際的な使い勝手が非常に優れている。この特性に着目したのが[[NEXTSTEP]]で、[[UNIX]]との互換性と先進的なオブジェクト指向環境の両立に成功し、その後のOS設計に大きな影響を与えることとなった。\n\n後続言語への影響としては、特に[[Java]]の基礎設計にその姿を見ることができる（[[サン・マイクロシステムズ]]が[[OPENSTEP]]に関わっていたことと関係がある）。\n\n{| class=\"wikitable\" style=\"margin:auto\"\n|+ オブジェクトシステムの概要\n|-\n!クラス\n|単一継承＋インタフェース多重継承（プロトコル）　通常はルートクラスから継承\n|-\n!オブジェクトシステム\n|[[名前束縛|動的束縛]]、[[メタクラス]]を持つ\n|-\n!型\n|[[動的型付け|動的型]]＋見た目の[[静的型付け|静的型]]のハイブリッド\n|-\n!実行速度\n|コードはCと同等のネイティブコンパイル、メソッド呼び出しは動的ディスパッチを行なうのでやや遅延する。平均してC/C++より多少遅く、中間コード型言語（Javaなど）より数倍程度高速といわれる。ただし、クリティカルな部分はいつでもCで書き直せるため、実行速度が問題になることはまずない。\n|-\n!その他\n|オブジェクトはポインタ互換、Cのスカラー型はオブジェクトではない\n|}	",&ruigiVec);
		assert(ruigiVec[0] == L"オブジェクティブ シー");
		assert(ruigiVec.size()==1);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"山形県野球場",L">{{野球場情報ボックス |\n スタジアム名称 = 中山公園野球場<br/>＜山形県野球場＞<br/>（荘内銀行・日新製薬スタジアムやまがた） |\n 愛称 = Yamagata Prefectural Baseball Stadium<br />(Shogin & Nissin Stadium Yamagata) |\n 画像 =  [[画像:yamagataStad1.jpg|300px|山形県営野球場]] |\n 所在地 = [[山形県]][[東村山郡]][[中山町]]大字長崎5081（中山公園内） |\n 起工 =  |\n 開場 = [[1980年]]（[[昭和]]55年） |\n 所有者 = 山形県 |\n 管理・運用者 = 中山町商工観光公社（[[指定管理者]]） |\n グラウンド = 内野：クレー舗装<br/>外野：[[芝|天然芝]] |\n 照明 = 照明塔：6基<br/>最大照度：投捕間2000Lx<br/>　　　　　　内野1200Lx<br/>　　　　　　外野1000Lx|\n 設計者 = |\n 旧称 = <nowiki></nowiki>\n* 山形蔵王タカミヤホテルズスタジアム（2008年4月1日 - 2011年3月31日）|\n 使用チーム、大会 = [[東北楽天ゴールデンイーグルス (ファーム)|東北楽天ゴールデンイーグルス二軍]]（[[2005年]] - 現在） |\n 収容能力 = 25,000人 |\n 規模 = グラウンド面積：12,690.8m&sup2;<br/>両翼 - 100 m<br/> 中堅 - 120 m |\n フェンスの高さ = 両翼：3.0 m、左中間・中堅：2.0 m\n}}\n'''山形県野球場'''（やまがたけん やきゅうじょう）は、[[山形県]][[東村山郡]][[中山町]]の中山公園内にある[[野球場]]で、山形県都市公園条例における施設名称は「'''中山公園野球場'''」（なかやまこうえん やきゅうじょう）と制定されており、「山形県野球場」は通称名として使用されている。\n\n山形県内を事業拠点とする[[荘内銀行]]と日新製薬の2社が共同で[[命名権|施設命名権]]（ネーミングライツ）を取得しており、[[2011年]][[4月1日]]から呼称を「'''荘内銀行・日新製薬スタジアムやまがた'''」（しょうないぎんこう・にっしんせいやくスタジアムやまがた）としている（施設命名権に関する詳細は[[#施設命名権|後述]]）。",&ruigiVec);
		assert(ruigiVec[0] == L"中山公園野球場");
		assert(ruigiVec[1] == L"荘内銀行・日新製薬スタジアムやまがた");
		assert(ruigiVec.size()==2);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"Windows bitmap",L"{{Infobox file format\n| name = Windows bitmap\n| icon = \n| extension = .bmp\n| mime = image/x-ms-bmp</tt>（非公式）<tt>\n| type code = <tt>'BMP '</tt><br /><tt>'BMPf'</tt><br /><tt>'BMPp'</tt>\n| uniform type = com.microsoft.bmp \n| magic = BM\n| owner = [[マイクロソフト]]\n| genre = [[ビットマップ]][[画像]]\n| container for = \n| contained by =\n| extended from = \n| extended to = \n| standard = \n}}\n'''BMP'''（ビーエムピー、Microsoft Windows '''B'''it'''m'''a'''p''' Image）または'''DIB'''（ディーアイビー、'''D'''evice '''I'''ndependent '''B'''itmap、デバイス独立ビットマップ）は、[[マイクロソフト]]と[[IBM]]が[[Microsoft Windows|Windows]]と[[OS/2]]にわかれる前の[[オペレーティングシステム|OS]]を共同で開発していた頃に作られた[[画像]]ファイル形式。[[データ圧縮|圧縮]]の方法についても定義されているが、Windowsが標準では無圧縮のファイルを生成するため、他のアプリケーションにおいても無指定時は、[[圧縮]]はされていない場合が多い。\n\nファイル形式の細部の変更が何度か行われており、その結果としてWindowsとOS/2で多少ファイル形式が異なることがある。\n\n機械独立のファイル形式として設計されたため、実際に存在する画像表示装置や、印刷装置が、画像を上方から処理するものがほぼ全てであるにもかかわらず、[[幾何学]]的なX軸、Y軸方向に座標を指定する形式となっている。その結果、画像を下から上に向かって記録している (Bottom up) のが特徴であるが、後に高さに負の値を指定することでその他大多数の画像ファイル形式と同じように画像を上から下へ向かって記録する (Top down) こともできるようになった。しかし互換性の面からProgramming Windowsではトップダウン形式のビットマップの作成を推奨していない。\n\nなお、[[ビットマップ画像|ビットマップ]]という呼称は画像データの表現方式のひとつであり、本項で述べている''マイクロソフト独自のファイル形式''を必ずしも指すわけではない<ref>より一般的な意味合いについては[[ビットマップ画像]]の項を参考。</ref>。",&ruigiVec);
		//ruigiVec = [2]("ビットマップ画像","ビットマップ")
		assert(ruigiVec.size()==2);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"ビジネスマン",L">'''ビジネスマン'''（{{Lang-en-short|Businessman}}）とは、英語の原義では[[実業家]]や[[経営者]]だが<ref>[[DUO]] 3.0、P556、[[鈴木陽一]]、ISBN 978-4900790056</ref><ref>[[:en:Businessperson]]</ref>、[[日本]]では特に[[営業]]を主とする交渉ごとに関わる[[サラリーマン|会社員]]を指してもこう呼ぶ。古くは同じ立場にある人を指して'''営業マン'''（えいぎょうマン）・また[[商社]]に勤めている人は'''商社マン'''（しょうしゃマン）とも呼んだ。\n\nなお注意すべきは、同じ立場にある女性の場合に“Businesswoman”（ビジネスウーマン）という呼称が用いられるが、海外では[[アメリカ合衆国|米語]][[俗語|スラング]]で街頭の[[娼婦]]をこう呼ぶケースがある他、性別を強調している事から[[セクシャルハラスメント|性差別]]と取られかねないとされる。\n\n性差を含まない呼称としては、英語では“Business person”や“Business people”というものがあり、日本でも性差に捉われず、また[[雇用の分野における男女の均等な機会及び待遇の確保等に関する法律|雇用機会均等法]]の改訂や、女性の社会進出などで社会の意識変化が起こり'''ビジネスパーソン'''が使われる（[[ポリティカル・コレクトネス]]）。\n\n==概要==\n日本における同語の扱いは、[[ホワイトカラー]][[労働者]]のうち、海外貿易の場で活躍する人を指すという用語法もあるが、1970年代以降、日本の純債権国化、[[金融自由化]]に伴い、[[シンジケートローン]]を嚆矢として、証券・外国為替取引を経て、[[M&A]]など会社そのものを取引対象とする[[投資銀行]]業務など、[[金融]]業に従事する者に対してより使用されることとなった。そもそも貿易業務は歴史的に見て金融業の発達を受けて発展してきたものであり、（貿易金融におけるコルレス契約を元として、為替業務、証券業務などは遠隔地における決済のリスクを軽減させるために発展してきた。損害保険についてもリスクヘッジ目的である。先物などデリバティブについてもリスクヘッジのために考案された。）貿易業務のあるところには必ず、金融業が介在する。また1980年代頃までは、いわゆる「総合商社」におけるホワイトカラーは高収入の代名詞であったが、[[バブル崩壊]]以後、総合商社の再編、リストラ（兼松の実質的なダウンサイジング、専門商社化。日商岩井、ニチメンの統合による双日の発足など）を経て彼らの給与は伸び悩んだ一方、[[経営学修士|MBA]]を取得し[[金融工学]]を駆使して世界各国のマーケットで巨大なディールを行う、外資系を中心とした金融業に携わる者が巨額の給与を得たことで、「ビジネスマン」像も変わっていった。\n\nなお労働の対価が他の労働者と比較して大きな格差の見られない[[訪問販売]]の営業マンや[[小売]]店の店頭販売員は、自ら[[リーダーシップ]]を発揮し利益を上げる「ビジネスマン」と呼ばれることは少ない。",&ruigiVec);
		assert(ruigiVec.size()==0);	//ビジネスパーソン、拾いたかったけど、まあいいか
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"西那須野駅",L">{{駅情報\n|社色 = green\n|文字色 =\n|駅名 = 西那須野駅\n|画像 = JR Nishinasuno Station 2010-05-02.jpg\n|pxl = \n|画像説明 = 西口（2010年5月）\n|よみがな = にしなすの\n|ローマ字 = Nishi-Nasuno\n|電報略号 = ナス\n|所属事業者= [[東日本旅客鉄道]]（JR東日本）\n|所在地 = [[栃木県]][[那須塩原市]]永田町1-1\n|座標 = {{ウィキ座標2段度分秒|36|53|3|N|139|59|11|E}}\n|開業年月日= [[1886年]]（[[明治]]19年）[[10月1日]]\n|駅構造 = [[地上駅]]（[[橋上駅]]）\n|ホーム = 2面3線（実質2面2線）\n|廃止年月日=\n|乗車人員 = 3,687\n|乗降人員 =\n|統計年度 = 2013年\n|所属路線 = {{Color|#f68b1e|■}}[[東北本線]]（[[宇都宮線]]）\n|前の駅 = [[野崎駅_(栃木県)|野崎]]\n|駅間A = 5.2\n|駅間B = 6.0\n|次の駅 = [[那須塩原駅|那須塩原]]\n|駅番号 =\n|キロ程 = 151.8km（[[東京駅|東京]]起点）<br />[[上野駅|上野]]から[[尾久駅|尾久]]経由で148.4\n|起点駅 =\n|乗換 =\n|備考 = [[日本の鉄道駅#業務委託駅|業務委託駅]]\n|備考全幅 =\n}}\n'''西那須野駅'''（にしなすのえき）は、[[栃木県]][[那須塩原市]]永田町にある、[[東日本旅客鉄道]]（JR東日本）[[東北本線]]の[[鉄道駅|駅]]である。「[[宇都宮線]]」の愛称区間に含まれている。",&ruigiVec);
		assert(ruigiVec.size()==0);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"三重エフエム放送",L">{{Notice|三重エフエム放送の表記については'''[[ノート:三重エフエム放送|ノート]]'''を参照してください。|重要|Attention}}\n{{日本のラジオ局|英名=Mie FM Broadcasting Co,. Ltd.\n|愛称=radio<sup>3</sup>（レディオキューブ FM三重）\n|特記事項=\n|}}",&ruigiVec);
		assert(ruigiVec[0] == L"radio3");
		assert(ruigiVec.size()==1);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"新潟医療福祉大学",L">{{大学\n| 国=日本\n| 大学名=新潟医療福祉大学\n| 大学の略称=特に無いが，学生の間では「'''医療福祉大'''｣「'''医療福祉'''」「'''福祉大'''」「'''医福大'''｣等と呼ぶ者が多い。又，表記上の略称としては，「'''新潟医福大'''」「'''新福大'''」と表記される場合もある\n| ウェブサイト=http://www.nuhw.ac.jp/\n}}" ,&ruigiVec);
		assert(ruigiVec[0] == L"医療福祉大");
		assert(ruigiVec[1] == L"医療福祉");
		assert(ruigiVec[2] == L"福祉大");
		assert(ruigiVec[3] == L"医福大");
		assert(ruigiVec.size()==4);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"アレニウスの式",L">{{出典の明記|date=2013年4月28日 (日) 11:26 (UTC)}}\n'''アレニウスの式'''（―しき、{{lang-en-short|Arrhenius equation}}）は、スウェーデンの科学者[[スヴァンテ・アレニウス]]が1884年に提出した、ある温度での[[化学反応]]の速度を予測する式である。5年後の1889年、[[ヤコブス・ヘンリクス・ファント・ホッフ]]によりこの式の[[物理学]]的根拠が与えられた。\n\n反応の[[速度定数]] ''k'' は\n\n:<math>k = A \\exp\\left(-\\frac{E_{\\mathrm{a}}}{RT}\\right)</math>\n::<math>A</math> ：温度に無関係な定数（[[頻度因子]]）\n::<math>E_{\\mathrm{a}}</math> ：[[活性化エネルギー]]（1[[モル]]あたり）\n::<math>R</math> ：[[気体定数]]\n::<math>T</math> ：絶対温度\nで表される。活性化エネルギー''E''<sub>a</sub> の単位として、1モルあたりではなく1粒子あたりで考えると、\n:<math>k = A \\exp\\left(-\\frac{E_{\\mathrm{a}}}{k_{\\mathrm{B}} T}\\right)</math>\n::<math>k_{\\mathrm{B}}</math> ：[[ボルツマン定数]]\nと表すことも出来る。\n\n活性化エネルギーはアレニウスパラメータとも呼ばれる。また指数関数部分 exp (-''E''<sub>a</sub> /''RT'' ) は[[ボルツマン因子]]と呼ばれる<ref name=tanaka>{{cite|和書 |author=田中一義 |author2=田中庸裕 |title=物理化学 |publisher=丸善 |year=2010 |isbn=978-4-621-08302-4 |pages=433-435}}</ref>。",&ruigiVec);
		assert(ruigiVec.size()==0);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"ヒップホップ",L">{{otheruses|ニューヨーク発祥の文化|ダンス|ヒップホップ (ダンス)|PHP処理系|HipHop Virtual Machine}}\n'''ヒップホップ''' (hip hop) は1970年代の[[アメリカ合衆国]][[ニューヨーク]]の[[ブロンクス区]]で、[[アフリカン・アメリカン|アフロ・アメリカン]]やカリビアン・アメリカン、[[ヒスパニック]]系の住民の[[コミュニティ]]で行われていた[[ブロックパーティ]]から生まれた文化。\n\n[[アフリカ・バンバータ]]による造語であり、「アフロ・アメリカンが、文化（音楽、ファッション、アート）を取り入れ、新しいスタイルを生み出すこと」をヒップホップ（hipもhopも弾ける、躍動するという意味）と呼称したのが始まりである。 これは[[1974年]][[11月]]のことだったとされる。この事から、11月を「Hip Hop History Month」として祝う習慣がある。\n\n単に「ヒップホップ」と言った場合、文化から派生した[[サンプリング]]や[[打ち込み]]を中心としたバックトラックに、[[MC (ヒップホップ)|MC]]による[[ラップ]]を乗せた音楽形態を特に指すことが一般化しているが、これらは本来はヒップホップ・ミュージックあるいはラップ・ミュージックと呼ぶのが正しい。\n\n[[ファイル:Hip hop.jpg|thumb|right|300px|ブレイクダンサー]]" ,&ruigiVec);
		assert(ruigiVec.size()==0);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"スラッシュドット効果",L">{{特筆性|date=2013年8月}}\n'''スラッシュドット効果'''（スラド効果）、'''スラッシュドット現象'''（スラド現象）、''Slashdot effect''とは、ある[[ウェブサイト]]が[[スラッシュドット]]に紹介されることで、そのサイトへの[[トラフィック]]（負荷）が爆発的に増え、サーバの限界を超えてしまうこと。一般的に[[ウェブサイト]]に短期に爆発的にトラフィック（負荷）発生している状態だけを指すときもある。\n\n弊害としてアクセス速度の低下が挙げられ、場合によってはサーバがダウンすることもある。ただし、これは一時的なものであり、それがおさまれば一転、ビジターが増えるという益もある。さらに二次的な弊害として、コミュニティサイトの場合に初心者ユーザーや[[荒らし]]ユーザーが増えて対応に困ることも少なくない。いわゆる[[ネットイナゴ]]の問題にも一部通じるものである。\n\n'''スラッシュドット効果'''が現在進行中でアクセス不能に陥っているサイトは''slashdotted''と言われることもある。用例：''The site is already totally slashdotted. I have mirrored the contents at my site.''\n\n日本においては、[[2ちゃんねる]]で紹介されたサイトに同様の現象が見られるため、これを'''2ちゃんねる効果'''と呼ぶ者もいる<ref>{{cite web|url=http://internet.watch.impress.co.jp/cda/news/2004/08/31/4434.html|title=P2P技術で“２ちゃんねる効果”を軽減できる無料CDNが正式公開|accessdate=2014-08-17}}</ref>。一方、2ちゃんねるよりも社会的影響力が低いスラッシュドット日本語版に関してそのトラフィック効果に関して言われることはほとんど無い<ref name=\"binary\">{{Cite encyclopedia|title=スラッシュドット効果|encyclopedia=IT用語辞典バイナリ|url=http://www.sophia-it.com/content/%E3%82%B9%E3%83%A9%E3%83%83%E3%82%B7%E3%83%A5%E3%83%89%E3%83%83%E3%83%88%E5%8A%B9%E6%9E%9C|accessdate=2014-08-17}}</ref>。\n\n同様の現象は、[[Yahoo!]]ニュースでも見られる。こちらはリンク先へ直接リンクが張られているのではなく、一旦Yahoo!内を経由しており、多数のアクセスにより混雑している、ページが正常に表示されない可能性があるという旨が表示されることがある<ref name=\"binary\" />。" ,&ruigiVec);
		assert(ruigiVec.size()==0);
	}
	{//異名は類義ではないということで.
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"ショーン・マイケルズ",L">{{Infobox プロレスラー\n| 名前 = ショーン・マイケルズ\n| 画像 = Shawn Michaels WM24 shot.jpg\n| 画像サイズ = \n| 画像説明 = \n| リングネーム = ショーン・マイケルズ\n| 本名 = マイケル・ショーン・ヒッケンボトム\n| ニックネーム = ハートブレイク・キッド（HBK）<br />ショーストッパー<br />Mr.レッスルマニア<br />ボーイ・トーイ<br />セクシー・ボーイ\n| 身長 = 186cm\n| 体重 = 102kg\n| 誕生日 = {{生年月日と年齢|1965|7|22}}\n| 死亡日 = \n| 出身地 = {{USA}}<br />[[テキサス州]][[サンアントニオ]]\n| 所属 = \n| スポーツ歴 = \n| トレーナー = [[ホセ・ロザリオ]]\n| デビュー = 1984年\n| 引退 = 2010年\n}}\n\n'''ショーン・マイケルズ'''（''Shawn Michaels''、[[1965年]][[7月22日]] - ）は、[[アメリカ合衆国]]の元[[プロレスラー]]。[[アリゾナ州]][[チャンドラー (アリゾナ州)|チャンドラー]]生まれ、[[テキサス州]][[サンアントニオ]]出身。身長186cm、体重102kg。左利き。本名は'''マイケル・ショーン・ヒッケンボトム'''（''Michael Shawn Hickenbottom''）。\n\n[[1990年代]]から[[2000年代]]を代表する[[WWE]]のスーパースター。[[1988年]]から2度目の引退をした[[2010年]]まで、約20年以上[[WWE]]に在籍した。プロレスラーとして卓越した技術と圧倒的な[[カリスマ]]性、ド派手なパフォーマンスなどで絶大な人気を誇り、[[ベビーフェイス (プロレス)|ベビーフェイス]]と[[ヒール (プロレス)|ヒール]]、シングルと[[タッグチーム|タッグ]]の両方で活躍した。'''ハートブレイク・キッド'''（''The Heart Break Kid''、略称'''HBK'''）、ショーストッパー=ショーの主役（The Showstopper）、ボーイ・トーイ（Boy Toy）、セクシー・ボーイ（Sexy Boy）、アイコン（The Icon）、メイン・イベンター（The Main Eventer）、ミスター・[[レッスルマニア]]（Mr. WrestleMania）など多くの異名を持つ。最終所属はWWEの[[WWE・ロウ|RAW]]。" ,&ruigiVec);
		assert(ruigiVec.size()==0);
	}

	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"皇學館大学",L">{{大学\n| 国=日本\n| 大学名=皇學館大学\n| 大学の略称=皇學館大だが、大学スポーツでは皇學大も使用している。非公式には系列校と同じく「學館」（がっかん）と呼ばれることもある\n|}" ,&ruigiVec);
		assert(ruigiVec[0] == L"皇學館大");
		assert(ruigiVec.size()==1);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"東洋大学",L">{{大学\n| 国=日本\n| 大学名=東洋大学\n| ふりがな=とうようだいがく\n| 大学の略称=東洋大および東洋。一部では洋大も使用されている\n}}" ,&ruigiVec);
		assert(ruigiVec[0] == L"東洋大");
		assert(ruigiVec[1] == L"東洋");
		assert(ruigiVec.size()==2);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"松本大学",L">{{大学\n| 国=日本\n| 大学名=松本大学\n| 大学の略称=松本大が大学の正式なものである。大学スポーツなどでは松大（まつだい）という略称も使用しているが、愛媛県松山市に所在する[[松山大学]]など、他にも略すと「松大」となる大学があるため、主に地元間での呼び名として定着している\n| ウェブサイト=http://www.matsumoto-u.ac.jp/\n}}" ,&ruigiVec);
		assert(ruigiVec[0] == L"松本大");
		assert(ruigiVec.size()==1);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"産業技術短期大学",L"{{大学 | 大学名=産業技術短期大学\n| ふりがな=さんぎょうぎじゅつたんきだいがく\n| 大学の略称=産技大、産技短、産短大、CIT（シー・アイ・ティー。英文名の略称）。また鉄大、鉄鋼短大、鉄短を用いる者もいる\n}}" ,&ruigiVec);
		assert(ruigiVec[0] == L"産技大");
		assert(ruigiVec[1] == L"産技短");
		assert(ruigiVec[2] == L"産短大");
		assert(ruigiVec[3] == L"CIT");
		assert(ruigiVec.size()==4);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"金沢大学",L">{{大学\n| 国=日本\n| 大学名=金沢大学\n| ふりがな=かなざわだいがく\n| 英称=Kanazawa University\n| 大学の略称=金大（きんだい）。[[旧官立大学]]で医学部は[[新潟大学]]、[[千葉大学]]、[[岡山大学]]、[[長崎大学]]、[[熊本大学]]とともに旧制六医科大学（[[旧六]]）をルーツとする\n| ウェブサイト=http://www.kanazawa-u.ac.jp/\n}}" ,&ruigiVec);
		assert(ruigiVec[0] == L"金大");
		assert(ruigiVec.size()==1);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"東京理科大学",L"><!--\nこの記事は[[プロジェクト:大学/大学テンプレート (日本国内)]]にしたがって作成されています。\n-->\n{{大学\n|国=日本\n|大学名=東京理科大学\n|ふりがな=とうきょうりかだいがく\n|英称=Tokyo University of Science ＜以前は Science University of Tokyo＞\n|公用語表記=\n|大学の略称=主に理科大。他に東京理科、東京理大、東理大、理大、TUS（以前はSUT）も使用されることがある\n|画像=Tokyo University of Science 2.jpg\n|画像説明=神楽坂キャンパス\n|大学設置年=1949年\n|創立年=1881年\n|学校種別=私立\n|設置者=[[学校法人東京理科大学]]\n|本部所在地=[[東京都]][[新宿区]][[神楽坂]]一丁目3\n|緯度度 =35 |緯度分 =41 |緯度秒 =57.8\n|経度度 =139 |経度分 =44 |経度秒 =29\n|キャンパス=神楽坂（[[東京都]][[新宿区]]）<br />葛飾（東京都[[葛飾区]]）<br />野田（[[千葉県]][[野田市]]）<br /> 久喜（[[埼玉県]][[久喜市]]）<br /> 長万部（[[北海道]][[山越郡]][[長万部町]]）\n|学部=理学部第一部<br />理学部第二部<br />工学部第一部<br />工学部第二部<br />薬学部<br />理工学部<br />基礎工学部<br />経営学部\n|研究科=理学研究科<br />工学研究科<br />総合化学研究科<br />科学教育研究科<br />薬学研究科<br />理工学研究科<br />基礎工学研究科<br />経営学研究科<br />生命科学研究科<br />イノベーション研究科<br />国際火災科学研究科\n|ウェブサイト=http://www.tus.ac.jp/\n}}" ,&ruigiVec);
		//ruigiVec = [6]("理科大","東京理科","東京理大","東理大","理大","TUS")
		assert(ruigiVec[0] == L"理科大");
		assert(ruigiVec[1] == L"東京理科");
		assert(ruigiVec[2] == L"東京理大");
		assert(ruigiVec[3] == L"東理大");
		assert(ruigiVec[4] == L"理大");
		assert(ruigiVec[5] == L"TUS");
		assert(ruigiVec.size()==6);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"国立西が丘サッカー場",L"><!--注意 この項目名は「国立西が丘サッカー場」です。命名権取得後の「味の素フィールド西が丘」に編集・移動しないで下さい。-->\n{{Pathnav|国立スポーツ科学センター|frame=1}}\n{{スタジアム情報ボックス\n| スタジアム名称 = 国立スポーツ科学センターサッカー場\n| 愛称 = 味の素フィールド西が丘\n| 画像 = [[画像:Nishigaoka Stadium 1.JPG|300px|西が丘サッカー場]]\n|所在地=[[東京都]][[北区 (東京都)|北区]][[西が丘]]3丁目15竏驤1<ref name=\"jleague\"/>\n|pushpin_map = Japan Wards of Tokyo\n|map_size = 250\n| 緯度度 = 35 | 緯度分 = 46 | 緯度秒 = 8.98 | N(北緯)及びS(南緯) = N \n| 経度度 = 139 |経度分 = 42 | 経度秒 = 28.29 | E(東経)及びW(西経) = E\n| 起工 = 1969年\n| 開場 = [[1972年]]\n| 修繕 = 2010年\n| 拡張 = \n| 閉場 = \n| 取り壊し = \n|所有者=[[日本スポーツ振興センター|独立行政法人日本スポーツ振興センター]]<!--[[国立スポーツ科学センター]]は「独立行政法人日本スポーツ振興センター」が管轄する一施設です。所有権者は「独立行政法人日本スポーツ振興センター」となります。-->\n|運用者=独立行政法人日本スポーツ振興センター\n|グラウンド = 天然芝\n|ピッチサイズ = 105m×68m\n| 照明 = 4基\n| 大型映像装置 = \n| 建設費 = \n| 設計者 = \n| 建設者 = \n| 旧称 = \n| 使用チーム、大会 = [[#開催された主なサッカーの大会|当項目]]を参照\n| 収容能力 = 7,258人\n| アクセス = [[#アクセス|当項目]]参照\n}}\n'''国立西が丘サッカー場'''（こくりつにしがおかサッカーじょう）は、[[東京都]][[北区 (東京都)|北区]]の[[国立スポーツ科学センター]]にある[[サッカー]]専用の球技場である。施設は[[日本スポーツ振興センター|独立行政法人日本スポーツ振興センター]]（以下「JSC」）が所有しており、JSCが運営・管理も行っている。なお、「国立西が丘サッカー場」は通称であり、正式な施設名称は「'''国立スポーツ科学センター（西が丘）サッカー場'''」である<ref name=\"aji130503\"/>。\n\n[[東京都]]中央区に本社を置く[[味の素]]が[[命名権]]を取得しており、2012年5月1日から「'''味の素フィールド西が丘'''」の名称を用いている（[[#命名権|後述]]）。\n\n== 施設概要 ==\n* 建設面積 : 1,186.65m{{sup|2}}（スタンド面積3,460m{{sup|2}}）<ref name=\"aji130503\"/>\n* 延べ面積 : 997.17m{{sup|2}}<ref name=\"aji130503\"/>\n* 芝生面積 : 10,614m{{sup|2}}（フィールド：105m×68m）\n* 夜間照明 : 4基、平均約1,200ルクス \n* 収容人員 : 7,258人（個席：5,073、立見席：2,180、障害者席：5）",&ruigiVec);
		assert(ruigiVec[0] == L"味の素フィールド西が丘");
		assert(ruigiVec.size()==1);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"一元体",L">[[数学]]において'''一元体'''（いちげんたい、{{lang-en-short|''field with one element''}}）あるいは'''標数 1 の体''' {{lang|en|(''field of characteristic one'')}} とは、「ただひとつの元からなる有限体」と呼んでもおかしくない程に[[有限体]]と類似の性質を持つ数学的対象を示唆する仮想的な呼称である。しばしば、一元体を '''F'''<sub>1</sub> あるいは '''F'''<sub>un</sub><ref group=\"note\">\"[[:wikt:un|un]]\" はフランス語で \"1\" の意味の単語であり、また一元体という対象がもつ数学的な豊かさへのわくわくする期待感を英語の[[:wikt:fun|fun]]と掛けたものともなっている。</ref> で表す。非常に紛らわしい点ではあるが、通常の[[抽象代数学]]的な意味での「ただひとつの元からなる体」は存在せず、「一元体」の呼称や「'''F'''<sub>1</sub>」といった表示はあくまで示唆的なものでしかないということには留意すべきである。その代わり、'''F'''<sub>1</sub> の概念は、抽象代数学を形作る旧来の材料である「集合と作用」が、もっとほかのより柔軟な数学的対象で置き換わるべきといった方法論を提供するものと考えられている。そういった新しい枠組みにおける理論で一元体を実現しているようなものは未だ存在していないが、[[標数]] 1 の体に類似した対象についてはいくつか知られており、それらの対象もやはり用語を流用して象徴的に一元体 '''F'''<sub>1</sub> と呼ばれている。なお、一元体上の数学は日本の[[黒川信重]]ら一部の数学者によって、[[絶対数学]]と呼ばれている。\n\n'''F'''<sub>1</sub> が旧来の意味の体にならないことは、体が通常[[加法単位元]] 0 と[[乗法単位元]] 1 という二つの元を持つことから明らかである。制限を緩めて、ただひとつの元からなる[[環 (数学)|環]]を考えても、それは 0 = 1 で全ての演算が潰れている[[零環]] {{lang|en|(trivial ring)}} であり、零環の振舞いと有限体の振る舞いは大きく違うものになってしまう。提案されている多くの '''F'''<sub>1</sub> 理論では抽象代数学をすっかり書き換えることが行われており、[[ベクトル空間]]や[[多項式環]]といった旧来の抽象代数学でしばしば扱われる数学的対象は、その抽象化された性質とよく似た性質を持つ新しい理論における対応物で置き換えられている。このような新しい理論は[[可換環論]]や[[代数幾何学]]の新しい基礎付けとそれに基づく展開を許すものである。こういった '''F'''<sub>1</sub> についての理論の決定的な特徴のひとつは、新しい基礎付けのもとで古典手粋な抽象代数学で扱ったものよりも多くの数学的対象が扱えるようになり、そのなかに標数 1 の体であるかのように振舞う対象が取り出しうるということである。\n\nおそらく、史上初めて '''F'''<sub>1</sub> の数学的研究の可能性が示唆されたのは1957年、[[ジャック・ティッツ]]による[[射影幾何学]]における対称性と[[単体的複体]]の組合せ論の間の類似性の基礎についての論文 {{Harv|Tits|1957}} においてである。'''F'''<sub>1</sub> は[[非可換幾何学]]や[[リーマン予想]]に関係するものとされた。'''F'''<sub>1</sub> に関する理論は数多く提案されているが、'''F'''<sub>1</sub> としてあるべき性質が全て満たされるような決定版といえるようなものは未だに明らかとされてはいない。",&ruigiVec);
		assert(ruigiVec[0] == L"絶対数学");
		assert(ruigiVec.size()==1);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"叡山電鉄",L">{{Redirect|えいでん|日本の家電量販店チェーン|エイデン}}\n{{参照方法|date=2011年9月}}<!--特に「概要」「運賃」セクションについて。-->\n{{基礎情報 会社|\n社名 = 叡山電鉄株式会社|\n英文社名 = Eizan Electric Railway Co., Ltd.|\nロゴ = |\n画像 = [[ファイル:Eiden 900 Series 01.jpg|280px]]|\n画像説明 = 900系電車「きらら」|\n種類 = [[株式会社]]|\n市場情報 = 株式非公開|\n略称 = 叡山電車、叡電|\n国籍 = {{JPN}}|\n本社郵便番号 = 606-8007|\n本社所在地 = [[京都府]][[京都市]][[左京区]]山端壱町田町8番地の80|\n本店郵便番号 = 606-8007|\n本店所在地 = 京都府京都市左京区山端壱町田町14番地の1|\n設立 = 1985年（昭和60年）7月6日|\n業種 = 陸運業|\n事業内容 = 旅客鉄道事業 他|\n代表者 = |\n資本金 = 2億5000万円|\n売上高 = |\n従業員数 = |\n決算期 = |\n主要株主 = [[京阪電気鉄道]] 100%|\n主要子会社 = |\n関係する人物 = |\n外部リンク = [http://eizandensha.co.jp  eizandensha.co.jp]|\n特記事項 = |\n}}\n'''叡山電鉄株式会社'''（えいざんでんてつ、''Eizan Electric Railway Co., Ltd.''）は、[[京都府]][[京都市]][[左京区]]の[[出町柳駅]]から八瀬・鞍馬への路線を運営する[[鉄道事業者|鉄道会社]]。通称「'''叡山電車'''」。略称は「'''叡電'''」（えいでん）<ref name=\"ryakushou\" />。本社は京都市左京区山端壱町田町8番地の80（修学院駅に隣接）、本店は京都市左京区山端壱町田町14番地の1。\n\n[[1985年]]に[[京福電気鉄道]]（京福）の完全子会社として設立されたが、[[1991年]]11月に[[京阪電気鉄道]]（京阪）が[[株主|筆頭株主]]となったのち、[[2002年]]3月からは同社の完全子会社となっている（後述）。\n\n[[全国登山鉄道‰会]]に加盟している。\n\n== 概要 ==\n[[叡山電鉄叡山本線|叡山本線]]・[[叡山電鉄鞍馬線|鞍馬線]]の2路線を運営し、[[比叡山]]・[[鞍馬]]方面への観光客輸送、周辺住民輸送を担っている。\n\nこれらの路線は、かつて[[京都電燈]]および[[鞍馬電気鉄道]]により運営され、電力事業の戦時統制による再編後は京福が運営していた。1960年代からのモータリゼーションにより[[1964年]]から乗客数が減少に転じていたが、[[1978年]]9月の[[京都市電]]全廃により他の鉄道路線からの連絡が絶たれたことで沿線から京都市内中心部を直結する[[路線バス]]に乗客が流れ、叡山本線、鞍馬線の利用客は一気に減少した。特に叡山本線は年間5億円以上の赤字を出し、京福電鉄全体、ひいては京福を配下にもつ京阪グループの経営を圧迫する事態となった<ref name=\"KSp253\"/>。\n\n叡山本線・鞍馬線の赤字を京福本体が負担し金利負担を低減すること、人件費の削減を目指して[[1985年]]7月に京福電鉄全額出資、全社員が京福電鉄から出向する形で叡山電鉄株式会社が設立された<ref name=\"KSp253\"/><ref name=\"KSp254\"/>。\n\n分離以前から電鉄側でも集客に努めており、各種イベントの開催や、京福に残った[[森のゆうえんち|八瀬遊園]]<ref>子会社の「比叡産業」に経営委託。</ref>の次々の改装のほか、特にこの時期においてさえも出町柳 - 宝ケ池間で日中毎時8本運転とし「待たずに乗れる」を印象付けていたことは特筆に値する。それでも乗客数は会社設立時の1985年でそれまでのピークであった1964年の4割にまで落ち込み、なお減少傾向であった。\n\n[[1989年]][[10月5日]]に[[京阪鴨東線]]が開業して叡山本線と出町柳で連絡することで叡山電鉄の旅客数は倍増、収入も2.5倍となったが、この状態でも収支はようやく黒字になる程度で、19億4千万円に達していた累積損失の解消のめどは立たず、京福電鉄傘下での経営再建は不可能と判断された<ref name=\"KSp254\"/>。京阪電鉄からの特別融資を低利で受けることなどを目的に[[1991年]]10月に株式の60%を京阪電鉄に売却、併せて人件費削減のため従業員全員が叡山電鉄に移籍した<ref name=\"KSp254\"/><ref name=\"KSp255\"/>。その後[[2002年]]3月に京阪が残りの全株式を取得し、叡山電鉄は京阪電鉄の100%子会社となった。\n\nいったん増加した利用客も[[1997年]]に[[京都市営地下鉄烏丸線]]が[[国際会館駅]]まで延長され、その後同駅を中心にバス路線網も再整備されたため、左京区岩倉地域の住民が市中心部へ向かう場合などの利用がそちらに移転することになった結果、利用者が減少しつつある。\n\nこれに対して1997年にパノラミック電車「[[叡山電鉄900系電車|きらら]]」を登場させ、各種[[イベント]]を開催するなど積極的な利用者確保を行なうと共に、[[2004年]]には原則として全列車の[[ワンマン運転|ワンマン]]化を行うなどの経費削減策も講じている。[[2005年]]は、[[日本放送協会|NHK]][[大河ドラマ]]『[[義経 (NHK大河ドラマ)|義経]]』のために鞍馬方面の[[観光]]が[[ブーム]]になり、近年にない賑わいを見せた。\n\n京都市建設局により、並行して走る[[鞍馬街道]]（[[京都府道38号京都広河原美山線]]）のバイパス建設や拡幅整備が計画されており<ref>{{cite web|url=http://www.city.kyoto.lg.jp/kensetu/page/0000012793.html|title=主要府道京都広河原美山線二ノ瀬バイパスの整備|publisher=京都市|accessdate=2014-09-18}}</ref>、完成すると鞍馬方面への大型観光バス等の通行が容易となることから、その将来は必ずしも安泰ではない。\n\n略称の「叡電」は現在の叡山本線が京都電燈の'''叡山電鉄部'''であったことに由来し、京福となったあとも叡山本線・鞍馬線の総称として一般に使われていた。分社によって、略称が文字通り会社名の省略形となった<ref name=\"ryakushou\">家電量販店の[[エディオン]]の前身の一つで、愛知県名古屋市に本社を置いていた「[[エイデン|株式会社エイデン]]」は旧社名の「栄電社」が由来であり叡山電鉄とは全く無関係である。<!--全国的にはTVCMなどで旧栄電社が「エイデン」として知られ、同社との関係を記述する必要があるが、本文に記するほどではない --></ref>。\n\n分社当時、京福が京阪の子会社<ref>京福の発行済株式の約40%を京阪が保有している。</ref>であったものの、車両技術面では[[阪神電気鉄道]]から[[阪神801形電車|831形]]を譲り受けて[[京福電気鉄道デナ500形電車|デナ500形]]として導入したことがあったことや、阪神の関連会社であった[[武庫川車両工業]]（現在は廃業）での車両製造の経緯などから阪神色が強いなど、独特の京福（叡電）色を有していた。1989年の京阪鴨東線開業あたりから、サービス面で徐々に京阪色も出てきていた<ref>自動券売機の路線図が京阪様式になる、制帽がいわゆる「京阪[[ケピ帽|ドゴール帽]]」になるなど。</ref>が、2002年に京福が[[京福電気鉄道越前本線列車衝突事故|越前本線列車衝突事故]]による経営悪化のため所有株をすべて京阪に売却し同社の完全子会社となって以降、京福色から京阪色の色合いが多岐にわたって濃くなってきている<ref>駅、車内の広告や告知、案内放送のほか、車両への部品再利用等が行われている<!-- 車両のみだけでない -->。</ref>。ロゴマークは2008年以降、京阪電鉄と同じ「'''KEIHAN'''」で、下に「叡山電車」の文字が入ったものである。\n\n[[スルッとKANSAI]]でカードに印字される符号は'''EZ'''である。",&ruigiVec);

		assert(ruigiVec[0] == L"叡山電車");
		assert(ruigiVec[1] == L"叡電");
		assert(ruigiVec.size()==2);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"笠間杲雄",L">{{出典の明記|date=2010年7月}}\n'''笠間 杲雄'''（かさま あきお [[1885年]][[11月]] - [[1945年]][[4月1日]]）は、[[日本]]の[[外交官]]。戦前日本においては数少ない[[イスラム世界|イスラーム圏]]に関する専門家の一人であり、また[[阿波丸事件]]の犠牲者としても知られている。\n\n== 経歴 ==\n[[東京府]][[東京市]][[神田区]]神田末広町（現在の[[東京都]][[千代田区]][[外神田]]三丁目）の出身。[[1909年]]、[[東京大学|東京帝国大学]][[東京大学大学院法学政治学研究科・法学部|法科大学]]卒業後、[[文官高等試験]]に合格し、[[鉄道院]]に入った。[[1918年]]には[[外務省]]に移り、[[参事官]]兼外務書記官に任官。その後、情報部第2課長、欧州各地の大使館在勤を経験した。特に[[1928年]]に開催された第2回[[国際移民会議]]日本代表を務め、その後は[[イラン|ペルシャ]]、[[ポルトガル]][[公使]]を歴任した後、[[1938年]]に外務省を退官した。\n\n1938年発足の国策研究機関「[[太平洋協会]]」の常務理事に就任し、[[太平洋戦争]]（[[大東亜戦争]]）開戦後の[[1942年]]には、[[陸軍省]]軍属・陸軍司政長官に任じられ、南方占領地の調査事業のうち太平洋協会が分担していた旧英領[[ボルネオ島|ボルネオ]]に赴くことになった。[[1943年]]1月より、[[第37軍 (日本軍)|ボルネオ守備軍]][[司令部]]付調査局長を務め<ref>[http://borneo.web.infoseek.co.jp/kagyou.htm ボルネオ歴史事典 か行] ボルネオ歴史事典／望月雅彦編纂</ref>、[[1945年]]3月に陸軍省[[軍務局]]付となった。しかし1945年4月1日、当時日本軍の占領下にあった[[シンガポール]]から輸送船[[阿波丸#阿波丸（日本郵船）|阿波丸]]にて日本へ帰国する途中、米潜水艦による魚雷爆撃で同船は沈没、遭難死した（[[阿波丸事件]]）。",&ruigiVec);
		assert(ruigiVec.size()==0);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"タイ・シンクロトロン光研究所",L"'''タイ・シンクロトロン光研究所''' ([[タイ語]]:{{lang|th|犧ｪ犧籾ｸｲ犧壟ｸｱ犧吭ｸｧ犧ｴ犧謂ｸｱ犧｢犹≒ｸｪ犧・ｸ金ｸｴ犧吭ｹもｸ・ｸ｣犧歩ｸ｣犧ｭ犧凩}、[[英語]]：Synchrotron Light Research Institute 英略称：SLRI）は、[[タイ王国]] [[内閣 (タイ)|内閣]][[科学技術省 (タイ)|科学技術省]]管轄の研究所。'''タイ国立シンクロトロン光源加速器研究センター'''（NSRC）として、[[1996年]][[3月5日]]、[[ナコーンラーチャシーマー県]][[スラナリー工科大学]]内に開所した。",&ruigiVec);
		assert(ruigiVec[0] == L"SLRI");
		assert(ruigiVec.size()==1);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"定形外郵便",L">'''定形外郵便'''（ていけいがいゆうびん）とは、「第一種定形外郵便物」の通称である。単に'''定形外'''ともいう。\n\n第一種郵便物とは、第二種（はがき）や第三種、第四種以外の郵便物のことで、主に封書や箱に入った郵便物のことである。第一種郵便物は大きさ・重量によって定形と定形外に分類され、このうち定形外に分類されるものを第一種定形外郵便物、通称：定形外郵便という。\n\n定形外郵便とは、郵便物の形状や大きさを表す表現でしかないため、郵便物の種別として呼ぶには不十分である。普通郵便の定形外は種別としては普通郵便であるし、書留の定形外は種別としては書留である。[[ネットオークション]]で俗に呼ばれている「定形外郵便」とは、種別でいえば[[普通郵便]]を指すのが通例である。\n\n{{Main2|定形外普通郵便の配達方法|普通郵便}}\n\n== 概要 ==\n定形サイズとは\n:*縦23.5cm以内×横12.0cm以内\n:*厚さ1cm以内\n:*50g以内\nの郵便物のことで、このいずれかを条件を超えると定形外となる。\n\n定形外の最大サイズは\n:*最も長い辺が60cm以下\n:*縦×横×高さの合計が90cm以下\n:*重量が4kg以内\nである。",&ruigiVec);
		assert(ruigiVec[0] == L"第一種定形外郵便物");
		assert(ruigiVec[1] == L"定形外");
		assert(ruigiVec.size()==2);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"ソパ・デ・マリスコス",L">[[File:Seafood soup.JPG|thumb|ソパ・デ・マリスコスの例。写真はインドネシアのジャカルタの食堂で撮影された海鮮スープである。]]\n\n'''ソパ・デ・マリスコス'''とは、[[魚介類]]の入った[[スープ]]全般をさす[[スペイン語]]、またはそのようなスープ料理全般のことである。特定の1種類のスープ料理をさす言葉ではない。スペイン語では「Sopa de mariscos」と書き、「Sopa」は「スープ」、「de」は「～からなる」、「mariscos」は「魚介類」を意味するので読んで字の如くである。ただしスープの具として使用する食材は必ずしも魚介類だけに限定されているわけではなく、魚介類の他に[[野菜]]などが入れられることもある。ソパ・デ・マリスコスは別にスペイン料理というわけではなく、魚介類の採れる地域であれば普通に見られる料理の1つでありアジアなどスペイン語圏以外でも見られる。つまり[[トムヤムクン]]（エビ入りスープ）、[[クラムチャウダー]]（二枚貝の入ったスープ）、[[三平汁]]（日本の魚介類を用いた汁物の1つ）などもスペイン語で言うソパ・デ・マリスコスである。",&ruigiVec);
		assert(ruigiVec.size()==0);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"地震",L">{{Otheruses||クルアーンのスーラ|地震 (クルアーン)}}\n'''地震'''（じしん、{{lang-en-short|Earthquake}}）という語句は、以下の2つの意味で用いられる<ref name=\"SSJ (2007)\">[[#SSJ (2007)|日本地震学会地震予知検討委員会(2007)]]</ref>。\n\n# [[地震学]]における定義: [[地球]]表面を構成している岩盤（[[地殻]]）の内部で、固く密着している岩石同士が、'''[[断層]]'''と呼ばれる破壊面を境目にして、急激にずれ動くこと。これによって大きな地面の[[振動]]が生じこれを'''[[地震動]]'''（じしんどう）という。\n# 地表面のゆれ: 地震動のことで一般的にはこちらも「地震」と呼ばれる。「地震」（なゐふる）という語句は『[[日本書紀]]』にも見え、その他[[古文書]]の記録にも登場するが、これらは今日の地震学における地震動のことであり、また「大地震」、「小地震」などと共に[[震度]]の程度を表すものでもあった<ref>{{PDFlink|[http://sakuya.ed.shizuoka.ac.jp/rzisin/kaishi_18/01-Usami.pdf 宇佐美龍夫 (2002)]}} 宇佐美龍夫 「歴史史料の「日記」の地震記事と震度について」『歴史地震』 第18号、1-14、2002年</ref>。\n\n地震を対象とした[[学問]]を'''[[地震学]]'''という。地震学は[[地球物理学]]の一分野であり、[[構造地質学]]と密接に関わっている。\n\n== 概要 ==\n[[ファイル:Nojima fault side view.jpg|thumb|200px|[[兵庫県南部地震]]（[[阪神・淡路大震災]]）によって発生した[[野島断層]]。地震の[[震源]]となった[[断層]]のずれが波及して「地表地震断層」として現れたものである。激しい揺れを起こした断層本体（震源断層、起震断層）とは別のものであり、また地下に存在する断層のほとんどは地表から観察できないので、防災上注意しなければならない。]]\n[[ファイル:Pwave.png|thumb|200px|地震計で観測された地震動のグラフ。]]\n地下の[[岩盤]]には様々な要因により[[力]]（ひずみ）がかかっており、急激な変形によってこれを解消する現象が地震である。[[地球]]の内部で起こる[[地質学|地質]]現象（地質活動）の一種。地震に対して、地殻が非常にゆっくりとずれ動く現象を[[地殻変動]]と呼ぶ。\n\n地震によって変形した[[岩石]]の断面を'''[[断層]]'''といい、地下数kmから数十kmの深さにあって地表までは達しないことが多いが、大きな地震の時にはその末端が地表にも現れて'''地表地震断層'''となる場合がある。一度断層となった面は強度が低下するため繰り返し地震を引き起こすと考えられている。特に[[カリフォルニア]]にある[[サンアンドレアス断層]]は1,000km以上に及ぶ長大なもので繰り返し地震を起こしており、日本の[[地震学]]者に地震と断層の結びつきを知らせたことで有名である。日本では[[兵庫県南部地震]]の[[野島断層]]、[[濃尾地震]]の[[根尾谷断層]]、[[北伊豆地震]]の[[丹那断層]]などが有名。\n\n地震によって生じる[[振動]]は高速の'''[[地震波]]'''となって地中を伝わり、[[人間]]が生活している地表でも[[地震動]]として感じられる。\n\n地震波は[[波]]の一種であり、地中を伝わる波（実体波）と地表を伝わる波（[[表面波]]）に大別される。実体波はさらに、速度が速い[[地震波#P波|P波]]（たて波、疎密波）と、速度が遅い[[地震波#S波|S波]]（横波、ねじれ波）に分けられる<ref group=\"注\">表面波も[[レイリー波]]と[[ラブ波]]に分けられる。</ref>。\n\n地震のはじめに感じられることが多い細かい震動（[[初期微動]]）はP波、地震の激しい震動（主要動）は主にS波による。P波とS波は伝わる速度が違うので、P波とS波の到達時間の差である初期微動の時間<ref group=\"注\">[[初期微動]]継続時間という。</ref>が[[震央]]と観測地点との間の距離に比例する。初期微動が長いほど震源は遠い。初期微動が長くかつ主要動が大きい場合は、震源が遠いにも関わらず[[振幅]]が大きいので、大地震の可能性が考えられる。また、P波はS波よりも速いので、P波を検知したときに[[警報]]を出せば被害が軽減できることから、[[緊急地震速報]]や緊急停止システム<ref group=\"注\">鉄道、[[新幹線]]・[[エレベーター]]の緊急停止（P波管制運転）などで使用されているシステム。</ref>で応用されている。\n\n地下で断層が動いた時、最初に動いた地点を'''[[震源]]'''と呼び、地上における震源の真上の地点を[[震央]]と呼ぶ。テレビや新聞などで一般的に使用される震源は震央の位置を示している。震源が動いた後もまわりに面状にずれが生じ、[[震源域]]と呼ばれるずれた部分全体が地震波を発する。\n\n地震波の速度はほぼ一定であり上記のように異種の波がある性質を利用して<ref group=\"注\">地震波の速度は地殻の[[密度]]（深さ）により異なるため、実際には観測に基づき地震波速度を予めまとめた「走時表」を用いて算出する。</ref>、'''[[地震計]]'''で地震波を観測することにより、1地点以上の観測で観測地点から震央までの距離<ref group=\"注\">地震計は東西方向、南北方向、上下方向の3種類の地震動の大きさをはかるので、大体の方向（16方位程度）がわかる。</ref>、2地点以上の観測で震央の位置、3地点以上の観測で震源の深さを求めることができる。この算出式は[[大森房吉]]が1899年に発表したので、「（震源の）[[大森公式]]」と呼ばれている。このほかに地震を含めた地下の諸現象の解明や、[[核実験]]の監視などに有用であることから世界的に地震観測網が整備されている。日本は地震災害が多いことから地震計や[[震度計]]が数千か所の規模で高密度に設置され、[[気象庁]]による迅速な[[地震情報]]発表や緊急地震速報などに活用されている。\n\nなお、一つの地震の地震波にはいろいろな[[周期]]（[[周波数]]）の成分が含まれており、その違いによって被害が異なるほか、近隣の地域でも[[地層|表層地盤]]の構造や[[建築物|建物]]の大きさ・形状によって揺れ方が大きく異なることが知られている（詳細は[[#地震動・地震波と揺れ|後述]]参照）。\n\nまた地震は、震源の深さによって、浅発地震、稍（やや）深発地震、[[深発地震]]の3つに分類される。前者の境界は60kmまたは70kmとされる場合が多く、後者の境界は200kmまたは300kmとされる場合が多いが、統一した定義はない。震源が深い地震は同じ規模の浅い地震に比べて地表での揺れは小さい。ただし、地下構造の影響により震央から離れた地点で大きく揺れる[[異常震域]]が現れることがある。\n\nこのほかに地震を特徴付けるものとして、[[発震機構]]とよばれる断層の動き方（[[#メカニズム|後述]]）や地震の大きさなどがある。",&ruigiVec);
		assert(ruigiVec.size()==0);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"グラフ誌",L"'''グラフ誌'''（{{lang-zh-short|画謚･}}<ref>[http://dictionary.goo.ne.jp/leaf/jc/33091/m0u/グラフ誌/ グラフ誌 - 日中辞書] - goo辞書（デイリーコンサイス日中辞典）</ref><ref group=\"注\">[[中華人民共和国|中国]]には『[[:zh:人民画謚･ (1946窶骭1948)|人民画謚･ (1946窶骭1948)]]』などがある。</ref>）は、グラフ･ジャーナリズムを体現した[[雑誌]]の通称<ref>[http://kotobank.jp/word/グラフ誌 グラフ誌 とは] - [[コトバンク]]（[[世界大百科事典]] 【グラフ･ジャーナリズム】）</ref><ref name=\"visual\">[http://www.ndl.go.jp/jp/event/exhibitions/visual_kaisetsu.pdf p.16,25 第2章 グラフ誌] - [[国立国会図書館]]企画展示「ビジュアル雑誌の明治・大正・昭和」（平成24年）</ref>。\n\n[[写真]]を主体とした雑誌<ref group=\"注\">報道路線ではない『[[ナショナルジオグラフィック (雑誌)|ナショナルジオグラフィック]]』のような雑誌もあるが、グラフ誌の範疇になるかは不明。　（[http://nationalgeographic.jp/nng/article/20120912/322684/index3.shtml 第30回『ナショジオ』と赤シャツと『ライフ』] - ナショナル ジオグラフィック(NATIONAL GEOGRAPHIC) 日本版公式サイト）</ref>という定義で単に'''グラフ'''<ref>[http://dictionary.goo.ne.jp/leaf/jn2/63605/m0u/ グラフ【graph】の意味 - 国語辞書] - goo辞書（デジタル大辞泉）</ref>や、'''グラフ雑誌'''という呼び方もされる。\n\n== 概要・歴史 ==\n日本においては『[[風俗画報]]』（1889-1916）を最初のグラフ誌とする説がある。初めて誌名に'''画報'''<ref group=\"注\">'''画報'''というタイトルでも、実際は挿絵雑誌やファッション雑誌であるものもある。</ref>という語を用いた雑誌ともいわれる<ref>[http://www.kinokuniya.co.jp/03f/kinoline/1303_01.pdf JKBooks 第一弾「Web 版風俗画報」リリース開始！] - [[紀伊國屋書店]]（2013）</ref>。\n\n『[[ライフ (雑誌)|LIFE]]』（{{Flagicon|USA}} 1936-1972、1978-2000、2004-2007）に代表されるように、[[1930年代]]に世界各地でグラフ雑誌文化が盛り上がったといわれる<ref>[http://doors.doshisha.ac.jp/webopac/bdyview.do?bodyid=TB12285934&elmid=Body&lfname=031000950003.pdf p.47 1．はじめに－『写真週報』とは] - 『[[写真週報]]』に見る人物表象の量的分析（家永梓）</ref>。[[報道写真]]を用いた構成は、[[1920年代]]にドイツの印刷物を中心に流行った「フォト・ルポルタージュ」（複数の写真で作者の意図を伝えようとする手法）がルーツであるという説がある。一方で、『[[FRONT]]』（{{Flagicon|JPN}} 1942-1945）のように構成主義を重んじたグラフ誌も登場した<ref>[http://www.lib.kobe-u.ac.jp/repository/81002916.pdf 「対外宣伝グラフ 雑誌『FRONT』における『立体性』] - Kobe University Repository</ref>。\n\n[[第二次世界大戦]]中には、各国で[[プロパガンダ]]にも用いられた<ref>[http://www.japandesign.ne.jp/KUWASAWAJYUKU/KOUZA/5/RENZOKU/YAKUSHIJIN/ 桑沢/99-5期/連続講座1/薬師神 親彦] - ジャパンデザインネット</ref>。\n\n写真を生かすため、[[A3]]サイズのもの（前述の『FRONT』の初期）や、全ページに[[コート紙]]を用いた特徴のもの<ref>[http://www.tn-p.co.jp/print_qanda.html 印刷Q&A] - 田中プリント</ref>もある。\n\n[[テレビ]]の登場・普及により、記録性はさておき、速報性と具体性のハンデからその需要が消えていったともいわれ、広告収入減少などにより著名誌も廃刊に至った<ref name=\"visual\"/>。2000年に休刊した[[アサヒグラフ]]は、2011年3月11日の[[東日本大震災]]発生後の23日に、緊急復刊号（週刊朝日臨時増刊扱い）を発行した。新聞社発行のグラフ誌は、戦後の大きな災害事故ごとに臨時増刊号を出してきたという歴史もあった。\n\n[[高校野球グラフ]]は2013年現在も、地方新聞社などが発行している。",&ruigiVec);
		assert(ruigiVec.size()==0);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"スマートフォン",L">[[File:Smartphones.jpg|thumb|[[Android]] OS スマートフォン]]\n'''スマートフォン'''（{{lang-en-short|[[w:Smartphone|Smartphone]]}}、'''スマホ'''）とは、[[携帯電話]]機の一形態を指す用語である。明確な定義はないが<ref>[http://www.soumu.go.jp/main_content/000143085.pdf 総務省 スマートフォンをめぐる現状と課題]</ref><ref>[http://www.soumu.go.jp/main_content/000101732.pdf 情報通信ネットワーク産業協会 スマートフォンにおけるセキュリティの課題と背景]</ref>、多機能携帯電話であること<ref>[http://www.kddi.com/yogo/%E3%83%A2%E3%83%90%E3%82%A4%E3%83%AB/%E3%82%B9%E3%83%9E%E3%83%BC%E3%83%88%E3%83%95%E3%82%A9%E3%83%B3.html KDDI用語集]</ref>が要件とされる場合が多い。[[タブレット (コンピュータ)|タブレット]]と同様に「[[スマートデバイス]]」の一種とされる<ref>[http://www.sophia-it.com/content/%E3%82%B9%E3%83%9E%E3%83%BC%E3%83%88%E3%83%87%E3%83%90%E3%82%A4%E3%82%B9 スマートデバイスとは]</ref>。",&ruigiVec);
		assert(ruigiVec[0] == L"スマホ");
		assert(ruigiVec.size()==1);
	}


	{
		std::vector<std::wstring> ruigiVec;
		parseAimai(L"ファットマン",L">'''ファットマン'''とは、[[英語]]で太った人の意。数々のおとぎ話にも登場する。\n\n* '''ファットマン・ビッグ''' ⇒ [[アニメ]]『[[戦闘メカ ザブングル]]』\n:* 金持ちの令嬢エルチ・カーゴに心酔する、寡黙で忠実なボディガード（[[声優|声]]：[[銀河万丈]]）。\n* '''ファットマン''' ⇒ [[漫画]]等『[[スプリガン (漫画)|スプリガン]]』（皆川亮二作）\n* '''ファットマン''' ⇒ [[ゲーム]]『[[メタルギアソリッド2]]』（[[コナミ]]）\n*: 名前は[[長崎市|長崎]]に投下された[[原爆]]の[[コードネーム]]から。\n\n{{aimai}}\n{{DEFAULTSORT:ふあつとまん}}\n[[Category:アニメの登場人物]]\n[[Category:漫画の登場人物]]",&ruigiVec);
		assert(ruigiVec.size()==0);
	}


	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"サクランボ",L"{{Otheruses|果実|太宰治の小説・櫻桃|太宰治|その他|さくらんぼ}}\n[[ファイル:W outou4051.jpg|thumb|サクランボ（桜桃）]]\n'''サクランボ'''または'''桜桃'''（おうとう）は、[[バラ科]]サクラ属サクラ亜属の[[果樹]]であるミザクラ(実桜)の[[果実]]。食用。\n\n== 概要 ==\n木を桜桃、果実をサクランボと呼び分ける場合もある。生産者は桜桃と呼ぶことが多く、商品化され店頭に並んだものはサクランボと呼ばれる。サクランボは、[[サクラ|桜]]の実という意味の「桜の坊」の「の」が[[撥音便]]となり、語末が短母音化したと考えられている。\n\n花を鑑賞する品種のサクラでは、実は大きくならない。果樹であるミザクラには東洋系とヨーロッパ系とがあり、日本で栽培される大半はヨーロッパ系である。品種数は非常に多く1,000種を超えるとされている。\n\n果実は丸みを帯びた赤い実が多く、中に[[種子]]が1つある核果類に分類される。品種によって黄白色や[[葡萄]]の[[巨峰]]のように赤黒い色で紫がかったものもある。生食用にされるのは甘果桜桃の果実であり、日本で食されるサクランボもこれに属する。その他調理用には酸味が強い酸果桜桃の果実が使われる。\n\n殆どの甘果桜桃は[[自家不和合性 (植物)|自家不和合性]]があり、他家[[受粉]]が必要である。受粉には最低限自家不和合性遺伝子型（S遺伝子型）が異なる必要があり、異なる品種なら何でも良いというわけではない。極僅かだが自家結実する品種もある。一方、酸果桜桃は全ての品種に自家和合性が有る。\n\n一般には「初夏の味覚」であり、サクランボや桜の実は[[夏]]の[[季語]]であるが、近年では[[温室]]栽培により[[1月]]初旬の出荷も行われている。[[正月]]の初出荷では贈答用として約30粒程度が入った300グラム詰めで3万円から5万円程度で取り引きされ、'''赤い宝石'''と呼ばれることがある。",&ruigiVec);
//		assert(ruigiVec[0] == L"赤い宝石");
		assert(ruigiVec.size()==0);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseAimai(L"ファットマン",L">'''ファットマン''' (Fat Man) とは、「太った人」という意味の[[英語]]。固有[[名詞]]としては、以下のものがある。\n\n* [[ファットマン]] - 人類が、史上二番目に実戦使用した[[原子爆弾]]の爆弾名。\n* [[ファットマン (架空の人物)]] - [[アニメ]]、[[漫画]]等に登場した架空の人物名。複数存在する。\n* [[ファットマン (ゲーム)]] - [[プロレス]]を題材とした[[日本]]の[[テーブルトークRPG]]。\n* [[メガドライブ]]用[[対戦格闘ゲーム]]。上記と同様に[[ファットマン (ゲーム)]]を参照。\n\n{{Aimai}}\n{{デフォルトソート:ふあつとまん}}",&ruigiVec);
		assert(ruigiVec.size()==0);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"八洲学園大学",L">{{大学 \n| 国=日本\n| 大学名=八洲学園大学\n| ふりがな=やしまがくえんだいがく\n| 英称=Yashima Gakuen University\n| 画像=Yashima Gakuen Univ. Yokohama.jpeg\n| 画像説明=\n| 大学設置年=2004年\n| 創立年=2004年\n| 学校種別=私立\n| 設置者=[[学校法人八洲学園]]\n| 本部所在地=[[神奈川県]][[横浜市]][[西区 (横浜市)|西区]][[桜木町]]7丁目42番地\n| キャンパス=横浜本学キャンパス\n| 学部=[[生涯学習学部]]\n| 研究科=なし\n| 大学の略称=特になし\n| ウェブサイト=http://www.yashima.ac.jp/univ/\n}}\n" ,&ruigiVec);
		assert(ruigiVec.size()==0);
	}

	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"サッカーノルウェー代表",L">{{Otheruses|男子代表|女子代表|サッカーノルウェー女子代表}}\n\n{{サッカーナショナルチーム\n|国={{NOR}}\n|協会=[[ノルウェーサッカー協会]]\n|愛称=-\n|監督={{Flagicon|NOR}} [[:en:Egil Olsen|エギル・オルセン]]\n|最多出場選手=[[ヨン・アルネ・リーセ]]\n|出場試合数=110\n|最多得点選手=[[:en:Jorgen Juve|ユルゲン・ユーヴェ]]\n|得点数=33\n|初の国際試合開催年=1908\n|初の国際試合開催月日=7月12日\n|初の国際試合対戦相手=スウェーデン\n|初の国際試合スコア=3-11\n|最大差勝利試合開催年=1946\n|最大差勝利試合開催月日=6月28日\n|最大差勝利試合対戦相手=フィンランド\n|最大差勝利試合スコア=12-0\n|最大差敗戦試合開催年=1917\n|最大差敗戦試合開催月日=10月7日\n|最大差敗戦試合対戦相手=デンマーク\n|最大差敗戦試合スコア=0-12\n|W杯出場回数=3\n|W杯初出場年=1938\n|W杯最高成績=ベスト16（[[1998 FIFAワールドカップ|1998フランスW杯]]）\n|ナショナルチームによる国際大会=[[UEFA欧州選手権]]\n|国際大会出場回数=1\n|国際大会最高成績=グループリーグ敗退\n|pattern_la1=_norwayh2010|pattern_b1=_umbro_neck_2010_white|pattern_ra1=_norwayh2010\n|leftarm1=FF0000|body1=FF0000|rightarm1=FF0000|shorts1=FFFFFF|socks1=203060\n|pattern_la2=_norwaya2009|pattern_b2=_norwaya2009|pattern_ra2=_norwaya2009\n|leftarm2=FFFFFF|body2=FFFFFF|rightarm2=FFFFFF|shorts2=203060|socks2=FFFFFF\n}}\n\n'''サッカーノルウェー代表'''（'''Norges herrelandslag i fotball'''）は、[[ノルウェーサッカー協会]](NFF)により編成される[[ノルウェー]]の[[サッカー]]のナショナルチームである。",&ruigiVec);
		assert(ruigiVec.size()==0);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"ブエノスアイレス",L">{{Otheruses|首都であるブエノスアイレス自治市|州|ブエノスアイレス州|映画|ブエノスアイレス (映画)}}\n{{Pathnav|世界|アメリカ州|南アメリカ大陸|[[アルゼンチン|アルゼンチン共和国]]|frame=1}}\n{{世界の市\n|正式名称 = ブエノスアイレス\n|公用語名称 = Buenos Aires<br/>{{flagicon|Argentina}}<br/> \n|愛称 = Reina del Plata（銀の女王）\n|標語 = \n|画像 =Buenos Aires City Collage.png\n|画像サイズ指定 = 300px\n|画像の見出し = 上から: ブエノスアイレス夜景、国会議事堂、乙女の橋、タンゴを踊る男女、カサ・ロサーダ、大聖堂、カビルド、オベリスコ、テアトロ・コロン、レコレータ墓地、プラネタリウム、ラ・ボカのカミニート\n|市旗 =Bandera de la Ciudad de Buenos Aires.svg \n|市章 = Nuevo escudo_de_la_Ciudad_de_Buenos_Aires.png\n|位置図 = Mapa de Buenos Aires.svg\n|位置図サイズ指定 = \n|位置図の見出し = \n|位置図2 =LocationBuenosAires.png\n|位置図サイズ指定2 =\n|位置図の見出し2 =アルゼンチン内のブエノスアイレスの位置\n|緯度度= 34|緯度分= 36|緯度秒= 13|N(北緯)及びS(南緯)= S\n|経度度= 58|経度分= 22|経度秒= 54|E(東経)及びW(西経)= W\n|成立区分 = 建設\n|成立日 = [[1536年]]\n|下位区分名 ={{ARG}}\n|下位区分種類1 = [[アルゼンチンの地方行政区画|州]]\n|下位区分名1 = \n|下位区分種類2 = \n|下位区分名2 = \n|下位区分種類3 = \n|下位区分名3 = \n|下位区分種類4 = \n|下位区分名4 = \n|規模 = 市<!--必須-->\n|最高行政執行者称号 = \n|最高行政執行者名 = \n|最高行政執行者所属党派 = \n|総面積(平方キロ) = 203\n|総面積(平方マイル) = \n|陸上面積(平方キロ) = \n|陸上面積(平方マイル) = \n|水面面積(平方キロ) = \n|水面面積(平方マイル) = \n|水面面積比率 = \n|市街地面積(平方キロ) = \n|市街地面積(平方マイル) = \n|都市圏面積(平方キロ) = 4,758\n|都市圏面積(平方マイル) = n|標高(メートル) = 25\n|標高(フィート) = \n|人口の時点 = 2010年\n|人口に関する備考 = \n|総人口 = 2,891,082\n|人口密度(平方キロ当たり) = 14,240\n|人口密度(平方マイル当たり) = \n|市街地人口 = \n|市街地人口密度(平方キロ) = \n|市街地人口密度(平方マイル) = \n|都市圏人口 = 12,801,364\n|都市圏人口密度(平方キロ) = 2,690\n|都市圏人口密度(平方マイル) = \n|等時帯 =UTC-3\n|協定世界時との時差 = -3\n|夏時間の等時帯 =UTC-2\n|夏時間の協定世界時との時差 = -2\n|郵便番号の区分 =郵便番号\n|郵便番号 = C1000 - C14??\n|市外局番 = 011\n|ナンバープレート = \n|ISOコード = \n|公式ウェブサイト = [http://www.buenosaires.gov.ar/ www.buenosaires.gov.ar]\n|備考 = \n}}\n'''ブエノスアイレス'''（'''Buenos Aires'''）は、人口289万人（[[2010年]]）<ref name=indecpop>{{cite web|url= http://www.censo2010.indec.gov.ar/preliminares/cuadro_totalpais.asp|title=Argentina: Censo2010 |accessdate=25 February 2011}}</ref>を擁する[[アルゼンチン]]の[[首都]]。\n\nどの[[州]]にも属しておらずブエノスアイレス自治市（{{lang-es-short|Ciudad Autonoma de Buenos Aires}}）とも呼ばれる（なお、[[1880年]]の首都令以来、[[ブエノスアイレス州]]の州都は[[ラ・プラタ市]]である）。意味はスペイン語で「buenos（良い）aires（空気、風）」の意。船乗りの望む「順風」が街の名前になったものである．[[ラ・プラタ川]]（''Rio de la Plata'' スペイン語で「銀の川」の意）に面しており、対岸は[[ウルグアイ]]の[[コロニア・デル・サクラメント]]。[[大ブエノスアイレス都市圏]]の[[世界の都市的地域の人口順位|都市圏人口]]は1,312万人であり、世界第20位である<ref>[http://www.demographia.com/db-worldua.pdf Demographia: World Urban Areas & Population Projections]</ref>。建国以来アルゼンチンの政治、経済、文化の中心である。[[2014年]]、アメリカの[[シンクタンク]]が公表した[[ビジネス]]・[[人材]]・[[文化]]・[[政治]]などを対象とした総合的な[[世界都市#世界都市指数|世界都市ランキング]]において、世界第20位の都市と評価されており<ref>[http://www.atkearney.com/documents/10192/4461492/Global+Cities+Present+and+Future-GCI+2014.pdf/3628fd7d-70be-41bf-99d6-4c8eaf984cd5 2014 Global Cities Index and Emerging Cities Outlook] (2014年4月公表)</ref>、南米の都市では第1位であった。アルゼンチンの縮図ともなっている一方で、内陸部との差異が大きすぎるため、しばしば「国内共和国」と呼ばれる。市民は[[ポルテーニョ]]（porteno,女性はポルテーニャportena; 港の人、港っ子の意）と呼ばれる<ref>{{cite web|url=http://www.portenospanish.com/word/138/Porte%C3%B1o |title=What is Porteno in English? 窶骭 Porteno Spanish |publisher=Portenospanish.com |accessdate=15 September 2011}}</ref>。\n\n== 概要 ==\n「[[南米]]の[[パリ]]」の名で親しまれ<ref name=\"Short history\">''Argentina: A Short History'' by Colin M. Lewis, Oneworld Publications, Oxford, 2002. ISBN 1-85168-300-3</ref><ref>[http://travel.canoe.ca/Travel/SouthAmerica/2005/03/06/953104-sun.html 'Paris of the South'] by Kenneth Bagnell, [[Canadian Online Explorer|Canoe]] travel, 7 March 2005.</ref>、南米の中で最も美しい町の一つとして数えられる。\n\n独立当時は「偉大な田舎」と呼ばれる人口5万人程の小さな町だったが、[[ドミンゴ・ファウスティーノ・サルミエント|サルミエント]](Sarmiento)政権による欧州化、文明化政策の実施以降数多くの移民が[[イタリア]]・[[スペイン]]などから渡来し、中南米の中でも最も欧州的な街になった。かの有名な[[アルゼンチン・タンゴ]]はこの街のボカ地区で育った。また、[[サッカー]]が盛んなことでも有名で、[[ディエゴ・マラドーナ|マラドーナ]]が在籍した[[ボカ・フニオルス]]や[[リーベル・プレート]]など名門チームを数多く擁する。",&ruigiVec);
		assert(ruigiVec[0] == L"Reina del Plata");
		assert(ruigiVec[1] == L"国内共和国");
		assert(ruigiVec.size()==2);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"無線LAN",L">[[Image:Wireless network.jpg|thumb|200px|無線LAN接続の一例]]\n'''無線LAN'''（むせんラン）とは、[[無線通信]]を利用して[[データ]]の送受信を行う[[Local Area Network|LAN]]システムのことである。'''ワイヤレスLAN'''（Wireless LAN、WaveLAN<ref>[[:en:WaveLAN|WaveLAN]]</ref>）、もしくはそれを略して'''WLAN'''とも呼ばれる。\n\n== 概要 ==\n{{Vertical_images_list\n|幅= 180px\n| 3=AirStation WHR-G54S.jpg\n| 4=無線LAN親機<br />（[[アクセスポイント (無線LAN)|アクセスポイント]]）\n| 5=PCMCIA-card-750px.jpg\n| 6=無線LAN子機<br />（[[PCカード]]型）\n| 7=Planet WL-8310.JPG\n| 8=無線LAN子機<br />（[[Peripheral Component Interconnect|PCI]]カード型）\n| 9=Buffalo WLI-U2-KG54L.jpg\n| 10=無線LAN子機<br />（[[ユニバーサル・シリアル・バス|USB]]型）\n}}\n無線LANにはさまざまな方式があり、[[IEEE 802.11]]シリーズが普及している。[[パーソナルコンピュータ|パソコン]]や[[携帯情報端末|PDA]]などにおいて、一般的に利用される。\n\nまた'''[[Wi-Fi]]'''（ワイファイ）とも呼ばれることがあるが、これはIEEE 802.11機器に関する業界団体である[[Wi-Fi Alliance]]による相互接続性の認定の名称である。\n\n[[日本]]においては、1992年（平成4年）に[[電波法]]令上のいわゆる[[小電力無線局]]の[[小電力データ通信システム]]の[[無線局]]とされ、技術基準が定められた。これにより[[無線局免許状|免許]]は不要であるが[[技術基準適合証明]]を要することとされた。\n<!--平成4年郵政省令第78号による電波法施行規則改正-->\n<!--平成4年郵政省令第79号による無線設備規則改正-->\n<!--平成4年郵政省令第80号による特定無線設備の技術基準適合証明に関する規則改正-->\nなお、電気通信回線に接続するものは[[技術基準適合認定]]も要する。\n",&ruigiVec);
		assert(ruigiVec[0] == L"WLAN");
//		assert(ruigiVec[1] == L"Wi-Fi");
		assert(ruigiVec.size()==1);
	}
	{	
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"GOA",L">{{Otheruses|[[トヨタ自動車]]の[[衝突安全ボディー]]|その他の用法|ゴア}}\n'''GOA'''（ゴア）は、[[トヨタ自動車]]の[[衝突安全ボディー]]の通称で、'''G'''lobal '''O'''utstanding '''A'''ssessment（クラス世界トップレベルを追求している安全性評価）の頭文字。",&ruigiVec);
		assert(ruigiVec[0] == L"ゴア");
		assert(ruigiVec.size()==1);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"ビリアル定理",L">'''ビリアル定理'''(Virial theorem)とは、多粒子系(N粒子系)において、[[粒子]]が動き得る範囲が[[有限]]である場合に、[[古典力学]]、[[量子力学]]系のいずれにおいても成立する以下の関係式のことである。\n\n:<math> \\left\\langle K \\right\\rangle = \\left\\langle \\sum_{i=1}^N { \\mathbf{p}_i^2 \\over {2 m_i} } \\right\\rangle = \\sum_{i=1}^N \\left\\langle { \\mathbf{p}_i^2 \\over {2 m_i} } \\right\\rangle  = -{1 \\over 2} \\sum_{i=1}^N \\left\\langle \\mathbf{F}_i \\cdot \\mathbf{r}_i \\right\\rangle </math>\n\nKは[[系]]全体の[[運動エネルギー]]\n:<math> K = \\sum_{i=1}^N { \\mathbf{p}_i^2 \\over {2 m_i} } </math>\nで、'''p'''<SUB>i</SUB>は粒子iの[[運動量]]、'''r'''<SUB>i</SUB>は粒子iの[[位置]]座標、'''F'''<SUB>i</SUB>は粒子iに働く[[力]]、m<SUB>i</SUB>は粒子iの[[質量]]である。<math> \\left\\langle \\cdots \\right\\rangle </math>は[[物理量]]の平均操作(一般に長時間平均)を意味する。\n\n粒子iに働く力'''F'''<SUB>i</SUB>が、系全体の[[ポテンシャルエネルギー]]<math> V = V(\\mathbf{r}_1, \\cdots, \\mathbf{r}_N) </math>を用いて<math> \\mathbf{F}_i = - \nabla_{\\mathbf{r}_i} V (\\mathbf{r}_1, \\cdots, \\mathbf{r}_i, \\cdots, \\mathbf{r}_N) </math>と表せるならば、ビリアル定理は、\n\n:<math> \\left\\langle K \right\rangle = {1 \\over 2} \\sum_{i=1}^N \\left\\langle \\nabla_{\\mathbf{r}_i} V \\cdot \\mathbf{r}_i \\right\\rangle </math>\n\nという形で表せる。\n\nポテンシャルエネルギーVが[[中心力]][[ポテンシャル]]で、粒子間の距離のn+1乗(r<SUP>n+1</SUP>)に比例する形、すなわち、\n\n:<math> V(\\mathbf{r}) = a\\mathbf{r}^{n+1}</math>\n\nという形で表せるならば、\n\n:<math> \\left\\langle K \\right\\rangle = {n+1 \\over 2} \\left\\langle V \\right\\rangle </math>\n\nとなる。中心力が[[電磁気力]]や[[重力]]の場合を考えると、n = -2であるから、\n\n:<math> \\left\\langle K \\right\\rangle = -{1 \\over 2}\\left\\langle V \\right\\rangle </math>\n\nとなる。ビリアル定理から次のことが言える。\n\n*系全体の運動エネルギーKの時間平均は、系全体のポテンシャルエネルギーVの時間平均の-1/2に等しい。\n\nまた、同等のこととして、\n\n*系全体のポテンシャルエネルギーVの時間平均は、系全体の全エネルギーの時間平均に等しい。\n\n*系全体の運動エネルギーKの時間平均と系全体の全エネルギーの時間平均を加えた物は0。\n\nということが示される。\n\nビリアル定理という名前は'''ビリアル'''([[ラテン語]]で「力」の意)と呼ばれる値に由来している。ビリアルは<math>G = \\sum_i \\mathbf{r}_i  \\cdot \\mathbf{p}_i </math>によって定義される値で、[[1870年]][[クラウジウス]]が命名した。",&ruigiVec);
		assert(ruigiVec.size()==0);
	}

	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"明玉珍",L">{{基礎情報 中国君主\n|名       =太祖 明玉珍\n|代数     =初代\n|呼称     =皇帝 \n|画像     =\n|説明     =\n|王朝     =夏\n|在位期間 =[[1363年]] - [[1366年]]\n|都城     =重慶\n|諱       =旻玉珍→明玉珍\n|字       =\n|諡号     =欽文昭武皇帝\n|廟号     =太祖\n|生年     =[[至順]]2年（[[1331年]]）\n|没年     =[[天統 (明玉珍)|天統]]4年（[[1366年]]）\n|父       =[[旻学文]]|母       =趙氏|皇后     =彭皇后|陵墓     =永昌陵|年号     =[[天統 (明玉珍)|天統]] : [[1363年]] - [[1366年]]|注釈     =}}'''明 玉珍'''（めい ぎょくちん）は[[元 (王朝)|元]]末の農民反乱軍の領袖、[[夏 (元末)|夏]]王朝の創始者<ref>[http://www.chongqing.cn.emb-japan.go.jp/Japanese%20pages/kuikigaiyou/juukeigaiyou.htm 重慶市概要-在重慶日本国総領事館],2011-02-13閲覧。</ref>。",&ruigiVec);
		assert(ruigiVec[0] == L"欽文昭武皇帝");
		assert(ruigiVec.size()==1);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"フリーウェア",L"\n{{Otheruses|無償提供される[[オンラインソフトウェア|オンラインソフト]]|[[フリーソフトウェア財団]]が提唱する「自由なソフトウェア」の概念|フリーソフトウェア}}\n'''フリーウェア''' (''freeware'') は、[[オンラインソフトウェア|オンラインソフト]]の中で、無料で提供される[[ソフトウェア]]のことである。'''フリーソフト'''、'''フリーソフトウェア'''とも呼ばれる。これに対し、有料、もしくは試用期間後や追加機能に課金されるオンラインソフトは'''[[シェアウェア]]'''と呼ばれる。なお、[[フリーソフトウェア財団]]の主張する「自由なソフトウェア」を意味する'''[[フリーソフトウェア]]''' ({{Lang-en-short|Free Software}})とは意味が異なる。本項では便宜上、「フリーウェア」の語を無料のソフトウェア、「フリーソフトウェア」の語を「自由なソフトウェア」の意味で用いている。\n\nフリーウェアは「無料で使用できる」ことに重点を置いた呼称であり、それ以外の[[ライセンス]]条件、とくに変更・再配布などの条件はまちまちで、[[ソースコード]]が付属しないために変更ができなかったり、有償配布（販売）や営利利用の禁止など一定の制限が課せられているものも多い。[[プロプライエタリ・ソフトウェア|プロプライエタリ]]なフリーウェアは、開発力のあるユーザーにソースコードのダウンロードや所持、貢献などを許可しながらも、開発の方向性とビジネスの可能性を残すことができる。個人が開発しているフリーウェアは有料化されシェアウェアとなったり、HDDのクラッシュ、PCの盗難、ライセンス上の問題、その他の理由で管理できなくなり更新・配布が停止されることが多々ある。",&ruigiVec);
		//ruigiVec = [4]("フリーソフトウェア","フリーソフト","自由なソフトウェア","フリーソフトウェア財団")
		assert(ruigiVec.size()==4);
	}
	{

		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"ポーランド",L"\n'''ポーランド共和国'''（ポーランドきょうわこく、{{lang-pl|Rzeczpospolita Polska}}）、通称'''ポーランド'''は、[[中央ヨーロッパ]]に位置する[[共和制]][[国家]]。[[欧州連合]]（EU）そして[[北大西洋条約機構]]（NATO）の加盟国。通貨は[[ズウォティ]]。首都は[[ワルシャワ]]。\n\n北は[[バルト海]]に面し、北東は[[ロシア]]の[[飛地]][[カリーニングラード州]]と[[リトアニア]]、東は[[ベラルーシ]]と[[ウクライナ]]、南は[[チェコ]]と[[スロバキア]]、西は[[ドイツ]]と国境を接する。\n\n[[10世紀]]に国家として認知され、[[14世紀]]から[[17世紀]]にかけ大王国を形成。[[18世紀]]、3度にわたり国土が隣国によって分割され消滅。[[第一次世界大戦]]後、[[1918年]]に独立したが、[[第二次世界大戦]]時、[[ナチス・ドイツ]]と[[ソビエト連邦]]に侵略され、再び国土が分割された。戦後、[[1952年]]、人民共和国として[[国家主権]]を復活、[[1989年]]、[[民主化]]により[[共和国]]となった。\n\n[[冷戦時代]]はソ連の影響下に置かれ、共産主義政権が支配したため、政治的に[[東欧]]に含められたが、国内の民主化とソ連の崩壊を経て、[[中欧]]または中欧のうち過去に東欧であった地域の[[中東欧]]として再び分類されるようになっている。\n\n[[2009年]]の[[世界同時不況]]においてヨーロッパでただ一国ポーランドだけは経済拡大を続けた。[[1993年]]以来、景気後退が一度もない堅実な経済を誇る。" ,&ruigiVec);
		assert(ruigiVec.size()==0);
	}
	{

		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"筑波大学",L">{{告知|提案|折りたたみ機能の適用}}\n{{画像提供依頼|東京キャンパス|date=2009年4月|cat=東京都}}\n{{大学\n|国 = 日本\n|大学名 = 筑波大学\n|ふりがな = つくばだいがく\n|英称 = University of Tsukuba\n|画像 = Univoftsukuba2006.jpg\n|画像説明 = 筑波キャンパス\n|大学設置年 = 1973年\n|創立年 = 1872年\n|学校種別 = 国立\n|設置者 = 国立大学法人筑波大学\n|本部所在地 = [[茨城県]][[つくば市]][[天王台 (つくば市)|天王台]]一丁目1番1号\n|緯度度 = 36|緯度分 = 6|緯度秒 = 41\n|経度度 = 140|経度分 = 6|経度秒 = 14\n|キャンパス = 筑波キャンパス（茨城県つくば市天王台）<br />東京キャンパス（文京校舎）\n|学部 = 人文・文化学群<br />社会・国際学群<br />人間学群<br />生命環境学群<br />理工学群<br />情報学群<br />医学群<br />体育専門学群<br />芸術専門学群\n|研究科 = 教育研究科<br />人文社会科学研究科<br />ビジネス科学研究科<br />数理物質科学研究科<br />システム情報工学研究科<br />生命環境科学研究科<br />人間総合科学研究科<br />図書館情報メディア研究科\n|大学の略称 = 特にないが、筑波（つくば）、筑波大（つくばだい）、筑大（つくだい）などと呼ばれている\n|ウェブサイト = http://www.tsukuba.ac.jp/\n}}" ,&ruigiVec);
		//ruigiVec = [3]("筑波","筑波大","筑大")
		assert(ruigiVec[0] == L"筑波");
		assert(ruigiVec[1] == L"筑波大");
		assert(ruigiVec[2] == L"筑大");
		assert(ruigiVec.size()==3);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"フェルミエネルギー",L">'''フェルミエネルギー''' (Fermi energy) とは、[[物理学]]（[[量子力学]]）において、[[絶対零度]]での[[フェルミ粒子]]系の状態において[[フェルミ粒子]]によって占められた準位のうちで最高の準位の[[エネルギー]]である<ref name=\"Kittel_Phys.\">[http://pub.maruzen.co.jp/book_magazine/book_data/search/4621076566.html キッテル 固体物理学入門], Charles Kittel・宇野良清・津屋昇・新関駒二郎・森田章・山下次郎, [http://pub.maruzen.co.jp/index.html 丸善出版], 2005年12月.</ref>。\n\n一方、'''フェルミ準位'''（フェルミじゅんい、Fermi level ）とは[[フェルミ分布関数|フェルミ・ディラック分布]]のパラメータあるいは[[フェルミ粒子]]系の[[化学ポテンシャル]]<math>\\mu</math>のことであり、(与えられた温度条件下において、)[[フェルミ粒子]]の存在確率が<math>\\frac{1}{2}</math>になる[[エネルギー]]の値を意味する<ref name=\"Taur_Ning_VLSI\">[http://pub.maruzen.co.jp/book_magazine/book_data/search/9784621085813.html タウア・ニン 最新VLSIの基礎], Yuan Taur・Tak H. Ning・芝原健太郎・宮本恭幸・内田建, [http://pub.maruzen.co.jp/index.html 丸善出版], 2013年1月.</ref>。\n\nすなわち、[[絶対零度]]では、[[フェルミ準位]]の値は[[フェルミエネルギー]]に等しくなる<ref>半導体の分野では「フェルミエネルギー」と「フェルミ準位」がよく互いに取り替えられている。例: [http://books.google.com/books?id=n0rf9_2ckeYC&pg=PA49 ''Electronics (fundamentals And Applications)''] by D. Chattopadhyay, [http://books.google.com/books?id=lmg13dHPKg8C&pg=PA113 ''Semiconductor Physics and Applications''] by Balkanski and Wallis. これらの場合、「フェルミエネルギー」が「絶対零度のフェルミエネルギー」と呼ばれている。</ref>。\n\n[[結晶]]中の[[電子]]のエネルギーは[[バンド構造]]を形成する。'''フェルミエネルギー'''は、[[金属]]の場合には[[電子]]をバンドの底から詰めていき、その数が系の全電子数になったところの電子のエネルギーであるが、[[半導体]]、[[絶縁体]]の場合にはフェルミ準位が[[伝導帯]]と[[価電子帯]]の間の[[禁止帯]]の中にある。\n\n例：自由電子系でのフェルミエネルギー： <math>\frac{h^2{k_F}^2}{{8\\pi^2}{m}}</math> （<math>h</math>：[[プランク定数]]、<math>k_F</math>：[[フェルミ半径]]、<math>m</math>：電子の質量）。",&ruigiVec);

		assert(ruigiVec.size()==0);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"温度",L"'''温度'''（おんど）とは、寒暖の度合いを数量で表したもの。具体的には[[物質]]を構成する[[分子]]運動のエネルギーの統計値。このため温度には下限が存在し、分子運動が止まっている状態が温度0K（[[絶対零度]]）である。ただし、分子運動が0となるのは古典的な極限としてであり、実際は、[[量子力学]]における[[不確定性原理]]から、絶対零度であっても、分子運動は0にならない（止まっていない）。\n\n温度はそれを構成する粒子の運動であるから、[[化学反応]]に直結し、それを元にするあらゆる現象における強い影響力を持つ。[[生物]]にはそれぞれ[[至適温度]]があり、ごく狭い範囲の温度の元でしか生存できない。なお、日常では単に温度といった場合、往々にして[[気温]]のことを指す。",&ruigiVec);

		assert(ruigiVec.size()==0);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseAimai(L"大統領選挙",L">'''大統領選挙'''（だいとうりょうせんきょ）とは、[[大統領]]を選ぶ[[選挙]]をいう。\n\n各国別の大統領選挙に関する記事は、以下を参照。\n\n* [[アメリカ合衆国]] - [[アメリカ合衆国大統領選挙]]、[[:Category:アメリカ合衆国大統領選挙窶讃]\n* [[イラン]] - [[イラン#大統領]]、[[イラン大統領選挙 (2005年)|2005年]]、[[イラン大統領選挙 (2009年)|2009年]]、[[イラン大統領選挙 (2013年)|2013年]]\n* [[ウクライナ]] - [[ウクライナ#政治]]、[[:Category:ウクライナ大統領選挙窶讃]\n* [[エジプト]] - [[2012年エジプト大統領選挙|2012年]]\n* [[大韓民国]] - [[:Category:韓国大統領選挙窶讃]\n* [[中華民国]]（[[台湾]]） - [[:Category:中華民国総統選挙]]窶蚕n* [[ジンバブエ]] - [[2008年ジンバブエ大統領選挙|2008年]]\n* [[スロバキア]] - [[2009年スロバキア大統領選挙|2009年]]\n* [[セネガル]] - [[セネガルの大統領#セネガル共和国大統領選挙|2000年]]\n* [[ニジェール]] - [[2004年ニジェール大統領選挙|2004年]]\n* [[東ティモール]] - [[2002年東ティモール大統領選挙|2002年]]、[[2007年東ティモール大統領選挙|2007年]]\n* [[フランス]] - [[フランス大統領選挙]]、[[:Category:フランス大統領選挙窶讃]\n* [[ベラルーシ]] - [[2010年ベラルーシ大統領選挙|2010年]]\n* [[ポーランド]] - [[:Category:ポーランド大統領選挙窶讃]\n* [[ポルトガル]] - [[:Category:ポルトガル大統領選挙窶讃]\n* [[リトアニア]] - [[:Category:リトアニア大統領選挙窶讃]\n* [[ロシア]] - [[:Category:ロシア大統領選挙]]\n\n{{aimai}}\n\n{{DEFAULTSORT:たいとうりようせんきよ}}\n[[Category:大統領選挙|*]]",&ruigiVec);

		assert(ruigiVec.size()==0);	//間違うぐらいなら取れない方がいい
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"古代エジプト",L">[[File:Ancient Egypt map-en.svg|thumb|250px|紀元前3150年から紀元前30年までの王朝時代における主要都市及び場所を示した古代エジプトの地図]]\n'''古代エジプト'''（こだいエジプト）は、[[古代]]の[[エジプト]]に対する呼称。具体的には[[紀元前3000年]]頃に始まった[[エジプト第1王朝|第1王朝]]から[[紀元前332年]]に[[アレクサンドロス3世|アレクサンドロス大王]]によって滅ぼされるまでの時代を指す。\n\n古い時代から[[砂漠]]が広がっていたため、[[ナイル川]]流域分の面積だけが居住に適しており、主な活動はその中で行われた。ナイル川の上流は谷合でありナイル川1本だけが流れ、下流はデルタ地帯（[[ナイル川デルタ|ナイル川デルタ]]）が広がっている。最初に上流地域（[[上エジプト]]）と下流地域（[[下エジプト]]）<ref>上下というのはナイル川の上流・下流という意味であり、ナイル川は北に向かって流れているため、北にあたる地域が下エジプトである（逆もまた然り）。</ref>でそれぞれ違った文化が発展した後に統一されたため、[[ファラオ]]（[[王]]）の称号の中に「上下エジプト王」という部分が残り、古代エジプト人も自国のことを「二つの国」と呼んでいた。\n\n\nナイル川は毎年[[氾濫]]を起こし、肥えた土を下流に広げたことがエジプトの繁栄のもとだといわれる。ナイル川の氾濫を正確に予測する必要から天文観測が行われ、[[太陽暦]]が作られた。太陽と[[シリウス]]星が同時に昇る頃、ナイル川は氾濫したという。また、氾濫が収まった後に農地を元通り配分するため、[[測量術]]、[[幾何学]]、[[天文学]]が発達した。\n\n\nエジプト文明と並ぶ最初期における農耕文明の一つである[[メソポタミア文明]]が、民族移動の交差点にあたり終始異民族の侵入を被り支配民族が代わったのと比べ、地理的に孤立した位置にあったエジプトは比較的安定しており、部族社会が城壁を廻らせて成立する[[都市国家]]の痕跡は今の所発見されていない。",&ruigiVec);

		assert(ruigiVec.size()==0);
	}
	{
		
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"エゴール・リガチョフ",L"\n\n'''エゴール・クジミッチ・リガチョフ'''（{{Lang-ru|'''Егоﾌ＞闊 Кузьмиﾌ＞隍 Лигачёв'''}}、ラテン文字転写の例：{{lang|ru-Lat|'''Egor Kuz'mich Ligachyov'''<ref>Ligachevとも</ref>}}、[[1920年]][[11月29日]] - ）は、[[ソビエト連邦]]および[[ロシア]]の[[政治家]]。[[ミハイル・ゴルバチョフ]]時代の[[ソビエト連邦共産党|ソ連共産党]]保守派の領袖。",&ruigiVec);
		assert(ruigiVec.size()==0);

	}

	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"BSD",L"\n>'''BSD'''（ビーエスディー）は、Berkeley Software Distribution の略語で、1977年から1995年まで[[カリフォルニア大学バークレー校]] (University of California, Berkeley, UCB) の [[Computer Systems Research Group]] (CSRG) が開発・配布したソフトウェア群、および[[UNIX]][[オペレーティングシステム]] (OS)。なお、今日「BSD」という名称は同OSを元に開発された[[BSDの子孫]]の総称として使われることもあるが、この項では主に前述のUCBによるソフトウェア群およびOSについて述べる。\n\n元となったコードベースと設計は[[AT&T]]のUNIXと共通であるため、歴史的にはBSDはUNIXの支流 \"BSD UNIX\" とみなされてきた。1980年代、[[ワークステーション]]クラスのシステムベンダーがプロプライエタリなUNIXとしてBSDを広く採用していた。例えば、[[ディジタル・イクイップメント・コーポレーション|DEC]]の[[Ultrix]]、[[サン・マイクロシステムズ]]の[[SunOS]]などである。これは、ライセンス条件の容易だったためと、当時の多くの技術系企業の創業者がBSDを熟知していたためである。\n\nそれらプロプライエタリなBSD派生OSは、1990年代には[[UNIX System V|UNIX System V Release 4]]と[[OSF/1]]に取って代わられ（どちらもBSDのコードを取り入れており、他の現代のUnixシステムの基盤となった）、後期のBSDリリースはいくつかの[[オープンソース]]開発プロジェクトの基盤となった。例えば、[[FreeBSD]]、[[NetBSD]]、[[OpenBSD]]、[[DragonFly BSD]]などが今も開発中である。さらにそれら（の全部あるいは一部）が最近のプロプライエタリなOSにも採用されている。例えば、[[Microsoft Windows|Windows]]の[[インターネット・プロトコル・スイート|TCP/IP]]コード（IPv4のみ）や[[アップル インコーポレイテッド|アップル]]の[[OS X|Mac OS X]]である。",&ruigiVec);
		assert(ruigiVec[0] == L"カリフォルニア大学バークレー校");
		assert(ruigiVec[1] == L"Computer Systems Research Group");
		assert(ruigiVec[2] == L"UNIX");
		assert(ruigiVec[3] == L"OS");
		assert(ruigiVec[4] == L"ビーエスディー");
		assert(ruigiVec.size()==5);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseAimai(L"瀬田川",L">'''瀬田川'''（せたがわ）は、[[日本]]各地を流れる[[河川]]名称。\n\n* [[淀川]]の[[滋賀県]]内での名称。\n* [[瀬田川 (岐阜県)]] - [[岐阜県]][[可児市]]を流れる[[木曽川]]水系[[可児川]]支流の[[一級河川]]。\n* 瀬田川 (和歌山県) - [[和歌山県]][[西牟婁郡]][[白浜町]]を流れる[[富田川 (和歌山県)|富田川]]水系の[[二級河川]]。\n* [[瀬田川 (山口県)]] - [[山口県]][[玖珂郡]][[和木町]]を流れる[[小瀬川]]水系の一級河川。\n* [[瀬田川 (愛媛県)]] - [[愛媛県]][[西予市]]を流れる[[肱川]]水系の一級河川。\n{{aimai}}\n{{デフォルトソート:せたかわ}}\n[[Category:同名の河川]]",&ruigiVec);

//		assert(ruigiVec[0] == L"淀川");
//		assert(ruigiVec.size()==1);
		assert(ruigiVec.size()==0);	//間違うぐらいなら取れない方がいい
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"殺人ジョーク",L">'''殺人ジョーク'''（さつじんジョーク、''killer joke''）は、[[イギリス]]の[[テレビ番組]]『[[空飛ぶモンティ・パイソン]]』からの[[スケッチ・コメディー]]である。「ジョーク戦争」 (joke warfare)、「世界一面白いジョーク」 (The Funniest Joke in the World) としても知られている。このスケッチはシリーズ1の第1話「カナダはどっちだ」に登場し、初の大当たりとなった。映画『[[モンティ・パイソン・アンド・ナウ]]』において、秀作スケッチの1つとしてリメイクされている。\n\n日本では、このスケッチを「殺人ジョーク」(killer joke) という題名で一般的に言及するが、脚本によるとこのスケッチのタイトルは\"The Funniest Joke in the World\"（世界一面白いジョーク）となっている。英語圏では\"killer joke\"よりむしろ\"The Funniest Joke in the World\"が、このスケッチを言及する題名となっている。\n\n\n作中の設定では、軍事使用目的に以前に作成されたジョークが存在するという設定がある。また、ドイツ語に翻訳されたジョーク(Wenn ist das Nunstuck git und Slotermeyer? Ja! ... Beiherhund das Oder die Flipperwaldt gersput)がこのスケッチでは音読されるが、これはナンセンスなドイツ語である。このスケッチでは、資料映像や写真が多数使われており(例:[[ミュンヘン会談]]、[[アドルフ・ヒトラー|ヒトラー]]の演説)、その字幕の置き換えがされるといったコメディ手法をとっている。",&ruigiVec);
		assert(ruigiVec[0] == L"The Funniest Joke in the World");
		assert(ruigiVec[1] == L"世界一面白いジョーク");
		assert(ruigiVec[2] == L"ジョーク戦争");
		assert(ruigiVec.size()==3);

	}

	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"淀川",L"\n'''淀川'''（よどがわ）は、[[琵琶湖]]から流れ出る唯一の[[河川]]。瀬田川、宇治川、淀川と名前を変えて大阪湾に流れ込む。[[滋賀県]]、[[京都府]]及び[[大阪府]]を流れる淀川水系の本流で[[一級河川]]。流路延長75.1km、流域面積8,240km&sup2;。\n\nまた、琵琶湖に流入する河川や木津川などを含めた淀川水系全体の[[支流]]（支川）数は965本で日本一多い。第2位は信濃川（880本）、第3位は利根川（819本）となっている。",&ruigiVec);

		assert(ruigiVec[0] == L"宇治川");
		assert(ruigiVec[1] == L"瀬田川");
		assert(ruigiVec.size()==2);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseAimai(L"子午線公園",L">'''子午線公園'''（しごせんこうえん）とは、以下の[[公園]]のことである。\n<!--近隣公園以上のものにリンクを貼付。街区公園はよほどの特筆性がない限り個別記事にすべきではないと思われる-->\n\n* 子午線公園<!-- (京都府)--> → [[京都府]][[京丹後市]][[網野町]]にある公園。\n* 子午線公園<!-- (兵庫県)--> → [[兵庫県]][[豊岡市]][[但東町]]にある公園。\n* [[日本へそ公園]] → 兵庫県[[西脇市]]にある公園。\n* 小野子午線公園 → 兵庫県[[小野市]]にある公園。\n\n{{aimai}}\n{{DEFAULTSORT:しこせんこうえん}}\n[[Category:日本の公園]]",&ruigiVec);

		assert(ruigiVec[0] == L"日本へそ公園");
		assert(ruigiVec.size()==1);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseAimai(L"沓沢",L">'''沓沢'''（くつざわ、くつさわ）　'''沓澤'''とも\n----\n\n*日本人の[[姓]]のひとつ。関連項目を参照。\n** [[沓沢氏]] : かつての[[出羽国]]由利郡沓沢付近を中心に勢力があった武士団。[[由利十二頭]]が一家。\n** [[沓沢朝治]] : 著名な[[鷹匠]]の一人。\n** [[沓沢周一郎]] : [[俳優]]。\n** [[沓沢龍一郎]] : [[イラストレーター]]・[[漫画家]]。\n*[[日本]]の[[地名]]のひとつ。\n** [[長野県]][[佐久市]]根岸'''沓沢'''。[[蓼科山]]東北麓の末端にあたる。\n** [[山梨県]][[南アルプス市]]芦安芦倉'''沓沢'''。\n** [[沓沢湖]] : 長野県[[塩尻市]]にある[[人工湖]]。\n\n== 関連項目 ==\n*[[特別:Prefixindex/沓沢|沓沢で始まる記事の一覧]]\n\n{{aimai}}\n{{DEFAULTSORT:くつさわ}}\n\n[[Category:日本語の姓]]\n[[Category:同名の地名]]",&ruigiVec);

		assert(ruigiVec[0] == L"沓沢氏");
		assert(ruigiVec[1] == L"沓沢朝治");
		assert(ruigiVec[2] == L"沓沢周一郎");
		assert(ruigiVec[3] == L"沓沢龍一郎");
		assert(ruigiVec[4] == L"沓沢湖");
		assert(ruigiVec.size()==5);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"僕は友達が少ない",L"{{Infobox animanga/Header\n|タイトル= 僕は友達が少ない\n|ジャンル= [[学園小説|学園]]、残念系<ref name=\"残念系\"> {{Cite journal ja-jp|author=多根清史|year=2012|title=キーワード3 新しい属性 残念系ヒロイン誕生!\n|journal   = オトナアニメ年鑑2012|serial=別冊オトナアニメ|publisher = 洋泉社|naid=|pages= pp. 58-61|ISBN 978-4-86248-858-9}}</ref>、[[ラブコメディ|ラブコメ]]、[[ハーレムもの|ハーレム]]\n}}\n{{Infobox animanga/Novel\n|著者= [[平坂読]]\n|イラスト= [[ブリキ (イラストレーター)|ブリキ]]\n|出版社= [[メディアファクトリー]]<ref group=\"注\" name=vKADOKAWAmfBC\">現・KADOKAWAメディアファクトリーブランドカンパニー</ref>\n|他出版社= {{Flagicon|ROC}}[[尖端出版]]<br />{{flagicon|韓国}}[[鶴山文化社]]\n|掲載誌= <!--ある場合のみ。-->\n|レーベル= [[MF文庫J]]\n|発売日= <!--全1巻の場合のみ。-->\n|開始日= [[2009年]]8月\n|終了日= \n|巻数= 既刊10巻+アンソロジー2巻+番外編1巻<!--「既刊xx巻（本編xx巻+番外編（短編）xx巻）」でもよい。完結の場合は「全xx巻」とする。-->\n}}\n|-\n| colspan=\"2\" style=\"padding: 0;\"|\n{| class=\"infobox bordered collapsible innercollapse autocollapse\" style=\"width: 100%; margin: 0;\"\n|-\n! colspan=\"2\" style=\"text-align: center; background-color: #ccf;\"| '''漫画'''\n{{Infobox animanga/Manga\n|タイトル= 僕は友達が少ない\n|作者= 平坂読\n|作画= いたち\n|出版社= メディアファクトリー<ref group=\"注\" name=\"KADOKAWAmfBC\">現・KADOKAWAメディアファクトリーブランドカンパニー</ref>\n|他出版社= {{Flagicon|ROC}}[[尖端出版]]<br />{{flagicon|韓国}}[[鶴山文化社]]\n|掲載誌= [[月刊コミックアライブ]]\n|レーベル= [[MFコミックス|MFコミックス・アライブシリーズ]]\n|開始号= [[2010年]]5月号\n|終了号=\n|巻数= 既刊10巻\n}}\n{{Infobox animanga/Manga\n|タイトル= 僕は友達が少ない+\n|作者= 春川三咲（脚本）\n|作画= 田口囁一\n|出版社= [[集英社]]\n|掲載誌= [[ジャンプSQ.19]]\n|レーベル= [[ジャンプ・コミックス]]<br />(JAMP COMICS SQ.)\n|開始号= 2010年秋号\n|終了号= [[2012年]]7月号\n|巻数= 全2巻\n}}\n{{Infobox animanga/Manga\n|タイトル= 僕は友達が少ない ショボーン!\n|作者= [[風華チルヲ]]（脚本）\n|作画= しらび\n|出版社= メディアファクトリー<ref group=\"注\" name=\"KADOKAWAmfBC\">現・KADOKAWAメディアファクトリーブランドカンパニー</ref>\n|掲載誌= 月刊コミックアライブ\n|レーベル= MFコミックス・アライブシリーズ\n|開始号= [[2011年]]12月号\n|終了号=\n|巻数= 既刊1巻\n}}\n{{Infobox animanga/Manga\n|タイトル=僕は友達が少ない はがない日和\n|作者=[[木瓜庵]]\n|作画=[[bomi]]\n|出版社=メディアファクトリー<ref group=\"注\" name=\"KADOKAWAmfBC\">現・KADOKAWAメディアファクトリーブランドカンパニー</ref>\n|掲載誌=月刊コミックアライブ\n|レーベル=アライブコミックス\n|開始号=2012年10月号\n|終了号=2013年4月号\n|開始日=2012年[[8月27日]]\n|終了日=2013年[[2月27日]]\n|巻数=全1巻\n|話数=全8話\n}}\n|}\n{{Infobox animanga/TVAnime\n|タイトル= 僕は友達が少ない（第1期）<br />僕は友達が少ないNEXT（第2期）\n|原作= 平坂読\n|監督= [[斎藤久]]（第1期）、喜多幡徹（第2期）\n|シリーズ構成= [[浦畑達彦]]、[[平坂読]]（第2期）\n|脚本= 浦畑達彦、砂山蔵澄（第2期）\n|キャラクターデザイン= 渡邊義弘\n|音楽= [[Tom-H@ck]]\n|アニメーション制作= [[アニメインターナショナルカンパニー|AIC Build]]\n|製作= 製作委員会は友達が少ない（第1期）<br />製作委員会は友達が少ないNEXT（第2期）<br />[[TBSテレビ|TBS]]\n|放送局= [[#放送局|放送局]]参照\n|放送開始= 第1期：2011年10月\n|放送終了= 12月<br />第2期：2013年1月 - 3月\n|話数= 第1期：全12話+OVA2話<br />第2期：全12話\n}}\n{{Infobox animanga/Radio\n|タイトル= 僕は友達が少ない on AIR RADIO\n|愛称=\n|放送開始= 第1期：2011年[[9月6日]]\n|放送終了= 2012年[[4月10日]]<br />第2期：2012年[[12月11日]] - 2013年[[5月14日]]\n|放送局= [[音泉]]、[[HiBiKi Radio Station]]\n|放送時間= 毎週[[火曜日]]\n|放送回数=\n|放送形式=\n|パーソナリティ= [[木村良平]]、[[井上麻里奈]]\n|その他=\n|インターネット=1\n}}\n{{Infobox animanga/Game\n|タイトル= 僕は友達が少ない ぽーたぶる\n|ゲームジャンル= 残念系青春Liveストーリー\n|対応機種= [[PlayStation Portable]]\n|開発元= [[ガイズウェア]]\n|発売元= [[バンダイナムコゲームス]]\n|メディア= [[ユニバーサル・メディア・ディスク|UMD]]\n|プレイ人数= 1人\n|発売日= 2012年[[2月23日]]\n|出荷本数=\n|売上本数= 56,969本<ref name = 4gamer>{{Cite web|date=2012-03-01|url=http://www.4gamer.net/games/117/G011794/20120229046/|title=「テイルズ オブ ザ ヒーローズ ツインブレイヴ」8万5000本,「牧場物語 はじまりの大地」8万1000本など,新作が上位を占めた「週間販売ランキング+」|work=4Gamer.net|author=|accessdate=2012-03-06 20:27}}</ref>\n|レイティング= {{CERO-C}}\n|その他=\n}}\n{{Infobox animanga/Movie\n|タイトル=僕は友達が少ない\n|監督=[[及川拓郎]]\n|制作=「僕は友達が少ない」製作委員会\n|配給=[[東映]]\n|封切日=2014年2月1日\n|上映時間=114分\n|その他=\n}}\n{{Infobox animanga/Cast\n|役名=羽瀬川小鷹<br />三日月夜空<br />柏崎星奈<br />羽瀬川小鳩<br/>高山マリア<br/>柏崎天馬<br/>\n|出演者=[[瀬戸康史]]（幼少期 大塚一慧）<br />[[北乃きい]]（幼少期 [[渡邉空美]]）<br />[[大谷澪]]<br />[[久保田紗友]]<br/>[[山田萌々香]]<br/>[[石原良純]]<br/>\n}}\n{{Infobox animanga/Footer\n|ウィキプロジェクト= [[プロジェクト:ライトノベル|ライトノベル]]・[[プロジェクト:漫画|漫画]]・[[プロジェクト:アニメ|アニメ]]・[[プロジェクト:コンピュータゲーム|コンピュータゲーム]]\n|ウィキポータル= [[Portal:文学|文学]]・[[Portal:漫画|漫画]]・[[Portal:アニメ|アニメ]]・[[Portal:ゲーム|ゲーム]]\n}}\n『'''僕は友達が少ない'''』（ぼくはともだちがすくない）は、[[平坂読]]による[[日本]]の[[ライトノベル]]とそれを原作とした各種作品群。原作の[[イラストレーション|イラスト]]は[[ブリキ (イラストレーター)|ブリキ]]が担当している。\n\n[[キャッチコピー]]は、「'''残念系青春ラブコメ'''」<ref name=\"残念系\" />。[[MF文庫J]]（[[メディアファクトリー]]<ref group=\"注\" name=\"KADOKAWAmfBC\">現・KADOKAWAメディアファクトリーブランドカンパニー</ref>）より、[[2009年]]8月から刊行されている。既刊10巻、2012年6月現在シリーズ累計450万部突破。公式略称は「'''はがない'''」<ref group=\"注\" name=\"haganai\">「僕『'''は'''』友達『'''が'''』少『'''ない'''』」（平仮名の部分）より。初出は第1巻のあとがき。作者によると「最初は自分が勝手に使っていた略称だったが、いつの間にか公式略称として定着していた」とのこと。またテレビアニメ第一期の各話サブタイトルに含まれる平仮名を繋げると、これも「はがない」になる。</ref>。タイトルの由来は「自分が見たら絶対に手に取りそうな青春小説のタイトル」とのこと。\n\n\n1巻発売直後に出版された『[[このライトノベルがすごい!]]』2010年度版にて作品部門ランキングの23位にランクイン。その後2011年度版では2位となった。",&ruigiVec);
		assert(ruigiVec[0] == L"はがない");
		assert(ruigiVec.size()==1);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseAimai(L"ロチェスター",L">'''ロチェスター'''（Rochester）は、英語圏の地名、姓。\n\n==地名==\n*'''[[ロチェスター (ニューヨーク州)]]''' - [[アメリカ合衆国]][[ニューヨーク州]]北西部、[[モンロー郡 (ニューヨーク州)|モンロー郡]]の都市。全米の「ロチェスター」という名の都市の中で最大。なおニューヨーク州には「ロチェスター」という名の自治体が2ヶ所に存在するが、単に「ニューヨーク州ロチェスター」と言った場合、たいていはこちらのほうを指す。\n*[[ロチェスター (ニューヨーク州 アルスター郡)]] - アメリカ合衆国ニューヨーク州南東部、[[ニューヨーク]]都市圏内の町。\n*[[ロチェスター (ニューハンプシャー州)]] - アメリカ合衆国[[ニューハンプシャー州]]南東部の都市。\n*[[ロチェスター (マサチューセッツ州)]] - アメリカ合衆国[[マサチューセッツ州]]南東部、[[ボストン]]郊外の町。\n*[[ロチェスター (ミシガン州)]] - アメリカ合衆国[[ミシガン州]]南東部、[[デトロイト]]の郊外都市。\n*[[ロチェスター (ミネソタ州)]] - アメリカ合衆国[[ミネソタ州]]南東部の都市。\n\nなお上記以外にも、アメリカ合衆国には[[イリノイ州]]、[[インディアナ州]]、[[ウィスコンシン州]]（2ヶ所）、[[オハイオ州]]、[[ケンタッキー州]]、[[テキサス州]]、[[バーモント州]]、[[ペンシルベニア州]]、[[ワシントン州]]にも「ロチェスター」という地名が存在するが、いずれも人口数百人～数千人の小都市・町村である。\n\n*[[ロチェスター (イングランド)]] - [[イングランド]]・[[ケント州]]北西部の都市。[[ロンドン]]南東の郊外都市。\n*[[ロチェスター (オーストラリア)]] - [[オーストラリア]]・[[ビクトリア州]]の町。\n{{aimai}}\n{{DEFAULTSORT:ろちえすた}}\n[[Category:同名の地名]]\n[[Category:英語の姓]]",&ruigiVec);
		assert(ruigiVec.size()==0);
	}

	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"ボイモルト",L">{{基礎情報 スペインの自治体2\n|名前={{lang|gl|Boimorto}}\n|市旗= |市旗幅=130px\n|市章=Boimorto.svg |市章幅=90px\n|画像= |画像幅=300px |画像説明=\n|州=ガリシア |県=ア・コルーニャ |コマルカ=[[コマルカ・デ・アルスーア|アルスーア]] |司法管轄区=[[アルスーア]]\n|面積=[[1 E8 m&sup2;|82.3km&sup2;]] <ref name=IGE>{{cite web|url=http://www.ige.eu/web/index.jsp?paxina=001&idioma=gl|title=IGE（ガリシア統計局）|publisher=ガリシア自治州政府|language=ガリシア語|accessdate=2012-09-05}}</ref>\n|教区数=13 |居住地区数=115 |標高=500\n|人口=2,211人（2011年） |人口密度=26.9人/km&sup2; <ref name=IGE/>\n|住民の呼称=boimortense |ガリシア語率=99.92 |自治体首長=ホセ・イグナシオ・ポルトス・バスケス<br />（[[ガリシア社会党|PSdeG-PSOE]]）\n|latd=43 |latm=0 |lats=27 |lond=8 |lonm=07 |lons=37 |EW=W\n}}\n[[画像:Situacion Boimorto.PNG|thumb||]]\n'''ボイモルト'''（{{lang|gl|Boimorto}}）は、[[スペイン]]、[[ガリシア州]]、[[ア・コルーニャ県]]の自治体、[[コマルカ・デ・アルスーア]]に属する。[[ガリシア統計局]]によると、2011年の人口は2,211人（2009年：2,288人、2006年：2,410人、2005年：2,461人、2004年：2,486人、2003年：2,599人）である<ref name=IGE/>。住民呼称は、男女同形のboimortense。\n\n\n[[ガリシア語]]話者の自治体住民に占める割合は99.92％（2001年）。",&ruigiVec);
		assert(ruigiVec.size()==0);//英語名がほしいわけじゃない
	}
	{
		
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"阿澄佳奈",L">{{声優\n| 名前 = 阿澄 佳奈\n| ふりがな = あすみ かな\n| 画像ファイル =\n| 画像サイズ =\n| 画像コメント =\n| 本名 =\n| 愛称 = あすみん、アスミス\n| 性別 = [[女性]]\n| 出生地 = {{JPN}}・[[福岡県]]\n| 死没地 =\n| 生年 = 1983\n| 生月 = 8\n| 生日 = 12\n| 没年 =\n| 没月 =\n| 没日 =\n| 血液型 = [[ABO式血液型|A型]]\n| 身長 = 161 [[センチメートル|cm]]\n| 職業 = [[声優]]、[[歌手]]\n| 事務所 = [[81プロデュース]]\n| 配偶者 = あり\n| 著名な家族 =\n| 公式サイト =\n| 活動 = {{声優/活動\n  | 職種 = 声優\n  | 活動名義 = \n  | 活動期間 = [[2005年]] - \n  | ジャンル = [[アニメ]]、[[ラジオ]]\n  | デビュー作 = ウェイトレス<ref name=\"声優データファイル2009\">{{Cite journal|和書 |title= 人気声優データファイル2009 |journal= アニメディア |issue= 2009年7月号第1付録 |pages= p.37 |publisher= 学習研究社 |accessdate= 2009-7-20 }}</ref><br/><small>（『[[Canvas2|Canvas2 縲恣Fのスケッチ縲彎]』）</small>\n}}{{声優/音楽活動\n  | 活動名義 = \n  | 活動期間 = [[1999年]] - \n  | ジャンル = [[J-POP]]、[[アニメソング]]\n  | 職種 = [[歌手]]\n  | 担当楽器 = \n  | レーベル = \n  | 共同作業者 = [[小梅伍]]、[[Friends (声優ユニット)|Friends]]、[[LISP (声優ユニット)|LISP]]\n  | 影響 = \n}}}}\n'''阿澄 佳奈'''（あすみ かな、[[1983年]][[8月12日]]<ref>{{Cite web|publisher=TSUTAYA online|url=http://www.tsutaya.co.jp/artist/00516254.html|title=阿澄佳奈 アーティストページ|accessdate=2014-06-09}}</ref> - ）は、[[日本]]の[[声優]]、[[歌手]]。\n\n\n[[福岡県]]出身。[[81プロデュース]]所属。{{VOICE Notice Hidden|冒頭部分に記載する代表作は、編集合戦誘発の原因となりますので、多数の出典で確認できるものに限ってください。[[プロジェクト:芸能人#記事の書き方]]にてガイドラインが制定されていますので、そちらも参照して下さい。}}",&ruigiVec);
		assert(ruigiVec[0] == L"あすみん");
		assert(ruigiVec[1] == L"アスミス");
		assert(ruigiVec.size()==2);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"スマネン",L"'''スマネン''' (sumanene) は、6員環を中心として[[フラーレン]]の一部を切り取った構造を持つ[[炭化水素]]化合物（[[バッキーボウル]]）。[[サンスクリット語]]・[[ヒンディー語]]で[[ヒマワリ]]を意味する\"Suman\"にちなみ命名された<ref>Mehta, G.; Shah, S. R.; Ravikumar, K. (1993). \"Towards the design of tricyclopenta [def, jkl, pqr] triphenylene (sumanene): a bowl-shaped hydrocarbon featuring a structural motif present in C<sub>60</sub> (buckminsterfullerene)\". ''J. Chem. Soc., Chem. Commun.'' (12): 1006窶骭1008. {{DOI|10.1039/C39930001006}}.</ref>。",&ruigiVec);
		assert(ruigiVec.size()==0);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"メノット",L">{{Infobox Film\n|作品名= メノット<br /><span style=\"font-size:70%\">''menotte''</span>\n|原題=\n|画像=\n|画像サイズ=\n|画像解説=\n|監督= [[及川中]]\n|製作総指揮=\n|製作= 中島仁\n|脚本= 甲谷利恵<br />及川中\n|ナレーター=\n|出演者= [[藤本綾]]<br />[[国分佐智子]]<br />[[金子昇]]<br />[[阿部進之介]]<br />[[魚谷輝明]]<br />[[甲本雅裕]]\n|音楽= [[神津裕之]]\n|主題歌=\n|撮影= 西村聡仁\n|編集= 大塚珠恵\n|製作会社= \n|配給= BLUE PLANET\n|公開= {{Flagicon|JPN}} [[2005年]][[7月23日]]\n|上映時間= 104分\n|製作国= {{JPN}}\n|言語= [[日本語]]\n|製作費=\n|興行収入=\n|前作=\n|次作=}}\n『'''メノット'''』は、[[2005年]]に公開された[[日本]]の[[サスペンス映画]]。[[映画のレイティングシステム#日本|R-15]]指定。\n\n\n[[藤本綾]]の映画初主演にして芸能界引退作品であり、劇中でヌードを披露して大胆な[[濡れ場]]を演じたことが話題を集めた。「メノット」は[[フランス語]]で「[[手錠]]」を意味する。",&ruigiVec);
		assert(ruigiVec.size()==0);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"ビキニ環礁",L">{{coord|11|35||N|165|23||E|display=title}}\n{{世界遺産概要表\n|site_img =File:Bikini Atoll.png\n|site_img_capt = ビキニ環礁の衛星写真 - NASA NLT Landsat 7 (Visible Color)\n|site_img_width=275px\n|ja_name = ビキニ環礁核実験場\n|en_name = Bikini Atoll Nuclear Test Site\n|fr_name = Site d’essais nucleaires de l’atoll de Bikini\n|country = マーシャル諸島\n|criterion_c = (4), (6)\n|rg_year = 2010年\n|ex_rg_year = \n|remarks = いわゆる[[負の世界遺産]]<ref>[[世界遺産アカデミー]]監修『くわしく学ぶ世界遺産300』[[マイナビ (企業)|マイナビ]]、2013年、p.216<!--負の遺産は公式な分類ではないので何らかの出典が必要--></ref>\n|url_no =1339\n|map_img = \n|map_img_width=\n|}}\n[[File:Flag of Bikini Atoll.svg|thumb|ビキニ環礁の地域旗　青地の中の23の白い星が当環礁の23の島、右上の3つの黒い星がキャッスル作戦で破壊された3つの島、右下の2つの黒星が島民が移住した2つの島を示している。旗の中の[[マーシャル語]]の記述は、1946年に米軍から退去を求められた際に首長が島民に語った言葉で「全ては神の手の内に」を意味する。]]\n[[File:Operation Crossroads Baker (wide).jpg|right|thumb|[[クロスロード作戦]]のベーカー核実験で発生した巨大な水柱]]\n[[画像:Castle Bravo Blast.jpg|thumb|[[キャッスル作戦]]・ブラボー実験の[[キノコ雲]]]]\n'''ビキニ環礁'''（ビキニかんしょう、Bikini Atoll）は、[[マーシャル諸島共和国]]に属する[[環礁]]。23の島嶼からなり、礁湖の面積は594.1平方キロメートル。\n\n[[1946年]]から[[1958年]]にかけて、[[太平洋核実験場]]の一つとして[[アメリカ合衆国]]が23回の[[核実験]]を行った<ref>太平洋核実験場全体では1946年から1963年の間に105回、同じマーシャル諸島ではエニウェトク環礁と合わせて69回の核実験が行われた。</ref>。\n\n2010年、[[第34回世界遺産委員会]]において、[[国際連合教育科学文化機関|ユネスコ]]の[[世界遺産]]リスト（[[文化遺産_(世界遺産)|文化遺産]]）に登録された<ref>{{cite web\n\n|url=http://whc.unesco.org/en/news/642\n|title=World Heritage Committee inscribes seven cultural sites on World Heritage List\n|accessdate=2010-08-01\n|publisher=UNESCO}}</ref>。マーシャル諸島共和国初の世界遺産となった。",&ruigiVec);
		assert(ruigiVec.size()==0);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"京都大学ト",L"><!--この記事は[[プロジェクト:大学/大学テンプレート (日本国内)]]にしたがって作成されています。-->\n{{大学 \n|国 = 日本\n|大学名 = 京都大学\n|ふりがな = きょうとだいがく\n|英称 = Kyoto University\n|画像 = Kyoto University.jpg\n|大学設置年 = 1897年\n|創立年 = 1869年\n|学校種別 = 国立\n|設置者 = 国立大学法人京都大学\n|本部所在地 = [[京都府]][[京都市]][[左京区]]吉田本町36番地1\n|緯度度 = 35|緯度分 = 1|緯度秒 = 34\n|経度度 = 135|経度分 = 46|経度秒 = 51\n|キャンパス = 吉田（京都府京都市左京区）<br>宇治（京都府[[宇治市]]）<br>桂（京都府京都市[[西京区]]）\n|学部 = 総合人間学部<br>文学部<br>教育学部<br>法学部<br>経済学部<br>理学部<br>医学部<br>薬学部<br>工学部<br>農学部\n|研究科= 文学研究科<br>教育学研究科<br>法学研究科<br>経済学研究科<br>理学研究科<br>医学研究科<br>薬学研究科<br>工学研究科<br>農学研究科<br>人間・環境学研究科<br>エネルギー科学研究科<br>アジア・アフリカ地域研究研究科<br>情報学研究科<br>生命科学研究科<br>地球環境学大学院<br>公共政策大学院<br>経営管理大学院<br>総合生存学館\n|全国共同利用施設 = 学術情報メディアセンター<br>放射線生物研究センター<br>生態学研究センター<br>地域研究統合情報センター<br>野生動物研究センター\n|大学の略称 = 京大（きょうだい）\n|ウェブサイト = http://www.kyoto-u.ac.jp/\n}}" , &ruigiVec);
		assert(ruigiVec[0] == L"京大");
		assert(ruigiVec.size()==1);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseAimai(L"ステュアート",L"'''ステュアート'''、'''スチュアート'''、'''スチュワート''' (Stuart, Stewart) は[[英語圏]]の姓、男性名。職業名 steward 「[[家政|家令]]、[[執事]]、[[宮宰]]」に由来する（[[古英語]]：stﾄｫ(g)weard 「屋敷を保護する者」より）。[[英語]]での発音は、米音で'''ストゥー'''アートもしくは'''ステュー'''アート <small>{{IPA|ﾋ・t''j''uﾋ惜嗾}}</small> 、[[容認発音|英音]]で'''ステュ'''アット <small>{{IPA|ﾋ・tjﾊ緩冲}}</small> （太字はアクセント）。後者の綴りの方は英音で'''ステュー'''アット<small>{{IPA|ﾋ・tjuﾋ惜冲}}</small>とも発音され得る。\n\n== 姓 ==\n:スコットランドを発祥とする貴族の家系ステュアート家の人物は、[[:Category:ステュアート家]]以下のカテゴリに収められている。\n* [[アレクサンダー・ヒュー・ホームズ・スチュアート]] - アメリカの政治家。\n* [[イアン・スチュワート (野球)]] - アメリカの野球選手。\n* [[イアン・スチュワート (陸上選手)]] - イギリスの陸上競技選手。\n* [[イアン・スチュワート (ミュージシャン)]] - スコットランドのキーボーディスト。\n* [[エリック・スチュワート]] - イギリスのミュージシャン、ソングライター。\n* [[ギルバート・ステュアート]] - アメリカの画家。\n* [[キンバリー・スチュワート]] - アメリカのファッションモデル、ソーシャライト。。\n* [[クリス・スチュワート]] - アメリカの野球選手。\n* [[クリステン・スチュワート]] - アメリカの女優。\n* [[クリストル・スチュワート]] - アメリカのモデル。\n* [[グロリア・スチュアート]] - アメリカの女優。\n* [[ケロン・スチュワート]] - ジャマイカの陸上競技選手。\n* [[ザック・スチュワート]] - アメリカの野球選手。\n* [[J・E・B・スチュアート]] - アメリカ南北戦争期の南軍の将軍。\n* [[ジェームズ・ステュアート (俳優)]] - アメリカの俳優。\n* [[ジェームス・スチュワート (レーサー)]] - アメリカのモトクロスライダー。\n* [[ジャッキー・スチュワート]] - スコットランド出身のF1レーサー。\n* [[シャノン・スチュワート]] - アメリカの野球選手。\n* [[ジョッシュ・スチュワート]] - アメリカの野球選手。\n* [[ジョン・スチュワート (コメディアン)]] - アメリカのコメディアン、風刺家、俳優、作家、テレビプロデューサー。\n* [[ジョン・ステュアート (第3代ビュート伯)]] - 18世紀イギリスの首相。\n* [[ジル・スチュアート]] - アメリカ出身のファッションデザイナーおよびそのファッションブランド。\n* [[ダミオン・スチュワート]] - ジャマイカのサッカー選手。\n* [[ディック・スチュアート]] - アメリカの野球選手。\n* [[デイブ・スチュワート (野球)]] - アメリカの野球選手。\n* [[デイヴ・スチュワート (ギタリスト)]] - [[ユーリズミックス]]のギタリスト。\n* [[デイヴ・スチュワート (キーボーディスト)]] - イギリスのキーボーディスト、ミュージシャン。\n* [[デリール・スチュワート]] - アメリカの天文学者。\n* [[トニー・スチュワート]] - アメリカのレーシングドライバー。\n* [[トマス・ピーター・アンダーソン・ステュアート]] - スコットランド出身の生理学者。\n* [[ノーマン・ベイリー＝スチュワート]] - イギリスの軍人。\n* [[パトリック・スチュワート]] - イギリス出身の俳優。\n* [[ブーブー・スチュワート]] - アメリカの歌手、ダンサー、モデル、俳優。\n* [[フローンデル・スチュアート]] - バルバドスの政治家。\n* [[ペイン・スチュワート]] - アメリカのプロゴルファー。\n* [[ポール・スチュワート (作家)]] - イギリスの作家。\n* [[マーク・スチュワート]] - イギリスのミュージシャン、ボーカリスト。\n* [[マーサ・スチュワート]] - アメリカの実業家。\n* [[マイケル・スチュワート]] - アメリカのバスケットボール選手。\n* [[マクラーレン・スチュワート]] - アメリカのアニメーター。\n* [[メルビン・スチュワート]] - アメリカの競泳選手。\n* [[レイモンド・スチュワート]] - ジャマイカの陸上競技選手。\n* [[ロッド・スチュワート]] - イギリスのミュージシャン。\n* [[ロナルド・ニール・スチュアート]] - イギリス海軍の軍人。\n* [[ロバート・ステュアート (カスルリー子爵)]] - アイルランドとイギリスの政治家、外交官。\n* [[ロバート・ダグラス・スチュアート]] - アメリカの実業家、外交官。\n",&ruigiVec);
		assert(ruigiVec.size()==0);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseAimai(L"日輪寺",L">'''日輪寺'''\n\n*[[茨城県]][[久慈郡]][[大子町]]にある[[天台宗]]の寺院。 - [[日輪寺 (大子町)]]\n*[[群馬県]][[前橋市]]にある[[真言宗豊山派]]の寺院。 - [[日輪寺 (前橋市)]]\n*[[東京都]][[文京区]]にある[[曹洞宗]]の寺院。 - [[日輪寺 (文京区)]]\n*東京都[[台東区]]にある[[時宗]]の寺院。 - [[日輪寺 (台東区)]]\n*[[福岡県]][[久留米市]]にある[[臨済宗妙心寺派]]の寺院。 - [[日輪寺 (久留米市)]]\n*[[熊本県]][[山鹿市]]にある曹洞宗の寺院。 - [[日輪寺 (山鹿市)]]\n\n{{aimai}}\n{{デフォルトソート:にちりんし}}\n\n[[Category:同名の寺]]\n",&ruigiVec);
		assert(ruigiVec.size()==0);
	}
	{
		std::vector<std::wstring> ruigiVec;
		bool r = parseRedirect(L"少年犯罪板",L">#転送 [[社会 (2ちゃんねるカテゴリ)]]\n\n[[Category:社会 (2ちゃんねるカテゴリ)]]\n{{DEFAULTSORT:しようねんはんさいいた}}",&ruigiVec);
		assert(ruigiVec.size()==0);

		parseRuigi(L"少年犯罪板",L">#転送 [[社会 (2ちゃんねるカテゴリ)]]\n\n[[Category:社会 (2ちゃんねるカテゴリ)]]\n{{DEFAULTSORT:しようねんはんさいいた}}",&ruigiVec);
		assert(ruigiVec.size()==0);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"ツナマヨネーズ",L"'''ツナマヨネーズ'''とは、[[ツナ#ツナ缶|ツナ缶]]のツナ（[[マグロ]]の油漬け）と[[マヨネーズ]]を和えたもの。また、[[カツオ]]の油漬けでも代用される。略称は、ツナマヨ。",&ruigiVec);

		assert(ruigiVec[0] == L"ツナマヨ");
		assert(ruigiVec.size()==1);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"フリーソフトウェア",L"\n{{Otheruses|[[フリーソフトウェア財団]]が提唱する'''自由な'''ソフトウェア|'''無償の'''ソフトウェア|フリーウェア}}\n'''フリーソフトウェア''' (free software) とは、[[ソフトウェア]]のうち、[[フリーソフトウェア財団]]が提唱する'''自由な'''ソフトウェアを指す。大半のフリーソフトウェアは無償（フリー）で配布されているが、定義に従えば、ここでいうフリーソフトウェアについて一次配布が無償である必要は必ずしもない。\n\nフリーソフトウェア財団は[[フリーソフトウェアの定義]]を提示している。[[ソフトウェアライセンス]]については[[フリーソフトウェアライセンス]]を参照。\n\n定義に照らして自由ではない、すなわち改造や再配布などに制限が掛かっていたり、ソースコードが開示されていない、'''無償で利用できるソフトウェアとは異なる概念'''であり、この場合は[[フリーウェア]]もしくは無料ソフトと呼ぶことが望ましいとフリーソフトウェア財団はしている。\n\n逆に定義に従ったソフトウェアであれば、一次的な配布が有償であってもフリーソフトウェアと呼ぶことができる。ただし、前述したように配布が自由であるため、ほとんどのフリーソフトウェアは無償で配布されている。\n\nまた、現状強い影響力を持つ定義として、フリーソフトウェア財団の定義の他に、[[Debianフリーソフトウェアガイドライン]]とそれをベースにした[[Open Source Initiative]]の[[オープンソースの定義]]がある。",&ruigiVec);
		assert(ruigiVec.size()==0);
	}

	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"アンパサンド",L"{{記号文字|&amp;}}\n[[Image:Trebuchet MS ampersand.svg|right|thumb|100px|[[Trebuchet MS]] フォント]]\n'''アンパサンド''' ({{lang|en|ampersand}}, '''&amp;''') とは「…と…」を意味する[[記号]]である。[[英語]]の {{lang|en|\"and\"}} に相当する[[ラテン語]]の {{lang|la|\"et\"}} の[[合字]]で、{{lang|en|\"etc.\"}} (et cetera = and so forth)を {{lang|en|\"&amp;c.\"}} と記述することがあるのはそのため。[[Trebuchet MS]]フォントでは、[[ファイル:Trebuchet MS ampersand.svg|10px]]と表示され \"et\" の合字であることが容易にわかる。\n__TOC__\n{{Clear}}",&ruigiVec);

		assert(ruigiVec.size()==0);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"簡文帝",L"'''簡文帝'''（かんぶんてい）は、[[東アジア]]の[[皇帝]]の[[諡号]]の一つ。\n\n* [[簡文帝 (東晋)]] （司馬昱、在位[[371年]] - [[372年]]）\n* [[簡文帝 (南朝梁)]] （蕭綱、在位[[549年]] - [[551年]]）\n\n== 関連項目 ==\n* [[同諡号廟号一覧]]\n\n[[Category:皇帝の諡号]]\n{{aimai}}<!--人名ではない-->\n{{DEFAULTSORT:かいてい}}",&ruigiVec);

		assert(ruigiVec.size()==0);
	}

	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"ビーフステーキ",L"[[Image:Sirloin steak.JPG|thumb|300px|ビーフステーキ]]\n'''ビーフステーキ'''（{{lang|en|Beef steak}}）とは、[[フライパン]]等の[[鉄板]]、あるいは[[金網]]等を使用して[[直火焼き]]するなど、[[牛肉]]のスライスを焼いた'''[[ステーキ]]'''、[[肉料理]]の一種である。\n\n語源は串に刺し焼いた肉を指す[[古ノルド語]]([[wikt:en:steik|steik]])からとされ、[[ロンドン]]で切り身を焼く形になった。日本では長らく略語としての「[[ビステキ]]」、また[[フランス語]]（bifteck）から[[ビフテキ]]<ref>北岡敬『そこが知りたい【事始め】の物語』雄鶏社</ref>とも呼び、古くは[[夏目漱石]]や[[和辻哲郎]]の作品にも記されているが、これらの呼称は[[レストラン]]などで使用される事はほとんど無い。",&ruigiVec);

		assert(ruigiVec[1] == L"ビフテキ");
		assert(ruigiVec[0] == L"ビステキ");
		assert(ruigiVec.size()==2);
	}


	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"じゃんけん",L"'''じゃんけん'''は、手だけを使って3種類の指の出し方（グー・チョキ・パー）で[[三すくみ]]を構成し、[[勝負|勝敗]]を決める手段である。[[日本]]で[[拳遊び]]を基に考案されたが、[[現代 (時代区分)|現代]]では世界的に普及が進んでいる。\n\n\n日本国内では「じゃいけん」「いんじゃん」など地域によって様々な呼び方がある。[[中華人民共和国|中国]]では「猜拳」と呼ぶ。[[英語圏]]の場合、[[イギリス]]では \"Scissors Paper Stone\" などと表現されることもあるが、イギリスや[[アメリカ合衆国]]を含めて多くは \"Rock-paper-scissors\" という呼称が使われている（表記上の揺れは数種類ある<ref group=\"注\">\"Rock Paper Scissors\", \"Rock, Paper, Scissors\", etc.</ref>。略号はRPS<ref>[[#WRPS|World Rock Paper Scissors (RPS) Society]].</ref>）。",&ruigiVec);
		assert(ruigiVec.empty());
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"記号学",L"{{出典の明記|date=2011年3月}}\n{{言語学}}\n'''記号学'''（きごうがく、[[英語]]：semiology）は、[[言語]]を始めとして、何らかの[[事象]]を別の事象で代替して[[表現]]する手段について[[研究]]する[[学問]]を指す。'''記号論'''（きごうろん、英語：semiotics）ともいう。",&ruigiVec);

		assert(ruigiVec[0] == L"記号論");
		assert(ruigiVec.size()==1);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"医療漫画",L"{{漫画}}\n'''医療漫画'''（いりょうまんが）は、[[日本]]における[[漫画作品一覧|漫画作品]]のジャンルの一つ。[[医師]]や[[看護師]]をはじめとした[[医療従事者]]を主人公としていたり、[[病院]]や[[診療所]]などの医療現場を舞台とするなど、主に[[医療]]をテーマにした漫画作品を指す。<ref name=\"kyoto\">京都マンガミュージアム「医療マンガの解剖学」</ref><ref>『読売新聞』（2007年12月29日）</ref>",&ruigiVec);

		assert(ruigiVec.empty());
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"統計学",L"'''統計学'''（とうけいがく、[[英語|英]]: statistics、[[ドイツ語|独]]: Statistik）とは、[[統計]]に関する[[研究]]を行う[[学問]]である。\n\n== 概要 ==\n{{出典の明記|section=1|date=2011年11月}}\n統計学は、経験的に得られたバラツキのある[[データ]]から、[[応用数学]]の手法を用いて数値上の性質や規則性あるいは不規則性を見いだす。統計的手法は、実験計画、データの要約や解釈を行う上での根拠を提供する学問であり、幅広い分野で応用されている。\n\n英語で統計または統計学を ''statistics'' と言うが、語源はラテン語で「状態」を意味する ''statisticum'' であり、この言葉が[[イタリア語]]で「[[国家]]」を意味するようになり、国家の人力、財力等といった[[国勢データ]]を比較検討する学問を意味するようになった。<ref group=\"注\">[[ラテン語]]で\"statisticum (collegium)\"という表現があるが、この意味は\"[[社会]]の状態の科学”である{{要出典|date=2014年9月}}。</ref>\n現在では、[[医学]]（[[疫学]]、[[根拠に基づいた医療|EBM]]）、[[薬学]]、[[経済学]]、[[社会学]]、[[心理学]]、[[言語学]]など、[[自然科学]]・[[社会科学]]・[[人文科学]]の実証分析を伴う分野について、必須の学問となっている。また、統計学は哲学の一分科である[[科学哲学]]においても重要なひとつのトピックスになっている。<ref group=\"注\">「{{要出典範囲|これは、統計学が科学的な研究において方法論上の基礎的な部分を構成していながら、[[確率]]という一種捉えがたい概念を扱っているためであり|date=2014年9月}}、その意味や在り方が帰納の正当性の問題などと絡めて真剣に議論されている。」{{誰|date=2014年9月}}</ref><!--ドイツ語Statistik(Statistics)の名前は[[1748年]]ドイツの政治学者　アッヘンワール([[1719年]] - [[1772年]]) が『ヨーロッパ諸国国家学綱要』の中でstate(国状学)をStatistik(統計学)と名づけたことによる。-->",&ruigiVec);

		assert(ruigiVec.empty());
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"地震",L"\n{{Otheruses||クルアーンのスーラ|地震 (クルアーン)}}\n'''地震'''（じしん、{{lang-en-short|Earthquake}}）という語句は、以下の2つの意味で用いられる<ref name=\"SSJ (2007)\">[[#SSJ (2007)|日本地震学会地震予知検討委員会(2007)]]</ref>。\n\n# [[地震学]]における定義: [[地球]]表面を構成している岩盤（[[地殻]]）の内部で、固く密着している岩石同士が、'''[[断層]]'''と呼ばれる破壊面を境目にして、急激にずれ動くこと。これによって大きな地面の[[振動]]が生じこれを'''[[地震動]]'''（じしんどう）という。\n# 地表面のゆれ: 地震動のことで一般的にはこちらも「地震」と呼ばれる。「地震」（なゐふる）という語句は『[[日本書紀]]』にも見え、その他[[古文書]]の記録にも登場するが、これらは今日の地震学における地震動のことであり、また「大地震」、「小地震」などと共に[[震度]]の程度を表すものでもあった<ref>{{PDFlink|[http://sakuya.ed.shizuoka.ac.jp/rzisin/kaishi_18/01-Usami.pdf 宇佐美龍夫 (2002)]}} 宇佐美龍夫 「歴史史料の「日記」の地震記事と震度について」『歴史地震』 第18号、1-14、2002年</ref>。\n\n地震を対象とした[[学問]]を'''[[地震学]]'''という。地震学は[[地球物理学]]の一分野であり、[[構造地質学]]と密接に関わっている。",&ruigiVec);

		assert(ruigiVec.empty());
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"長野県",L"'''長野県'''（ながのけん）は、[[本州]][[内陸]]部に位置する[[日本]]の[[県]]。[[令制国]]名の[[信濃国]]に因み「'''[[信州]]'''」とも呼ばれる。海に面していない、いわゆる[[内陸県]]であり、大規模な山岳地があるため可住地面積率は小さい。\n[[県庁所在地]]は[[長野市]]で、[[善光寺]]の[[門前町]]として発展し、第18回[[冬季オリンピック]]の開催地となった都市である。",&ruigiVec);

		assert(ruigiVec[0] == L"信州");
		assert(ruigiVec[1] == L"信濃国");
		assert(ruigiVec.size()==2);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"コムギ",L"[[File:USDA wheat.jpg|thumb|様々なコムギ食品]]\n'''コムギ'''（'''小麦'''、英名: [[w:Wheat|Wheat]]）は、[[イネ科]]'''コムギ属'''に属する[[一年生植物|一年草]]の[[植物]]。一般的には[[パンコムギ]]（[[学名]]: ''[[w:Triticum aestivum|Triticum aestivum]]''）を指すが、広義には''T. compactum''（クラブコムギ、{{lang-en-short|club wheat}}）や ''[[w:Triticum durum|T. durum]]''（デュラムコムギ、{{lang-en-short|[[w:Durum wheat|Durum wheat]]}}、マカロニコムギ、{{lang-en-short|[[w:Macaroni wheat|Macaroni wheat]]}}）などコムギ属 ([[学名]]: ''[[w:Triticum|Triticum]]''、{{lang-en-short|''[[w:Wheat|Wheat]]''}}) 植物全般を指す。[[世界三大一覧#食|世界三大穀物]]の一つ。古くから栽培され、世界で最も生産量の多い[[穀物]]のひとつである。年間生産量は6億トン近くあり[[トウモロコシ]]の8億トンに並んでいる。\n\n他の三大穀物と同じく基礎食料であり、各国で生産された小麦はまずは国内で[[消費]]され、剰余が[[輸出]]される。\n\n日本国内において、麦（小麦・[[大麦]]・[[はだか麦]]）は[[食糧法]]により[[価格統制]]が存在する。",&ruigiVec);

		assert(ruigiVec.empty());
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"単位胞",L"'''単位胞'''（たんいぼう、Unit cell）とは、[[結晶]]中の[[空間格子]]の格子点がつくる平行6面体のうち、空間格子の構造単位として選ばれたものである。'''単位格子'''と言うこともある。つまり、単位胞は結晶構造の周期パターンの単位となる平行6面体であり、結晶構造は単位胞の敷き詰めで表現される。\n\n[[Image:Celda unitaria1.jpg|thumb|'''空間格子'''と'''単位胞'''<br />a,b,c,''α'',''β'',''γ''は'''格子定数''']]\n単位胞の頂点から伸びる、3つの稜を表す3本の[[ベクトル]]〈'''''a''''', '''''b''''', '''''c'''''〉は'''基本ベクトル'''と呼ばれる。ベクトルの大きさ〈距離〉と単位ベクトルの成す角、''α''=&ang;bc、''β''=&ang;ca、''γ''=&ang;abは単位胞の'''格子定数'''と呼ばれる。\n\n頂点以外に空間格子の格子点を含まない単位格子を'''単純単位格子'''と呼び、頂点以外にも格子点を含む場合は'''多重単位格子'''と呼ぶ。\n\n単純単位格子のうち、距離a,b,cが最短になるように選択した単純単位格子は'''既約単位格子'''と呼ばれる。その場合、''α''、''β''、''γ''はすべて鈍角かすべて鋭角となる。\n\n多重単位格子には[[体心格子]]、[[面心格子]]、[[底心格子]]が含まれる。\n\nある空間格子が存在するとき、格子点に違いがなければ一つの空間格子に対して複数種類の単位胞を設定することが可能である。実際の結晶では、つまりイオン結晶などでは格子点に異なる[[原子]]・[[分子]]等が配置されるため単位胞の選択に対して[[対称性]]・[[並進性]]に関する制約が発生する。",&ruigiVec);

		assert(ruigiVec.empty());
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"排他制御",L"\n[[ファイル:Mutual exclusion example with linked list.png|thumb|排他制御せずに ''i'' と ''i+1'' という2つのノードを同時に[[連結リスト]]から外す操作を行うと、結果として ''i+1'' のノードが外れないという状態になりうる。]]\n'''排他制御'''（はいたせいぎょ）とは、[[プログラム (コンピュータ)|コンピュータ・プログラム]]の実行において、複数の[[プロセス]]が利用出来る共有資源に対し、複数のプロセスからの同時アクセスにより[[競合状態|競合]]が発生する場合に、あるプロセスに資源を独占的に利用させている間は、他のプロセスが利用できないようにする事で整合性を保つ処理の事をいう。'''相互排除'''または'''相互排他'''（{{lang|en|mutual exclusion}}）ともいう。最大k個のプロセスが共有資源にアクセスして良い場合を k-相互排除という。\n\n換言すれば1つの[[クリティカルセクション]]に複数のプロセス（またはスレッド）が同時に入ることを防ぐことである。クリティカルセクションとは、プロセスが[[共有メモリ]]などの共有資源にアクセスしている期間を指す。排他制御の問題は1965年、[[エドガー・ダイクストラ]]が ''Solution of a problem in concurrent programming control''（並行プログラミング制御における問題の解法）と題した論文で扱ったのが最初である<ref>E. W. Dijkstra. [http://www.di.ens.fr/~pouzet/cours/systeme/bib/dijkstra.pdf Solution of a problem in concurrent programming control]. Communications of the ACM, 8(9), page 569, September, 1965.</ref><ref  name=\"Taubenfeld:2004\">Taubenfeld. [http://www.cs.tau.ac.il/~afek/gadi.pdf The Black-White Bakery Algorithm]. In Proc. Distributed Computing, 18th international conference, DISC 2004. Vol 18, 56-70, 2004</ref>。\n\n排他制御の重要性を示す例として、片方向[[連結リスト]]がある（右図）。このような連結リストからノードを削除するには、1つ前のノードにある次のノードを指すポインタを削除したいノードの次のノードを指すように書き換える（例えば、ノード ''i'' を削除するには、ノード ''i-1'' のnextポインタをノード ''i+1'' を指すよう書き換える）。このとき、その連結リストを複数プロセスが共有しているなら、2つのプロセスがそれぞれ別のノードを削除しようとして次のような問題を生じる可能性がある。\n* 2つのプロセスはそれぞれノード ''i'' と ''i+1'' を同時に削除しようとする。どちらのノードも連結リストの途中にあり、先頭でも最後尾でもないとする。\n* ノード ''i-1'' のnextポインタはノード ''i+1'' を指すよう書き換えられ、ノード ''i'' のnextポインタはノード ''i+2'' を指すよう書き換えられる。\n* 両方の削除処理が完了した状態を見ると、ノード ''i-1'' がノード ''i+1'' を指すよう書き換えられたために、ノード ''i+1'' は連結リストに残ってしまう。\nこの問題は排他制御を施して複数の状態更新処理が同時に行われないようにすれば解決する。",&ruigiVec);

		assert(ruigiVec[0] == L"相互排他");
		assert(ruigiVec[1] == L"相互排除");
		assert(ruigiVec.size()==2);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"オセロ",L"'''オセロ''' ('''Othello''') とは、2人用の[[ボードゲーム]]。交互に盤面へ石を打ち、相手の石を挟むと自分の石の色に変わる。最終的に石の多い側が勝者となる。単純なルールながら、ゲームとしての複雑さは人間が[[展開型ゲーム#ゲームの木|ゲームの木]]の全展開を把握可能な程度を越えており、2014年9月現在まだ、コンピュータによる全解析は達成されていない。「オセロ」を商標として登録し、用品等を商品として販売していたツクダは“A minute to learn, a lifetime to master”（覚えるのに1分、極めるのに一生）がこのゲーム[[キャッチフレーズ]]であるとしていた。\n\n一般名としては「'''リバーシ''' ('''Reversi''') 」としても知られている。[[#オセロとリバーシ]]の節も参照。",&ruigiVec);

		assert(ruigiVec[0] == L"リバーシ");
		assert(ruigiVec.size()==1);
	}

	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"ハンカチ落し",L"'''ハンカチ落し'''（ハンカチおとし）とは、複数の人が野外または野内で行う遊び。\n\n[[鬼]]とそれ以外に分かれ、鬼以外は円になって座った後、内側を向いて、鬼の動作を見守る。鬼は[[ハンカチ]]を持ってそれらの人々の後ろを走りまわり、そ知らぬ顔で、ハンカチを落とす。鬼がハンカチを落としたら、自分の後ろにハンカチを落とされた人は、鬼が1周して自分の所に戻ってくる前に、鬼を追いかけなければならない。鬼が追いつかれずに、ハンカチを落とされた人のところに座り込めば、鬼の勝ちとなり、鬼が交代する。追いつかれた場合には、再び鬼とならなければならない。\n\nまた、鬼が一周してくるまでハンカチに気付かず、鬼にタッチされた人は円の中心に座らされて遊びに参加出来なくなるというルールがある場合もある。この場所は「便所」などと呼ばれ、新たに「便所」に入る人が現れるまで出る事が出来ないが、「便所」の人への配慮として、「鬼が5回変わるまで」などの時間制限が設けられる事もある。",&ruigiVec);

		assert(ruigiVec.empty());
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"シンセサイザー",L"{{Otheruses|楽器のシンセサイザー|電子工学のシンセサイザー|周波数シンセサイザ}}\n[[ファイル:MinimoogVoyager.jpg|right|200px|thumb|シンセの古典的機種「[[モーグ・シンセサイザー#ミニモーグ|ミニモーグ]]」]]\n'''シンセサイザー'''（{{lang-en|synthesizer}}）は、一般的には主に[[電子工学]]的手法により楽音等を合成（{{lang-en|synthesize}}：シンセサイズ）する[[楽器]]「ミュージック・シンセサイザー」の総称。[[電子楽器]]、[[音源]]と呼ばれることもある。\n\n:<small>以降、記述の煩雑化を避けるため、正式名称「シンセサイザー」を適宜「シンセ」と略記する。</small><!-- 左記、書き手の「手抜き」ではないのか? -->",&ruigiVec);

		assert(ruigiVec[0] == L"シンセ");
		assert(ruigiVec.size()==1);
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"蝦夷",L"\n{{Otheruseslist|集団としての蝦夷|地域名としての蝦夷|蝦夷地|[[小惑星]]|蝦夷 (小惑星)|[[蘇我氏]]の人物|蘇我蝦夷|その他の「えびす」|えびす (曖昧さ回避)}}\n[[File:Monument to Aterui and More2.jpg|thumb|right|[[アテルイ]]、モレの顕彰碑<br>（[[京都市]][[清水寺]]）]]\n'''蝦夷'''（えみし、えびす、えぞ）は、[[ヤマト王権|大和朝廷]]から続く歴代の中央政権から見て、[[日本列島]]の東方（現在の[[関東地方]]の一部と[[東北地方]]）や、北方（現在の[[北海道|北海道地方]]）に住む人々を異端視・異族視した呼称である。\n\n中央政権の征服地域が広がるにつれ、この言葉が指し示す人々および地理的範囲は変化した。[[近世]]以降は、[[北海道]]・[[樺太]]・[[千島]]の先住民族で、[[アイヌ語]]を[[母語]]とする[[アイヌ]]を指す。",&ruigiVec);
		
		assert(ruigiVec.size()==0);
	}

	{
		std::vector<std::wstring> ruigiVec;
		parseMultipleRuigi(L"エジプト",L"'''エジプト・アラブ共和国'''",&ruigiVec);

		assert(ruigiVec[0] == L"エジプト・アラブ共和国");
	}
	{
		std::vector<std::wstring> ruigiVec;
		parseRuigi(L"エジプト",L"'''エジプト・アラブ共和国'''（エジプト・アラブきょうわこく）、通称'''エジプト'''は、[[中東]]・[[アフリカ]]の[[共和国]]。[[首都]]は[[カイロ]]。",&ruigiVec);
		assert(ruigiVec.empty());
	}

}
