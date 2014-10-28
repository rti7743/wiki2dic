
#include "common.h"
#include "XLWStringUtil.h"
#include "XLFileUtil.h"
#include "wiki2eng.h"
#include "wiki2yomi.h"
#include "wiki2util.h"
#include "option.h"



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


//読みが汚れていることがあるので綺麗にします.
static std::wstring cleaningYomi(const std::wstring& yomi)
{
	if (yomi.empty() ) return yomi;
	//{{}}が始まるゴミがあったら消します	yomi = "げんご {{lang-la-short|Lingua}}、{{lang-en-short|Language}}"
	std::wstring w = XLWStringUtil::strsnip(yomi,L"{{",L"}}");
//	w = XLWStringUtil::strsnip(w,L"[",L"]");

	w = XLWStringUtil::replace(w,L"Republic of " ,L" ");
	w = XLWStringUtil::replace(w,L"Kingdom of " ,L" ");
	w = XLWStringUtil::replace(w,L"State of " ,L" ");
	w = XLWStringUtil::replace(w,L"The " ,L" ");
	w = XLWStringUtil::replace(w,L" Corporation" ,L" ");
	w = XLWStringUtil::replace(w,L" Corp" ,L" ");
	w = XLWStringUtil::replace(w,L"[[w:" ,L" ");


	w = cleaningYomiForWord(w,L"、");
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
	
//	w = cleaningYomiForWord(w,L"-");
//	if (w.empty())		return L"";

	w = XLWStringUtil::replace(w,L"'" ,L"");

	w = cleaningYomiForWord(w,L"|");
	if (w.empty())		return L"";

	//アルファベット以外を飛ばす.
	for(unsigned int i = 0 ; i < w.size() ; i++)
	{
		if ( XLWStringUtil::isAlpha(w[i]) || XLWStringUtil::isSpace(w[i]) || w[i] == '-' )
		{
			continue;
		}
		else
		{
			w[i] = L'あ'; 
		}
	}
	w = XLWStringUtil::replace(w,L"あ" ,L"");
//	w = XLWStringUtil::replace(w,L"　" ,L"");
	w = XLWStringUtil::chop(w,L" 　");

	return w;
}

static inline bool checkCase(WCHAR a,WCHAR b)
{
	return tolower(a) == b;
}

static bool checkFirstChar(const std::wstring& titleW,const std::wstring& word)
{
	if (titleW.size() <= 0) return false;
	if (word.size() <= 2) return false;
	switch (titleW[0])
	{
	case L'あ': 	case L'ア':
		return ( checkCase(word[0],'a') || checkCase(word[0],'u') || checkCase(word[0],'i') || checkCase(word[0],'o') );
	case L'い':		case L'イ':
		return ( checkCase(word[0],'i') || checkCase(word[0],'e') || checkCase(word[0],'y') );
	case L'う':		case L'ウ':
		return ( checkCase(word[0],'u') || checkCase(word[0],'w') || checkCase(word[0],'o') || checkCase(word[0],'v') );
	case L'え':		case L'エ':
		return ( checkCase(word[0],'a') || checkCase(word[0],'m') || checkCase(word[0],'x') || checkCase(word[0],'e') || checkCase(word[0],'f') || checkCase(word[0],'s') || checkCase(word[0],'h') || checkCase(word[0],'l') || checkCase(word[0],'a') );
	case L'お':		case L'オ':
		return ( checkCase(word[0],'o') || checkCase(word[0],'a') );
	case L'か':		case L'カ':
		return ( checkCase(word[0],'k') || checkCase(word[0],'c') );
	case L'き':		case L'キ':
		return ( checkCase(word[0],'k') || checkCase(word[0],'c') || checkCase(word[0],'x') );
	case L'く':		case L'ク':
		return ( checkCase(word[0],'k') || checkCase(word[0],'c') || checkCase(word[0],'x') || checkCase(word[0],'q') );
	case L'け':		case L'ケ':
		return ( checkCase(word[0],'k') || checkCase(word[0],'c') || checkCase(word[0],'x') || checkCase(word[0],'q') );
	case L'こ':		case L'コ':
		return ( checkCase(word[0],'k') || checkCase(word[0],'c') );
	case L'さ':		case L'サ':
		return ( checkCase(word[0],'s') || checkCase(word[0],'c') || checkCase(word[0],'t') );
	case L'し':		case L'シ':
		return ( checkCase(word[0],'s') || checkCase(word[0],'t') || checkCase(word[0],'c') );
	case L'す':		case L'ス':
		return ( checkCase(word[0],'s') || checkCase(word[0],'t')  );
	case L'せ':		case L'セ':
		return ( checkCase(word[0],'s') || checkCase(word[0],'c') );
	case L'そ':		case L'ソ':
		return ( checkCase(word[0],'s') );
	case L'た':		case L'タ':
		return ( checkCase(word[0],'t') );
	case L'ち':		case L'チ':
		return ( checkCase(word[0],'t') || checkCase(word[0],'c') );
	case L'つ':		case L'ツ':
		return ( checkCase(word[0],'t') );
	case L'て':		case L'テ':
		return ( checkCase(word[0],'t') );
	case L'と':		case L'ト':
		return ( checkCase(word[0],'t') );
	case L'な':		case L'ナ':
		return ( checkCase(word[0],'n') || checkCase(word[0],'k') );
	case L'に':		case L'ニ':
		return ( checkCase(word[0],'n') );
	case L'ぬ':		case L'ヌ':
		return ( checkCase(word[0],'n') );
	case L'ね':		case L'ネ':
		return ( checkCase(word[0],'n') );
	case L'の':		case L'ノ':
		return ( checkCase(word[0],'n') );
	case L'は':		case L'ハ':
		return ( checkCase(word[0],'h') );
	case L'ひ':		case L'ヒ':
		return ( checkCase(word[0],'h') );
	case L'ふ':		case L'フ':
		return ( checkCase(word[0],'f') || checkCase(word[0],'p') || checkCase(word[0],'v') );
	case L'へ':		case L'ヘ':
		return ( checkCase(word[0],'h') );
	case L'ほ':		case L'ホ':
		return ( checkCase(word[0],'h') );
	case L'ま':		case L'マ':
		return ( checkCase(word[0],'m') );
	case L'み':		case L'ミ':
		return ( checkCase(word[0],'m') );
	case L'む':		case L'ム':
		return ( checkCase(word[0],'m') );
	case L'め':		case L'メ':
		return ( checkCase(word[0],'m') );
	case L'も':		case L'モ':
		return ( checkCase(word[0],'m') );
	case L'や':		case L'ヤ':
		return ( checkCase(word[0],'y') || checkCase(word[0],'j') );
	case L'ゆ':		case L'ユ':
		return ( checkCase(word[0],'y') || checkCase(word[0],'e') || checkCase(word[0],'j') || checkCase(word[0],'u') );
	case L'よ':		case L'ヨ':
		return ( checkCase(word[0],'y') );
	case L'ら':		case L'ラ':
		return ( checkCase(word[0],'r') || checkCase(word[0],'l') );
	case L'り':		case L'リ':
		return ( checkCase(word[0],'r') || checkCase(word[0],'l') );
	case L'る':		case L'ル':
		return ( checkCase(word[0],'r') || checkCase(word[0],'l') );
	case L'れ':		case L'レ':
		return ( checkCase(word[0],'r') || checkCase(word[0],'l') );
	case L'ろ':		case L'ロ':
		return ( checkCase(word[0],'r') || checkCase(word[0],'l') );
	case L'わ':		case L'ワ':
		return ( checkCase(word[0],'w') || checkCase(word[0],'o') );
	case L'を':		case L'ヲ':
		return ( checkCase(word[0],'w') );
	case L'ん':		case L'ン':
		return ( checkCase(word[0],'n') );
	case L'が':		case L'ガ':
		return ( checkCase(word[0],'g') );
	case L'ぎ':		case L'ギ':
		return ( checkCase(word[0],'g') );
	case L'ぐ':		case L'グ':
		return ( checkCase(word[0],'g') );
	case L'げ':		case L'ゲ':
		return ( checkCase(word[0],'g') );
	case L'ご':		case L'ゴ':
		return ( checkCase(word[0],'g') );
	case L'ざ':		case L'ザ':
		return ( checkCase(word[0],'t')||checkCase(word[0],'z')||checkCase(word[0],'x')||checkCase(word[0],'s') );
	case L'じ':		case L'ジ':
		return ( checkCase(word[0],'g')||checkCase(word[0],'j')||checkCase(word[0],'z')||checkCase(word[0],'t') );
	case L'ず':		case L'ズ':
		return ( checkCase(word[0],'z') );
	case L'ぜ':		case L'ゼ':
		return ( checkCase(word[0],'z') || checkCase(word[0],'j')|| checkCase(word[0],'x') );
	case L'ぞ':		case L'ゾ':
		return ( checkCase(word[0],'z') );
	case L'だ':		case L'ダ':
		return ( checkCase(word[0],'d') );
	case L'ぢ':		case L'ヂ':
		return ( checkCase(word[0],'d') || checkCase(word[0],'j') );
	case L'づ':		case L'ヅ':
		return ( checkCase(word[0],'z')  );
	case L'で':		case L'デ':
		return ( checkCase(word[0],'d')  );
	case L'ど':		case L'ド':
		return ( checkCase(word[0],'d')  );
	case L'ば':		case L'バ':
		return ( checkCase(word[0],'b')  );
	case L'び':		case L'ビ':
		return ( checkCase(word[0],'b') ||  checkCase(word[0],'v')  );
	case L'ぶ':		case L'ブ':
		return ( checkCase(word[0],'b') );
	case L'べ':		case L'ベ':
		return ( checkCase(word[0],'b') ||  checkCase(word[0],'v')  );
	case L'ぼ':		case L'ボ':
		return ( checkCase(word[0],'b')  );
	case L'ぱ':		case L'パ':
		return ( checkCase(word[0],'p')  );
	case L'ぴ':		case L'ピ':
		return ( checkCase(word[0],'p')  );
	case L'ぷ':		case L'プ':
		return ( checkCase(word[0],'p')  );
	case L'ぺ':		case L'ペ':
		return ( checkCase(word[0],'p')  );
	case L'ぽ':		case L'ポ':
		return ( checkCase(word[0],'p')  );
	case L'ヴ':
		return ( checkCase(word[0],'v')  );
	}
	return true;
}

