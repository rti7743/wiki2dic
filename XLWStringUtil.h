
#pragma once

class XLWStringUtil
{
public:
	//みんな大好きPHPのstrtoupper
	static std::wstring strtoupper(const std::wstring & str);

	//みんな大好きPHPのstrtolower
	static std::wstring strtolower(const std::wstring & str);

	//次の文字列へ
	static inline const WCHAR* nextChar(const WCHAR * p)
	{
//	#if _MSC_VER
//		if (((0x81 <= (unsigned char)(*p) && (unsigned char)(*p) <= 0x9f) || (0xe0 <= (unsigned char)(*p) && (unsigned char)(*p) <= 0xfc)))
//		{
//			return p + 2;
//		}
//		return p + 1;
//	#else
//		if ( (((unsigned char)*p) & 0x80) == 0) return p + 1; 
//		if ( (((unsigned char)*p) & 0x20) == 0) return p + 2;
//		if ( (((unsigned char)*p) & 0x10) == 0) return p + 3;
//		if ( (((unsigned char)*p) & 0x08) == 0) return p + 4;
//		if ( (((unsigned char)*p) & 0x04) == 0) return p + 5;
//		return p + 6;
//	#endif
		return p + 1;	//ああ～ UTF16は楽すぎるんじゃあ～ 
	}
	//次の文字列へ
	static inline WCHAR* nextChar(WCHAR * p)
	{
		return (WCHAR*)nextChar((const WCHAR*)p);
	}

	//マルチバイトか？
	static inline bool isMultiByte(const WCHAR * p)
	{
//	#if _MSC_VER
//		return ((0x81 <= (unsigned char)(*p) && (unsigned char)(*p) <= 0x9f) || (0xe0 <= (unsigned char)(*p) && (unsigned char)(*p) <= 0xfc));
//	#else
//		return ( (((unsigned char)*p) & 0x80) != 0); 
//	#endif
		return *p >= 0x80;
	}
	
	//strstrのマルチバイトセーフ 文字列検索
	static const WCHAR* strstr(const WCHAR* target, const WCHAR* need );
	//strstrのマルチバイトセーフ 文字列検索 //若干非効率
	static const WCHAR* strrstr(const std::wstring& target, const std::wstring & need );
	//strstrのマルチバイトセーフ 文字列検索 //若干非効率
	static const WCHAR* strrstr(const WCHAR* target, const WCHAR* need );
	//なぜか標準にない /^foo/ みたいに前方マッチ.
	static const WCHAR* strfirst(const WCHAR* s,const WCHAR* n);
	//なぜか標準にない /foo$/ みたいに後方マッチ.
	static const WCHAR* strend(const WCHAR* s,const WCHAR* n);
	//マルチバイト対応 複数の候補を一括置換 const char * replacetable[] = { "A","あ"  ,"I","い"  , "上","うえ" , NULL , NULL}  //必ず2つ揃えで書いてね
	static std::wstring replace(const std::wstring &inTarget ,const WCHAR** replacetable,bool isrev);
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
	static std::wstring mb_convert_kana(const std::wstring &inTarget,const std::wstring& option);
	

	static bool isKanji(WCHAR p);
	static bool isKana(WCHAR p);
	static bool isKata(WCHAR p);
	static bool isAlphanum(WCHAR p);
	static bool isAlpha(WCHAR p);
	static bool isSpace(WCHAR p);

	//置換
	static std::wstring replace(const std::wstring &inTarget ,const std::wstring &inOld ,const std::wstring &inNew);

	// s - e の範囲を消します
	static std::wstring strsnip(const std::wstring& str,const std::wstring& s,const std::wstring& e);
	static std::wstring strsnipOne(const std::wstring& strW,const std::wstring& startKako,const std::wstring& endKako);

	// a - b の範囲から文字列を取得します 直近の最初だけです.
	static std::wstring strab(const std::wstring& str,const std::wstring& a,const std::wstring& b,int* outGetPosstion = NULL);

	//先頭と末尾の余白の整理
	static std::wstring chop(const std::wstring & str,const WCHAR * replaceTable=NULL);
	
	//split
	static std::list<std::wstring> split(const std::wstring& glue, const std::wstring & inTarget );
	static std::vector<std::wstring> split_vector(const std::wstring& glue, const std::wstring & inTarget );

	//一番最初に来るもの.
	static bool firstfindPos(const std::wstring& innerW,int pos,int endpos,int* outHitStartPos,int* outHitEndPos
		,const std::wstring& w1,const std::wstring& w2 = std::wstring(),const std::wstring& w3 = std::wstring(),const std::wstring& w4 = std::wstring(),const std::wstring& w5 = std::wstring()
		,const std::wstring& w6 = std::wstring(),const std::wstring& w7 = std::wstring(),const std::wstring& w8 = std::wstring(),const std::wstring& w9 = std::wstring(),const std::wstring& w10 = std::wstring()
		,const std::wstring& w11= std::wstring(),const std::wstring& w12= std::wstring(),const std::wstring& w13= std::wstring(),const std::wstring& w14= std::wstring(),const std::wstring& w15 = std::wstring()
		,const std::wstring& w16= std::wstring(),const std::wstring& w17= std::wstring(),const std::wstring& w18= std::wstring(),const std::wstring& w19= std::wstring(),const std::wstring& w20 = std::wstring()
		);

	static bool firstrrfindPos(const std::wstring& innerW,int pos,int endpos,int* outHitStartPos,int* outHitEndPos
		,const std::wstring& w1,const std::wstring& w2 = std::wstring(),const std::wstring& w3 = std::wstring(),const std::wstring& w4 = std::wstring(),const std::wstring& w5 = std::wstring()
		,const std::wstring& w6 = std::wstring(),const std::wstring& w7 = std::wstring(),const std::wstring& w8 = std::wstring(),const std::wstring& w9 = std::wstring(),const std::wstring& w10 = std::wstring()
		,const std::wstring& w11= std::wstring(),const std::wstring& w12= std::wstring(),const std::wstring& w13= std::wstring(),const std::wstring& w14= std::wstring(),const std::wstring& w15 = std::wstring()
		,const std::wstring& w16= std::wstring(),const std::wstring& w17= std::wstring(),const std::wstring& w18= std::wstring(),const std::wstring& w19= std::wstring(),const std::wstring& w20 = std::wstring()
		);
};
