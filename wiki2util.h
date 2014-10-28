
#pragma once

std::string ParseTitleOneLine(const std::string& line) ;


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
;
	
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
;

//カッコ内のパース 括弧が内部指定された簡易版
bool parseKakoREx(const std::wstring &strW	//対象文字列
	,int pos                            //skipする位置
	,int endpos                         //終了する位置
	,bool isSearchStartKako             //最初の括弧を true->find するか、false->[0]にある
	,int* outStartKakoPos				//開始括弧位置
	,int* outStart2KakoPos				//開始括弧をすぎて本文が始まる位置
	,int* outEndKakoPos					//終了括弧位置
	,int* outEnd2KakoPos)				//終了括弧をすぎて本文が始まる位置
;

//カッコ内のパース 括弧が内部指定された簡易版 (逆方向)
bool parseKakoEx(const std::wstring &strW	//対象文字列
	,int pos                            //skipする位置
	,int endpos                         //終了する位置
	,bool isSearchStartKako             //最初の括弧を true->find するか、false->[0]にある
	,int* outStartKakoPos				//開始括弧位置
	,int* outStart2KakoPos				//開始括弧をすぎて本文が始まる位置
	,int* outEndKakoPos					//終了括弧位置
	,int* outEnd2KakoPos)				//終了括弧をすぎて本文が始まる位置
;


bool isAsciiOnlyTitle(const WCHAR* str );

//一覧表示か
bool checkIchiran(const std::wstring& title);

//ゴミページか
bool checkGomiPage(const std::wstring& title);

std::wstring cleaningInnerText(const std::wstring& str);


//飛び先に曖昧さ解決の()が入っていたら消します.
std::wstring snipAimaiKako(const std::wstring& r);
//曖昧さの解決のページかどうか
bool isAimai(const std::wstring& innerW);

//曖昧さの解決のページかどうか
bool isAimai(const std::string& inner);

bool isRedirect(const std::wstring& innerW);

bool isRedirect(const std::string& inner);

//頭出し
std::wstring makeInnerHead(const std::wstring& innerW);

std::wstring getInfoboxStr(const std::wstring& innerHeadW,const std::wstring& key );

std::wstring makeInnerText(const std::wstring& innerW);

//豆腐に中止ながらUTF16から表示用のマルチバイトに変換。
void ConvertUTF16toMultibye(const std::wstring& a1,const std::wstring& a2,std::string* outAA1,std::string* outAA2);

//[[]] を消します.
std::wstring snipWikiKeywordSyntax(const std::wstring& r);

//など と最後に書かれていたら削る
std::wstring snipNado(const std::wstring& w);

//wiki表記で、パースの邪魔になる \n{{ ....  }}   [[ファイル  ]]  などの広域な構文のみを消します. 全部消すと問題あるので、この2つに限り消す.
std::wstring killWikiKako( const std::wstring& strW);

//brタグみたいなのを飛ばす.
std::wstring killHTMLTagBR(const std::wstring& innerW,const std::wstring& name);
//HTMLタグを飛ばす (完璧ではないです)
std::wstring killHTMLTag(const std::wstring& innerW,const std::wstring& name);

//カタカナのみ
bool isKatakanaOnly(const std::wstring& str);

//アルファベットのみ
bool isAplhaOnly(const std::wstring& str);

//小文字のみ
bool isAplhaSmallOnly(const std::wstring& str);

//アルファベットと数字のみ
bool isAlphanumOnly(const std::wstring& str);

//ひらがなとカタカナのみ
bool isHiraKataOnly(const std::wstring& str);

//漢字のみ
bool isKanjiOnly(const std::wstring& str);

//括弧の開始か？
bool isKakoStart(WCHAR c);

//よみがなから余計な文字を取り払います
std::wstring convertPlainYomi(const std::wstring & str);

//nameのwikiブロックだけとりだします
std::wstring getWikiBlock(const std::wstring& innerW,const std::wstring& name);

//nameのwikiブロックを消します
std::wstring cutWikiBlock(const std::wstring& innerW,const std::wstring& name);