static bool checkGobaku(const std::wstring& titleW,const std::wstring& searchW,const std::wstring& innerW,const std::wstring& word)
{
	//検索に使ったワードとご判定をけるため消す.
	std::wstring sss = XLWStringUtil::replace(innerW,searchW,L"　　　");

	int pos = sss.find(word);
	if (pos == std::wstring::npos )
	{
		return true;
	}

	//ヒットした場所から、前に英数とかっこがあれば戻します.
	for( ; pos > 0 ; pos --)
	{
		if ( XLWStringUtil::isAlphanum( sss[pos] ) )
		{
			continue;
		}
		else if ( XLWStringUtil::isSpace( sss[pos] ) )
		{
			continue;
		}
		else if ( sss[pos] == L'\n' )
		{
			break;
		}
		else if ( sss[pos] == L'|' || sss[pos] == L'-' || sss[pos] == L':' || sss[pos] == L'{' || sss[pos] == L'(' || sss[pos] == L'（'|| sss[pos] == L'「' )
		{
			continue;
		}
		else 
		{
			break;
		}
	}

	//ヒットしたワードの場所から、後ろにN文字以内に漢字が
	//「狩猟」（{{lang-en-short|hunting}}）
	int endpos = MAX( pos - 10 , 0);
	for(int i = pos ; i > endpos ; i-- )
	{
		if ( XLWStringUtil::isKanji( sss[i] ) )
		{
			return true;
		}
		else if (sss[pos] == L'\n')
		{
			break;
		}
		else if (sss[pos] == L'。')
		{
			break;
		}
	}

	if (! checkFirstChar(titleW,word) )
	{//最初の一文字比較.
		return true;
	}

	return false;
}

std::wstring parseEngWordForEn(const std::wstring& innerW,const std::wstring& start,const std::wstring& end,int maxLine = 1 )
{
	int pos = 0;
	int endpos = 0;
/*
	for(int i = 0 ; i < maxLine ; i ++ )
	{
		endpos = innerW.find(L"。",endpos);
		if (endpos == std::wstring::npos)
		{
			endpos = innerW.size();
			break;
		}
		endpos ++; //skip 。
	}
*/
	endpos = innerW.size();

	int hit = innerW.find(start,pos);
	if (hit == std::wstring::npos || hit > endpos)
	{
		return L"";
	}
	hit += start.size();
	
	int hyphenpos = innerW.find(L"|",hit);
	int closepos = innerW.find(end,hit);
	if (hyphenpos == std::wstring::npos)
	{
		if (closepos == std::wstring::npos)
		{
			return L"";
		}
	}
	else
	{
		if (closepos == std::wstring::npos)
		{
			closepos = hyphenpos;
		}
		else
		{
			if (closepos > hyphenpos)
			{
				closepos = hyphenpos;
			}
		}
	}

	if (closepos > endpos)
	{
		return L"";
	}
	
	std::wstring a = innerW.substr(hit , closepos - hit);
	a = cleaningYomi(a);
	return a;
}

static std::wstring EqAlphaString(const std::wstring& innerW)
{
	//先頭のスペースを飛ばす.
	int pos = 0;
	for( ; pos < (int)innerW.size() ; pos++)
	{
		if ( XLWStringUtil::isSpace( innerW[pos] ) )
		{
			continue;
		}
		else if ( innerW[pos] == L'=' ||innerW[pos] == L'[' ||innerW[pos] == L'(' || innerW[pos] == L'(' || innerW[pos] == L'「')
		{
			continue;
		}
		else
		{
			break;
		}
	}

	//英字とスペースが続く限りつづけます.
	int start = pos;
	for( ; pos < (int)innerW.size() ; pos++)
	{
		if ( XLWStringUtil::isAlpha( innerW[pos] ) || XLWStringUtil::isSpace( innerW[pos] ) )
		{
			continue;
		}
		else
		{
			break;
		}
	}

	if (pos - start - 1 <= 0)
	{
		return L"";
	}

	std::wstring r = std::wstring(innerW,start , pos - start - 1 );
	r = cleaningYomi(r);
	return r;
}


