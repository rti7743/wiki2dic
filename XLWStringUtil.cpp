#include "common.h"
#include "XLWStringUtil.h"


/*
　 ,j;;;;;j,. ---一､ ｀ 　―--‐､_ l;;;;;; 
　｛;;;;;;ゝ T辷iﾌ i 　 　f'辷jァ　 !i;;;;;　 
　 ヾ;;;ﾊ　　　　ﾉ　　　　　　　.::!lﾘ;;rﾞ　　　文字コードは UTF16だ WCHARだ。
　　 `Z;i　　　〈.,_..,.　　　　　　ﾉ;;;;;;;;>　　 　　 
　 　,;ぇﾊ、　､_,.ｰ-､_',. 　 　,ｆﾞ: Y;;f 　　　　そんなふうに考えていた時期が 
　　 ～''戈ヽ 　 ｀二´ 　　 r'´:::.　`!　　　　俺にもありました 

*/

//みんな大好きPHPのstrtoupper
std::wstring XLWStringUtil::strtoupper(const std::wstring & str)
{
	if (str.empty())
	{
		return str;
	}
	std::wstring r = str;
	
	WCHAR * p = &r[0];
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
		std::wstring a = XLWStringUtil::strtoupper(L"aBcDefg");
		std::wstring b = L"ABCDEFG";
		SEXYTEST_EQ(a ,b); 
	}
}