static std::wstring parseEngWordForLang(const std::wstring& titleW,const std::wstring& searchW,const std::wstring& innerW)
{
	std::wstring r;
	r = parseEngWordForEn(innerW,L"{{lang-en-short|",L"}}",1 );
	if (!r.empty())
	{
		if (! checkGobaku(titleW,searchW,innerW,r) )
		{
			return r;
		}
	}
	r = parseEngWordForEn(innerW,L"{{Lang-en-short|",L"}}",1 );
	if (!r.empty())
	{
		if (! checkGobaku(titleW,searchW,innerW,r) )
		{
			return r;
		}
	}

	r = parseEngWordForEn(innerW,L"[[:en:",L"]]",1 );
	if (!r.empty())
	{
		if (! checkGobaku(titleW,searchW,innerW,r) )
		{
			return r;
		}
	}
	r = parseEngWordForEn(innerW,L"{{Lang-en|",L"}}",1 );
	if (!r.empty())
	{
		if (! checkGobaku(titleW,searchW,innerW,r) )
		{
			return r;
		}
	}
	r = parseEngWordForEn(innerW,L"{{lang-en|",L"}}",1 );
	if (!r.empty())
	{
		if (! checkGobaku(titleW,searchW,innerW,r) )
		{
			return r;
		}
	}
	r = parseEngWordForEn(innerW,L"{{Lang|en|",L"}}",1 );
	if (!r.empty())
	{
		if (! checkGobaku(titleW,searchW,innerW,r) )
		{
			return r;
		}
	}
	r = parseEngWordForEn(innerW,L"{{lang|en|",L"}}",1 );
	if (!r.empty())
	{
		if (! checkGobaku(titleW,searchW,innerW,r) )
		{
			return r;
		}
	}
	r = parseEngWordForEn(innerW,L"{{en|",L"}}",1 );
	if (!r.empty())
	{
		if (! checkGobaku(titleW,searchW,innerW,r) )
		{
			return r;
		}
	}
	r = parseEngWordForEn(innerW,L"{{EN|",L"}}",1 );
	if (!r.empty())
	{
		if (! checkGobaku(titleW,searchW,innerW,r) )
		{
			return r;
		}
	}

	r = parseEngWordForEn(innerW,L"（[[英語]]：",L"）",1 );
	if (!r.empty())
	{
		if (! checkGobaku(titleW,searchW,XLWStringUtil::replace(innerW,L"[[英語]]：",L"") ,r) )
		{
			return r;
		}
	}
	
	r = parseEngWordForEn(innerW,L"（[[英語]]:",L"）",1 );
	if (!r.empty())
	{
		if (! checkGobaku(titleW,searchW,XLWStringUtil::replace(innerW,L"[[英語]]:",L"") ,r) )
		{
			return r;
		}
	}

	r = parseEngWordForEn(innerW,L"（英語：",L"）",1 );
	if (!r.empty())
	{
		if (! checkGobaku(titleW,searchW,XLWStringUtil::replace(innerW,L"英語：",L"") ,r) )
		{
			return r;
		}
	}
	
	r = parseEngWordForEn(innerW,L"（英語:",L"）",1 );
	if (!r.empty())
	{
		if (! checkGobaku(titleW,searchW,XLWStringUtil::replace(innerW,L"英語:",L"") ,r) )
		{
			return r;
		}
	}

	r = parseEngWordForEn(innerW,L"（英：",L"）",1 );
	if (!r.empty())
	{
		if (! checkGobaku(titleW,searchW,XLWStringUtil::replace(innerW,L"英：",L"") ,r) )
		{
			return r;
		}
	}
	
	r = parseEngWordForEn(innerW,L"（英:",L"）",1 );
	if (!r.empty())
	{
		if (! checkGobaku(titleW,searchW,XLWStringUtil::replace(innerW,L"英:",L"") ,r) )
		{
			return r;
		}
	}

	//そろそろ wikipedianはフォーマットの統一ということを覚えたらいいんじゃないの？

	r = parseEngWordForEn(innerW,L"（[[英語|英]]：",L"）",1 );
	if (!r.empty())
	{
		if (! checkGobaku(titleW,searchW,XLWStringUtil::replace(innerW,L"（[[英語|英]]：",L"") ,r) )
		{
			return r;
		}
	}

	r = parseEngWordForEn(innerW,L"（[[英語|英]]:",L"）",1 );
	if (!r.empty())
	{
		if (! checkGobaku(titleW,searchW,XLWStringUtil::replace(innerW,L"（[[英語|英]]:",L"") ,r) )
		{
			return r;
		}
	}

	r = parseEngWordForEn(innerW,L"（[[w:",L"）",1 );
	if (!r.empty())
	{
		if (! checkGobaku(titleW,searchW,XLWStringUtil::replace(innerW,L"（[[w:",L"") ,r) )
		{
			return r;
		}
	}

	r = parseEngWordForEn(innerW,L"（\"",L"\"）",1 );
	if (!r.empty())
	{
		if (! checkGobaku(titleW,searchW,innerW ,r) )
		{
			return r;
		}
	}
	r = parseEngWordForEn(innerW,L"（&quot;",L"&quot;",1 );
	if (!r.empty())
	{
		if (! checkGobaku(titleW,searchW,innerW ,r) )
		{
			return r;
		}
	}

	r = EqAlphaString(innerW);
	if (!r.empty())
	{
		if (! checkGobaku(titleW,searchW,innerW,r) )
		{
			return r;
		}
	}

	return L"";
}


static std::wstring parseEngWordForText(const std::wstring& titleW,const std::wstring& searchW,const std::wstring& innerW)
{
	int pos = 0;
	pos = innerW.find(searchW,pos);
	if (pos == std::wstring::npos)
	{
		return L"";
	}
	pos += searchW.size();

	if (innerW[pos] == L'』'|| innerW[pos] == L'）'|| innerW[pos] == L')')
	{
		pos ++;
	}
	else if (innerW[pos] == L']' && innerW[pos+1] == L']')
	{
		pos+=2;
	}
	else if ( XLWStringUtil::isSpace(innerW[pos] ) )
	{
		pos++;
	}

	if (innerW[pos] == L'は')
	{
		pos ++;
	}
	else if (innerW[pos] == L'の')
	{
		pos ++;
	}
	else if (innerW[pos] == L'で' && innerW[pos+1] == L'あ' && innerW[pos+2] == L'り')
	{
		pos +=3;
	}
	else if ( XLWStringUtil::isSpace(innerW[pos] ) )
	{
		pos++;
	}

	if (innerW[pos] == L'、' || innerW[pos] == L',' || innerW[pos] == L':' || innerW[pos] == L'：' )
	{
		pos ++;
	}
	else if ( XLWStringUtil::isSpace(innerW[pos] ) )
	{
		pos++;
	}

	std::wstring r;
	if (innerW[pos] == L'（')
	{
		int endpos = innerW.find(L'）',pos+1);
		if (endpos == std::wstring::npos) endpos = innerW.size();

		r = innerW.substr(pos+1,endpos-(pos+1) );
		r = cleaningYomi(r);
	}
	else if (innerW[pos] == L'「')
	{
		int endpos = innerW.find(L'」',pos+1);
		if (endpos == std::wstring::npos) endpos = innerW.size();

		r = innerW.substr(pos+1,endpos-(pos+1) );
		r = cleaningYomi(r);
	}
	else if (innerW[pos] == L'(')
	{
		int endpos = innerW.find(L')',pos+1);
		if (endpos == std::wstring::npos) endpos = innerW.size();

		r = innerW.substr(pos+1,endpos-(pos+1) );
		r = cleaningYomi(r);
	}
	else if (innerW[pos] == L'\'' && innerW[pos+1] == L'\'' && innerW[pos+2] == L'\'' )
	{
		int endpos = innerW.find(L"'''",pos+3);
		if (endpos == std::wstring::npos) endpos = innerW.size();

		r = innerW.substr(pos+3,endpos-(pos+3) );
		r = cleaningYomi(r);
	}
	else if (innerW[pos] == L'\'' && innerW[pos+1] == L'\'' )
	{
		int endpos = innerW.find(L"''",pos+2);
		if (endpos == std::wstring::npos) endpos = innerW.size();

		r = innerW.substr(pos+2,endpos-(pos+2) );
		r = cleaningYomi(r);
	}

	if (!r.empty())
	{
		if ( checkGobaku(titleW,searchW,innerW,r) )
		{//誤爆しているのでクリアする
			r = L"";
		}
	}

	if (r.empty() )
	{
		std::wstring newInnerW = innerW.substr(pos);
		r = parseEngWordForLang(titleW,searchW, newInnerW );
		if (!r.empty() )
		{
			if ( checkGobaku(titleW,searchW,newInnerW,r) )
			{//誤爆しているのでクリアする
				r = L"";
			}
		}
	}


	return r;
}