//みんな大好きPHPのstrtolower
std::wstring XLWStringUtil::strtolower(const std::wstring & str)
{
	if (str.empty())
	{
		return str;
	}
	std::wstring r = str;
	
	WCHAR * p = &r[0];
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


//strstrのマルチバイトセーフ 文字列検索
const WCHAR* XLWStringUtil::strstr(const WCHAR* target, const WCHAR* need )
{
	return ::wcsstr (target ,need );
}

//strstrのマルチバイトセーフ 文字列検索 //若干非効率
const WCHAR* XLWStringUtil::strrstr(const std::wstring& target, const std::wstring & need )
{
	const int size = need.size();
	const WCHAR* p = target.c_str();
	const WCHAR* lastmatch = NULL;
	while(p = XLWStringUtil::strstr(p,need.c_str()) )
	{
		lastmatch = p;
		p += size;
	}
	
	return lastmatch;
}

//strstrのマルチバイトセーフ 文字列検索 //若干非効率
const WCHAR* XLWStringUtil::strrstr(const WCHAR* target, const WCHAR* need )
{
	const int size = wcslen(need);
	const WCHAR* p = target;
	const WCHAR* lastmatch = NULL;
	while(p = XLWStringUtil::strstr(p,need))
	{
		lastmatch = p;
		p += size;
	}
	
	return lastmatch;
}
//なぜか標準にない /^foo/ みたいに前方マッチ.
const WCHAR* XLWStringUtil::strfirst(const WCHAR* s,const WCHAR* n)
{
	const WCHAR* p = XLWStringUtil::strstr(s,n);
	if (p == NULL) return NULL;
	
	if ( p != s ) return NULL;
	return p;
}

//なぜか標準にない /foo$/ みたいに後方マッチ.
const WCHAR* XLWStringUtil::strend(const WCHAR* s,const WCHAR* n)
{
	const WCHAR* p = XLWStringUtil::strrstr(s,n);
	if (p == NULL) return NULL;

	if ( wcslen(p) != wcslen(n) )
	{
		return NULL;
	}
	return p;
}


//マルチバイト対応 複数の候補を一括置換 const char * replacetable[] = { "A","あ"  ,"I","い"  , "上","うえ" , NULL , NULL}  //必ず2つ揃えで書いてね
std::wstring XLWStringUtil::replace(const std::wstring &inTarget ,const WCHAR** replacetable,bool isrev)
{
	std::wstring ret;
	ret.reserve( inTarget.size() );	//先読み.

	if (inTarget.empty())
	{
		return inTarget;
	}

	const WCHAR * p = inTarget.c_str();
	for(; *p ; )
	{
		const WCHAR * pp = p;
		p = nextChar(p);

		int compareIndex = isrev == false ? 0 : 1;
		int replaceIndex = isrev == false ? 1 : 0;
		const WCHAR ** r1 = replacetable;
		for( ; *r1 != NULL ; r1+=2)
		{
			const WCHAR * ppp = pp;
			const WCHAR * r2 = *(r1+compareIndex);
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
std::wstring XLWStringUtil::mb_convert_kana(const std::wstring &inTarget,const std::wstring& option)
{
	std::wstring ret = inTarget;
	static const WCHAR *replaceTableAplha[] = {
		 L"Ａ",L"A"
		,L"Ｂ",L"B"
		,L"Ｃ",L"C"
		,L"Ｄ",L"D"
		,L"Ｅ",L"E"
		,L"Ｆ",L"F"
		,L"Ｇ",L"G"
		,L"Ｈ",L"H"
		,L"Ｉ",L"I"
		,L"Ｊ",L"J"
		,L"Ｋ",L"K"
		,L"Ｌ",L"L"
		,L"Ｍ",L"M"
		,L"Ｎ",L"N"
		,L"Ｏ",L"O"
		,L"Ｐ",L"P"
		,L"Ｑ",L"Q"
		,L"Ｒ",L"R"
		,L"Ｓ",L"S"
		,L"Ｔ",L"T"
		,L"Ｕ",L"U"
		,L"Ｖ",L"V"
		,L"Ｗ",L"W"
		,L"Ｘ",L"X"
		,L"Ｙ",L"Y"
		,L"Ｚ",L"Z"
		,L"ａ",L"a"
		,L"ｂ",L"b"
		,L"ｃ",L"c"
		,L"ｄ",L"d"
		,L"ｅ",L"e"
		,L"ｆ",L"f"
		,L"ｇ",L"g"
		,L"ｈ",L"h"
		,L"ｉ",L"i"
		,L"ｊ",L"j"
		,L"ｋ",L"k"
		,L"ｌ",L"l"
		,L"ｍ",L"m"
		,L"ｎ",L"n"
		,L"ｏ",L"o"
		,L"ｐ",L"p"
		,L"ｑ",L"q"
		,L"ｒ",L"r"
		,L"ｓ",L"s"
		,L"ｔ",L"t"
		,L"ｕ",L"u"
		,L"ｖ",L"v"
		,L"ｗ",L"w"
		,L"ｘ",L"x"
		,L"ｙ",L"y"
		,L"ｚ",L"z"
		,L"ｰ",L"ー"
		,L"‘",L"'"
		,L"’",L"'"
		,L"“",L"\""
		,L"”",L"\""
		,L"（",L"("
		,L"）",L")"
		,L"〔",L"["
		,L"〕",L"]"
		,L"［",L"["
		,L"］",L"]"
		,L"｛",L"{"
		,L"｝",L"}"
		,L"〈",L"<"
		,L"〉",L">"
		,L"《",L"<"
		,L"》",L">"
		,L"「",L"{"
		,L"」",L"}"
		,L"『",L"{"
		,L"』",L"}"
		,L"【",L"["
		,L"】",L"]"
		,L"・",L"･"
		,L"！",L"!"
		,L"♯",L"#"
		,L"＆",L"&"
		,L"＄",L"$"
		,L"？",L"?"
		,L"：",L":"
		,L"；",L";"
		,L"／",L"/"
		,L"＼",L"\\"
		,L"＠",L"@"
		,L"｜",L"|"
		,L"－",L"-"
		,L"＝",L"="
		,L"≒",L"="
		,L"％",L"%"
		,L"＋",L"+"
		,L"－",L"-"
		,L"÷",L"/"
		,L"＊",L"*"
		,L"～",L"~" //UTF-8だと別の～もあるから判断が難しい・・・
		,NULL,NULL
	};
//r	 「全角」英字を「半角」に変換します。
//R	 「半角」英字を「全角」に変換します。
//a	 「全角」英数字を「半角」に変換します。
//A	 「半角」英数字を「全角」に変換します 
	if ( option.find(L"r") != -1 ||   option.find(L"a") != -1 )
	{
		ret = XLWStringUtil::replace(ret ,replaceTableAplha,false );
	}
	else if ( option.find(L"R") != -1 ||  option.find(L"A") != -1 )
	{
		ret = XLWStringUtil::replace(ret ,replaceTableAplha,true );
	}

	static const WCHAR *replaceTableNum[] = {
		 L"１",L"1"
		,L"２",L"2"
		,L"３",L"3"
		,L"４",L"4"
		,L"５",L"5"
		,L"６",L"6"
		,L"７",L"7"
		,L"８",L"8"
		,L"９",L"9"
		,L"０",L"0"
		,NULL,NULL
	};
//n	 「全角」数字を「半角」に変換します。
//N	 「半角」数字を「全角」に変換します。
//a	 「全角」英数字を「半角」に変換します。
//A	 「半角」英数字を「全角」に変換します 
	if ( option.find(L"n") != -1 ||  option.find(L"a") != -1 )
	{
		ret = XLWStringUtil::replace(ret ,replaceTableNum,false );
	}
	else if ( option.find(L"N") != -1 ||  option.find(L"A") != -1)
	{
		ret = XLWStringUtil::replace(ret ,replaceTableNum,true );
	}

	static const WCHAR *replaceTableSpace[] = {
		 L"　",L" "
		,NULL,NULL
	};
//s	 「全角」スペースを「半角」に変換します
//S	 「半角」スペースを「全角」に変換します
	if ( option.find(L"s") != -1 )
	{
		ret = XLWStringUtil::replace(ret ,replaceTableSpace,false );
	}
	else if ( option.find(L"S") != -1)
	{
		ret = XLWStringUtil::replace(ret ,replaceTableSpace,true );
	}

	static const WCHAR *replaceTableHankanaToHiragana[] = {
		 L"ｳﾞ",L"う゛"
		,L"ｶﾞ",L"が"
		,L"ｷﾞ",L"ぎ"
		,L"ｸﾞ",L"ぐ"
		,L"ｹﾞ",L"げ"
		,L"ｺﾞ",L"ご"
		,L"ｻﾞ",L"ざ"
		,L"ｼﾞ",L"じ"
		,L"ｽﾞ",L"ず"
		,L"ｾﾞ",L"ぜ"
		,L"ｿﾞ",L"ぞ"
		,L"ﾀﾞ",L"だ"
		,L"ﾁﾞ",L"ぢ"
		,L"ﾂﾞ",L"づ"
		,L"ｾﾞ",L"ぜ"
		,L"ｿﾞ",L"ぞ"
		,L"ﾊﾞ",L"ば"
		,L"ﾋﾞ",L"び"
		,L"ﾌﾞ",L"ぶ"
		,L"ﾍﾞ",L"べ"
		,L"ﾎﾞ",L"ぼ"
		,L"ﾊﾟ",L"ぱ"
		,L"ﾋﾟ",L"ぴ"
		,L"ﾌﾟ",L"ぷ"
		,L"ﾍﾟ",L"ぺ"
		,L"ﾎﾟ",L"ぽ"
		,L"ｱ",L"あ"
		,L"ｲ",L"い"
		,L"ｳ",L"う"
		,L"ｴ",L"え"
		,L"ｵ",L"お"
		,L"ｶ",L"か"
		,L"ｷ",L"き"
		,L"ｸ",L"く"
		,L"ｹ",L"け"
		,L"ｺ",L"こ"
		,L"ｻ",L"さ"
		,L"ｼ",L"し"
		,L"ｽ",L"す"
		,L"ｾ",L"せ"
		,L"ｿ",L"そ"
		,L"ﾀ",L"た"
		,L"ﾁ",L"ち"
		,L"ﾂ",L"つ"
		,L"ﾃ",L"て"
		,L"ﾄ",L"と"
		,L"ﾅ",L"な"
		,L"ﾆ",L"に"
		,L"ﾇ",L"ぬ"
		,L"ﾈ",L"ね"
		,L"ﾉ",L"の"
		,L"ﾊ",L"は"
		,L"ﾋ",L"ひ"
		,L"ﾌ",L"ふ"
		,L"ﾍ",L"へ"
		,L"ﾎ",L"ほ"
		,L"ﾏ",L"ま"
		,L"ﾐ",L"み"
		,L"ﾑ",L"む"
		,L"ﾒ",L"め"
		,L"ﾓ",L"も"
		,L"ﾔ",L"や"
		,L"ﾕ",L"ゆ"
		,L"ﾖ",L"よ"
		,L"ﾗ",L"ら"
		,L"ﾘ",L"り"
		,L"ﾙ",L"る"
		,L"ﾚ",L"れ"
		,L"ﾛ",L"ろ"
		,L"ｦ",L"を"
		,L"ﾜ",L"わ"
		,L"ﾝ",L"ん"
		,L"ｧ",L"ぁ"
		,L"ｨ",L"ぃ"
		,L"ｩ",L"ぅ"
		,L"ｪ",L"ぇ"
		,L"ｫ",L"ぉ"
		,L"ｬ",L"ゃ"
		,L"ｭ",L"ゅ"
		,L"ｮ",L"ょ"
		,L"ｯ",L"っ"
		,L"ｰ",L"ー"
		,NULL,NULL
	};
	static const WCHAR *replaceTableHankanaToKatakana[] = {
		 L"ｳﾞ",L"ヴ"
		,L"ｶﾞ",L"ガ"
		,L"ｷﾞ",L"ギ"
		,L"ｸﾞ",L"グ"
		,L"ｹﾞ",L"ゲ"
		,L"ｺﾞ",L"ゴ"
		,L"ｻﾞ",L"ザ"
		,L"ｼﾞ",L"ジ"
		,L"ｽﾞ",L"ズ"
		,L"ｾﾞ",L"ゼ"
		,L"ｿﾞ",L"ゾ"
		,L"ﾀﾞ",L"ダ"
		,L"ﾁﾞ",L"ヂ"
		,L"ﾂﾞ",L"ヅ"
		,L"ｾﾞ",L"ゼ"
		,L"ｿﾞ",L"ゾ"
		,L"ﾊﾞ",L"バ"
		,L"ﾋﾞ",L"ビ"
		,L"ﾌﾞ",L"ブ"
		,L"ﾍﾞ",L"ベ"
		,L"ﾎﾞ",L"ボ"
		,L"ﾊﾟ",L"パ"
		,L"ﾋﾟ",L"ピ"
		,L"ﾌﾟ",L"プ"
		,L"ﾍﾟ",L"ペ"
		,L"ﾎﾟ",L"ポ"
		,L"ｱ",L"ア"
		,L"ｲ",L"イ"
		,L"ｳ",L"ウ"
		,L"ｴ",L"エ"
		,L"ｵ",L"オ"
		,L"ｶ",L"カ"
		,L"ｷ",L"キ"
		,L"ｸ",L"ク"
		,L"ｹ",L"ケ"
		,L"ｺ",L"コ"
		,L"ｻ",L"サ"
		,L"ｼ",L"シ"
		,L"ｽ",L"ス"
		,L"ｾ",L"セ"
		,L"ｿ",L"ソ"
		,L"ﾀ",L"タ"
		,L"ﾁ",L"チ"
		,L"ﾂ",L"ツ"
		,L"ﾃ",L"テ"
		,L"ﾄ",L"ト"
		,L"ﾅ",L"ナ"
		,L"ﾆ",L"ニ"
		,L"ﾇ",L"ヌ"
		,L"ﾈ",L"ネ"
		,L"ﾉ",L"ノ"
		,L"ﾊ",L"ハ"
		,L"ﾋ",L"ヒ"
		,L"ﾌ",L"フ"
		,L"ﾍ",L"ヘ"
		,L"ﾎ",L"ホ"
		,L"ﾏ",L"マ"
		,L"ﾐ",L"ミ"
		,L"ﾑ",L"ム"
		,L"ﾒ",L"メ"
		,L"ﾓ",L"モ"
		,L"ﾔ",L"ヤ"
		,L"ﾕ",L"ユ"
		,L"ﾖ",L"ヨ"
		,L"ﾗ",L"リ"
		,L"ﾘ",L"リ"
		,L"ﾙ",L"ル"
		,L"ﾚ",L"レ"
		,L"ﾛ",L"ロ"
		,L"ｦ",L"ヲ"
		,L"ﾜ",L"ワ"
		,L"ﾝ",L"ン"
		,L"ｧ",L"ァ"
		,L"ｨ",L"ィ"
		,L"ｩ",L"ゥ"
		,L"ｪ",L"ェ"
		,L"ｫ",L"ォ"
		,L"ｬ",L"ャ"
		,L"ｭ",L"ュ"
		,L"ｮ",L"ョ"
		,L"ｯ",L"ッ"
		,L"ｰ",L"ー"
		,NULL,NULL
	};
	static const WCHAR *replaceTableKatakanaToHiragana[] = {
		 L"ヴ",L"う゛"
		,L"ア",L"あ"
		,L"イ",L"い"
		,L"ウ",L"う"
		,L"エ",L"え"
		,L"オ",L"お"
		,L"カ",L"か"
		,L"キ",L"き"
		,L"ク",L"く"
		,L"ケ",L"け"
		,L"コ",L"こ"
		,L"サ",L"さ"
		,L"シ",L"し"
		,L"ス",L"す"
		,L"セ",L"せ"
		,L"ソ",L"そ"
		,L"タ",L"た"
		,L"チ",L"ち"
		,L"ツ",L"つ"
		,L"テ",L"て"
		,L"ト",L"と"
		,L"ナ",L"な"
		,L"ニ",L"に"
		,L"ヌ",L"ぬ"
		,L"ネ",L"ね"
		,L"ノ",L"の"
		,L"ハ",L"は"
		,L"ヒ",L"ひ"
		,L"フ",L"ふ"
		,L"ヘ",L"へ"
		,L"ホ",L"ほ"
		,L"マ",L"ま"
		,L"ミ",L"み"
		,L"ム",L"む"
		,L"メ",L"め"
		,L"モ",L"も"
		,L"ヤ",L"や"
		,L"ユ",L"ゆ"
		,L"ヨ",L"よ"
		,L"ラ",L"ら"
		,L"リ",L"り"
		,L"ル",L"る"
		,L"レ",L"れ"
		,L"ロ",L"ろ"
		,L"ワ",L"わ"
		,L"ヲ",L"を"
		,L"ン",L"ん"
		,L"ァ",L"ぁ"
		,L"ィ",L"ぃ"
		,L"ゥ",L"ぅ"
		,L"ェ",L"ぇ"
		,L"ォ",L"ぉ"
		,L"ガ",L"が"
		,L"ギ",L"ぎ"
		,L"グ",L"ぐ"
		,L"ゲ",L"げ"
		,L"ゴ",L"ご"
		,L"ザ",L"ざ"
		,L"ジ",L"じ"
		,L"ズ",L"ず"
		,L"ゼ",L"ぜ"
		,L"ゾ",L"ぞ"
		,L"ダ",L"だ"
		,L"ヂ",L"ぢ"
		,L"ヅ",L"づ"
		,L"デ",L"で"
		,L"ド",L"ど"
		,L"バ",L"ば"
		,L"ビ",L"び"
		,L"ブ",L"ぶ"
		,L"ベ",L"べ"
		,L"ボ",L"ぼ"
		,L"パ",L"ぱ"
		,L"ピ",L"ぴ"
		,L"プ",L"ぷ"
		,L"ペ",L"ぺ"
		,L"ポ",L"ぽ"
		,L"ャ",L"ゃ"
		,L"ュ",L"ゅ"
		,L"ョ",L"ょ"
		,L"ッ",L"っ"
		,L"ヮ",L"ゎ"
		,NULL,NULL
	};
//k	 「全角カタカナ」を「半角カタカナ」に変換します。
//K	 「半角カタカナ」を「全角カタカナ」に変換します。
	if ( option.find(L"k") != -1 )
	{
		ret = XLWStringUtil::replace(ret ,replaceTableHankanaToKatakana,true );
	}
	else if ( option.find(L"K") != -1)
	{
		ret = XLWStringUtil::replace(ret ,replaceTableHankanaToKatakana,false );
	}

//c	 「全角カタカナ」を「全角ひらがな」に変換します。
//C	 「全角ひらがな」を「全角カタカナ」に変換します。
	if ( option.find(L"c") != -1 )
	{
		ret = XLWStringUtil::replace(ret ,replaceTableKatakanaToHiragana,false );
	}
	else if ( option.find(L"C") != -1)
	{
		ret = XLWStringUtil::replace(ret ,replaceTableKatakanaToHiragana,true );
	}

//h	 「全角ひらがな」を「半角カタカナ」に変換します。
//H	 「半角カタカナ」を「全角ひらがな」に変換します。
	if ( option.find(L"h") != -1 )
	{
		ret = XLWStringUtil::replace(ret ,replaceTableHankanaToHiragana,true );
	}
	else if ( option.find(L"H") != -1)
	{
		ret = XLWStringUtil::replace(ret ,replaceTableHankanaToHiragana,false );
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
		std::wstring a = XLWStringUtil::mb_convert_kana(L"あいうえおアイウエオｱｲｳｴｵ　123１２３ ABCＡＢＣ",L"h"); //「全角ひらがな」を「半角カタカナ」に変換します。
		std::wstring b = L"ｱｲｳｴｵアイウエオｱｲｳｴｵ　123１２３ ABCＡＢＣ";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::wstring a = XLWStringUtil::mb_convert_kana(L"あいうえおアイウエオｱｲｳｴｵ　123１２３ ABCＡＢＣ",L"H"); //「半角カタカナ」を「全角ひらがな」に変換します。
		std::wstring b = L"あいうえおアイウエオあいうえお　123１２３ ABCＡＢＣ";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::wstring a = XLWStringUtil::mb_convert_kana(L"あいうえおアイウエオｱｲｳｴｵ　123１２３ ABCＡＢＣ",L"c"); //「全角カタカナ」を「全角ひらがな」に変換します。
		std::wstring b = L"あいうえおあいうえおｱｲｳｴｵ　123１２３ ABCＡＢＣ";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::wstring a = XLWStringUtil::mb_convert_kana(L"あいうえおアイウエオｱｲｳｴｵ　123１２３ ABCＡＢＣ",L"C"); //「全角ひらがな」を「全角カタカナ」に変換します。
		std::wstring b = L"アイウエオアイウエオｱｲｳｴｵ　123１２３ ABCＡＢＣ";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::wstring a = XLWStringUtil::mb_convert_kana(L"あいうえおアイウエオｱｲｳｴｵ　123１２３ ABCＡＢＣ",L"k"); //「全角カタカナ」を「半角カタカナ」に変換します。
		std::wstring b = L"あいうえおｱｲｳｴｵｱｲｳｴｵ　123１２３ ABCＡＢＣ";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::wstring a = XLWStringUtil::mb_convert_kana(L"あいうえおアイウエオｱｲｳｴｵ　123１２３ ABCＡＢＣ",L"K"); //「半角カタカナ」を「全角カタカナ」に変換します。
		std::wstring b = L"あいうえおアイウエオアイウエオ　123１２３ ABCＡＢＣ";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::wstring a = XLWStringUtil::mb_convert_kana(L"あいうえおアイウエオｱｲｳｴｵ　123１２３ ABCＡＢＣ",L"n"); //n	 「全角」数字を「半角」に変換します。
		std::wstring b = L"あいうえおアイウエオｱｲｳｴｵ　123123 ABCＡＢＣ";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::wstring a = XLWStringUtil::mb_convert_kana(L"あいうえおアイウエオｱｲｳｴｵ　123１２３ ABCＡＢＣ",L"N"); //N	 「半角」数字を「全角」に変換します。
		std::wstring b = L"あいうえおアイウエオｱｲｳｴｵ　１２３１２３ ABCＡＢＣ";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::wstring a = XLWStringUtil::mb_convert_kana(L"あいうえおアイウエオｱｲｳｴｵ　123１２３ ABCＡＢＣ",L"a"); //a	 「全角」英数字を「半角」に変換します。
		std::wstring b = L"あいうえおアイウエオｱｲｳｴｵ　123123 ABCABC";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::wstring a = XLWStringUtil::mb_convert_kana(L"あいうえおアイウエオｱｲｳｴｵ　123１２３ ABCＡＢＣ",L"A"); //A	 「半角」英数字を「全角」に変換します。
		std::wstring b = L"あいうえおアイウエオｱｲｳｴｵ　１２３１２３ ＡＢＣＡＢＣ";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::wstring a = XLWStringUtil::mb_convert_kana(L"あいうえおアイウエオｱｲｳｴｵ　123１２３ ABCＡＢＣ",L"s"); //s	 「全角」スペースを「半角」に変換します
		std::wstring b = L"あいうえおアイウエオｱｲｳｴｵ 123１２３ ABCＡＢＣ";
		SEXYTEST_EQ(a ,b); 
	}
	{
		std::wstring a = XLWStringUtil::mb_convert_kana(L"あいうえおアイウエオｱｲｳｴｵ　123１２３ ABCＡＢＣ",L"S"); //S	 「半角」スペースを「全角」に変換します
		std::wstring b = L"あいうえおアイウエオｱｲｳｴｵ　123１２３　ABCＡＢＣ";
		SEXYTEST_EQ(a ,b); 
	}
}


bool XLWStringUtil::isKanji(WCHAR p)
{
//	if (p >= L'一' && p <= L'龠' )
	//http://pentan.info/doc/unicode_database_block.html
	if (p >= 0x4E00 && p <= 0x9FFF )
	{
		return true;
	}

	//http://www.tamasoft.co.jp/ja/general-info/unicode.html
	//諸説あるんだろうけど、Π(0x03A0) とか Σ(0x03A0) とかもセーフにするか.
	if (p >= 0x0384 && p <= 0x03CE)
	{
		return true;
	}
	return false;
}

bool XLWStringUtil::isKana(WCHAR p)
{
	//http://pentan.info/doc/unicode_database_block.html
//	if (p >= L'ぁ' && p <= L'ん' )
	if (p >= 0x3040 && p <= 0x309F )
	{
		return true;
	}
	if (p == L'ー' )
	{
		return true;
	}
	if (p == L'～' )
	{
		return true;
	}
	return false;
}
bool XLWStringUtil::isKata(WCHAR p)
{
//http://pentan.info/doc/unicode_database_block.html
//	if (p >= L'ァ' && p <= L'ヶ' )
	if (p >= 0x30A0 && p <= 0x30FF )
	{
		return true;
	}
	if (p == L'ー' )
	{
		return true;
	}
	if (p == L'～' )
	{
		return true;
	}
	if (p == L'・' )
	{
		return true;
	}
	return false;
}
bool XLWStringUtil::isAlphanum(WCHAR p)
{
	if (p >= L'a' && p <= L'z' )
	{
		return true;
	}
	if (p >= L'A' && p <= L'Z' )
	{
		return true;
	}
	if (p >= L'0' && p <= L'9' )
	{
		return true;
	}
	return false;
}
bool XLWStringUtil::isAlpha(WCHAR p)
{
	if (p >= L'a' && p <= L'z' )
	{
		return true;
	}
	if (p >= L'A' && p <= L'Z' )
	{
		return true;
	}
	return false;
}

bool XLWStringUtil::isSpace(WCHAR p)
{
	if (p == L' ' || p == L'　' || p==L'\t' )
	{
		return true;
	}
	return false;
}


//置換
std::wstring XLWStringUtil::replace(const std::wstring &inTarget ,const std::wstring &inOld ,const std::wstring &inNew)
{
	if (inOld.empty()) return inTarget;

	std::wstring ret;
	ret.reserve( inTarget.size() );	//先読み.

	const WCHAR * p = inTarget.c_str();
	const WCHAR * match;
	while( match = wcsstr( p , inOld.c_str() ) )
	{
		//ret += std::wstring(p,0,(int)(match - p));
		ret.append(p,(int)(match - p));
		ret += inNew;

		p = match + inOld.size();
	}
	//残りを足しておしまい.
	return ret + p;
}


// s - e の範囲を消します nestにも対応
//aa[[bb]]cc -> [[ ]] を消すと --> aacc
//ab[[c[[d]]e]]zx[[aa]]12 --> abzx12
//ab12 -> ab12
std::wstring XLWStringUtil::strsnip(const std::wstring& strW,const std::wstring& startKako,const std::wstring& endKako)
{
	std::wstring retMatch;
	int pos = 0;
	int nest = 0;

	while(1)
	{
		int s = strW.find(startKako,pos);
		if (s == std::wstring::npos)
		{//開始括弧がない
			retMatch += strW.substr(pos);
			return retMatch;
		}

		if (nest <= 0)
		{
			retMatch += strW.substr(pos,s-pos);
		}

		pos = s + startKako.size();
		nest ++;

		while(1)
		{
			int e = strW.find(endKako,pos);
			int i2 = strW.find(startKako,pos);

			if (e == std::wstring::npos)
			{
				retMatch += strW.substr(pos,s-pos);
				return retMatch;
			}
			if (i2 == std::wstring::npos || e < i2)
			{
				nest --;
				pos = e + endKako.size();
				if (nest <= 0)
				{
					break;
				}
			}
			else
			{
				nest ++;
				pos = i2 + startKako.size();
			}
		}
	}
}

SEXYTEST()
{
	{
		std::wstring a = L">{{Otheruses|端末間通信技術|P2P技術を利用したファイル共有システム|ファイル共有ソフトウェア}}\n[[ファイル:P2P-network.svg|thumb|200px|P2P型ネットワーク（図はピュアP2P型）。コンピューター同士が対等に通信を行うのが特徴である。]]\n'''ピアトゥピア'''または'''ピアツーピア'''（{{lang-en-short|peer to peer}}）とは、多数の端末間で通信を行う際の[[アーキテクチャ]]のひとつで、対等の者（Peer、ピア）同士が通信をすることを特徴とする通信方式、通信モデル、あるいは、通信技術の一分野を指す。'''P2P'''と[[Leet|略記]]することが多く、以下本記事においてもP2Pとする。";
		std::wstring w = XLWStringUtil::strsnip(a ,L"{{",L"}}");
		assert(w == L">\n[[ファイル:P2P-network.svg|thumb|200px|P2P型ネットワーク（図はピュアP2P型）。コンピューター同士が対等に通信を行うのが特徴である。]]\n'''ピアトゥピア'''または'''ピアツーピア'''（）とは、多数の端末間で通信を行う際の[[アーキテクチャ]]のひとつで、対等の者（Peer、ピア）同士が通信をすることを特徴とする通信方式、通信モデル、あるいは、通信技術の一分野を指す。'''P2P'''と[[Leet|略記]]することが多く、以下本記事においてもP2Pとする。");
	}
}

// s - e の範囲を消します nestにも対応 (全部消すのではなくて1回だけバージョン)
//aa[[bb]]cc -> [[ ]] を消すと --> aacc
//ab[[c[[d]]e]]zx[[aa]]12 --> abzx[[aa]]12
//ab12 -> ab12
std::wstring XLWStringUtil::strsnipOne(const std::wstring& strW,const std::wstring& startKako,const std::wstring& endKako)
{
	std::wstring retMatch;
	int pos = 0;
	int nest = 0;

	int s = strW.find(startKako,pos);
	if (s == std::wstring::npos)
	{//開始括弧がない
		retMatch += strW.substr(pos);
		return retMatch;
	}

	if (nest <= 0)
	{
		retMatch += strW.substr(pos,s-pos);
	}

	pos = s + startKako.size();
	nest ++;

	while(1)
	{
		int e = strW.find(endKako,pos);
		int i2 = strW.find(startKako,pos);

		if (e == std::wstring::npos)
		{
			retMatch += strW.substr(pos,s-pos);
			return retMatch;
		}
		if (i2 == std::wstring::npos || e < i2)
		{
			nest --;
			pos = e + endKako.size();
			if (nest <= 0)
			{
				retMatch += strW.substr(pos);
				return retMatch;
			}
		}
		else
		{
			nest ++;
			pos = i2 + startKako.size();
		}
	}
	return strW;
}
SEXYTEST()
{
	{
		std::wstring w = L"\n[[ファイル:P2P-network.svg|thumb|200px|P2P型ネットワーク（図はピュアP2P型）。コンピューター同士が対等に通信を行うのが特徴である。]]\n'''ピアトゥピア'''または'''ピアツーピア'''（）とは、多数の端末間で通信を行う際の[[アーキテクチャ]]のひとつで、対等の者（Peer、ピア）同士が通信をすることを特徴とする通信方式、通信モデル、あるいは、通信技術の一分野を指す。'''P2P'''と[[Leet|略記]]することが多く、以下本記事においてもP2Pとする。";
		std::wstring r = XLWStringUtil::strsnipOne(w,L"[[",L"]]");
		assert(r == L"\n\n'''ピアトゥピア'''または'''ピアツーピア'''（）とは、多数の端末間で通信を行う際の[[アーキテクチャ]]のひとつで、対等の者（Peer、ピア）同士が通信をすることを特徴とする通信方式、通信モデル、あるいは、通信技術の一分野を指す。'''P2P'''と[[Leet|略記]]することが多く、以下本記事においてもP2Pとする。");
	}
	{
		std::wstring w = L"aa[[bb]]cc";
		std::wstring r = XLWStringUtil::strsnipOne(w,L"[[",L"]]");
		assert(r == L"aacc");
	}
	{
		std::wstring w = L"ab[[c[[d]]e]]zx[[aa]]12";
		std::wstring r = XLWStringUtil::strsnipOne(w,L"[[",L"]]");
		assert(r == L"abzx[[aa]]12");
	}
	{
		std::wstring w = L"ab12";
		std::wstring r = XLWStringUtil::strsnipOne(w,L"[[",L"]]");
		assert(r == L"ab12");
	}
}

// a - b の範囲から文字列を取得します 直近の最初だけです.
std::wstring XLWStringUtil::strab(const std::wstring& str,const std::wstring& a,const std::wstring& b,int* outGetPosstion)
{
	int posA = str.find(a);
	if (posA == std::wstring::npos)
	{
		return L"";
	}

	int posB = str.find(b, posA+a.size() );
	if (posB == std::wstring::npos)
	{
		return L"";
	}
	
	if (outGetPosstion)
	{
		*outGetPosstion = posA;
	}
	
	return str.substr(posA+a.size() , posB - (posA+a.size()) );
}

//先頭と末尾の余白の整理
std::wstring XLWStringUtil::chop(const std::wstring & str,const WCHAR * replaceTable)
{
	if (replaceTable == NULL)
	{
		replaceTable = L" 　\t\r\n";
	}

	const WCHAR * p = str.c_str();
	//頭出し
	while(*p)
	{
		const WCHAR * rep = replaceTable;
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

	const WCHAR * start = p;
	const WCHAR * lastEffectvie = p;

	//終端削り
	while(*p)
	{
		const WCHAR * rep = replaceTable;
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
	return std::wstring(start , 0 , (lastEffectvie - start) );
}

//split
std::list<std::wstring> XLWStringUtil::split(const std::wstring& glue, const std::wstring & inTarget )
{
	std::list<std::wstring> r;

	int oldpos = 0;
	int pos = 0;
	while( (pos = inTarget.find( glue , oldpos)) != std::wstring::npos )
	{
		std::wstring k = inTarget.substr(oldpos , pos - oldpos);

		r.push_back(k);

		oldpos = pos+glue.size();
	}
	//最後の残り
	{
		std::wstring k = inTarget.substr(oldpos , pos - oldpos);
		r.push_back(k);
	}
	return r;
}

//split
std::vector<std::wstring> XLWStringUtil::split_vector(const std::wstring& glue, const std::wstring & inTarget )
{
	std::vector<std::wstring> r;

	int oldpos = 0;
	int pos = 0;
	while( (pos = inTarget.find( glue , oldpos)) != std::wstring::npos )
	{
		std::wstring k = inTarget.substr(oldpos , pos - oldpos);

		r.push_back(k);

		oldpos = pos+glue.size();
	}
	//最後の残り
	{
		std::wstring k = inTarget.substr(oldpos , pos - oldpos);
		r.push_back(k);
	}
	return r;
}

//一番最初に来るもの.
bool XLWStringUtil::firstfindPos(const std::wstring& innerW,int pos,int endpos,int* outHitStartPos,int* outHitEndPos
	,const std::wstring& w1,const std::wstring& w2,const std::wstring& w3,const std::wstring& w4,const std::wstring& w5
	,const std::wstring& w6,const std::wstring& w7,const std::wstring& w8,const std::wstring& w9,const std::wstring& w10
	,const std::wstring& w11,const std::wstring& w12,const std::wstring& w13,const std::wstring& w14,const std::wstring& w15
	,const std::wstring& w16,const std::wstring& w17,const std::wstring& w18,const std::wstring& w19,const std::wstring& w20
	)
{
	int minmatch = INT_MAX;
	int hitlength = 0;
	const std::wstring* mapping[] = {&w1,&w2,&w3,&w4,&w5,&w6,&w7,&w8,&w9,&w10,&w11,&w12,&w13,&w14,&w15,&w16,&w17,&w18,&w19,&w20 ,NULL};
	
	for(const std::wstring** p = mapping; *p ; p++ )
	{
		if ( (*p)->empty())
		{
			continue;
		}
		int a1 = innerW.find(*(*p),pos);
		if (a1 != std::wstring::npos && a1 < minmatch && a1 < endpos )
		{
			minmatch = a1;
			hitlength = (*p)->size();
		}
	}

	if (minmatch == INT_MAX)
	{//どれにもヒットしなかったら、ねーよを返す.
		if (outHitStartPos) *outHitStartPos = 0;
		if (outHitEndPos) *outHitEndPos = 0;
		return false;
	}

	if (outHitStartPos) *outHitStartPos = minmatch;
	if (outHitEndPos) *outHitEndPos = minmatch + hitlength;
	return true;
}


//一番最初に来るもの.(の rfind )
bool XLWStringUtil::firstrrfindPos(const std::wstring& innerW,int pos,int r_endpos,int* outHitStartPos,int* outHitEndPos
	,const std::wstring& w1,const std::wstring& w2,const std::wstring& w3,const std::wstring& w4,const std::wstring& w5
	,const std::wstring& w6,const std::wstring& w7,const std::wstring& w8,const std::wstring& w9,const std::wstring& w10
	,const std::wstring& w11,const std::wstring& w12,const std::wstring& w13,const std::wstring& w14,const std::wstring& w15
	,const std::wstring& w16,const std::wstring& w17,const std::wstring& w18,const std::wstring& w19,const std::wstring& w20
	)
{
	int maxmatch = INT_MIN;
	int hitlength = 0;

	const std::wstring* mapping[] = {&w1,&w2,&w3,&w4,&w5,&w6,&w7,&w8,&w9,&w10,&w11,&w12,&w13,&w14,&w15,&w16,&w17,&w18,&w19,&w20 ,NULL};
	
	for(const std::wstring** p = mapping; *p ; p++ )
	{
		if ((*p)->empty())
		{
			continue;
		}
		int a1 = innerW.rfind(*(*p),pos);
		if (a1 != std::wstring::npos && a1+(int)(*p)->size() > maxmatch && a1 > r_endpos )
		{
			maxmatch = a1;
			hitlength = (*p)->size();
		}
	}

	if (maxmatch == INT_MIN)
	{//どれにもヒットしなかったら、ねーよを返す.
		if (outHitStartPos) *outHitStartPos = 0;
		if (outHitEndPos) *outHitEndPos = 0;
		return false;
	}

	if (outHitStartPos) *outHitStartPos = maxmatch;
	if (outHitEndPos) *outHitEndPos = maxmatch + hitlength;
	return true;
}