std::wstring parseEngWordImpl(const std::wstring titleW, const std::wstring& searchW,const std::wstring& innerW)
{
	std::wstring r;
	r = parseEngWordForText(titleW,searchW,innerW);
	if (!r.empty())
	{
		return r;
	}
	r = parseEngWordForText(titleW,L"'''"+searchW+L"'''",innerW);
	if (!r.empty())
	{
		return r;
	}

	return L"";
}


static std::wstring makeInnerTextEx(const std::wstring& innerW)
{
	//ただし、あんまり下の方まで行くと登場人物などが書いてある場合があるので適当なところで切ります.
	int partCount = 0;
	int pos = 0;
	while(innerW[pos])
	{
		pos = innerW.find(L"\n==",pos);
		if (pos == std::wstring::npos)
		{
			break;
		}

		partCount++;
		//とりあえずN段落目で切ってみるか.
		if (partCount >= 2)
		{
			break;
		}
		pos += sizeof("\n==")-1;
	}
	std::wstring w = innerW.substr(0,pos);

	w = killWikiKako(w);
	w = cleaningInnerText(w);

	return w;
}


std::wstring parseEngWord(const std::wstring& titleW,const std::wstring& innerW)
{
	std::wstring r;

	r = parseEngWordImpl(titleW,titleW,innerW);
	if (!r.empty())
	{
		return r;
	}
	r = parseEngWordImpl(titleW,L"英語表記",innerW);
	if (!r.empty())
	{
		return r;
	}

	r = parseEngWordImpl(titleW,L"英語名",innerW);
	if (!r.empty())
	{
		return r;
	}

	r = parseEngWordImpl(titleW,L"英称",innerW);
	if (!r.empty())
	{
		return r;
	}

	r = parseEngWordImpl(titleW,L"英語圏の[[姓]]",innerW);
	if (!r.empty())
	{
		return r;
	}
	r = parseEngWordImpl(titleW,L"英語圏の[[名]]",innerW);
	if (!r.empty())
	{
		return r;
	}
	r = parseEngWordImpl(titleW,L"[[英語]]では",innerW);
	if (!r.empty())
	{
		return r;
	}
	r = parseEngWordImpl(titleW,L"英語では",innerW);
	if (!r.empty())
	{
		return r;
	}

	return L"";
}

static std::wstring findEng(const std::wstring& titleW,const std::wstring& innerW)
{
	std::wstring r,w;

	std::wstring innerTextExW = makeInnerTextEx(innerW);
	//和製英語を再優先で落とします。
	//本当の英名がほしいのではなく、日本人に通用する英語がほしいのです。
	r = parseEngWordImpl(titleW,L"和製英語",innerTextExW);
	if (!r.empty())
	{
		return r;
	}

	//ページの infoboxなどの箇条書き部から読みを取ってみます.
	std::wstring innerHeadW = makeInnerHead(innerW);
	r = parseEngWordImpl(titleW,L"英語名",innerHeadW);
	if (!r.empty())
	{
		return r;
	}
	r = parseEngWordImpl(titleW,L"英名",innerHeadW);
	if (!r.empty())
	{
		return r;
	}

	//ページ先頭から読みを取ります
	r = parseEngWord(titleW,innerTextExW);
	if (!r.empty())
	{
		return r;
	}

	//それでもダメなら、==語源== があれば試してみます。
	//コーヒーなどのオランダ語、ポルトガルト語などの江戸時代に入ってきたアレです.
	w = getWikiBlock(innerW,L"語源");
	if (!w.empty())
	{
//		XLFileUtil::write("_語源_"+_W2A(titleW)+".txt",_W2A(w));
		r = parseEngWord(titleW,w);
		if (!r.empty())
		{
			return r;
		}
	}

	//ページタイトル名で探してみます(最後の手段)
	r = parseEngWordForLang(titleW,titleW,innerTextExW);
	if (!r.empty())
	{
		return r;
	}
//	XLFileUtil::write("_無理_"+_W2A(titleW)+".txt",_W2A(w));

	//無理っぽ
	return L"";
}


static void AnalizeEng(const std::wstring& titleW,const std::wstring& innerW)
{
	if ( isRedirect(innerW) )
	{//リダイレクトは処理しない.
		return;
	}

	const std::wstring _TitleW = cleaningInnerText(titleW);
	//曖昧さの解決のための()があったら消します.
	const std::wstring clipTitleW = snipAimaiKako(titleW);
	if (clipTitleW.empty())
	{
		return;
	}

	std::wstring engwordW;
	std::wstring yomiW;
	
	if ( isKatakanaOnly(clipTitleW) )
	{//カタカナだけのページにしか興味はないね
		yomiW = convertPlainYomi(clipTitleW);
		engwordW = findEng(clipTitleW,innerW);
	}
	else if ( isAplhaOnly(clipTitleW) )
	{//英語だけのページにも興味ありました
		yomiW = convertPlainYomi(findYomi(clipTitleW,innerW));
		engwordW = clipTitleW;

		if ( checkGobaku(yomiW,L"",engwordW,engwordW) )
		{
			yomiW = L"";
		}
	}
	else
	{
		return ;
	}

	//結果の表示.

	if ( Option::m()->getShow() == Option::TypeShow_NoEmpty)
	{//空ならば表示しない?
		if (engwordW.empty() || yomiW.empty() )
		{
			return ;
		}
	}
	else if ( Option::m()->getShow() == Option::TypeShow_Empty)
	{//空だけ表示する
		if ( (!engwordW.empty() || !yomiW.empty() ) )
		{
			return;
		}
	}

	//カタカナひらがな変換.
	switch ( Option::m()->getCase())
	{
	case Option::TypeCase_Kana:	yomiW = XLWStringUtil::mb_convert_kana(yomiW,L"Hc"); break;
	case Option::TypeCase_Kata:	yomiW = XLWStringUtil::mb_convert_kana(yomiW,L"KC"); break;
	}

	//tofuに注意しながらMultiByteに修正します.
	if ( Option::m()->getAimai() == Option::TypeAimai_Del)
	{
		wprintf(L"%ls	%ls\n",yomiW.c_str(),engwordW.c_str() );
	}
	else
	{//消す消さない以前に処理的に存在しないかなあ
		wprintf(L"%ls	%ls\n",yomiW.c_str(),engwordW.c_str() );
	}
}

static void ReadAllForEng(FILE* fp)
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
					|| checkIchiran(titleW))
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
				AnalizeEng(titleW,innerW);
			}
		}
	}
}

void wiki2engConvert(const std::string& filename)
{
	
	FILE * fp = NULL;

	fp = fopen(filename.c_str() , "rb");
	if (!fp)
	{
		fwprintf(stderr,L"can not open %ls file\n" , _A2W(filename).c_str() );
		return ;
	}
	AutoClear ac([&](){ fclose(fp); });
	
	ReadAllForEng(fp);
}

SEXYTEST()
{
	


	{
		std::wstring r = findEng(L"コーヒー",L"\n{{otheruses}}\n[[ファイル:A small cup of coffee.JPG|thumb|right|コーヒー]]\n{{栄養価 | name=コーヒー（instant, regular, powder）| water =3.1 g| kJ =1008| protein =12.2 g| fat =0.5 g| carbs =41.1 g| fiber =0 g| sugars =0 g| calcium_mg =141| iron_mg =4.41| magnesium_mg =327| phosphorus_mg =303| potassium_mg =3535| sodium_mg =37| zinc_mg =0.35| manganese_mg =1.712| selenium_μg =12.6| vitC_mg =0| thiamin_mg =0.008| riboflavin_mg =0.074| niacin_mg =28.173| pantothenic_mg =0.097| vitB6_mg=0.029| folate_ug =0| choline_mg =101.9| vitB12_ug =0| vitA_ug =0| betacarotene_ug =0| lutein_ug =0| vitE_mg =0| vitD_iu =0| vitK_ug =1.9| satfat =0.197 g| monofat =0.041 g| polyfat =0.196 g| tryptophan =0.03 g| threonine =0.142 g| isoleucine =0.172 g| leucine =0.478 g| lysine =0.096 g| methionine =0.023 g| cystine =0.202 g| phenylalanine =0.262 g| tyrosine =0.165 g| valine =0.276 g| arginine =0.053 g| histidine =0.165 g| alanine =0.335 g| aspartic acid =0.478 g| glutamic acid =2.03 g| glycine =0.441 g| proline =0.351 g| serine =0.126 g| opt1n =[[カフェイン]]| opt1v =3142 mg| opt2n =[[テオブロミン]]| opt2v =0 mg| right=1 | source_usda=1 }}\n\n'''コーヒー'''（{{lang-nl|koffie}}<ref name=\"itohiroshi\"/> {{ipa|ˈkɔfi}} {{Audio|Nl-koffie.ogg|コフィ}}）は、[[コーヒー豆]]（[[コーヒーノキ]]の種子）を[[焙煎]]し挽いた粉末から、湯または[[水]]で成分を抽出した[[飲料]]。歴史への登場は[[アルコール]]や[[茶]]には遅れるが、世界で最も多くの国で飲用されている嗜好飲料であり、家庭や飲食店、職場などで飲用され、また、コーヒーの専門ショップも多数存在する。また、抽出前の粉末や粉砕前の焙煎豆も、同じくコーヒーと呼ばれることもある。日本語では「'''珈琲'''」と[[当て字]]されている<ref name=\"atejinoomoshirozatsugaku_p125\">フリーランス雑学ライダーズ編『あて字のおもしろ雑学』 p.125 1988年 永岡書店</ref>。\n\n世界各国において、コーヒーを提供する場の[[喫茶店]]（[[コーヒー・ハウス]]、[[カフェ]]、[[カフェー]]）は近代、知識人や文学、美術などさまざまな分野の芸術家の集まる場として、文化的にも大きな役割を果たしてきた。更に、[[石油]]に次いで貿易規模が大きい一次産品であるため、経済上も重要視されている。大体[[北回帰線]]と[[南回帰線]]の間（'''コーヒーベルト'''）の約70箇国で生産され、[[アメリカ合衆国|アメリカ]]、[[ヨーロッパ]]、[[日本]]など全世界に輸出されている。[[カフェイン]]に代表される薬理活性成分を含むことから[[医学]]・[[薬学]]の面から研究の対象となっている。\n\n== 歴史 =={{Main|コーヒーの歴史}}\nコーヒーがいつ頃から人間に利用されていたかは、はっきりしていない。[[果実]]の赤い[[果肉]]は甘く食べられるため、[[種子]]の効用を知る機会も多かったと考えれば、有史以前から野生種が利用されていても不思議ではない。実際、アラビカ種は原産地エチオピアで古くから利用されていたとする説があり、リベリカ種は[[西アフリカ]]沿岸でヨーロッパ人が「発見」する以前から[[栽培]]・利用されていた。栽培史概略は[[コーヒーノキ]]参照。\n\n== 語源 ==\n「コーヒー」は[[アラビア語]]でコーヒーを意味する'''[[カフワ・アラビーヤ|カフワ]]''' ({{lang-ar|قهوة}} ; qahwa) が転訛したものである。元々[[ワイン]]を意味していたカフワの語が、ワインに似た覚醒作用のあるコーヒーに充てられたのがその語源である。一説には[[エチオピア]]にあったコーヒーの産地'''カッファ''' (Kaffa) がアラビア語に取り入れられたものとも。\n\nこの語がコーヒーの伝播に伴って、[[トルコ]]（{{lang-tr|kahve}}）、[[イタリア]]（{{lang-it|caffè}}）を経由し、ヨーロッパ（{{lang-fr|café}}、{{lang-de|Kaffee}}、{{lang-en|coffee}}）から世界各地に広まった。日本語の「コーヒー」は、[[江戸時代]]に[[オランダ]]からもたらされた際の、{{lang-nl|koffie}} （コーフィー）に由来する<ref name=\"itohiroshi\">伊藤博『コーヒー事典』保育社、1994年、ISBN 978-4-586-50869-3</ref>。\n\n日本では漢字で「珈琲」のほか「可否」「架非」「加非」「咖啡」などの字もあてられてきた<ref name=\"atejinoomoshirozatsugaku_p125\"/>。\n\n漢字による当て字である「珈琲」は、[[津山藩]]医で[[蘭学者]][[宇田川榕菴]]（うだがわ ようあん）が考案し、蘭和対訳辞典で使用したのが、最初であると言われている。これ以外にも、「可否」（[[コーヒーの歴史|可否茶館]]）、「カウヒイ」（[[大田南畝]]『瓊浦又綴（けいほゆうてつ）』）、「哥非乙」（[[宇田川榕菴]]『哥非乙説』）<ref>[[奥山儀八郎]]「かうひい異名熟字一覧（木版画）」『珈琲遍歴』四季社、1957年</ref>などの表記も過去には用いられた。\n\nなお、[[中国語]]においても、訳語に関して19世紀に試行錯誤があり、現在はこの宇田川榕菴の当て字を借用している<ref>http://www.lingviko.net/feng/loanword-zwfeng.pdf</ref>。ただし、漢字は[[王偏]]を[[口偏]]に変え「{{lang|zh|咖啡}}」（{{unicode|kāfēi}}）と表記される。\n\n== コーヒーノキ ==\n{{Main2|詳細は[[コーヒーノキ]]を}}\n\nコーヒーの原料となるコーヒー豆は、3～3.5mほどの常緑低木で[[ジャスミン]]に似た香りの白い花を咲かせる'''[[コーヒーノキ]]'''の果実から得られる。\n");
		assert(r==L"coffee");
	}
	{
		std::wstring r = findEng(L"ミュージックステーション",L">{{otheruses|テレビ朝日で放送されている音楽番組}}\n{{基礎情報 テレビ番組\n|番組名 = ミュージックステーション<br />MUSIC STATION\n|画像 = [[ファイル:TV Asahi Headquarters 2010.jpg|250px]]\n|画像説明 =当番組の[[生放送]]が行われている<br />[[テレビ朝日]]本社第1[[スタジオ]]外観（[[六本木ヒルズ]]内）|ジャンル = [[音楽番組]]\n|放送時間 = [[金曜日]] 20:00 - 20:54\n|放送分 = 54\n|放送期間 = [[1986年]][[10月24日]] - 現在\n|放送回数 = 1000\n|放送国 = {{JPN}}\n|制作局 = [[テレビ朝日]]\n|企画 =\n|製作総指揮 = \n|監修 =\n|演出 =\n|原作 =\n|脚本 =\n|プロデューサー = 荒井祥之 / 栗井淳（GP）<br />[[山本たかお]]（[[エグゼクティブプロデューサー|EP]]）\n|出演者 = [[タモリ]]<br />[[弘中綾香]]<small>（[[テレビ朝日]][[アナウンサー]]）</small><br /><small>ほか ゲストアーティスト数組</small>\n|ナレーター = [[ウォード・セクストン]]、[[服部潤]]<br />[[ユキ・ラインハート]]\n|音声 = [[ステレオ放送]]<br />（[[1986年]][[10月24日]] - ）\n|字幕 = [[リアルタイム字幕放送]]<br />（[[2013年]][[10月18日]] - ）\n|データ放送 = [[データ放送|データ連動放送]]<br />（[[2010年]][[1月15日]] - ）\n|OPテーマ = [[松本孝弘]]「[[1090 縲弋housand Dreams縲忿# 1090 縲弋housand Dreams縲彎]」\n|EDテーマ = 松本孝弘「[[華 (アルバム)|# 1090[千夢一夜]]]」\n|外部リンク = http://www.tv-asahi.co.jp/music/\n|外部リンク名 = 公式サイト\n|特記事項 = ・放送回数は[[2010年]][[2月5日]]生放送分まで。<!--回数を追加したからと言って毎週更新するようなことはしないこと。3・4ヶ月に一度、編集で書き換える。上記の[[スタッフ]]は、[[2014年]][[2月14日]]生放送分以降を記載。-->\n|番組名1= 1986年10月から2000年3月まで【第1期】\n|放送時間1= [[金曜日]] 20:00 - 20:54\n|放送分1= 54\n|放送枠1=\n|放送期間1= [[1986年]][[10月24日]] - [[2000年]][[3月24日]]\n|放送回数1=\n|番組名2= 2000年4月から同年9月まで\n|放送時間2= 金曜日 19:54 - 20:48\n|放送分2= 54\n|放送枠2=\n|放送期間2= 2000年[[4月14日]] - 同年[[9月15日]]\n|放送回数2=\n|番組名3= 2000年10月から現在【第2期】\n|放送時間3= 金曜日 20:00 - 20:54\n|放送分3= 54\n|放送枠3=\n|放送期間3= 2000年[[10月6日]] - 現在\n|放送回数3= 1000\n}}\n『'''ミュージックステーション'''』（英称：''MUSIC STATION''）は、[[テレビ朝日]]（[[All-nippon News Network|ANN系列]]）で、[[1986年]]（[[昭和]]61年）[[10月24日]]から毎週[[金曜日]]の20:00 - 20:54（[[日本標準時|JST]]）に[[生放送]]されている[[音楽番組]]である。[[ステレオ放送]]、[[ハイビジョン]]制作<ref>[[2003年]]（平成15年）[[10月3日]]から。</ref>、[[番組連動データ放送]]<ref>[[2010年]]（平成22年）[[1月15日]]から。</ref>、[[リアルタイム字幕放送]]<ref>[[2013年]][[10月18日]]から。</ref>を実施している。通称は「'''Mステ'''（エムステ）」「'''MS'''（エムエス）」<ref>同局の[[報道番組]]『[[ニュースステーション]]』（Nステ）の通称'''NS'''にちなむ。『Nステ』放送終了後、この通称は消滅した模様。</ref>「'''Mステーション'''（エムステーション）」<ref>[[新聞]]の[[番組表|ラジオ・テレビ番組表（ラ・テ欄）]]では、この表記を使用している（特に[[Gコード]]普及以後）。</ref>など。\n\n本番組直前に生放送されている[[ミニ番組]]『'''[[#ミニステ|ミニステ]]'''』についてもこの項目で扱う。");
		assert(r==L"MUSIC STATION");
	}
	{
		std::wstring r = findEng(L"ヱビスビール",L"{{redirect|ヱビス|その他の用法|えびす (曖昧さ回避)}}\n[[File:YEBISU_BEER_6_item_cans.jpg|thumb|240px|『ヱビスビール』各種（2009年現在）]][[File:YEBISU CREAMY STOUT SAPPARO JAPAN JUNE 2012 (7456188018).jpg|thumb|240px|ヱビス・スタウト・クリーミートップ]][[File:Joelrobuchon-beer-japan.jpg|thumb|240px|薫り華やぐヱビス]]\n'''ヱビスビール'''（ゑ〈え〉びすびーる、恵比寿ビール、戎ビール、YEBISUとも）は、[[サッポロビール]]株式会社の[[麦芽]]100%の[[ビール]]のブランド名。[[プレミアムビール]]に分類される。");
		assert(r==L"");
	}
	{
		//windowsの MultiByteToWideCharを経由させると、スペイン語名の Division が Division となり、取得できてしまうが、本当はスペイン語のためできないのが正しい。
		std::wstring r = findEng(L"プリメーラ・ディビシオン",L"{{redirect|プリメーラ・ディビシオン}}\n{{Infobox football league\n| logo      = \n| pixels    = \n| country   = {{flagicon|ESP}} [[スペイン]]\n| confed    = [[UEFA]]\n| ranking   = 3\n| founded   = 1929\n| teams     = 20\n|relegation = [[セグンダ・ディビシオン]]\n| levels    = 1\n| domest_cup= [[コパ・デル・レイ]]\n| confed_cup= [[UEFAチャンピオンズリーグ]]<br>[[UEFAヨーロッパリーグ]]\n| champions = [[アトレティコ・マドリード]] (10回)\n| season    = 2013-14\n| most successful club = [[レアル・マドリード]] (32回)\n| sponsorship_name = リーガBBVA\n| tv       = [[Canal+ (Spanish satellite broadcasting company)|Canal+]], [[Gol Television|GolT]], [[laSexta]]<br>[[FORTA|autonomic channels]]<br />[[Cuatro (channel)|Cuatro]] and [[Television Espanola|TVE]] (highlights)\n| website  = [http://www.lfp.es/ www.lfp.es]\n| current  = 2014窶骭15\n}}\n{{座標一覧}}\n'''プリメーラ・ディビシオン'''（'''Primera Division''', 1部リーグ）は、[[スペイン]]の[[サッカー]]リーグ、[[リーガ・エスパニョーラ]]のトップディビジョンの名称である。\n[[2006年]]8月からスペインの[[ビルバオ・ビスカヤ・アルヘンタリア銀行]]（BBVA）により[[命名権]]が買い取られ、'''リーガBBVA'''（'''Liga BBVA''')と冠されている。\n\nリーグには、[[UEFAチャンピオンズリーグ]]歴代最多となる10度の優勝と32度（2013-14シーズンまで）のリーグ優勝を誇る[[レアル・マドリード]]、最大のライバルである[[FCバルセロナ|バルセロナ]]や[[アトレティコ・マドリード]]など、多くの強豪クラブがひしめき合っている。最近はレアル・マドリードとバルセロナとそれ以外のチームの勝ち点差が大きく開く2強体制となっており、リーグの放映権料の41%が2チームに渡るシステムの不均衡が格差の主な原因だとして、「G-6」と呼ばれる中堅クラブの反発を招いている<ref>{{cite web |title=バルサとレアルでリーガが破滅する!? 「2強18弱」の歪んだ経済バランス。 |publisher=Number Web |date=2010-08-10 |url=http://number.bunshun.jp/articles/-/152569 |accessdate=2012-03-25}}</ref><ref>{{cite web |title=セビージャ会長：「リーガは欧州ではなく世界のゴミ」 |publisher=Goal.com |date=2010-08-31 |url=http://www.goal.com/jp/news/73/%E3%82%B9%E3%83%9A%E3%82%A4%E3%83%B3/2011/08/31/2643410/%E3%82%BB%E3%83%93%E3%83%BC%E3%82%B8%E3%83%A3%E4%BC%9A%E9%95%B7%E3%83%AA%E3%83%BC%E3%82%AC%E3%81%AF%E6%AC%A7%E5%B7%9E%E3%81%A7%E3%81%AF%E3%81%AA%E3%81%8F%E4%B8%96%E7%95%8C%E3%81%AE%E3%82%B4%E3%83%9F |accessdate=2012-03-25}}</ref>。");
//		assert(r==L"");
	}
	{
		std::wstring r = findEng(L"オホーツク",L"{{Coor title dm|59|22|N|143|15|E|}}\n{{Otheruses|[[ロシア]]の[[ハバロフスク地方]]に所在する都市|「オホーツク」のその他の用法}}\n[[画像:Coat of Arms of Okhotsk (Khabarovsk krai) (1790).png|thumb|150px|オホーツクの紋章]]\n'''オホーツク'''（[[ロシア語]]:''{{lang|ru|Охоﾌ＞тヤ{}}''、<small>アホーツク</small>）は、[[オホーツク海]]に面した[[ロシア]]の[[ハバロフスク地方]]に属する町（[[都市型集落]]）である。人口は[[1989年ソ連国勢調査|1989年国勢調査]]で9,298人、[[2002年全ロシア国勢調査|2002年国勢調査]]で5,738人、[[2004年]]の推計では5,500人。\n== 名称 ==\n[[オホータ川]]や[[オホーツク海]]の名称の由来である \"{{lang|ru|Охота}}\"（{{lang|en|Okhota}}）とは、「狩猟」（{{lang-en-short|hunting}}）という意味である。");
		assert(r==L"Okhota");
	}
	{
		std::wstring r = findEng(L"イエスタデイ",L"「'''イエスタデイ''' 」 （\"Yesterday\"）は[[イギリス]]の[[ロック (音楽)|ロック]]・[[バンド (音楽)|バンド]]、[[ビートルズ]]の楽曲である。");
		assert(r==L"Yesterday");
	}
	{
		std::wstring r = findEng(L"フロッピー",L"'''フロッピー'''（{{lang-en-short|FLOPPY}}）、柔らかいの意味。\n\n* [[フロッピーディスク]]の略。\n* [[FLOPPY (ユニット)]] - 日本の[[テクノポップ]]ユニット。\n* [[:en:The Floppy Show|The Floppy Show]] - [[アメリカ合衆国|アメリカ]]の子供用テレビ番組。\n{{Aimai}}\n{{デフォルトソート:ふろつひい}}");
		assert(r==L"FLOPPY");
	}
	{
		std::wstring r = findEng(L"ゲーム",L"{{Otheruses}}\n\n'''ゲーム'''（{{Lang-en-short|[[w:Game|Game]]}}）とは、[[遊び]]や遊戯と訳される。勝敗を決めるなどの[[規則|ルール]]や[[環境]]または他人との[[相互作用]]を元にした[[普通]]楽しみのために行なわれる活動である。");
		assert(r==L"Game");
	}
	{
		std::wstring r = findEng(L"ペプシコーラ",L"{{Otheruses|ペプシコ社の製品であるペプシコーラ|この製品を製造している食品・飲料会社|ペプシコ}}\n{{infobox 飲料\n| name         = ペプシコーラ\n| type         = [[コーラ (飲料)|コーラ]]\n| image =[[File:Pepsi logo 2008.svg|200px]]\n| industry = 飲料\n| manufacturer = [[ペプシコ]]\n| origin       = {{USA}}\n| introduced   = 1893年\n| related      = [[コカ・コーラ]]<br />[[7 Up]]<br />他多数\n}}\n'''ペプシコーラ'''（[[英語]]：Pepsi-Cola）は、[[アメリカ合衆国]][[ニューヨーク州]]に本社を置く[[ペプシコ]]社の所有により、全世界で展開されている[[ソフトドリンク]]ブランドである。現在は同社の菓子ブランドである[[フリトレー]]と合併しているが、以前は社名でもあった。\n\n== 概要 ==\nアメリカの大手飲料メーカーであるペプシコ・インコーポレイテッド（PepsiCo, Inc,[[ニューヨーク証券取引所|NYSE]]:[http://www.nyse.com/about/listed/lcddata.html?ticker=PEP PEP]）が製造・販売を担当している。[[コーラ飲料|コーラ]]の販売を主としていて、[[日本]]でもペプシコーラやダイエットペプシの名で、様々な種類のコーラを販売している。\n\n[[コカ・コーラ]]や[[ロイヤルクラウン・コーラ|RC]]などの、ライバル企業もある中、ペプシコーラは独自の味を売りとして、大きな売り上げを果たしている。");
		assert(r==L"Pepsi-Cola");
	}
	{
		std::wstring r = findEng(L"オカルト",L"{{Otheruses|オカルトという言葉|オカルトという名前で活動していたオランダのブラックメタルバンド|リージョン・オブ・ザ・ダムド}}\n{{複数の問題\n|参照方法=2014年8月\n|独自研究=2014年8月\n}}\n'''オカルト'''（occult）は、");
		assert(r==L"occult");
	}
	{
		std::wstring r = findEng(L"インサイダー",L"'''インサイダー'''（insider）");
		assert(r==L"insider");
	}
	{
		std::wstring r = findEng(L"トマト",L"{{Otheruses}}\n{{生物分類表\n|名称 = トマト\n|色 = lightgreen\n|画像=[[ファイル:Tomatoes-on-the-bush.jpg|250px]]\n|界 = [[植物界]] [[:en:Plantae|Plantae]]\n|門 = [[被子植物門]] [[:en:Magnoliophyta|Magnoliophyta]]\n|綱 = [[双子葉植物綱]] [[:en:Magnoliopsida|Magnoliopsida]]\n|目 = [[ナス目]] [[:en:Solanales|Solanales]]\n|科 = [[ナス科]] [[:en:Solanaceae|Solanaceae]]\n|属 = [[ナス属]] ''[[:en:Solanum|Solanum]]''\n|種 = '''トマト''' ''S. lycopersicum''\n|学名 = ''Solanum lycopersicum''<br/>L., [[1753年|1753]]\n|和名 = トマト<br/>赤茄子<br/>蕃茄<br/>小金瓜\n|英名 = [[:en:Tomato|tomato]]\n}}\n\n[[ファイル:Fleurtomate.jpg|thumb|right|240px|トマトの花]]\n[[ファイル:ミニトマトの実の付け方.JPG|thumb|right|240px|ミニトマトは一度に10個以上実をつけることも珍しくない。地植えにして支柱を立ててやると1本のトマトから100個以上は収穫できる]]\n[[ファイル:Tomatoes_plain_and_sliced.jpg|thumb|right|240px|トマトの実。縦断面と横断面]]\n[[ファイル:Tomaten im Supermarktregal.jpg|thumb|right|240px|ミニトマト(またはプチトマト)。付け合せやお弁当用のトマトとして日本でも広く普及している。]]\n\n{{栄養価 | name=トマト（red, ripe, raw, year round average）| water =94.52 g| kJ =74| protein =0.88 g| fat =0.2 g| carbs =3.89 g| fiber =1.2 g| sugars =2.63 g| calcium_mg =10| iron_mg =0.27| magnesium_mg =11| phosphorus_mg =24| potassium_mg =237| sodium_mg =5| zinc_mg =0.17| manganese_mg =0.114| selenium_μg =0| vitC_mg =13.7| thiamin_mg =0.037| riboflavin_mg =0.019| niacin_mg =0.594| pantothenic_mg =0.089| vitB6_mg=0.08| folate_ug =15| choline_mg =6.7| vitB12_ug =0| vitA_ug =42| betacarotene_ug =449| lutein_ug =123| vitE_mg =0.54| vitD_iu =0| vitK_ug =7.9| satfat =0.028 g| monofat =0.031 g| polyfat =0.083 g| tryptophan =0.006 g| threonine =0.027 g| isoleucine =0.018 g| leucine =0.025 g| lysine =0.027 g| methionine =0.006 g| cystine =0.009 g| phenylalanine =0.027 g| tyrosine =0.014 g| valine =0.018 g| arginine =0.021 g| histidine =0.014 g| alanine =0.027 g| aspartic acid =0.135 g| glutamic acid =0.431 g| glycine =0.019 g| proline =0.015 g| serine =0.026 g| right=1 | source_usda=1 }}\n\n'''トマト'''（学名：''Solanum lycopersicum''）は、[[南アメリカ]]の[[アンデス山脈]]高原地帯（[[ペルー]]、[[エクアドル]]）原産の[[ナス科]][[ナス属]]の[[植物]]。また、その[[果実]]のこと。[[多年生植物]]で、果実は食用として利用される。[[緑黄色野菜|緑黄色野菜の一種]]である。日本語では唐柿（とうし）、赤茄子（あかなす）、蕃茄（ばんか）、小金瓜（こがねうり）などの異称もある。\n");
		assert(r==L"Tomato");
	}
	{
		std::wstring r = findEng(L"エミュレータ",L"{{Otheruseslist|エミュレータの定義|コンピュータなどを模擬的に動かす行為|エミュレータ (コンピュータ)|特にゲーム|ゲームエミュレータ}}\n\n'''エミュレータ''' ('''Emulator''')とは、[[コンピュータ]]や[[機械]]の模倣装置あるいは模倣[[ソフトウェア]]のことである。");
		assert(r==L"Emulator");
	}
	{
		std::wstring r = findEng(L"レッドハット",L"'''レッドハット'''('''Red Hat''')は、2012年現在最大手の、[[Linux]][[Linuxディストリビューション|ディストリビューション]]を製品として配付・サポートしている会社である。主力事業・製品は1.LINUX OS\n([[Red Hat Enterprise Linux]]) 2. [[サービス指向アーキテクチャ|SOA]]/[[MIDDLEWARE]]([[JBoss Enterprise Middleware]]) 3. [[仮想化]]([[Kernel-based Virtual Machine|KVM]]/[[Red Hat Virtualization]]) 4.[[クラウドコンピューティング|クラウド]]関連Software ([[IaaS]]: [[CloudForms]], [[PaaS]]: [[Open Shift]]) 5.ストレージ([[Gluster]]:\n[[スケールアウト型]][[非構造型]][[ストレージ管理]]ソフト)など。近年世界や日本のトップCloud Service Provider\n([[Amazon.com|アマゾン]]、[[IBM]]、[[富士通]]、[[NTT]]、[[ソフトバンク]]、[[テレストラ]]、等ら）とPartnershipを結んでおり\n急速にオープン[[クラウドコンピューティング|クラウド]]・[[仮想化]]事業を伸ばしている。\n");
		assert(r==L"Red Hat");
	}
	{
		std::wstring r = findEng(L"ソースコード",L"'''ソースコード'''（Source code、'''ソースプログラム'''、'''原始プログラム'''）とは、人間が記述した、[[ソフトウェア]]（[[プログラム (コンピュータ)|コンピュータプログラム]]）の元となる一連の文字の羅列である。&lt;!--HDLのようなハードウェアを記述する言語の場合は「ソースコード」とは呼ばないのだろうか？--&gt;");
		assert(r==L"Source code");
	}
	{
		std::wstring r = findEng(L"イラストレーター"
					,L"{{Otheruses||ソフトウェア|Adobe Illustrator}}\n[[Image:Ciruelo2007.jpg|thumb|right|200px|[[フランクフルト・ブックフェア]]で[[イラストレーション|イラスト]]を描くイラストレーター（[[シルエロ・カブラル]]）]]\n'''イラストレーター'''（{{lang-en-short|Illustrator}}）とは、[[情報]]や概念の[[可視化|視覚化]]、[[図解]]、娯楽化など、何らかの[[コミュニケーション]]を主目的とした絵＝'''[[イラストレーション]]'''（イラスト）を描くことを[[職業|生業]]としている人のこと。[[挿絵]]、[[表紙]]、[[絵本]]、[[広告]]、[[パッケージデザイン|パッケージ]]、[[ポスター]]などを主な活動範囲とする。&lt;!-- 詳細に定義すると[[イラストレーション]]とかぶるため端折ります --&gt;'''絵師'''と呼ぶこともある&lt;ref&gt;{{cite news |title=ポップカルチャーの“聖地”、大阪・日本橋で「絵師１００人展０４」開幕 |newspaper=[[産経新聞]] |date=2014-8-19 |url=http://sankei.jp.msn.com/west/west_life/news/140819/wlf14081922100018-n1.htm |accessdate=2014-8-19 }}&lt;/ref&gt;。	"
					);
		assert(r==L"Illustrator");
	}
	{
		std::wstring r = findEng(L"ソフトウェア",L"{{WikipediaPage|ウィキペディアの閲覧・編集に利用されるソフトウェアについては、[[Wikipedia:ツール]]をご覧ください。}}\n[[Image:Operating system placement.svg|thumb|ソフトウェアの階層図。上から[[ユーザー]]（人間）、[[アプリケーションソフトウェア|アプリケーション]]、[[オペレーティングシステム]]、[[ハードウェア]]。]]\n'''ソフトウェア'''（{{lang-en-short|software}}）は、[[コンピューター]]システム上で何らかの処理を行う[[プログラム (コンピュータ)|プログラム]]や[[手続き]]、およびそれらに関する[[ソフトウェアドキュメンテーション|文書]]を指す言葉である&lt;ref&gt;{{cite web| title = Wordreference.com: WordNetR 2.0| publisher = Princeton University| url = http://www.wordreference.com/definition/software | accessdate =  2007年8月19日 }}&lt;/ref&gt;。日本語では略して「ソフト」ともいう。");
		assert(r==L"software");
	}
}