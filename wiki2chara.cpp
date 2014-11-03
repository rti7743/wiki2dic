
#include "common.h"
#include "XLWStringUtil.h"
#include "wiki2chara.h"
#include "wiki2util.h"
#include "XLFileUtil.h"
#include "wiki2yomi.h"
#include "option.h"

static std::wstring cleanWord(const std::wstring & str)
{
	std::wstring w = str;
	w = XLWStringUtil::replace(w,L"[",L"");
	w = XLWStringUtil::replace(w,L"]",L"");
	w = XLWStringUtil::replace(w,L"'''",L"");
	w = XLWStringUtil::replace(w,L"''",L"");
	w = XLWStringUtil::replace(w,L"{{",L"");
	w = XLWStringUtil::replace(w,L"}}",L"");
	w = XLWStringUtil::replace(w,L"==",L"");
//	w = XLWStringUtil::replace(w,L" ",L"");
//	w = XLWStringUtil::replace(w,L"　",L"");

	w = XLWStringUtil::chop(w);

	return w;
}
	
std::wstring makeInnerTextEx(const std::wstring& str)
{
	std::wstring w = cleaningInnerText(str);
//	w = XLWStringUtil::replace(w,L"{{Visible anchor|",L"、");
//	w = XLWStringUtil::replace(w,L"}}",L"、");
//	w = XLWStringUtil::replace(w,L"{{",L"");
//	w = XLWStringUtil::replace(w,L"}}",L"");
	w = XLWStringUtil::replace(w,L"&lt;",L" ");
	w = XLWStringUtil::replace(w,L"&gt;",L" ");
	w = w + L"\n";
	
	return w;
}

//キャラクターに対応するために、・とか = などのものも追加
bool isHiraKataOnlyEx(const std::wstring& str)
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
		if ( str[i] == L'_' || str[i] == L'・' || str[i] == L'＝' || str[i] == L'=' )
		{
			continue;
		}
		return false;
	}
	return true;
}

//空白を消す
static std::wstring cleanSpace(const std::wstring & str)
{
	std::wstring w = XLWStringUtil::replace(str,L" ",L"");
	w = XLWStringUtil::replace(w,L"　",L"");

	return w;
}


static bool isKugiriMoji(WCHAR c)
{
	if (c==L'('
		|| c==L')'
		|| c==L'（'
		|| c==L'）'
		|| c==L'，'
		|| c==L','
		|| c==L','
		|| c==L'；'
		|| c==L';'
//		|| c==L'。'
		|| c==L'、'
		|| c==L'-'
		|| c==L'：'
		|| c==L':'
		|| c==L'/'
		|| c==L'／'
		|| c==L'⇒'
		|| c==L'→'
		|| c==L'['
		|| c==L'{'
		)
	{
		return true;
	}
	return false;
}

//役者情報であるフラグ
static bool isActorFlag(const std::wstring& str)
{
	//
	//wikiepdianは表記の統一というのをそろそろ覚えよう。
	//
	if(    str == L"声" || str == L"声優" || str == L"CV" || str == L"-"
		|| str == L"キャスティングボイス" || str == L"CV キャスティングボイス"
		|| str == L"日本語吹替" || str == L"吹替" || str == L"日本語吹替版" || str == L"吹替版" 
		|| str == L"日本語吹き替え" || str == L"日本語吹替え"  || str == L"日本語版吹替=" || str == L"吹き替え"
		|| str == L"日本語版" || str == L"日本語" || str == L"原語・日本語版"
		|| str == L"声・" || str == L"英語"	|| str == L"英" || str == L"英語版"
		|| str == L"日本語・英語共に"
		|| str == L"日本版" || str == L"日"
		|| str == L"香港版" || str == L"香"
		|| str == L"台湾版" || str == L"台"
		|| str == L"インターナショナル版"
		|| str == L"演" || str == L"俳優"
		|| str == L"声・操演" || str == L"操演"
		)
	{
		return true;
	}
	return false;
}

//元[[XXXX]]みたいなのを飛ばします.
static bool isDoubleSkipWord(const std::wstring& str)
{
	if(    str == L"元" || str == L"当時"
		)
	{
		return true;
	}
	return false;
}

//役者情報に現れたら消す
static bool isNoActorName(const std::wstring& str)
{
	if(    str == L"主人公" || str == L"本人役" || str == L"二役"  || str == L"三役"  || str == L"四役" 
		|| str == L"モーションアクター"
		)
	{
		return true;
	}
	return false;
}

static bool checkStopString(const std::wstring& str)
{
	if (str.empty()) return false;

	const WCHAR* strW0[] = {
		 L"作詞"
		,L"原作"
		,L"原画"
		,L"脚本"
		,L"音楽"
		,L"照明"
		,L"製作"
		,L"エグゼクティブプロデューサー"
		,L"プロデューサー"
		,L"ディレクター"
		,L"スタイリスト"
		,NULL
	};
	for(const WCHAR** p = strW0 ; *p ; p++)
	{
		if ( str == *p)
		{
			return true;
		}
	}

	const WCHAR* strW2[] = {
		 L"。"
		,L"これも"
		,L"これらは"
		,L"オープニング"
		,L"エンディング"
		,L"することにより"
		,L"関連項目"
		,L"キャラクターデザイン"
		,L"製作プロダクション"
		,L"プロモーションビデオ"

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

static bool checkGomiString(const std::wstring& str)
{
	if (str.empty()) return false;

	if (str.find(L'#') != std::wstring::npos)
	{
		return true;
	}
	if (checkStopString(str))
	{
		return true;
	}
	if (str.size() == 1)
	{
		if (! XLWStringUtil::isKanji(str[0]) )
		{//漢字ではない一文字のデータはいらぬ.
			return true;
		}
	}

	//完全マッチ ==word
	const WCHAR* strW0[] = {
		 L"lang"
		,L"en" 
		,L"本名"
		,L"なし" 
		,L"無し"
		,L"夫"
		,L"妻"
		,L"父"
		,L"母" 
		,L"現" 
		,L"DS" 
		,L"PSP"
		,L"ー"
		,L"歌"
		,L"PC"
		,L"では"
		,L"匠"
		,L"新"
		,L"反"
		,L"奪"
		,L"奪"
		,L"黄"
		,L"我"
		,L"千"
		,L"朝"
		,L"汚"
		,L"白"
		,L"蝶"
		,L"女"
		,L"男"
		,L"連続"
		,L"単発"
		,L"アニメ"
		,L"テレビ"
		,L"パイロット版"
		,L"以降不定期"
		,L"不明"
		,L"映画版"
		,L"概要"
		,L"概歴"
		,L"劇中"
		,L"解決編" 
		,L"幼少期"
		,L"幼年期"
		,L"少年期"
		,L"少女期"
		,L"少年時"
		,L"少女時"
		,L"青年期"
		,L"青年時"
		,L"若年期"
		,L"壮年期"
		,L"変装時"
		,L"赤子時代"
		,L"幼少時代"
		,L"子供時代"
		,L"少年時代"
		,L"少女時代"
		,L"小学時代"
		,L"中校時代"
		,L"高校時代"
		,L"大学時代"
		,L"学生時代"
		,L"小学生時代"
		,L"中学生時代"
		,L"高学生時代"
		,L"大学生時代"
		,L"幼馴染"
		,L"幼少"
		,L"老年"
		,L"幼少期のみ"
		,L"登場時のみ"
		,L"TVアニメ"
		,L"テレビアニメ"
		,L"ラジオドラマ"
		,L"CDドラマ"
		,L"ドラマCD"
		,L"OVA"
		,L"ファンディスク"
		,L"まんがDVD"
		,L"ゲーム"
		,L"テレビ版"
		,L"アニメ版"
		,L"実写"
		,L"Webアニメ"
		,L"※"	
		,L"出版社"
		,L"回想"
		,L"？"
		,L"?"
		,L"仮の姿"
		,L"真の姿"
		,L"同左"
		,L"VOMIC"
		,L"Age"
		,L"成長後"
		,L"♂"
		,L"♀"
		,L"TVSP"
		,L"小学"
		,L"中学"
		,L"高校"
		,L"大学"
		,L"の娘で"
		,L"過去"
		,L"OAD"
		,L"サンデーCM劇場"
		,L"次回予告"
		,L"誕生日"
		,L"牡羊座"
		,L"牡牛座"
		,L"双子座"
		,L"蟹座"
		,L"獅子座"
		,L"乙女座"
		,L"天秤座"
		,L"蠍座"
		,L"射手座"
		,L"山羊座"
		,L"水瓶座"
		,L"魚座"
		,L"血液型"
		,L"O型"
		,L"A型"
		,L"B型"
		,L"AB型"
		,L"子供の"
		,L"ジョジョの奇妙な冒険"
		,L"コンシューマ版"
		,L"オリジナル版"
		,L"堪能で"
		,L"若い頃"
		,L"・"
		,L"じじい"
		,L"変身前"
		,L"通常時"
		,L"電撃文庫"
		,L"クリーム"
		,L"ショコラ"
		,L"特番"
		,L"TV版"
		,L"スーパー"
		,L"総集編"
		,L"2人"
		,L"CD"
		,L"カメオ出演"
		,L"友情出演"
		,L"ゲスト"
		,L"ゲスト出演"
		,L"クレジットタイトル"
		,L"明記なし"
		,L"イメージアルバム"
		,L"DVD版"
		,L"身長"
		,L"体重"
		,L"スリーサイズ"
		,L"初代"
		,L"SS版"
		,L"SS版"
		,L"ミュージカル版"
		,L"後述"
		,L"無印"
		,L"放送開始"
		,L"BOX版"
		,L"がいるほか"
		,L"モノローグ"
		,L"幼年時"
		,L"若い頃"
		,L"幼い"
		,L"ヒーローズファンタジア"
		,L"イベント版"
		,L"本編"
		,L"が出現"
		,L"騒動となった"
		,L"初期の"
		,L"両者とも"
		,L"子供"
		,L"子役"
		,L"大人"
		,L"現代"
		,L"アーカイブ"
		,L"スペシャルアニメ"
		,L"プロサッカー選手"
		,L"パチスロ"
		,L"アダルト"
		,L"ナレーションも"
		,L"スペシャル"
		,L"兄役"
		,L"声優版"
		,L"コロちゃん役"
		,L"世界大会編"
		,L"息子"
		,L"土曜ワイド劇場"
		,L"クレジットなし"
		,L"ラジオ"
		,L"女 不明"
		,L"男 不明"
		,L"シーズン"
		,L"リマスター版"
		,L"効果音"
		,L"DC PS"
		,L"PS DC"
		,L"子犬"
		,L"PCゲーム"
		,L"原作のみ"
		,L"原作版"
		,L"原作漫画"
		,L"原作第"
		,L"原作と"
		,L"原作系"
		,L"知り合い"
		,L"ノベル"
		,L"コミック"
		,L"原作版設定"
		,L"PC版"
		,L"OVAのみ"
		,L"ラブラブおもちゃ箱のみ"
		,L"とらハ2OVAのみ"
		,L"美女"
		,L"ドラマCDのみ"
		,L"テレビアニメのみ"
		,L"多数の声優が担当"
		,L"※ドラマCDと2のみ"
		,L"ドラマCD版"
		,L"PSPゲーム版のみ"
		,L"SHINY版のみ"
		,L"アニメ版のみ"
		,L"ドラマCDのみボイスあり"
		,L"エピローグのみ"
		,L"全国編"
		,L"アニメ第一作"
		,L"アニメ第ニ作"
		,L"アニメ第三作"
		,L"アニメ第四作"
		,L"ディレクターズカット"
		,L"実写映画版"
		,L"外伝"
		,L"全作品"
		,L"FC"
		,L"SFC"
		,L"GB"
		,L"GB1"
		,L"GB2"
		,L"GBA"
		,L"N64"
		,L"Wii"
		,L"妖精"
		,L"貴族"
		,L"職人"
		,L"パチンコ"
		,L"パチンコ版"
		,L"BeeTV版"
		,NULL
	};
	for(const WCHAR** p = strW0; *p ; p++)
	{
		if ( str == *p )
		{
			return true;
		}
	}

	//先頭マッチ /^word.*/
	const WCHAR* strW1[] = {
		 L"原作では"
		,L"原作と"
		,L"現・"
		,L"過去の"
		,L"Ver."
		,L"と同棲"
		,L"大尉の"
		,L"が好き"
		,L"が怒り"
		,L"声優さん"
		,L"予見されて"
		,L"漫画版のみ"
		,L"DVD EDITION"
		,L"に置き換えられた"
		,L"原作第五期"
		,L"原作第四期"
		,L"原作第三期"
		,L"原作第ニ期"
		,L"登場するが"
		,L"テレビアニメ第"
		,L"無印"
		,L"一部ルートのみ"
		,L"NEW VISIONの"
		,L"戦闘中のみ"
		,L"外伝以外"
		,NULL
	};
	for(const WCHAR** p = strW1; *p ; p++)
	{
		if ( str.find(*p) == 0)
		{
			return true;
		}
	}

	//文章中の何処かに /*word*/
	const WCHAR* strW3[] = {
		 L"【"
		,L"】"
		,L"「"
		,L"」"
		,L"〈"
		,L"〉"
		,L"〔"
		,L"〕"
		,L"『"
		,L"』"
		,L"シリーズ"
		,L"シーズン"
		,L"エピソード"
		,L"台詞なし"
		,L"ボイスなし"
		,L"登場せず"
		,L"未公表"
		,L"だった"
		,L"名前のみ"
		,L"おまけシナリオ"
		,L"登場人物"
		,L"登場作品"
		,L"劇場版"
		,L"新II"
		,L"植物図鑑"
		,L"動物図鑑"
		,L"マンガ版"
		,L"兼任"
		,L"役として"
		,L"ワールド映画"
		,L"デスクトップアクセサリ"
		,L"スマッシュブラザーズ"
		,L"代役"
		,L"キャストは未公開"
		,L"スーパーロボット"
		,L"上映版"
		,L"厳密には"
		,L"表記なし"
		,L"分割提案"
		,L"機械化人"
		,L"いくなど"
		,L"テレビ版"
		,L"CM版"
		,L"称されていない"
		,L"知り合いで"
		,L"と恋人騒動があった"
		,L"長髪が特徴的な"
		,L"アダルト"
		,NULL
	};
	for(const WCHAR** p = strW3 ; *p ; p++)
	{
		if ( str.find(*p) != std::wstring::npos)
		{
			return true;
		}
	}
	
	//第NN話とかが書いてある文字列はゴミ
	{
		int i = 0;
		if ( str[i] == L'第' )
		{
			i++;
		}
		if ( str[i] == L'T'&& str[i+1] == L'V')
		{
			i+=2;
		}
		if ( str[i] == L'S'&& str[i+1] == L'P')
		{
			i+=2;
		}
		if ( str[i] == L'D'&& str[i+1] == L'S')
		{
			i+=2;
		}
		if ( str[i] == L'P'&& str[i+1] == L'S')
		{
			i+=2;
		}
		if ( str[i] == L'M'&& str[i+1] == L'G'&& str[i+2] == L'S')
		{
			i+=3;
		}
		if ( str[i] == L'S'&& str[i+1] == L'.')
		{
			i+=2;
		}
		bool isNumber = false;
		while(1)
		{
			if (    ((str[i] >= L'0' && str[i] <= L'9') || str[i] == L'.')
				||  ((str[i] >= L'０' && str[i] <= L'９') || str[i] == L'．') )
			{
				i++;
				isNumber = true;
			}
			else
			{
				break;
			}
		}
//		if (str[i] == L'話'|| str[i] == L'回' || str[i] == L'作' || str[i] == L'シ' || str[i] == L'巻'
//			|| str[i] == L'年' || str[i] == L'月' || str[i] == L'歳' || str[i] == L'次'|| str[i] == L'、')
		{
			if (isNumber)
			{//ゴミ 第NN話など
				return true;
			}
		}
//		if (isNumber)
//		{
//			return isNumber;
//		}
	}

	//ゴミではない.
	return false;
}
static int parseWikiWord(const std::wstring& line,int pos,std::vector<std::wstring>* outVec,bool isFirstLine)
{
	assert(line[pos] == '[');
	assert(line[pos+1] == '[');

	int endpos = line.find(L"]]",pos+2);
	if (endpos == std::wstring::npos)
	{//閉じがない
		return line.size();
	}

	std::wstring cap = line.substr(pos+2,endpos - (pos+2) );
	//クリーニング
	{
		//:en: などの多言語へのリンク
		int coronPos = cap.find(L':');
		if (coronPos != std::wstring::npos)	
		{
			cap = cap.substr(coronPos+1);
			coronPos = cap.find(L':');
			if (coronPos != std::wstring::npos)	
			{//非効率だけどこれでいいや..
				cap = cap.substr(coronPos+1);
			}
		}

		const int kakoBarPos = cap.find(L'(');
		if (kakoBarPos != std::wstring::npos)	
		{//曖昧さの解決がある
			cap = cap.substr(0,kakoBarPos);
		}

		const int orBarPos = cap.find(L'|');
		if (orBarPos != std::wstring::npos)	
		{
			cap = cap.substr(0,orBarPos);
		}
	}

	if (checkGomiString(cap) )
	{
		//nop
	}
	else if (isActorFlag(cap) || isNoActorName(cap)		//役者とかのフラグなので wikimarkをつけない。
			|| (! isFirstLine)							//最初の行ではないなら、wikimarkはつけない。
			||line[endpos+2]==L'。'						//]]。 となっているパティーンは、wikimarkをつけない。
			)
	{
		outVec->push_back( XLWStringUtil::chop(cap) );
	}
	else
	{//wikiキーワードを表す [ を先頭につけておくわ.
		outVec->push_back(L"["+ XLWStringUtil::chop(cap) );
	}
	return endpos + 2; //+2は [[ のサイズ.
}

//分離するハイフンかどうか   a-a ->NG      a - a ->OK
static bool checkSepalateHyphen(const std::wstring line,int i)
{
	assert (line[i]==L'-');

	if (i==0 || i+1>=(int)line.size())
	{
		return false;
	}
	if (!XLWStringUtil::isSpace( line[i-1] ))
	{
		return false;
	}
	if (!XLWStringUtil::isSpace( line[i+1] ))
	{
		return false;
	}
	return true;
}

static void	parseOneLine(const std::wstring& line,std::vector<std::wstring>* outVec,bool capturedhyphen);


static int parseKakoWord(const std::wstring& line,int pos,std::vector<std::wstring>* outVec)
{
	assert(line[pos] == '{');
	assert(line[pos+1] == '{');

	int endpos = line.find(L"}}",pos+2);
	if (endpos == std::wstring::npos)
	{//閉じがない
		return line.size();
	}

	std::wstring cap = line.substr(pos+2,endpos - (pos+2) );
	//一番長い分割点の入手
	std::vector<std::wstring> vec = XLWStringUtil::split_vector(L"|",cap);
	if (vec.empty())
	{
		return line.size();
	}
	auto it = vec.begin();
	if (vec.size()==1)
	{
		cap = *it;
	}
	else
	{
		it++;//最初のは{{Anchor|などのキーワードなので捨てる.
		std::vector<std::wstring>::iterator maxIT = it;
		size_t maxValue = it->size();
		for(it = maxIT+1  ; it != vec.end() ; it++ )
		{
			if ( it->size() > maxValue)
			{
				maxValue = it->size();
				maxIT = it;
			}
		}
		cap = *maxIT;
	}	

	if (checkGomiString(cap) )
	{
		//nop
	}
	else
	{//この中に更に読みが書かれていることがあるので親関数を再帰呼び出しする.
		parseOneLine(cap,outVec,false);
	}
	return endpos + 2; //+2は {{ のサイズ.
}

static void	parseOneLine(const std::wstring& line,std::vector<std::wstring>* outVec,bool isFirstLine)
{
	if (line.empty()) return;

	int startpos=0;
	int i=0;
	bool isWikiWord = false;
	for(;i<(int)line.size();i++)
	{
		if (isKugiriMoji(line[i]))
		{
			if (line[i]==L'-' )
			{
				if (!checkSepalateHyphen(line,i))
				{//無視
					continue;
				}
			}

			const std::wstring str = cleanWord(line.substr(startpos,i-startpos));
			if (str.empty())
			{
			}
			else if (checkStopString(str))
			{//。が入ってくるということは、説明文章だろうからもういいです。
				return;
			}
			else if (checkGomiString(str))
			{//無視 (checkStopStringの処理を含むので、下でやってね)
			}
			else
			{//普通のワード
				outVec->push_back(str);
				if (isFirstLine && line[i] == L'-')
				{
					outVec->push_back(L"-");
				}
			}

			if (line[i]==L'[' && line[i+1]==L'[')
			{//wiki keyword
				i = parseWikiWord(line,i,outVec,isFirstLine);
				startpos=i+1;
			}
			if (line[i]==L'{' && line[i+1]==L'{')
			{//{{ keyword
				i = parseKakoWord(line,i,outVec);
				startpos=i+1;
			}
			else
			{
				startpos=i+1;
			}
		}
	}

	if (i-startpos > 0)
	{
		const std::wstring str = cleanWord(line.substr(startpos,i-startpos));			
		if (! (str.empty() || checkGomiString(str) ) )
		{
			outVec->push_back(str);
		}
	}
}

SEXYTEST()
{

	{
		
		std::vector<std::wstring> vec;
		parseOneLine(L"[[孫悟空 (ドラゴンボール)|孫悟空]]（[[ジャスティン・チャットウィン]]、日本語吹き替え - [[山口勝平]]）",&vec,true);
		assert(vec[0] == L"[孫悟空");	//先頭の[ はwikiキーワードマーク
		assert(vec[1] == L"[ジャスティン・チャットウィン");
		assert(vec[2] == L"日本語吹き替え");
		assert(vec[3] == L"-");
		assert(vec[4] == L"[山口勝平");

	}

	{	
		std::vector<std::wstring> vec;
		parseOneLine(L" 、デスギラー将軍、（第1 - 49話）",&vec,true);
		assert(vec[0] == L"デスギラー将軍");
	}
}

//parseOneLineでつけたwikiマークがあるかどうか
static bool isWikimarkWord(const std::wstring & str)
{
	if( str.empty() ) return false;
	return  str[0] == L'[';
}

//parseOneLineでつけたwikiマークがあったら消す
static std::wstring chopWikimarkWord(const std::wstring & str)
{
	if( str.empty() ) return L"";
	if ( isWikimarkWord(str) )
	{
		return str.substr(1);
	}
	return str;
}

//アルファベットの中に、ひらがなとカタカナがはいっているか
bool isHiraKataInner(const std::wstring& str)
{
	bool ret = false;
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

		if ( XLWStringUtil::isKana(str[i]) || XLWStringUtil::isKata(str[i]) )
		{
			ret = true;
			continue;
		}
		return false;
	}
	return ret;
}

//ひらがな・カタカナ・漢字
bool isHiraKataKanjiOnly(const std::wstring& str)
{
	for(unsigned int i = 0 ; i < str.size() ; i++)
	{
		if ( XLWStringUtil::isSpace(str[i]))
		{
			continue;
		}
		if ( str[i] == L'_' || str[i] == L'.' || str[i] == L'・' || str[i] == L'＝')
		{
			continue;
		}
		if ( XLWStringUtil::isKana(str[i]) || XLWStringUtil::isKata(str[i]) || XLWStringUtil::isKanji(str[i])  )
		{
			continue;
		}
		return false;
	}
	return true;
}

//漢字+スペース
static bool isKanjiOnly2(const std::wstring& str)
{
	const WCHAR* p = str.c_str();
	for( ; *p ; p++ )
	{
		if ( XLWStringUtil::isKanji(*p))
		{
			continue;
		}
		if ( XLWStringUtil::isSpace(*p))
		{
			continue;
		}
		return false;
	}
	return true;
}

//よみがなが正しいそうかどうか検証する.
static bool checkYomigana(const std::wstring& name,const std::wstring& yomi)
{
	if ( name.empty() )
	{
		return !yomi.empty();
	}
	if (yomi.empty())
	{
		return false;
	}

	if (XLWStringUtil::isKata(name[0]) )
	{
		if (XLWStringUtil::isKata(yomi[0]))
		{
			return name[0] == yomi[0];
		}
		else if (XLWStringUtil::isKana(yomi[0]))
		{
			return name[0] == (yomi[0]+0x60);
		}
		//不明なのでただしといっておく
		return true;
	}
	else if (XLWStringUtil::isKana(name[0]) )
	{
		if (XLWStringUtil::isKata(yomi[0]))
		{
			return name[0] == yomi[0]-0x60;
		}
		else if (XLWStringUtil::isKana(yomi[0]))
		{
			return name[0] == yomi[0];
		}
		//不明なのでただしといっておく
		return true;
	}

	int nameLast = name.size() -1;
	int yomiLast = yomi.size() -1;

	if (XLWStringUtil::isKata(name[nameLast]) )
	{
		if (XLWStringUtil::isKata(yomi[yomiLast]))
		{
			return name[nameLast] == yomi[yomiLast];
		}
		else if (XLWStringUtil::isKana(yomi[yomiLast]))
		{
			return name[nameLast] == (yomi[yomiLast]+0x60);
		}
		//不明なのでただしといっておく
		return true;
	}
	else if (XLWStringUtil::isKana(name[nameLast]) )
	{
		if (XLWStringUtil::isKata(yomi[yomiLast]))
		{
			return name[nameLast] == yomi[yomiLast]-0x60;
		}
		else if (XLWStringUtil::isKana(yomi[yomiLast]))
		{
			return name[nameLast] == yomi[yomiLast];
		}
		//不明なのでただしといっておく
		return true;
	}

	//不明なのでただしといっておく
	return true;
}

SEXYTEST()
{
	bool r;
	{
		r = checkYomigana(L"あい",L"うえお");
		assert(!r);
	}
	{
		r = checkYomigana(L"あい",L"あえお");
		assert(r);
	}
	{
		r = checkYomigana(L"アい",L"アえお");
		assert(r);
	}
	{
		r = checkYomigana(L"アい",L"あえお");
		assert(r);
	}
	{
		r = checkYomigana(L"あい",L"アえお");
		assert(r);
	}
	{
		r = checkYomigana(L"亜い",L"アえお");
		assert(!r);
	}
	{//範囲外でよくわからない場合は true にする.
		r = checkYomigana(L"優子",L"優味");
		assert(r);
	}
}

//これが最良の読みなのか検証する.  先頭に愛称がかかれているパティーンがあるので。
static bool checkYomiBestmatch(std::vector<std::wstring>::const_iterator nowIT,std::vector<std::wstring>::const_iterator endIT,const std::wstring& name)
{
	int maxlength = nowIT->size()+2;//+2は先行特典のマジック
	auto bestIT = nowIT;

	for(auto it = nowIT+1 ; it != endIT ; it++ )
	{
		if( isActorFlag(*it) )
		{//役者フラグが立ってしまうと、もうでてこれないのでおしまい
			break;
		}
		if(   *it == L"読み"	)
		{//よみと書かれたものがあったらもう仕方ない
			return false;
		}
		if (! (it->size() >= 2 &&  (! isWikimarkWord(*it)) ) )
		{//よみとしてて的確かどうか
			continue;
		}
		if (!checkYomigana(name,*it))
		{//名前と比較して、このよみは不的確
			continue;
		}
//		if(! (isHiraKataOnlyEx(*it) || isHiraKataInner(*it) )  )
		if(! (isHiraKataOnlyEx(*it) )  )
		{//ひらがなとカタカタで作られていない
			continue;
		}
		//これは多分読み.
		if ( (int)it->size() > maxlength)
		{
			maxlength = it->size();
			bestIT = it;
		}
	}
	return bestIT == nowIT;
}

static void completeFantasicCharaImpl(FantasicCharaSt* outST,const std::vector<std::wstring>& vec,bool isFirstLine)
{
	for(auto it = vec.begin() ; it != vec.end() ; it++ )
	{
		if( isActorFlag(*it) )
		{//役者が複数書かれていることもあるので、この行は役者しか出てこないと仮定して読みまくる.
			while(1)
			{
				it++;
				if ( it == vec.end() ) break;
				if ( isActorFlag(*it) || isNoActorName(*it) )
				{
					it++;
					if ( it == vec.end() ) break;
				}
				if ( isDoubleSkipWord(*it) )
				{
					it++;
					if ( it == vec.end() ) break;
					it++;
					if ( it == vec.end() ) break;
				}

				const std::wstring w = chopWikimarkWord(*it);
				if ( outST->actor1.empty() )
				{
					outST->actor1 = w;
					continue;
				}
				if (outST->actor1 == w) continue;
				if ( outST->actor2.empty() )
				{
					outST->actor2 = w;
					continue;
				}
				if (outST->actor2 == w) continue;
				if ( outST->actor3.empty() )
				{
					outST->actor3 = w;
					continue;
				}
			}
			if ( it == vec.end() ) break;
			continue;
		}

		if(   *it == L"読み"	)
		{
			it++;
			if ( it == vec.end() ) break;
			if (*it == L"-")
			{
				it++;
				if ( it == vec.end() ) break;
			}
			assert( ! isWikimarkWord(*it) );
			if (!checkYomigana(outST->name,*it))
			{
				continue;
			}

			if( isHiraKataOnlyEx(*it) || isHiraKataInner(*it)   )
			{//ひらがなとカタカタで作られている
				outST->yomi = cleanSpace(*it);
			}
			continue;
		}

		if ( isFirstLine && outST->yomi.empty() && it->size() >= 2 &&  (! isWikimarkWord(*it))  ) 
		{
			if (checkYomigana(outST->name,*it))
			{
				if( isHiraKataOnlyEx(*it) || isHiraKataInner(*it)   )
				{//ひらがなとカタカタで作られている
					if (!checkYomiBestmatch(it,vec.end() ,outST->name  ))
					{
						continue;
					}
					outST->yomi = cleanSpace(*it);
					continue;			
				}
			}
		}

		if ( outST->name.empty() && it->size() >= 2)
		{
			const std::wstring name = chopWikimarkWord(*it);

			if(isKanjiOnly2(name))
			{//名前が空で、漢字のみの部分があれば、名前
				outST->name = cleanSpace(name);
				continue;
			}
			else if(isHiraKataKanjiOnly(name))
			{//名前が空で、ひらがな、カタカナ、漢字のみの部分があれば、名前
				outST->name = cleanSpace(name);
				continue;
			}
			else if(isAlphanumOnly(name))
			{//名前が空で、アルファベット等のみであれば名前かなあ
				outST->name = cleanSpace(name);
				continue;
			}
		}

		//ここまで一致しないととなると、役者フラグが立っていない役者？
		if ( outST->actor1.empty() && isWikimarkWord(*it) )
		{
			outST->actor1 = chopWikimarkWord(*it);
		}
	}

	if ( outST->yomi.size() >= 1) 
	{
		if (outST->name.empty() )
		{//読みがあるが名前がない、場合は、名前とよみが同じ
			outST->name = outST->yomi;
		}
		outST->yomi = convertPlainYomi(outST->yomi);
	}
	else
	{
		if (outST->name.size() >= 1)
		{//名前はあるが読みがない
			if(isHiraKataOnlyEx(outST->name))
			{//名前が　ひらがなとカタカナだったら読みにも入れてあげる
				outST->yomi = convertPlainYomi(outST->name);
			}
		}
	}
}


static void completeFantasicChara(const std::wstring& titleW,const std::wstring& top,const std::wstring& sub1,const std::wstring& sub2,const std::wstring& sub3,std::vector<FantasicCharaSt>* outCharaList,bool isKomokuPattern)
{
	FantasicCharaSt st;

	bool isFirstLine = true;
//	isFirstLine = !(sub1.empty() && sub2.empty() && sub3.empty());
	{
		std::vector<std::wstring> vec;
		parseOneLine(top,&vec,isFirstLine);
		completeFantasicCharaImpl(&st,vec,isFirstLine);
	}

	isFirstLine = isKomokuPattern ? true : false;
	{
		std::vector<std::wstring> vec;
		parseOneLine(sub1,&vec,isFirstLine);
		completeFantasicCharaImpl(&st,vec,isFirstLine);
	}
	{
		std::vector<std::wstring> vec;
		parseOneLine(sub2,&vec,false);
		completeFantasicCharaImpl(&st,vec,false);
	}
	{
		std::vector<std::wstring> vec;
		parseOneLine(sub3,&vec,false);
		completeFantasicCharaImpl(&st,vec,false);
	}

	if (st.name.size()>=1 )
	{
		if (!st.yomi.empty())
		{
			if ( ! checkYomigana(st.name, st.yomi) )
			{
				st.yomi = L"";
			}
		}
		outCharaList->push_back(st);
	}
}

static int trySemicolonPattern(const std::wstring& titleW,const std::wstring& innerW)
{
	//こういうセミコロンで書いてあるパティーン
	//; 赤座 あかり（あかざ あかり）
	//: 声 - [[三上枝織]]
	int pos = 0;
	int count = 0;
	
	while(1)
	{
		pos = innerW.find(L"\n;",pos);
		if (pos == std::wstring::npos)
		{
			break;
		}
		int endpos = innerW.find(L"\n",pos+2);
		if (endpos == std::wstring::npos)
		{
			endpos = innerW.size()-1;
		}
		if (innerW[endpos+1]!=L':')
		{//次の行が : で始まること.
			if (innerW[endpos+1]==L'{' && innerW[endpos+2]==L'{')
			{//ただし {{ も容認するわ.
			}
			else
			{
				pos = endpos;
				continue;
			}
		}
		pos = endpos + 2;

		count++;
	}
	return count;
}

static void parseSemicolonPattern(const std::wstring& titleW,const std::wstring& innerW,std::vector<FantasicCharaSt>* outCharaList)
{
	//こういうセミコロンで書いてあるパティーン
	//; 赤座 あかり（あかざ あかり）
	//: 声 - [[三上枝織]]
	int pos = 0;
	
	while(1)
	{
#ifdef _DEBUG
		auto aaa = innerW.substr(pos);
#endif
		pos = innerW.find(L"\n;",pos);
		if (pos == std::wstring::npos)
		{
			break;
		}
		int endpos = innerW.find(L"\n",pos+2);
		if (endpos == std::wstring::npos)
		{
			endpos = innerW.size() - 1;
		}
		if (innerW[endpos+1]!=L':')
		{//次の行が : で始まること.
			if (innerW[endpos+1]==L'{' && innerW[endpos+2]==L'{')
			{//ただし {{ も容認するわ.
			}
			else
			{
				pos = endpos;
				continue;
			}
		}
		const std::wstring top = innerW.substr(pos+2,endpos-(pos+2));

		pos = endpos + 2;
		if (innerW[pos]==L'*')
		{//:* となるので pos + 1;
			pos +=1;
		}

		endpos = innerW.find(L"\n",pos);
		const std::wstring sub1 = innerW.substr(pos,endpos==std::wstring::npos ? -1 : endpos-(pos));
		if (endpos == std::wstring::npos || innerW[endpos+1]!=L':')
		{
			completeFantasicChara(titleW,top,sub1,L"",L"",outCharaList,false);
			continue;
		}

		pos = endpos + 2;
		endpos = innerW.find(L"\n",pos);
		const std::wstring sub2 = innerW.substr(pos,endpos==std::wstring::npos ? -1 : endpos-(pos));
		if (endpos == std::wstring::npos || innerW[endpos+1]!=L':')
		{
			completeFantasicChara(titleW,top,sub1,sub2,L"",outCharaList,false);
			continue;
		}

		pos = endpos + 2;
		endpos = innerW.find(L"\n",pos);
		const std::wstring sub3 = innerW.substr(pos,endpos==std::wstring::npos ? -1 : endpos-(pos));
		completeFantasicChara(titleW,top,sub1,sub2,sub3,outCharaList,false);

		if (endpos != std::wstring::npos)
		{
			pos = endpos;
#ifdef _DEBUG
			aaa = innerW.substr(pos);
#endif
		}
		continue;
	}
}

//
//次の行を調べる
static bool nextLineForSemicolonPatternS(const std::wstring& innerW,int pos,WCHAR synbol1,WCHAR synbol2,int *outNextPos)
{
	if (pos == std::wstring::npos) return false;
	if ((int)innerW.size() <= pos+3) return false;
	int nextPos = pos;

	if (innerW[pos+1]==L'{' && innerW[pos+2]==L'{')
	{//ただし {{ も容認するわ.
		nextPos += 2;
	}
	else
	{
#ifdef _DEBUG
		auto aaa = innerW.substr(pos);
#endif
		nextPos += 1;
		if (synbol1 == 0 )
		{//何もないでいきなり文章のパティーン
			if ( (innerW[pos+1]==L':' || innerW[pos+1]==L'*' || innerW[pos+1]==L';' ) )
			{//逆に何かシンボルがあるとダメ
				return false;
			}
			//共に０以外認めない.
			assert(synbol2 == 0);
		}
		else if (innerW[pos+1]!=synbol1)
		{//シンボルではないとダメ
			return false;
		}
		else
		{
			nextPos += 1;
		}


		if (synbol2 == 0 )
		{//２つ目のシンボルチェックはしない
		}
		else if (innerW[pos+2]!=synbol2)
		{//シンボルではないとダメ
			return false;
		}
		else
		{
			nextPos += 1;
		}

		if (innerW[pos+3]==L'*')
		{//:* となるので pos + 1;
			nextPos +=1;
		}
	}

	*outNextPos = nextPos;
	return true;
}



static int trySemicolonPatternS(const std::wstring& titleW,const std::wstring& innerW,WCHAR synbol1,WCHAR synbol2)
{
	//こういうセミコロンで書いてあるパティーン
	//; 赤座 あかり（あかざ あかり）
	//: 声 - [[三上枝織]]
	int pos = 0;
	int count = 0;

	
	while(1)
	{
		pos = innerW.find(L"\n;",pos);
		if (pos == std::wstring::npos)
		{
			break;
		}
		int endpos = innerW.find(L"\n",pos+2);
		if (endpos == std::wstring::npos)
		{
			endpos = innerW.size() - 1;
		}

#ifdef _DEBUG
		auto aaa = innerW.substr(pos);
#endif
		const int startPos = pos+2; //+2は \n;
		//次の行について調べる
		bool r = nextLineForSemicolonPatternS(innerW,endpos,synbol1,synbol2,&pos);
		if (!r)
		{//次の行は違う行らしい
			pos = endpos + 1; // +1は \n
			continue;
		}

#ifdef _DEBUG
		const std::wstring top = innerW.substr(startPos,endpos-startPos);
#endif
		count ++;

		endpos = innerW.find(L"\n",pos);
#ifdef _DEBUG
		const std::wstring sub1 = innerW.substr(pos,endpos==std::wstring::npos ? -1 : endpos-(pos));
#endif
		r = nextLineForSemicolonPatternS(innerW,endpos,synbol1,synbol2,&pos);
		if (!r)
		{
			continue;
		}

		endpos = innerW.find(L"\n",pos);
#ifdef _DEBUG
		const std::wstring sub2 = innerW.substr(pos,endpos==std::wstring::npos ? -1 : endpos-(pos));
#endif
		r = nextLineForSemicolonPatternS(innerW,endpos,synbol1,synbol2,&pos);
		if (!r)
		{
			continue;
		}

		endpos = innerW.find(L"\n",pos);
#ifdef _DEBUG
		const std::wstring sub3 = innerW.substr(pos,endpos==std::wstring::npos ? -1 : endpos-(pos));
#endif
		r = nextLineForSemicolonPatternS(innerW,endpos,synbol1,synbol2,&pos);
		if (!r)
		{
			pos = endpos;
		}
		continue;
	}
	return count;
}


static void parseSemicolonPatternS(const std::wstring& titleW,const std::wstring& innerW,std::vector<FantasicCharaSt>* outCharaList,WCHAR synbol1,WCHAR synbol2)
{
	//こういうセミコロンで書いてあるパティーン
	//; 赤座 あかり（あかざ あかり）
	//: 声 - [[三上枝織]]
	int pos = 0;
	
	while(1)
	{
		pos = innerW.find(L"\n;",pos);
		if (pos == std::wstring::npos)
		{
			break;
		}
		int endpos = innerW.find(L"\n",pos+2);
		if (endpos == std::wstring::npos)
		{
			endpos = innerW.size() - 1;
		}

#ifdef _DEBUG
		auto aaa = innerW.substr(pos);
#endif
		const int startPos = pos+2; //+2は \n;
		//次の行について調べる
		bool r = nextLineForSemicolonPatternS(innerW,endpos,synbol1,synbol2,&pos);
		if (!r)
		{//次の行は違う行らしい
			pos = endpos + 1; // +1は \n
			continue;
		}
		const std::wstring top = innerW.substr(startPos,endpos-startPos);

		endpos = innerW.find(L"\n",pos);
		const std::wstring sub1 = innerW.substr(pos,endpos==std::wstring::npos ? -1 : endpos-(pos));
		r = nextLineForSemicolonPatternS(innerW,endpos,synbol1,synbol2,&pos);
		if (!r)
		{
			completeFantasicChara(titleW,top,sub1,L"",L"",outCharaList,false);
			continue;
		}

		endpos = innerW.find(L"\n",pos);
		const std::wstring sub2 = innerW.substr(pos,endpos==std::wstring::npos ? -1 : endpos-(pos));
		r = nextLineForSemicolonPatternS(innerW,endpos,synbol1,synbol2,&pos);
		if (!r)
		{
			completeFantasicChara(titleW,top,sub1,sub2,L"",outCharaList,false);
			continue;
		}

		endpos = innerW.find(L"\n",pos);
		const std::wstring sub3 = innerW.substr(pos,endpos==std::wstring::npos ? -1 : endpos-(pos));
		completeFantasicChara(titleW,top,sub1,sub2,sub3,outCharaList,false);

		r = nextLineForSemicolonPatternS(innerW,endpos,synbol1,synbol2,&pos);
		if (!r)
		{
			pos = endpos;
		}
		continue;
	}
}
//
//== と複数回の == で段落されているパティーン
static int tryKoumokuPatternN(const std::wstring& titleW,const std::wstring& innerW,int koumokuSize)
{
	//こういうタイトルとして書いてあるパティーン
	//===== {{Anchor|高坂穂乃果|高坂 穂乃果（こうさか ほのか）}} =====
	//: 声 - [[新田恵海]]
	//: 誕生日 - [[8月3日]]（獅子座）
	//: 血液型 - O型
	int pos = 0;
	int count = 0;
	std::wstring startline;
	assert(koumokuSize>=2 && koumokuSize <= 5);
	if (koumokuSize == 2) startline = L"\n==";
	else if (koumokuSize == 3) startline = L"\n===";
	else if (koumokuSize == 4) startline = L"\n====";
	else if (koumokuSize == 5) startline = L"\n=====";


	while(1)
	{
#ifdef _DEBUG
		auto aaa = innerW.substr(pos);
#endif
		pos = innerW.find(startline,pos);
		if (pos == std::wstring::npos)
		{
			break;
		}
		int endpos = innerW.find(L"\n",pos+2);
		if (endpos == std::wstring::npos)
		{
			endpos = innerW.size() - 2;
		}

		if ( innerW[pos+(koumokuSize+1)]==L'=')
		{//\n====== と = が予定数より続かないように
			pos = endpos;
			continue;
		}
		if (! (innerW[endpos+1]==L':' || innerW[endpos+1]==L'*')) 
		{//次の行が : or * で始まること.
			pos = endpos;
			continue;
		}
		std::wstring top = innerW.substr(pos+(koumokuSize+1),endpos-(pos+(koumokuSize+1)));
		top = XLWStringUtil::chop( top , L" 　\n=");

		pos = endpos + 2;
		if ( top.find(L"キャラクター") != std::wstring::npos || top.find(L"人物") != std::wstring::npos )
		{
			continue;
		}
		
		if ( XLWStringUtil::isSpace( innerW[pos]))
		{
			pos++;
		}

		//: [[ ]]\n となっている行であれば誤爆の可能性あり。
		if (innerW[pos]==L'[' && innerW[pos+1]==L'[')
		{
			int newEndpos = innerW.find(L"\n",pos+2);
			if (newEndpos == std::wstring::npos)
			{
				break;
			}
			if ( XLWStringUtil::isSpace( innerW[newEndpos]))
			{
				newEndpos--;
			}
			if ( innerW[newEndpos-1]==L']' && innerW[newEndpos-2]==L']' )
			{//ここではない
				pos = newEndpos+2;
				continue;
			}
		}

		count ++;
	}
	
	return count;
}


static void parseKoumokuPatternN(const std::wstring& titleW,const std::wstring& innerW,std::vector<FantasicCharaSt>* outCharaList,int koumokuSize)
{
	//こういうタイトルとして書いてあるパティーン
	//===== {{Anchor|高坂穂乃果|高坂 穂乃果（こうさか ほのか）}} =====
	//: 声 - [[新田恵海]]
	//: 誕生日 - [[8月3日]]（獅子座）
	//: 血液型 - O型
	int pos = 0;
	std::wstring startline;
	assert(koumokuSize>=2 && koumokuSize <= 5);
	if (koumokuSize == 2) startline = L"\n==";
	else if (koumokuSize == 3) startline = L"\n===";
	else if (koumokuSize == 4) startline = L"\n====";
	else if (koumokuSize == 5) startline = L"\n=====";


	while(1)
	{
#ifdef _DEBUG
		auto aaa = innerW.substr(pos);
#endif
		pos = innerW.find(startline,pos);
		if (pos == std::wstring::npos)
		{
			break;
		}
		int endpos = innerW.find(L"\n",pos+2);
		if (endpos == std::wstring::npos)
		{
			endpos = innerW.size() - 2;
		}
		if (innerW[pos+(koumokuSize+1)]==L'=')
		{//\n====== と = が予定数より続かないように
			pos = endpos;
			continue;
		}
		if (! (innerW[endpos+1]==L':' || innerW[endpos+1]==L'*') )
		{//次の行が : or * で始まること.
			pos = endpos;
			continue;
		}
		std::wstring top = innerW.substr(pos+(koumokuSize+1),endpos-(pos+(koumokuSize+1)));
		top = XLWStringUtil::chop( top , L" 　\n=");

		pos = endpos + 2;
		//* [[ ]]\n となっている行であれば誤爆の可能性あり。
		if (innerW[pos]==L'[' && innerW[pos+1]==L'[')
		{
			int newEndpos = innerW.find(L"\n",pos+2);
			if (newEndpos == std::wstring::npos)
			{
				break;
			}
			if ( XLWStringUtil::isSpace( innerW[newEndpos]))
			{
				newEndpos--;
			}
			if ( innerW[newEndpos-1]==L']' && innerW[newEndpos-2]==L']' )
			{//ここではない
				pos = newEndpos+2;
				continue;
			}
		}

		endpos = innerW.find(L"\n",pos);


		const std::wstring sub1 = innerW.substr(pos,endpos==std::wstring::npos ? -1 : endpos-(pos));
		if (endpos == std::wstring::npos || !(innerW[endpos+1]==L':' || innerW[endpos+1]==L'*') )
		{
			completeFantasicChara(titleW,top,sub1,L"",L"",outCharaList,true); 
			continue;
		}

		pos = endpos + 2;
		endpos = innerW.find(L"\n",pos);
		const std::wstring sub2 = innerW.substr(pos,endpos==std::wstring::npos ? -1 : endpos-(pos));
		if (endpos == std::wstring::npos || !(innerW[endpos+1]==L':' || innerW[endpos+1]==L'*') )
		{
			completeFantasicChara(titleW,top,sub1,sub2,L"",outCharaList,true); 
			continue;
		}

		pos = endpos + 2;
		endpos = innerW.find(L"\n",pos);
		const std::wstring sub3 = innerW.substr(pos,endpos==std::wstring::npos ? -1 : endpos-(pos));
		completeFantasicChara(titleW,top,sub1,sub2,sub3,outCharaList,true);
		if (endpos != std::wstring::npos)
		{
			pos = endpos;
#ifdef _DEBUG
			aaa = innerW.substr(pos);
#endif
		}
	}
}

//テーブルレイアウトされているパティーン
static int tryTablePatter(const std::wstring& titleW,const std::wstring& innerW)
{
	//{| border=1 class=wikitable style=background:#ffffff; font-size:smaller;
	//! 名前 !! 誕生日 !! 学年 !! 所属 !! 声優
	//|-
	//|colspan=5 style=background:#ffcccc; text-align:center; font-weight:bold;|SWEET
	//|-
	//|朝比奈桃子（あさひな ももこ）||7月19日||1年生||軽音楽部||[[小倉唯]] !--2期追加-- 
	//|-
	//|浅見景（あさみ けい）||4月12日||3年生||バレー部||[[伊藤美来]]
	int pos = 0;
	int count = 0;
	std::wstring startline;

	pos = innerW.find(L"wikitable");
	if(pos == std::wstring::npos)
	{
		return 0;
	}
	
	while(1)
	{
#ifdef _DEBUG
		auto aaa = innerW.substr(pos);
#endif
		pos = innerW.find(L"\n|",pos);
		if (pos == std::wstring::npos)
		{
			break;
		}
		int endpos = innerW.find(L"\n",pos+2);
		if (endpos == std::wstring::npos)
		{
			endpos = innerW.size();
		}
		if (innerW[pos+2]==L'-')
		{//\n|- は無視.
			pos = endpos;
			continue;
		}

		int n =innerW.find(L"|SWEET\n",pos+2);
		if(n!=std::wstring::npos)
		{//色とかの指定行
			pos = endpos;
			continue;
		}

//		std::wstring top = innerW.substr(pos+2,endpos-(pos+2));
//		top = XLWStringUtil::replace(top,L"||",L" ");//テーブルの区切り文字をスペースに

		pos = endpos;
		
		count ++;
	}
	
	return count;
}


//テーブルレイアウトされているパティーン
static void parseTablePattern(const std::wstring& titleW,const std::wstring& innerW,std::vector<FantasicCharaSt>* outCharaList)
{
	//{| border=1 class=wikitable style=background:#ffffff; font-size:smaller;
	//! 名前 !! 誕生日 !! 学年 !! 所属 !! 声優
	//|-
	//|colspan=5 style=background:#ffcccc; text-align:center; font-weight:bold;|SWEET
	//|-
	//|朝比奈桃子（あさひな ももこ）||7月19日||1年生||軽音楽部||[[小倉唯]] !--2期追加-- 
	//|-
	//|浅見景（あさみ けい）||4月12日||3年生||バレー部||[[伊藤美来]]
	int pos = 0;
	int count = 0;
	std::wstring startline;

	pos = innerW.find(L"wikitable");
	if(pos == std::wstring::npos)
	{
		return ;
	}
	
	while(1)
	{
#ifdef _DEBUG
		auto aaa = innerW.substr(pos);
#endif
		pos = innerW.find(L"\n|",pos);
		if (pos == std::wstring::npos)
		{
			break;
		}
		int endpos = innerW.find(L"\n",pos+2);
		if (endpos == std::wstring::npos)
		{
			endpos = innerW.size();
		}
		if (innerW[pos+2]==L'-')
		{//\n|- は無視.
			pos = endpos;
			continue;
		}

		int n =innerW.find(L"|SWEET\n",pos+2);
		if(n!=std::wstring::npos)
		{//色とかの指定行
			pos = endpos;
			continue;
		}

		std::wstring top = innerW.substr(pos+2,endpos-(pos+2));
		top = XLWStringUtil::replace(top,L"||",L" ");//テーブルの区切り文字をスペースに
		
		completeFantasicChara(titleW,top,L"",L"",L"",outCharaList,false);

		pos = endpos;
	}
}

static int tryAsterLinerPattern(const std::wstring& titleW,const std::wstring& innerW)
{
	//*で箇条書きされているパティーン
	//=== 闇の潜入捜査班 ===
	//概要：警察や民法では手に負えない謎の事件を闇の潜入捜査で事件解決に挑む。大介は潜入捜査時、警察の身分を隠す為、警察手帳と拳銃を携帯していない。
	//*山下大介警視:[[三浦洋一 (俳優)|三浦洋一]]
	//**キャリア、闇の潜入捜査班の発案者であり、リーダー。表の顔は警視庁史料編纂室・室長。格闘技の使い手の英語教師（夏休みの事件簿編では体育教師）として潜入する。実際は弱く、陰から他のメンバー（特におケイやトン子）に助けられている。
	//*松村恵子:[[伊藤かずえ]]
	//**少年院収容者、学校へ生徒としての潜入担当。コードネームは「おケイ」。次回予告の顔出しナレーションも担当。

	//== 登場人物 == などと、最初が == で始まること
	//この条件が成立しないと、parseKoumokuPattern とかとの誤爆が酷いことになるので入れておく.
	if (! (innerW[0] == L'=' && innerW[1] == L'=') )
	{
		return 0;
	}

	int pos = 0;
	int count = 0;	
	while(1)
	{
#ifdef _DEBUG
		auto aaa = innerW.substr(pos);
#endif
		pos = innerW.find(L"\n*",pos);
		if (pos == std::wstring::npos)
		{
			break;
		}
		int endpos = innerW.find(L"\n",pos+2);
		if (endpos == std::wstring::npos)
		{
			endpos = innerW.size();
		}
#ifdef _DEBUG
		auto bbb = innerW.substr(pos);
#endif
		if (innerW[pos+2]==L'*')
		{//初期ヒットは、\n* 一つだけであること \n**ではない
			pos = endpos;
			continue;
		}
		const std::wstring top = innerW.substr(pos+2,endpos-(pos+2));
		pos = endpos ;

		//箇条書きなので、文章の途中の。以外は許可しない.
		int maruPos = top.find(L"。");
		if (maruPos >= 0)
		{
			if ( (int)top.size() <= maruPos+1 || top[maruPos+1] != L'\n' )
			{
				continue;
			}
		}

		count ++;
	}

	return count;
}

static void parseAsterLinerPattern(const std::wstring& titleW,const std::wstring& innerW,std::vector<FantasicCharaSt>* outCharaList)
{
	//*で箇条書きされているパティーン
	//=== 闇の潜入捜査班 ===
	//概要：警察や民法では手に負えない謎の事件を闇の潜入捜査で事件解決に挑む。大介は潜入捜査時、警察の身分を隠す為、警察手帳と拳銃を携帯していない。
	//*山下大介警視:[[三浦洋一 (俳優)|三浦洋一]]
	//**キャリア、闇の潜入捜査班の発案者であり、リーダー。表の顔は警視庁史料編纂室・室長。格闘技の使い手の英語教師（夏休みの事件簿編では体育教師）として潜入する。実際は弱く、陰から他のメンバー（特におケイやトン子）に助けられている。
	//*松村恵子:[[伊藤かずえ]]
	//**少年院収容者、学校へ生徒としての潜入担当。コードネームは「おケイ」。次回予告の顔出しナレーションも担当。

	//== 登場人物 == などと、最初が == で始まること
	//この条件が成立しないと、parseKoumokuPattern とかとの誤爆が酷いことになるので入れておく.
	if (! (innerW[0] == L'=' && innerW[1] == L'=') )
	{
		return ;
	}

	int pos = 0;
	while(1)
	{
#ifdef _DEBUG
		auto aaa = innerW.substr(pos);
#endif
		pos = innerW.find(L"\n*",pos);
		if (pos == std::wstring::npos)
		{
			break;
		}
		int endpos = innerW.find(L"\n",pos+2);
		if (endpos == std::wstring::npos)
		{
			endpos = innerW.size();
		}
		if (innerW[pos+2]==L'*')
		{//初期ヒットは、\n* 一つだけであること \n**ではない
			pos = endpos;
			continue;
		}
#ifdef _DEBUG
		auto bbb = innerW.substr(pos);
#endif
		std::wstring top = innerW.substr(pos+2,endpos-(pos+2));
		pos = endpos;

		//箇条書きなので、文章の途中の。以外は許可しない.
		int maruPos = top.find(L"。");
		if (maruPos >= 0)
		{
			if ( (int)top.size() <= maruPos+1 || top[maruPos+1] != L'\n' )
			{
				continue;
			}
		}

		// - は、役者と誤爆するので危険なので消す.
		top = XLWStringUtil::replace(top,L" - ",L"");
		completeFantasicChara(titleW,top,L"",L"",L"",outCharaList,false);
	}
}

static int trySemiColonLinerPattern(const std::wstring& titleW,const std::wstring& innerW)
{
	//;で箇条書きされているパティーン
	//== 登場人物 ==
	//=== 教師 ===
	//; 田辺圭介（たなべ けいすけ）/24歳: 主人公。社会科の教師で、水商第一寮長、フーゾク科の担任。前の学校で生徒とのトラブルがあり都立水商へ異動。何でも首を突っ込むお節介な存在。だが、いつもそのおかげで都立水商に貢献している。酒を飲むと理性を失くすほど悪酔いし、友人を何人も失くしている。吉岡に惚れており、吉岡からも好かれているが、互いの思いには気づいていない。また複数の女性や一部の男性から好意を持たれるものの、超鈍感なために告白を受けるまでそれに気がつかなかった。最終的に吉岡と結婚し、北海道に新設された姉妹校へ転任する。
	//; 矢倉茂夫（やぐら しげお）: 都立水商の校長。いろんな意味で凄い人。お水の世界は落ちこぼれの世界ではないと常に語っており、学校のためならば自分を犠牲にもしかねない人物。

	//== 登場人物 == などと、最初が == で始まること
	//この条件が成立しないと、parseKoumokuPattern とかとの誤爆が酷いことになるので入れておく.
	if (! (innerW[0] == L'=' && innerW[1] == L'=') )
	{
		return 0;
	}

	int pos = 0;
	int count = 0;	
	while(1)
	{
#ifdef _DEBUG
		auto aaa = innerW.substr(pos);
#endif
		pos = innerW.find(L"\n;",pos);
		if (pos == std::wstring::npos)
		{
			break;
		}
#ifdef _DEBUG
		auto bbb = innerW.substr(pos);
#endif
		int endpos = innerW.find(L"\n",pos+2);
		if (endpos == std::wstring::npos)
		{
			endpos = innerW.size();
		}
#ifdef _DEBUG
		auto ccc = innerW.substr(pos);
#endif
		if (innerW[pos+2]==L';' || innerW[pos+2]==L':' )
		{//初期ヒットは、\n; 一つだけであること \n;: または \nl;; ではない
			pos = endpos;
			continue;
		}
		std::wstring top = innerW.substr(pos+2,endpos-(pos+2));
		pos = endpos ;

		// 一行が長いので、誤爆が怖いから、: または、 。で切る。
		int splitPos = 0;
		bool r = XLWStringUtil::firstfindPos(top,0,top.size(),&splitPos,NULL
			,L":"
			,L"："
			,L"。"
			);
		if (!r)
		{
			continue;
		}
		top = top.substr(0,splitPos);

		count ++;
	}

	return count;
}

static void parseSemiColonLinerPattern(const std::wstring& titleW,const std::wstring& innerW,std::vector<FantasicCharaSt>* outCharaList)
{
	//;で箇条書きされているパティーン
	//== 登場人物 ==
	//=== 教師 ===
	//; 田辺圭介（たなべ けいすけ）/24歳: 主人公。社会科の教師で、水商第一寮長、フーゾク科の担任。前の学校で生徒とのトラブルがあり都立水商へ異動。何でも首を突っ込むお節介な存在。だが、いつもそのおかげで都立水商に貢献している。酒を飲むと理性を失くすほど悪酔いし、友人を何人も失くしている。吉岡に惚れており、吉岡からも好かれているが、互いの思いには気づいていない。また複数の女性や一部の男性から好意を持たれるものの、超鈍感なために告白を受けるまでそれに気がつかなかった。最終的に吉岡と結婚し、北海道に新設された姉妹校へ転任する。
	//; 矢倉茂夫（やぐら しげお）: 都立水商の校長。いろんな意味で凄い人。お水の世界は落ちこぼれの世界ではないと常に語っており、学校のためならば自分を犠牲にもしかねない人物。

	//== 登場人物 == などと、最初が == で始まること
	//この条件が成立しないと、parseKoumokuPattern とかとの誤爆が酷いことになるので入れておく.
	if (! (innerW[0] == L'=' && innerW[1] == L'=') )
	{
		return ;
	}

	int pos = 0;
	int count = 0;	
	while(1)
	{
#ifdef _DEBUG
		auto aaa = innerW.substr(pos);
#endif
		pos = innerW.find(L"\n;",pos);
		if (pos == std::wstring::npos)
		{
			break;
		}
		int endpos = innerW.find(L"\n",pos+2);
		if (endpos == std::wstring::npos)
		{
			endpos = innerW.size();
		}
#ifdef _DEBUG
		auto bbb = innerW.substr(pos);
#endif
		if (innerW[pos+2]==L';' || innerW[pos+2]==L':' )
		{//初期ヒットは、\n; 一つだけであること \n;: または \nl;; ではない
			pos = endpos;
			continue;
		}
		std::wstring top = innerW.substr(pos+2,endpos-(pos+2));
		pos = endpos ;

		// 一行が長いので、誤爆が怖いから、: または、 。で切る。
		int splitPos = 0;
		bool r = XLWStringUtil::firstfindPos(top,0,top.size(),&splitPos,NULL
			,L":"
			,L"："
			,L"。"
			);
		if (!r)
		{
			continue;
		}
		top = top.substr(0,splitPos);
		completeFantasicChara(titleW,top,L"",L"",L"",outCharaList,false);
	}
}

//
static int tryKoumokuGAPatternN(const std::wstring& titleW,const std::wstring& innerW,int koumokuSize)
{
	//これはひどいパターン
	//項目+よみがな その後説明があり、役者情報がある特殊パティーン.
	//=== ミルフィーユ・桜葉 ===
	//（ミルフィーユ・さくらば）
	//
	//「ミルフィーユ」は[[菓子]]の[[ミルフィーユ]]より。「桜葉」は[[桜餅]]などの材料に使われる[[桜]]の葉。
	//
	//[[声優|声]]：[[新谷良子]]
	int pos = 0;
	int count = 0;
	std::wstring startline;
	assert(koumokuSize>=2 && koumokuSize <= 5);
	if (koumokuSize == 2) startline = L"\n==";
	else if (koumokuSize == 3) startline = L"\n===";
	else if (koumokuSize == 4) startline = L"\n====";
	else if (koumokuSize == 5) startline = L"\n=====";

	while(1)
	{
		pos = innerW.find(startline,pos);
		if (pos == std::wstring::npos)
		{
			break;
		}
#ifdef _DEBUG
		auto aaa = innerW.substr(pos);
#endif
		int endpos = innerW.find(L"\n",pos+2);
		if (endpos == std::wstring::npos)
		{
			endpos = innerW.size() - 2;
		}

		if ( innerW[pos+(koumokuSize+1)]==L'=')
		{//\n====== と = が予定数より続かないように
			pos = endpos;
			continue;
		}
		if ( innerW[endpos+1]==L':' || innerW[endpos+1]==L'*' || innerW[endpos+1]==L';' || innerW[endpos+1]==L'=' )
		{//次の行が :  * ; = で始ouebs
			pos = endpos;
			continue;
		}

		std::wstring top = innerW.substr(pos+(koumokuSize+1),endpos-(pos+(koumokuSize+1)));
		top = XLWStringUtil::chop( top , L" 　\n=");

		pos = endpos + 1;
		if ( top.find(L"キャラクター") != std::wstring::npos || top.find(L"人物") != std::wstring::npos )
		{
			continue;
		}

		if ( XLWStringUtil::isSpace( innerW[pos]))
		{
			pos++;
		}

		//: [[ ]]\n となっている行であれば誤爆の可能性あり。
		if (innerW[pos]==L'[' && innerW[pos+1]==L'[')
		{
			int newEndpos = innerW.find(L"\n",pos+2);
			if (newEndpos == std::wstring::npos)
			{
				break;
			}
			if ( XLWStringUtil::isSpace( innerW[newEndpos]))
			{
				newEndpos--;
			}
			if ( innerW[newEndpos-1]==L']' && innerW[newEndpos-2]==L']' )
			{//ここではない
				pos = newEndpos+2;
				continue;
			}
		}
		
		endpos = innerW.find(L"\n",pos);

		const std::wstring sub1 = innerW.substr(pos,endpos==std::wstring::npos ? -1 : endpos-(pos));
		if (endpos == std::wstring::npos || sub1.empty() )
		{
			continue;
		}

		//役者を探す。 役者はどこどだ。
		std::wstring sub2;
		bool foundactor=false;
		while(1)
		{
			pos = endpos + 2;
			if ((int)innerW.size() < pos)
			{
				break;
			}
			endpos = innerW.find(L"\n",pos);
			sub2 = innerW.substr(pos,endpos==std::wstring::npos ? -1 : endpos-(pos));
			if (endpos == std::wstring::npos)
			{
				break;
			}
			if( sub2.empty() )
			{
				continue;
			}
			if( sub2[0]==L'=')
			{//次項目っぽい
				pos -= 2;//行き過ぎた分を戻す.
				if (pos < 0 ) pos = 0;  
				break;
			}
			bool r =XLWStringUtil::firstfindPos(sub2,0,sub2.size(),NULL,NULL
					,L"声]]："
					,L"声："
					,L"声優]]："
					,L"声優："
					,L"演："

					,L"声]]:"
					,L"声:"
					,L"声優]]:"
					,L"声優:"
					,L"演:"
				);
			if (r)
			{
				foundactor=true;
				break;
			}
			continue;
		}
		
		if (!foundactor)
		{//役者がいない。
			continue;
		}

		count++;
	}

	return count;
}

static void parseKoumokuGAPatternN(const std::wstring& titleW,const std::wstring& innerW,std::vector<FantasicCharaSt>* outCharaList,int koumokuSize)
{
	//これはひどいパターン
	//項目+よみがな その後説明があり、役者情報がある特殊パティーン.
	//=== ミルフィーユ・桜葉 ===
	//（ミルフィーユ・さくらば）
	//
	//「ミルフィーユ」は[[菓子]]の[[ミルフィーユ]]より。「桜葉」は[[桜餅]]などの材料に使われる[[桜]]の葉。
	//
	//[[声優|声]]：[[新谷良子]]
	int pos = 0;
	int count = 0;
	std::wstring startline;
	assert(koumokuSize>=2 && koumokuSize <= 5);
	if (koumokuSize == 2) startline = L"\n==";
	else if (koumokuSize == 3) startline = L"\n===";
	else if (koumokuSize == 4) startline = L"\n====";
	else if (koumokuSize == 5) startline = L"\n=====";

	while(1)
	{
		pos = innerW.find(startline,pos);
		if (pos == std::wstring::npos)
		{
			break;
		}
#ifdef _DEBUG
		auto aaa = innerW.substr(pos);
#endif
		int endpos = innerW.find(L"\n",pos+2);
		if (endpos == std::wstring::npos)
		{
			endpos = innerW.size() - 2;
		}

		if ( innerW[pos+(koumokuSize+1)]==L'=')
		{//\n====== と = が予定数より続かないように
			pos = endpos;
			continue;
		}
		if ( innerW[endpos+1]==L':' || innerW[endpos+1]==L'*' || innerW[endpos+1]==L';' || innerW[endpos+1]==L'=' )
		{//次の行が :  * ; = で始ouebs
			pos = endpos;
			continue;
		}

		std::wstring top = innerW.substr(pos+(koumokuSize+1),endpos-(pos+(koumokuSize+1)));
		top = XLWStringUtil::chop( top , L" 　\n=");

		pos = endpos + 1;
		if ( top.find(L"キャラクター") != std::wstring::npos || top.find(L"人物") != std::wstring::npos )
		{
			continue;
		}

		if ( XLWStringUtil::isSpace( innerW[pos]))
		{
			pos++;
		}

		//: [[ ]]\n となっている行であれば誤爆の可能性あり。
		if (innerW[pos]==L'[' && innerW[pos+1]==L'[')
		{
			int newEndpos = innerW.find(L"\n",pos+2);
			if (newEndpos == std::wstring::npos)
			{
				break;
			}
			if ( XLWStringUtil::isSpace( innerW[newEndpos]))
			{
				newEndpos--;
			}
			if ( innerW[newEndpos-1]==L']' && innerW[newEndpos-2]==L']' )
			{//ここではない
				pos = newEndpos+2;
				continue;
			}
		}
		
		endpos = innerW.find(L"\n",pos);

		const std::wstring sub1 = innerW.substr(pos,endpos==std::wstring::npos ? -1 : endpos-(pos));
		if (endpos == std::wstring::npos || sub1.empty() )
		{
			continue;
		}

		//役者を探す。 役者はどこどだ。
		std::wstring sub2;
		bool foundactor=false;
		while(1)
		{
			pos = endpos + 2;
			if ((int)innerW.size() < pos)
			{
				break;
			}
			endpos = innerW.find(L"\n",pos);
			sub2 = innerW.substr(pos,endpos==std::wstring::npos ? -1 : endpos-(pos));
			if (endpos == std::wstring::npos)
			{
				break;
			}
			if( sub2.empty() )
			{
				continue;
			}
			if( sub2[0]==L'=')
			{//次項目っぽい
				pos -= 2;//行き過ぎた分を戻す.
				if (pos < 0 ) pos = 0;  
				break;
			}
			bool r =XLWStringUtil::firstfindPos(sub2,0,sub2.size(),NULL,NULL
					,L"声]]："
					,L"声："
					,L"声優]]："
					,L"声優："
					,L"演："

					,L"声]]:"
					,L"声:"
					,L"声優]]:"
					,L"声優:"
					,L"演:"
				);
			if (r)
			{
				foundactor=true;
				break;
			}
			continue;
		}
		
		if (!foundactor)
		{//役者がいない。
			continue;
		}
		completeFantasicChara(titleW,top,sub1,sub2,L"",outCharaList,true);
	}
}

	
//
//架空の人物としてページができているパティーン.
void parseFantasicCharaPattern(const std::wstring& titleW,const std::wstring& innerW,std::vector<FantasicCharaSt>* outCharaList)
{
	FantasicCharaSt st;
	st.name = titleW;
	st.yomi = convertPlainYomi(findYomi(titleW,innerW));
//	st.actor1 = findActor()	//とりあえず役者は空にしておこう。
//	st.actor3 = L"!!!!!!!!!!!!!!!!1";

	outCharaList->push_back(st);
}


static std::wstring makeCharaInner(const std::wstring& innerW)
{
	std::wstring w = innerW;
	w = cutWikiBlock(w,L"凡例");
	w = cutWikiBlock(w,L"ゲスト");
	w = cutWikiBlock(w,L"脚注");
	w = cutWikiBlock(w,L"参考資料");
	w = cutWikiBlock(w,L"関連項目");
	w = cutWikiBlock(w,L"参考資料");
	w = cutWikiBlock(w,L"オープニング");
	w = cutWikiBlock(w,L"エンディング");
	w = cutWikiBlock(w,L"主題歌");
	w = cutWikiBlock(w,L"テーマ曲");
	w = cutWikiBlock(w,L"関連項目");
	w = cutWikiBlock(w,L"キャラクターデザイン");
	w = cutWikiBlock(w,L"解説");
	w = cutWikiBlock(w,L"あらすじ");
	w = cutWikiBlock(w,L"スタッフ");
	w = cutWikiBlock(w,L"音楽");
	w = cutWikiBlock(w,L"スピンオフ短編");
	w = cutWikiBlock(w,L"短編");
	w = cutWikiBlock(w,L"スピンオフ");
	w = cutWikiBlock(w,L"脚注・出典");
	w = cutWikiBlock(w,L"脚注");
	w = cutWikiBlock(w,L"出典");
	w = cutWikiBlock(w,L"ストーリー");
	w = cutWikiBlock(w,L"概要");
	w = cutWikiBlock(w,L"主なスタッフ");
	w = cutWikiBlock(w,L"開発スタッフ");
	
	return w;
}

bool findFantasicCharaImpl(const std::wstring& titleW,const std::wstring& innerW,std::vector<FantasicCharaSt>* outCharaList)
{
	const std::wstring w = makeCharaInner(makeInnerTextEx(innerW));
	if (w.size() <= 20)
	{//短すぎ
		return false;
	}

	//めっちゃたくさんパティーンがあるので、一番うまくパースで来たものを採用します。
	const int koumokuPattern2 = tryKoumokuPatternN(titleW,w,2);					//2
	const int koumokuPattern3 = tryKoumokuPatternN(titleW,w,3);					//3
	const int koumokuPattern4 = tryKoumokuPatternN(titleW,w,4);					//4
	const int koumokuPattern5 = tryKoumokuPatternN(titleW,w,5);					//5
//	const int KoumokuGAPattern2 = tryKoumokuGAPatternN(titleW,w,2);				//6
//	const int KoumokuGAPattern3 = tryKoumokuGAPatternN(titleW,w,3);				//7
//	const int KoumokuGAPattern4 = tryKoumokuGAPatternN(titleW,w,4);				//8
//	const int KoumokuGAPattern5 = tryKoumokuGAPatternN(titleW,w,5);				//9
	const int semicolonPattern = trySemicolonPattern(titleW,w);					//10
	const int asterPattern = tryAsterLinerPattern(titleW,w);					//11
	const int tablePattern = tryTablePatter(titleW,w);							//12
	const int smiconlinerPattern = trySemiColonLinerPattern(titleW,w);			//13
	const int smiconSNonePattern = trySemicolonPatternS(titleW,w,0,0);			//14
	const int smiconSSCPattern = trySemicolonPatternS(titleW,w,L';',L':');		//15

	int max = 0;
	int route = 2;
	if (koumokuPattern2 > max)
	{
		max = koumokuPattern2;
		route = 2;
	}
	if (koumokuPattern3 > max)
	{
		max = koumokuPattern3;
		route = 3;
	}
	if (koumokuPattern4 > max)
	{
		max = koumokuPattern4;
		route = 4;
	}
	if (koumokuPattern5 > max)
	{
		max = koumokuPattern5;
		route = 5;
	}
	if (semicolonPattern > max)
	{
		max = semicolonPattern;
		route = 10;
	}
	if (tablePattern > max)
	{
		max = tablePattern;
		route = 12;
	}
	if (asterPattern > max)
	{
		max = asterPattern;
		route = 11;
	}
	if (smiconSNonePattern > max)
	{
		max = smiconSNonePattern;
		route = 14;
	}
	if (smiconSSCPattern > max)
	{
		max = smiconSSCPattern;
		route = 15;
	}
	if (smiconlinerPattern > max)
	{
		max = smiconlinerPattern;
		route = 13;
	}
/*
	if (KoumokuGAPattern3 > max)
	{
		max = KoumokuGAPattern3;
		route = 7;
	}
	if (KoumokuGAPattern4 > max)
	{
		max = KoumokuGAPattern4;
		route = 8;
	}
	if (KoumokuGAPattern5 > max)
	{
		max = KoumokuGAPattern5;
		route = 9;
	}
*/

	if(titleW == L"ギャラクシーエンジェルの登場人物")
	{//これはどうしようもない。強制するしかない.
		route=7;
	}
	if (titleW == L"風来のシレンのキャラクター一覧")
	{//これはどうしようもない。強制するしかない.
		route=10;
	}
	if (titleW==L"BB戦士三国伝")
	{//無理 ↓これは無理だろう。かんべんしてくれ
		//; [[劉備]]ガンダム（[[ガンダム (架空の兵器)|RX-78-2 ガンダム]]）
		//: 声 - [[梶裕貴]]
		route=0;
	}

	switch(route)
	{
	case 2: parseKoumokuPatternN(titleW,w,outCharaList,2); break;
	case 3: parseKoumokuPatternN(titleW,w,outCharaList,3); break;
	case 4: parseKoumokuPatternN(titleW,w,outCharaList,4); break;
	case 5: parseKoumokuPatternN(titleW,w,outCharaList,5); break;
	case 6: parseKoumokuGAPatternN(titleW,w,outCharaList,2); break;
	case 7: parseKoumokuGAPatternN(titleW,w,outCharaList,3); break;
	case 8: parseKoumokuGAPatternN(titleW,w,outCharaList,4); break;
	case 9: parseKoumokuGAPatternN(titleW,w,outCharaList,5); break;
	case 10: parseSemicolonPattern(titleW,w,outCharaList); break;
//	case 10: parseSemicolonPatternS(titleW,w,outCharaList,L':',0); break;
	case 11: parseAsterLinerPattern(titleW,w,outCharaList); break;
	case 12: parseTablePattern(titleW,w,outCharaList); break;
	case 13: parseSemiColonLinerPattern(titleW,w,outCharaList); break;
	case 14: parseSemicolonPatternS(titleW,w,outCharaList,0,0); break;
	case 15: parseSemicolonPatternS(titleW,w,outCharaList,L';',L':'); break;
	}
	
/*
	//デバッグ用 分類できないものをダンプ.
	if(outCharaList->empty())
	{
		if ( innerW.find(L"の登場人物") == std::wstring::npos )
		{
			if ( innerW.find(L"の登場キャラクター") == std::wstring::npos )
			{
				if ( innerW.find(L"一覧}") == std::wstring::npos )
				{
					if ( innerW.find(L"{{節stub}}") == std::wstring::npos )
					{
						XLFileUtil::write(_W2A(titleW)+".txt", _W2A(w) );
					}
				}
			}
		}
	}
*/

	return ( outCharaList->size() >= 1);
}

//架空のキャラクターのページなのかどうか
static bool isFantasicCharaPage(const std::wstring& titleW,const std::wstring& innerW)
{
	if (isAimai(innerW) )
	{
		return false;
	}

	bool r; 
	r = XLWStringUtil::firstfindPos(innerW,0,innerW.size(),NULL,NULL
		,L"架空の人物"
		,L"架空のキャラクター"
		);
	if (!r)
	{
		return false;
	}
	//これだけだと、人気漫画家とかがヒットすることがあった。大変失礼なことだww
	//もう少し、追加する.
	r = XLWStringUtil::firstfindPos(innerW,0,innerW.size(),NULL,NULL
		,L"登場する[["
		,L"登場する架空の"
		,L"登場人物。"
		,L"登場人物・"
		);
	if (!r)
	{
		return false;
	}
	r = XLWStringUtil::firstfindPos(innerW,0,innerW.size(),NULL,NULL
		,L"存命人物]]"
		,L"Infobox 漫画家"
		,L"SF漫画家"
		,L"市町村"
		,L"同人誌即売会"
		,L"学校記事"
		,L"短編小説"
		,L"女性を指す俗語"
		,L"男性を指す俗語"
		,L"人名の曖昧さ回避"
		,L"スコットランドヤード"
		,L"{{Infobox Film"
		,L"映画作品"
		,L"西遊記の成立史"
		,L"歴史上の人物"
		);
	if (r)
	{//実在する人/もの
		return false;
	}
	r = XLWStringUtil::firstfindPos(innerW,0,innerW.size(),NULL,NULL
		,L"架空の学校"
		,L"架空の病院"
		,L"架空の建物"
		,L"架空の企業"
		,L"架空の建築物"
		,L"架空の道具"
		);
	if (r)
	{//人以外は対象外.
		return false;
	}

	return true;
}



bool findFantasicChara(const std::wstring& titleW,const std::wstring& innerW,std::vector<FantasicCharaSt>* outCharaList)
{
	int pos,endpos;

	bool r = XLWStringUtil::firstfindPos(innerW,0,innerW.size(),&pos,&endpos
		,L"\n== 登場人物 =="
		,L"\n== 主要登場人物 =="
		,L"\n== 主要人物 =="
		,L"\n== 登場キャラクター =="
		,L"\n== 主な登場人物 =="
		,L"\n== 主な人物 =="
		,L"\n== 主なキャラクター =="
		,L"\n== 主要キャラクター =="

		,L"\n==登場人物=="
		,L"\n==主要登場人物=="
		,L"\n==主要人物=="
		,L"\n==登場キャラクター=="
		,L"\n==主な登場人物=="
		,L"\n==主な人物=="
		,L"\n==主なキャラクター=="
		,L"\n==主要キャラクター=="
		);
	if (r)
	{//登場人物の項目があった
		endpos = innerW.find(L"\n== ",endpos);
		if (endpos == -1)
		{
			return findFantasicCharaImpl(titleW,innerW.substr(pos),outCharaList);
		}
		else
		{
			return findFantasicCharaImpl(titleW,innerW.substr(pos,endpos-pos),outCharaList);
		}
	}

	//登場人物の段落か現れなかったけど、タイトルには、登場人物とある場合はいろいろなフォーマットを試してみる.
	r = XLWStringUtil::firstfindPos(titleW,0,titleW.size(),NULL,NULL
		,L"の登場人物"
		,L"のキャラクター"
		,L"の登場キャラクター"
		,L"オリジナルキャラクター"
		);
	if (r)
	{//登場人物の紹介ページらしい
		return findFantasicCharaImpl(titleW,innerW,outCharaList);
	}

	//それでもダメなら、カテゴリタグが入っているかどうかを見る.
	r = XLWStringUtil::firstfindPos(innerW,0,innerW.size(),NULL,NULL
			,L"[[Category:アニメの登場人物の一覧]]"
			,L"[[Category:漫画の登場人物の一覧]]"
			,L"[[Category:テレビドラマの登場人物の一覧]]"
			,L"[[Category:登場キャラクターの一覧]]"
			,L"[[Category:ライトノベルの登場人物の一覧]]"
			,L"[[Category:文学の登場人物の一覧]]"
			,L"[[Category:ゲームの登場人物の一覧]]"
			,L"[[Category:映画の登場人物の一覧]]"
		);
	if (r)
	{//登場人物の項目があった
		return findFantasicCharaImpl(titleW,innerW,outCharaList);
	}

	//個別のキャラクタとしてあるのかなあ・・・
	if ( isFantasicCharaPage(titleW,innerW) )
	{
		parseFantasicCharaPattern(titleW,innerW,outCharaList);
		return ( outCharaList->size() >= 1);
	}

	return false;
}

static void AnalizeChara(const std::wstring& titleW,const std::wstring& innerW)
{
	if ( isRedirect(innerW) || isAimai(innerW) )
	{//リダイレクトと曖昧さの解決は処理しない
		return;
	}

	const std::wstring _TitleW = cleaningInnerText(titleW);
	//曖昧さの解決のための()があったら消します.
	const std::wstring clipTitleW = snipAimaiKako(titleW);
	if (clipTitleW.empty())
	{
		return;
	}

	std::vector<FantasicCharaSt> charaList;
	bool r = findFantasicChara(clipTitleW,innerW,&charaList);
	if (!r)
	{
		return ;
	}
	
	//結果の表示.
	for(auto it = charaList.begin() ; it != charaList.end() ; it ++ )
	{
		std::wstring yomiW = it->yomi;
		std::wstring attributeW ;

		if ( Option::m()->getAimai() == Option::TypeAimai_Del)
		{
			attributeW = it->name
				+ L"\t" + clipTitleW
				+ L"\t" + it->actor1
				+ L"\t" + it->actor2
				+ L"\t" + it->actor3
			;
		}
		else
		{
			attributeW = it->name
				+ L"\t" + _TitleW
				+ L"\t" + it->actor1
				+ L"\t" + it->actor2
				+ L"\t" + it->actor3
			;
		}

		if ( Option::m()->getShow() == Option::TypeShow_NoEmpty)
		{//空ならば表示しない
			if (yomiW.empty() )
			{
				continue;
			}
		}
		else if ( Option::m()->getShow() == Option::TypeShow_Empty)
		{//空だけ表示する
			if ( !yomiW.empty() )
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

		wprintf(L"%ls	%ls\n",yomiW.c_str(),attributeW.c_str() );
	}


}


static void ReadAllForChara(FILE* fp)
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

				if ( checkGomiPage(titleW) )
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
				AnalizeChara(titleW,innerW);
			}
		}
	}
}

void wiki2charaConvert(const std::string& filename)
{
	
	FILE * fp = NULL;

	fp = fopen(filename.c_str() , "rb");
	if (!fp)
	{
		fwprintf(stderr,L"can not open %ls file\n" , _A2W(filename).c_str() );
		return ;
	}
	AutoClear ac([&](){ fclose(fp); });
	
	ReadAllForChara(fp);
}

SEXYTEST()
{



	{
		//こんなのパース出るわけ無いだろう。[[劉備]]ガンダムとか、いいかげんにしろ
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"BB戦士三国伝",L">\n; [[劉備]]ガンダム（[[ガンダム (架空の兵器)|RX-78-2 ガンダム]]）\n: 声 - [[梶裕貴]]\n: '''龍帝を継ぐ者'''。本作の[[主人公]]。三璃紗の北部、幽州の楼桑村出身の若きサムライ。かつての白龍頑駄無（白龍大帝）同様、遥か昔に三璃紗を治めていた古代の英雄・三侯のひとりである'''龍帝'''の血をひき、その魂を受け継ぐ若者。決して悪を許さず、正義のためには何者にも恐れず立ち向かう侠の中の侠で、義に厚く頼れる兄貴分として慕われ、彼の元に多くの武将達が集う。曲がったことが大嫌いな熱血漢だが、当初は若さと義侠心ゆえに直情のまま行動しがちであった。普段は優しい陽気な性格だが、楽天的なお人よしでもあるため、ややドジで[[天然ボケ]]な一面もある。\n: [[桃園の誓い]]の後に反董卓連合に参加し、虎牢城の戦いでは後に宿敵となる曹操を始めとする様々な侠たちの信念や死とぶつかり戸惑っていたが、己が正義を定め、戦いに身を投じることを決意する。",&charaList);
		assert(charaList.size() == 0);
	}
	{
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"風来のシレンのキャラクター一覧",L"\n\n{{Pathnav|風来のシレン|frame=1}}\n'''風来のシレンのキャラクター一覧'''（ふうらいのシレンのキャラクターいちらん）は、[[チュンソフト]]から発売されている[[ローグライクゲーム]]、不思議のダンジョンシリーズの『[[風来のシレン]]』に登場するキャラクター一覧。\n\n以下は、本シリーズ作品に登場する、架空のキャラクターの一覧である。括弧内は登場作品。声優は現在パチンコのみ存在。\n\nなお、作品名は以下の略称を用いる。\n\n* SFC: [[不思議のダンジョン2 風来のシレン]]\n* GB1: [[不思議のダンジョン 風来のシレンGB 月影村の怪物]]\n* GB2: [[不思議のダンジョン 風来のシレンGB2 砂漠の魔城]]\n* N64: [[不思議のダンジョン 風来のシレン2 鬼襲来!シレン城!]]\n* 外伝: [[不思議のダンジョン 風来のシレン外伝 女剣士アスカ見参!]]\n* DS: [[不思議のダンジョン 風来のシレンDS]]\n* Wii: [[不思議のダンジョン 風来のシレン3 からくり屋敷の眠り姫]]\n* DS2: [[不思議のダンジョン 風来のシレンGB2 砂漠の魔城|不思議のダンジョン 風来のシレンDS2 砂漠の魔城]]\n* 4: [[不思議のダンジョン 風来のシレン4 神の眼と悪魔のヘソ]]\n* 5: [[不思議のダンジョン 風来のシレン5 フォーチュンタワーと運命のダイス]]\n\n== 主人公 ==\n; シレン (外伝以外の全作品)\n: 友の形見である[[三度笠]]と縞[[合羽]]を身に纏い、語り[[イタチ]]のコッパと共にこばみ谷へとやってきた風来人。男性。今は亡き友？との約束を果たすため、テーブルマウンテンの頂上にあるという「太陽の大地」を目指す（SFC、DS）。幼少期にはナタネ村で城を築き（N64）、太陽の大地への到達後は月影村にて怪物オロチを倒し（GB1）、その後迷い込んだ砂漠の中にあった魔城に挑む（GB2）など、数々の冒険を乗り越える風来人。特殊な場合を除けば、喋ることはない（選択肢の会話によると一人称は「俺」と思われる）。3の過去世界によると、先祖は豪族であるかぐやの父に仕える武士だった。かぐやの悲劇の後は暇乞いし、風来人に。以後、先祖代々風来人をしている。\n; アスカ (外伝)\n: [[声優|声]] - [[日笠陽子]] (パチンコ)\n: かつてナタネ村の騒動のとき（N64）に、シレンと共に旅をしたことがある風来人。女性。修行の旅の途中で天輪国に訪れた際、シレンの相棒コッパと再会する。\n",&charaList);
		assert(charaList.size() == 2);
		assert(charaList[0].name == L"シレン");
		assert(charaList[0].yomi == L"シレン");
		assert(charaList[0].actor1 == L"");
		assert(charaList[0].actor2== L"");
		assert(charaList[1].name == L"アスカ");
		assert(charaList[1].yomi == L"アスカ");
		assert(charaList[1].actor1 == L"日笠陽子");
		assert(charaList[1].actor2== L"");
	}
	{
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"桜Trick",L"\n== 登場人物 ==\n; 高山 春香（たかやま はるか）\n: [[声優|声]] - [[戸松遥]]\n: 誕生日：[[8月25日]] / 星座：[[処女宮|乙女座]]<ref name=\"ikkan\">{{Cite book|和書|author=タチ|title=桜Trick 1|page=2|publisher=[[芳文社]]|isbn=978-4-8322-4187-9|date=2012-09-11}}</ref> / 血液型：A型 / 身長：158㎝ / 体重：47㎏<ref name=\"sirabus\">『桜Trick TVアニメ公式ガイドブック 縲怎qミツのシラバス縲怐x</ref>\n: 本作の主人公。一人称は「私」。美里西高校の一年生。大きな白い[[リボン]]がトレードマーク。ニックネームは「春ぽっぽ」。\n; 園田 優（そのだ ゆう）\n: 声 - [[井口裕香]]\n: 誕生日：[[6月24日]] / 星座：[[巨蟹宮|蟹座]]<ref name=\"ikkan\" />/血液型：AB型 / 身長：149㎝ / 体重：42㎏<ref name=\"sirabus\"/>\n: 本作のもう一人の主人公。一人称は「私」。春香の親友でクラスメイト。花の髪飾り（蛍光塗料付き）がトレードマーク。\n\n",&charaList);
		assert(charaList[0].name == L"高山春香");
		assert(charaList[0].yomi == L"たかやまはるか");
		assert(charaList[0].actor1 == L"戸松遥");
		assert(charaList[0].actor2== L"");
		assert(charaList[1].name == L"園田優");
		assert(charaList[1].yomi == L"そのだゆう");
		assert(charaList[1].actor1 == L"井口裕香");
		assert(charaList[1].actor2== L"");
		assert(charaList.size() == 2);
	}
	{
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"都立水商!",L"\n== 登場人物 ==\n=== 教師 ===\n; 田辺圭介（たなべ けいすけ）/24歳: 主人公。社会科の教師で、水商第一寮長、フーゾク科の担任。前の学校で生徒とのトラブルがあり都立水商へ異動。何でも首を突っ込むお節介な存在。だが、いつもそのおかげで都立水商に貢献している。酒を飲むと理性を失くすほど悪酔いし、友人を何人も失くしている。吉岡に惚れており、吉岡からも好かれているが、互いの思いには気づいていない。また複数の女性や一部の男性から好意を持たれるものの、超鈍感なために告白を受けるまでそれに気がつかなかった。最終的に吉岡と結婚し、北海道に新設された姉妹校へ転任する。\n; 矢倉茂夫（やぐら しげお）: 都立水商の校長。いろんな意味で凄い人。お水の世界は落ちこぼれの世界ではないと常に語っており、学校のためならば自分を犠牲にもしかねない人物。\n\n",&charaList);
		assert(charaList[0].name == L"田辺圭介");
		assert(charaList[0].yomi == L"たなべけいすけ");
		assert(charaList[0].actor1 == L"");
		assert(charaList[0].actor2== L"");
		assert(charaList[1].name == L"矢倉茂夫");
		assert(charaList[1].yomi == L"やぐらしげお");
		assert(charaList[1].actor1 == L"");
		assert(charaList[1].actor2== L"");
		assert(charaList.size() == 2);
	}
	{
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"ホワイトアウト",L"\n== 登場人物 ==\n=== 奥遠和ダム ===\n;富樫輝男：[[織田裕二]]\n;:奥遠和ダム作業員。事件前年[[11月]]、山に来ていた遭難者(実はテロリストのメンバー)を助けるために同僚の吉岡和志と救助に出かけたが、吉岡が死亡、彼が生き残った。吉岡の婚約者である平川千晶らを助けるためにたった一人でテロリストに挑む。\n;:漫画版ではある程度AKを使いこなしているような描写がされたが、映画版では演じた織田の意向から、銃の扱いは素人である描写がされている。\n;平川千晶：[[松嶋菜々子]]\n;:東京で仕事をしている、吉岡和志の婚約者。彼がどのような仕事をしていたのかに対する関心から、奥遠和を訪れるが、そこで人質になり給仕係としてこき使われてしまう。\n;:テロリストのメンバーの一人である、笠原についてはその異質性を見抜いていた。また、映画では終始富樫の事を「吉岡を見殺しにした」として憎んでおり、作中では健二を通して手に入れた木嶋のAKで桑名を射殺するシーンが追加されている。\n;吉岡和志：[[石黒賢]]\n;:富樫の同僚で千晶の婚約者。原作の中では、富樫を描いたところでは「吉岡」、千晶を描いたところでは「和志」と書かれている。遭難者救出の最中に足を折って富樫に救助を呼ぶように指示し、遭難者2名と共にビバークするもその後遺体となって収容されてしまう。原作では遭難者の一人を運んでいる最中に強風にあおられて坂道を転落して足を折るが、映画では背負っていた遭難者が突然暴れ出した為に坂道を転落して足を折るという描写になっている。また、漫画では足場の雪が突然崩れ、これに巻き込まれて足を折るという描写になっている。\n",&charaList);
		assert(charaList.size() == 3);

		assert(charaList[0].name == L"富樫輝男");
		assert(charaList[0].yomi == L"");
		assert(charaList[0].actor1 == L"織田裕二");

		assert(charaList[1].name == L"平川千晶");
		assert(charaList[1].yomi == L"");
		assert(charaList[1].actor1 == L"松嶋菜々子");
	}
	{
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"X JAPAN Virtual Shock 001",L"\n== 登場人物 ==\n;主人公\n:プレーヤー自身。Ｘ ＪＡＰＡＮのライブチケットを入手できなかったが、カメラマンと勘違いされてドーム内に潜入することになる。\n=== X JAPANメンバー ===\n;TOSHI\n:彼の楽屋に入るとスタッフからの電話が鳴り、東京ドームの４階へ行く。この時、Ｔｏｓｈｉがカメラ目線で最初と最後の時にピースサインをするので、必ず撮影しておく必要がある。ちなみに彼の楽屋にはアコースティックギターが置いてある。\n;hide\n:日本酒が好物。彼のイベントで日本酒を渡すと主人公を気に入り、ｈｉｄｅと日本酒を飲む。間違ったアイテムを渡すと最悪の場合怒り出してしまいゲームオーバーになる。ちなみにワインを渡してしまう場合、彼の名演技を見る事ができるが、これもゲームオーバーになるので、注意が必要。\n;PATA\n;HEATH",&charaList);
		assert(charaList.size() == 3);
		assert(charaList[0].name == L"主人公");
		assert(charaList[0].yomi == L"");
		assert(charaList[0].actor1 == L"");

		assert(charaList[1].name == L"TOSHI");
		assert(charaList[1].yomi == L"");
		assert(charaList[1].actor1 == L"");

		assert(charaList[2].name == L"hide");
		assert(charaList[2].yomi == L"");
		assert(charaList[2].actor1 == L"");
	}

	{// === と = 3つで書くパティーン
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"いないいないばあっ",L"\n== 登場キャラクター ==\n* '''ワンワン'''、'''おねえさん'''、'''操り人形キャラクター'''が番組を繰り広げている。 !-- \n\n各出演者の経歴・特性、個人的なエピソード、私生活の様子などは、各出演者個人のページへお願いします。以上の件に関しては[[ノート:いないいないばあっ!#過剰な内容の整理・一部転記提案について]]をご参照ください。 -- \n\n=== ワンワン ===\n* 声・操演 - [[チョー (俳優)|チョー]]\n* 1996年の番組開始から参加している大きな犬の男の子。一人称は「ワンワン」。好きな食べ物は納豆。犬が苦手。[[ラテン文字]]表記は「Wanwan」。初期設定ではペンタの魔法で羊に変身した犬で年齢は5歳らしい。\n* 声優であるチョー（旧芸名：長島雄一）'''自らが入って操演し、同時に話している'''珍しいケースである。これは、『[[おはよう!こどもショー]]』（[[日本テレビ放送網|日本テレビ]]）のロバくん（操演・声は共に[[愛川欽也]]）に影響を受けたものである。\n\n=== おねえさん ===\n* '''ゆうなちゃん'''（[[杉山優奈]]）\n** [[2011年]]（平成23年）[[3月28日]]より、先代ことちゃんに代わって仲間入りした5代目おねえさん。\n** 開始当時小学3年生の8歳。\n** 衣装は花をイメージした黄色を基調としている。",&charaList);
		assert(charaList[0].name == L"ワンワン");
		assert(charaList[0].yomi == L"ワンワン");
		assert(charaList[0].actor1 == L"チョー");
		assert(charaList[0].actor2== L"");

		assert(charaList[1].name == L"おねえさん");
		assert(charaList[1].yomi == L"おねえさん");
		assert(charaList[1].actor1 == L"杉山優奈");
		assert(charaList[1].actor2== L"");
		assert(charaList.size() == 2);
	}
	{//テーブルレイアウトパティーン
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"ガールフレンド(仮)",L"\n== 登場キャラクター ==\n登場するキャラクターは、一部の限定カードを除き'''聖櫻学園'''の生徒・関係者となっている。（プレイヤーキャラクターは2年生の設定）\n\n{| border=1 class=wikitable style=background:#ffffff; font-size:smaller;\n! 名前 !! 誕生日 !! 学年 !! 所属 !! 声優\n|-\n|colspan=5 style=background:#ffcccc; text-align:center; font-weight:bold;|SWEET\n|-\n|朝比奈桃子（あさひな ももこ）||7月19日||1年生||軽音楽部||[[小倉唯]] !--2期追加-- \n|-\n|浅見景（あさみ けい）||4月12日||3年生||バレー部||[[伊藤美来]]\n|-",&charaList);
		assert(charaList.size() == 2);
		assert(charaList[0].name == L"朝比奈桃子");
		assert(charaList[0].yomi == L"あさひなももこ");
		assert(charaList[0].actor1 == L"小倉唯");
		assert(charaList[0].actor2 == L"");

		assert(charaList[1].name == L"浅見景");
		assert(charaList[1].yomi == L"あさみけい");
		assert(charaList[1].actor1 == L"伊藤美来");
		assert(charaList[1].actor2 == L"");
	}
	{//{{独自研究}}{{出典の明記}}に邪魔されるパティーン せめてもっと下に書いてくれよ
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"ご注文はうさぎですか？",L"\n== 登場人物 ==\n{{独自研究|section=1|date=2014年10月}}\n{{出典の明記|date=2014年10月}}\n登場人物の名前の多くは喫茶店で供される飲み物に由来している。（後述） \n=== ラビットハウス ===\n; ココア / 保登 心愛（ほと ここあ）\n: [[声優|声]] - [[佐倉綾音]]\n: 血液型：B型、誕生日：[[4月10日]]、身長：154cm。\n: 本作の主人公。15歳の高校1年。半分になった花の形の髪飾りがトレードマーク。\n: 高校入学を機に木組みの家と石畳の街に引っ越してきて、喫茶店ラビットハウスに下宿している。当初は産地や銘柄による味や香りの違いを判別できないほどコーヒーには疎かったが、ラビットハウスの店員として働くうちに喫茶店に相応の技能を身につけていった。\n: とても前向きで明るく朗らかな性格をしているが少々ドジなところもあり、チノやリゼにフォローされることが多い。可愛い物やモフモフしたものが大好き。実家では4人兄妹の末っ子だったために妹という存在に憧れており、チノのことを実の妹のように可愛がっている。そのため、チノが他の誰かに妹扱いされたり、他の誰かを姉扱いして接したりしていると、動揺して泣いてしまうこともある。しかし、チノのクラスメートであるマヤ、メグに対しても姉のように振る舞うことから、チノには「年下なら誰でもいい」と不満を持たれてしまったこともあった。\n; チノ / 香風 智乃（かふう ちの）\n: 声 - [[水瀬いのり]]\n: 血液型：AB型、誕生日：[[12月4日]]、身長：144cm。\n: ラビットハウスオーナーの孫娘。13歳の中学2年。髪の左右にヘアピンを×印状に付けている。学校へ行くとき以外はいつもティッピーというアンゴラうさぎを頭に乗せている。\n: 香りだけで産地や銘柄を当てられるほどコーヒーに精通しているが、砂糖とミルクがないと飲めないなど、歳相応な面もある。\n: 誰にでも敬語で話し、どちらかと言えばクールな性格をしており、当初は勝手に妹扱いしてくるハイテンションなココアに困惑していたが、段々とまんざらでもない態度を見せるようになり、ココアが新しく妹にしたクラスメートのマヤやメグに嫉妬したこともあった。\n; リゼ / 天々座 理世（てでざ りぜ）<ref>[[Koi]] 『ご注文はうさぎですか？ volume.1』[[芳文社]]、6頁より。天手座という表記がみられる場合もある。</ref>\n: 声 - [[種田梨沙]]\n: 血液型：A型、誕生日：[[2月14日]]、身長：160cm。\n: ラビットハウスのバイト店員。16歳の高校2年。登場人物の中では大人を除けば最年長であるが、年長者扱いを受けることはあまり多くなく、ココアからも当初は年上と思われていなかった。髪型はツインテール。軍人の娘に生まれ、常にモデルガンを所持しているなどワイルドで男勝りな性格をしている。容姿端麗で周りからもスタイルのいい人という印象を持たれている。\n: 幼い頃から護身術などを叩きこまれていたためか、少女にも関わらず相当な力持ちであり、重い荷物も楽々と持ち上げる。また、暗記力もあり手先も器用なので喫茶店の仕事もそつなくこなす。普通の女の子っぽさに憧れている一面があり、素直な気持ちでチノと仲良くするココアを羨ましいと思ったり、そんな自分を寂しいと思ったりすることもある。\n; ティッピー\n: 声 - [[清川元夢]]\n: 本名はティッピーゴールデンフラワリーオレンジペコ。ラビットハウスで飼われている[[アンゴラウサギ|アンゴラうさぎ]]（[[:en:Angora rabbit]]）。普段はチノの頭の上に乗っていることが多く、モフモフとした姿からココアに好かれている。\n: 実はティッピーこそがラビットハウスのオーナー、つまりチノの祖父なのだが、今はメスうさぎの姿になっている。現状のようになった理由や経緯は物語中では明かされていないが、劇中のチノの発言などによると祖父自身は亡くなっており、ティッピーは生前からラビットハウスで飼われていた。そのためココアなど若い女の子に抱きつかれたりすると困惑することもある。生前にココアと出会っておりその際もうさぎになりたいと思っていたが、「夏は暑そうだから」とティッピーのようなうさぎにはなりたくなかったらしい。\n: 人間だった頃は粋人だったらしく、チェスを嗜んだりもしていた。うさぎとしては珍しい品種であるためか、犬と間違われたり、単なる毛玉扱いされることも少なくない。正体は家族以外には秘密らしく、前述のように普段声を発するときはチノの腹話術ということにしている。\n: 声 - [[速水奨]]\n: ラビットハウスのバータイムのマスターで、とてもダンディな容姿と性格をしている。フルネームは「香風タカヒロ(漢字表記は不明)」。チノの友達などにも「渋くてカッコいい」と言われているが手作りのお弁当は可愛らしい物が多く、うさぎ柄のネクタイを喜ぶなど、ファンシーな事物も好む。\n: 青山ブルーマウンテンの書いた小説の登場人物のモデルになっており、チノによるとかつて経営難だったラビットハウスを得意のジャズで盛り上げて救ったことがあるらしい。リゼの父親とは戦場での昔馴染みであり、ティッピー秘蔵のワインを勝手に譲ったこともある。\n\n=== 甘兎庵 ===\n; 宇治松 千夜（うじまつ ちや）\n: 声 - [[佐藤聡美]]\n: 血液型：O型、誕生日：[[9月19日]]、身長：157cm。\n: 和風喫茶甘兎庵の娘で、ココアのクラスメイト。15歳の高校1年。黒髪の長髪で姫カットな容姿に違わぬ大和撫子然とした性格をしており面倒見もいいが、やや早とちりなところがある。\n: シャロとは幼馴染であり、彼女のフォローに回ることも少なくないが逆にシャロが可愛い（というより面白い）あまり、彼女をからかうことも多くシャロに怒られる。\n: \n:; あんこ\n:: 甘兎庵の看板うさぎ。恥ずかしがり屋のオスで基本的に置物と思われるほど大人しいがシャロやティッピーを見ると勢いよく飛びつく。\n:: 千夜曰く羊羹を食べるらしく、他のうさぎも羊羹を食べるものと思っていた。\n:: 小さい頃、シャロをよく噛んでいたためシャロはうさぎ嫌いになり、現在もあんこを苦手としている。\n\n; シャロ / 桐間 紗路（きりま しゃろ）\n: 声 - [[内田真礼]]\n: 血液型：A型、誕生日：[[7月15日]]、身長：151cm。\n: 甘兎庵の隣に住んでいる少女で、リゼの後輩。15歳の高校1年。ウェーブの掛かった(くせ毛)金髪でカチューシャをよく着用している。\n: 格好いい先輩であるリゼのことを慕っており、不良野良うさぎに襲われそうになったところを助けてもらったことから親しくなった。\n: お嬢様学校に通っているものの実家は古びていて小さく、千夜の家の隣にあるということもあって、ココアはシャロの家を「千夜ちゃんちの物置」と勘違いしていた。貧乏ゆえに普段はハーブティ専門の喫茶店であるフルール・ド・ラパン、[[クレープ]]の屋台、[[ジェラート]]スタンドなど各種バイトに大忙しの毎日を送っている。お嬢様のような身なりをしているため、ココアやチノにはお金持ちだと勘違いされており、リゼに貧乏であることを知られたくないこともあって本当のことを言えないでいたが、家から出てくるところを偶然目撃されたため打ち明けることとなった。\n:; ワイルドギース\n:: シャロの家に住み着いたうさぎ。右目の十字傷が特徴的な強面で、野良だった頃は「不良野良うさぎ」としてシャロから恐れられていたが、家賃代わりに草（シャロが庭で育てていたハーブ）を置くなどなかなかに義理堅い性格をしている。\n:: ワイルドギースという名前は元々リゼの持っているうさぎのぬいぐるみのもので、その名前に反応したこともあって命名された。ティッピーやあんこと違い家の外にあるうさぎ小屋に住んでいるが、家に入って来てはシャロにじゃれついている。食事に関してはシャロとの間に各自調達という約束が交わされているらしい。\n\n=== 街の人々 ===\n; マヤ / 条河 麻耶（じょうが まや）\n: 声 - [[徳井青空]]\n: 血液型：O型、誕生日：[[8月8日]]、身長：140cm。\n: チノのクラスメートで、彼女よりも小柄な女の子。[[八重歯]]が特徴的。サッパリとした性格をしており、年上相手にもくだけた口調で話すことが多い。\n: テレビで見聞きした[[CQC]]ができるといったことから、年下ながら軍の関係者とリゼに勘違いされ、そのまま親近感を持たれてしまっている。そんなリゼに違和感を覚えてはいるものの、特に訂正する機会もなく、勘違いされていることに気付いてもいない。\n: リゼからはチノ・マヤ・メグの頭文字を取って「チマメ（隊）」と呼ばれている。\n; メグ / 奈津 恵（なつ めぐみ）\n: 声 - [[村川梨衣]]\n: 血液型：A型、誕生日：[[11月2日]]、身長：145cm。\n: チノのクラスメートで、彼女と同じぐらいの体格の女の子。礼儀正しくおっとりとした性格をしており、優しく、パン作りが上手なココアのことを「素敵な人」だと思い目標にしている。\n: 手先はさほど器用な方ではなく、お菓子作りには成功したが、ココアに制服を着せてあげることができないなど、失敗も多い。\n; 青山 ブルーマウンテン（あおやま ブルーマウンテン）/ 青山 翠（あおやま みどり）\n: 声 - [[早見沙織]]\n: 血液型：B型、誕生日：[[10月27日]]、身長：163cm。\n: ココアが公園で出会った女流小説家。学生時代にラビットハウスの常連だったことがあり、今は亡きマスターに勧められたことから小説の投稿を始め、小説家になった。\n; リゼの父\n: 左目に眼帯をしている。チョイ悪な雰囲気だが普通の父親らしく娘の将来を心配するシーンが見受けられる。\n\n== 舞台 ==\n",&charaList);
		assert(charaList.size() == 10);
		assert(charaList[0].name == L"保登心愛");
		assert(charaList[0].yomi == L"ほとここあ");
		assert(charaList[0].actor1 == L"佐倉綾音");
		assert(charaList[0].actor2 == L"");

		assert(charaList[1].name == L"香風智乃");
		assert(charaList[1].yomi == L"かふうちの");
		assert(charaList[1].actor1 == L"水瀬いのり");
		assert(charaList[1].actor2 == L"");
	}
	{//{{独自研究}}に邪魔されるパティーン せめてもっと下に書いてくれよ
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"機動戦士ガンダムSEED ASTRAYシリーズの登場人物",L"\n== 民間人 ==\n=== ジャンク屋 ===\n==== ロウ・ギュール ====\n{{独自研究|section=1|date=2008年10月}}\n: 声：[[小野坂昌也]]\n:【性別：男性・[[コーディネイターとナチュラル|ナチュラル]] / 年齢：18歳 / 所属：[[コズミック・イラの勢力#ジャンク屋組合|ジャンク屋組合]] / 搭乗機：[[地球連合軍の機動兵器#ミストラル|ミストラル]]、[[ガンダムアストレイ#レッドフレーム|レッドフレーム]]、[[ガンダムアストレイ#改|レッドフレーム改]]、[[ザフトの機動兵器#ザウート|ザウート]] / 登場作品：○|○|○|○|○|○|○|-】\n: 第1作『ASTRAY』の主人公で、全作に登場。[[コズミック・イラの勢力#ジャンク屋組合|ジャンク屋組合]]の一員。かなりのメカ好きで、ジャンクであれ武器であれ物を大事にする。[[コーディネイターとナチュラル|ナチュラル]]であるが、[[コーディネイターとナチュラル|コーディネイター]]に対する偏見はない。「戦うジャンク屋」として様々な事件に関与し、[[コズミック・イラ|C.E.]]の歴史に大きな影響を与える事も少なくない。\n\n==== 山吹樹里（ヤマブキキサト） ====\n: 声：[[豊口めぐみ]]（ゲーム『[[SDガンダム GGENERATION#SDガンダム GGENERATION SEED|SDガンダム GジェネレーションSEED]]』/[[倉田雅世]]（プロモーションビデオ『[[ガンダムSEED MSV]]』）\n: 【性別：女性・ナチュラル / 年齢：16歳 / 所属：ジャンク屋組合 / 搭乗機：キメラ、[[バクゥ#バクゥ バルトフェルド専用改修タイプ|プロトラゴゥ]] / 登場作品：○|○|○|○|○|○|○|-】\n: 『ASTRAY』に初登場。でロウ・ギュール達と行動を共にするジャンク屋組合の一員。自分に自信が持てない臆病な性格で、物事をマイナスに考えがちである。そのため、常にロウの突拍子もない行動に振り回されている。しかし、ロウのことを誰よりも心配しており、[[ガンダムアストレイ#天（未完成）|ゴールドフレーム天]]との戦闘でロウがピンチに陥った際、彼を救うために勇気を振り絞って出撃する一面もある。",&charaList);
		assert(charaList.size() == 2);
		assert(charaList[0].name == L"ロウ・ギュール");
		assert(charaList[0].yomi == L"ロウギュール");
		assert(charaList[0].actor1 == L"小野坂昌也");
		assert(charaList[0].actor2 == L"");

		assert(charaList[1].name == L"山吹樹里");
		assert(charaList[1].yomi == L"ヤマブキキサト");
		assert(charaList[1].actor1 == L"豊口めぐみ");
		assert(charaList[1].actor2 == L"倉田雅世");
		assert(charaList[1].actor3 == L"");
	}
	{
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"ピクシーゲール",L"\n== 主な登場人物 ==\n=== 主要人物 ===\n; 渡会 りかの（わたらい りかの）\n:* '''職業：'''魔法使・神父（異端調査課・第16分室に所属）\n:* '''年齢：'''15歳\n:* '''髪型：'''金髪のショートヘアーで右側の一束だけは長く、普段はスカーフ様の大きな赤いリボンでまとめている（私服時や扉絵では別のデザインのリボンで束ねている時もある）",&charaList);
		assert(charaList[0].name == L"渡会りかの");
		assert(charaList[0].yomi == L"わたらいりかの");
		assert(charaList[0].actor1 == L"");
		assert(charaList.size() == 1);
	}
	{// ; ではなく、 == 名前 == *で代用するパティーン
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"古畑任三郎の登場人物",L">\n== 古畑任三郎 ==\n* 読み - ふるはた にんざぶろう\n* 演 - [[田村正和]]（中学生時代：[[山田涼介]]）\n[[警視庁]]刑事部捜査一課の刑事で、階級は[[警部補]]。\n",&charaList);

		assert(charaList.size() == 1);
		assert(charaList[0].name == L"古畑任三郎");
		assert(charaList[0].yomi == L"ふるはたにんざぶろう");
		assert(charaList[0].actor1 == L"田村正和");
		assert(charaList[0].actor2 == L"山田涼介");
	}
	{// * で一行で箇条書きするパティーン
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"ザ・スクールコップ",L"\n==登場人物==\n=== 闇の潜入捜査班 ===\n概要：警察や民法では手に負えない謎の事件を闇の潜入捜査で事件解決に挑む。大介は潜入捜査時、警察の身分を隠す為、警察手帳と拳銃を携帯していない。\n*山下大介警視:[[三浦洋一 (俳優)|三浦洋一]]\n**キャリア、闇の潜入捜査班の発案者であり、リーダー。表の顔は警視庁史料編纂室・室長。格闘技の使い手の英語教師（夏休みの事件簿編では体育教師）として潜入する。実際は弱く、陰から他のメンバー（特におケイやトン子）に助けられている。\n*松村恵子:[[伊藤かずえ]]\n**少年院収容者、学校へ生徒としての潜入担当。コードネームは「おケイ」。次回予告の顔出しナレーションも担当。\n",&charaList);
		assert(charaList.size() == 2);
		assert(charaList[0].name == L"山下大介警視");
		assert(charaList[0].yomi == L"");
		assert(charaList[0].actor1 == L"三浦洋一");
		assert(charaList[0].actor2 == L"");

		assert(charaList[1].name == L"松村恵子");
		assert(charaList[1].yomi == L"");
		assert(charaList[1].actor1 == L"伊藤かずえ");
		assert(charaList[1].actor2 == L"");
	}

	{	//; では ===== 名前 ===== :で代用するパティーン　　専門家気取りどもは無駄に詳しいが、フォーマットの統一ということだけは知らないようだ。
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"u'sの登場人物",L"\n== 登場人物 ==\n主に基礎設定、複数のメディアで共通する設定を記述する。各メディア固有の設定は別途記述する。付記された[[声優]]は、特記ない限り、テレビアニメ版における担当を指す。\n\n===== {{Anchor|高坂穂乃果|高坂 穂乃果（こうさか ほのか）}} =====\n: 声 - [[新田恵海]]\n: 誕生日 - [[8月3日]]（獅子座）\n: 血液型 - O型\n: 本作の主人公。16歳の高校2年生<ref name=\"hobby1002\">{{Cite journal|和書|title=電撃アニ☆ホビ|journal=[[電撃ホビーマガジ",&charaList);
		assert(charaList[0].name == L"高坂穂乃果");
		assert(charaList[0].yomi == L"こうさかほのか");
		assert(charaList[0].actor1 == L"新田恵海");
		assert(charaList[0].actor2 == L"");
	}

	{
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"スレイヤーズの登場人物",L">{{Pathnav|スレイヤーズ|frame=1}}\n{{Pathnav|スレイヤーズ|スレイヤーズ (アニメ)|frame=1}}\n'''スレイヤーズの登場人物'''（スレイヤーズのとうじょうじんぶつ）では、[[ライトノベル]]・[[漫画]]・[[アニメ]]・[[コンピュータゲーム|ゲーム]]作品『[[スレイヤーズ]]』に登場する人物や、それに付随する武器等を記述する。\n\n== 主人公パーティ ==\n;リナ＝インバース:[[声優|声]] - [[林原めぐみ]]\n{{see|リナ＝インバース}}\n;ガウリイ＝ガブリエフ:声 - [[松本保典]]\n{{see|ガウリイ＝ガブリエフ}}\n;白蛇のナーガ:声 - [[川村万梨阿]]\n{{see|白蛇のナーガ}}",&charaList);
		assert(charaList[0].name == L"リナ＝インバース");
		assert(charaList[0].yomi == L"リナインバース");
		assert(charaList[0].actor1 == L"林原めぐみ");
		assert(charaList[0].actor2 == L"");

		assert(charaList.size() == 3);
	}

	{
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"魔法少女リリカルなのはINNOCENT",L"\n== 登場人物 ==\n{{節stub}}\nここでは「TVシリーズやゲームなどで一般に知られている設定」を便宜上「'''原典'''」とし、「原典との相違点」がある場合に関してはその旨も記述する。\n\n=== ホビーショップT&amp;H関連 ===\n; 高町なのは\n: 使用デバイス：レイジングハート、セイクリッド・ハート（クリス）、ストライクカノン、フォートレスVer.N、シュベルトクロイツ\n: アバター：セイクリッドタイプ、セイクリッドII、LOGスタイル\n: 漫画版第1部の主人公格の一人。私立海聖小学校4年1組。アリサとすずかに誘われたことでブレイブデュエルを始めた。原典と比べると困った時になると一人称が自分の名前で呼ぶようになり、ですます口調になるなどの違いがある。（ただし、原典でも自分のことを『なのは』と呼んだことがあり、目上に対してはですます口調である）\n; フェイト・テスタロッサ\n: 使用デバイス：バルディッシュ、バルディッシュII\n: アバター：ライトニングタイプ、ライトニングII\n: プレシアの下の娘でロケテスト全国2位の実力者。なのは達と初めて出会った翌日に私立海聖小学校4年1組に転校してきた。大人しい性格で姉である明るく元気なアリシアが大好きなお姉ちゃん子。\n: 原典ではアリシアのクローンであったが今作では普通の人間として生まれている。他にも原典では家族の愛情に飢えていたに対し、今回は恵まれていたためかなのはに対しての好意が出ている。他にも原典では呼び捨てだったシグナムもさん付けしている。",&charaList);

		assert(charaList.size() == 2);
		assert(charaList[0].name == L"高町なのは");
		assert(charaList[0].yomi == L"");
		assert(charaList[0].actor1 == L"");
		assert(charaList[0].actor2 == L"");

		assert(charaList[1].name == L"フェイト・テスタロッサ");
		assert(charaList[1].yomi == L"フェイトテスタロッサ");
		assert(charaList[1].actor1 == L"");
		assert(charaList[1].actor2 == L"");
	}
	{
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"カードキャプターさくらの登場人物",L"\n== 主要人物 ==\n[[File:カードキャプターさくら 主人公の家系図.jpg|thumb|right|カードキャプターさくら 主人公の家系図]]\n; 木之本桜（きのもと さくら）\n: 声 - [[丹下桜]]\n:  主人公。通称・さくら。私立友枝小学校に通う小学4年生。[[4月1日]]生まれ（[[誕生花]]は[[ソメイヨシノ]]）。[[ABO式血液型|A型]]。好きな花は桜。好きな色はピンクと白。「はにゃーん」「ほえ?」（時々「はう?」）などが口癖。[[チアリーダー|チアリーディング]]部所属。\n; ケルベロス\n: 声 - [[久川綾]]（仮の姿） / [[小野坂昌也]]（真の姿）\n: クロウによって創られた、クロウカードの2人の守護者のうちの1人で「封印の獣」であり「選定者」。通称・ケロちゃん。好きな花は[[向日葵]]。好きな色は赤とオレンジ。さくらの魔力を認めてカードキャプターに選んだ。普段は&lt;!--魔力が足りないため--&gt;ぬいぐるみのような小さい姿&lt;ref&gt;正体が発覚する前は関係者以外の前ではぬいぐるみの振りをしていることが多かった。劇場版第2作では、雪兎の前でぬいぐるみの振りをしていた。&lt;/ref&gt;。真の姿は翼の生えた[[ライオン]]を思わせる獣。本の表紙には真の姿で描かれている。クロウカードの入っていた本が長い間大阪にあった影響で、大阪弁でしゃべる&lt;ref&gt;クロウが生きていた時は[[標準語]]で話していた。そのときの一人称は「俺」（現在の一人称は「わい」）。&lt;/ref&gt;。性格は非常に陽気で[[ナルシスト]]。シンボルが太陽であるため、食事を摂ることで魔力を補給することが出来る。お菓子と[[たこ焼き]]と[[モダン焼き]]が大好物で、食いしん坊。「人生楽しまんとな」が信条。クロウカードに絡む事件がない時は、さくらの部屋でよく[[テレビゲーム]]に没頭している。就寝時はさくらのベッドで寝ている（アニメ版では、さくらの勉強机の一番下にある大きな引き出しの中）。\n: [[ギリシア神話]]の[[ケルベロス|Cerberus]]とは、甘い物の好きな番犬という性質以外は共通点がほとんどない。番犬でありながら、クロウカードの封印が解かれるまでおよそ30年間居眠りをしていた&lt;!--クロウが記憶を弄った可能性あり--&gt;。同じくクロウの創り出した[[モコナ|モコナ=モドキ達]]と面識がある&lt;ref name=&quot;name&quot;&gt;『[[ソエルとラーグ モコナ=モドキの冒険]]』より。&lt;/ref&gt;。\n; 大道寺知世（だいどうじ ともよ）\n: 声 - [[岩男潤子]]\n: さくらの同級生で一番の親友。[[9月3日]]生まれ。A型。好きな花は[[木蓮]]と桜。好きな色はベージュと白。好きな科目は音楽と国語。資産家の一人娘で、ボディガードが何人もいる。[[合唱|コーラス]]部所属で、発表会などではソロで歌うこともあるほど歌が得意。趣味は料理＆裁縫＆ビデオ撮影。見た目はおっとりしており、母親が撫子に似せるために伸ばした長い黒髪が特徴。「?ですわ」などのお嬢様口調でしゃべる。歳に見合わぬ落ち着いた性格で思慮深い。\n== 関連項目 ==\n* [[カードキャプターさくら]]\n* [[クロウカード]]",&charaList);

		assert(charaList.size() == 3);
		assert(charaList[0].name == L"木之本桜");
		assert(charaList[0].yomi == L"きのもとさくら");
		assert(charaList[0].actor1 == L"丹下桜");
		assert(charaList[0].actor2 == L"");

		assert(charaList[1].name == L"ケルベロス");
		assert(charaList[1].yomi == L"ケルベロス");
		assert(charaList[1].actor1 == L"久川綾");
		assert(charaList[1].actor2 == L"小野坂昌也");
		assert(charaList[1].actor3 == L"");

		assert(charaList[2].name == L"大道寺知世");
		assert(charaList[2].yomi == L"だいどうじともよ");
		assert(charaList[2].actor1 == L"岩男潤子");
		assert(charaList[2].actor2 == L"");
	}
	{
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"新・必殺仕置人",L">\n== 登場人物 ==\n\n; 与力 高井\n: 演 - [[辻萬長]]（第2、3、5、7話）\n; 同心 真木\n: 演 - 三好久夫（第16、17、19、20話）\n; [[元締・虎]]\n: 演 - [[藤村富美男]]（元[[阪神タイガース]]）\n: 仕置人組織「寅の会」を束ねる大元締。\n: 江戸中に散在していた仕置人達を一斉にまとめ上げ、「寅の会」を作り上げる。かつては凄腕の仕置人であり、劇中では粛清のため、配下の仕置人を葬っている。",&charaList);
		assert(charaList.size() == 3);
		assert(charaList[0].name == L"与力高井");
		assert(charaList[0].yomi == L"");
		assert(charaList[0].actor1 == L"辻萬長");
		assert(charaList[0].actor2 == L"");

		assert(charaList[2].name == L"元締・虎");
		assert(charaList[2].yomi == L"");
		assert(charaList[2].actor1 == L"藤村富美男");
		assert(charaList[2].actor2 == L"");
	}
	{
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"アップルシード",L">\n== 登場人物 ==\n=== 主要人物 ===\n; デュナン・ナッツ\n: 2105年誕生。女性。父はカール・ナッツ。かなり複雑な混血。\n: 幼少時より、父である[[SWAT]]の指揮官・カールの指導により、年齢に似合わぬ戦闘能力と状況判断能力を持つ。利き腕は右腕だが左腕でも銃等が扱えるように訓練されている。目も同様。\n: 体内にESWATの標準装備である数種のプラントが埋め込んである。これは大抵の薬物やウィルスはろ過でき、特に[[クロロホルム]]B、[[リシン (毒物)|リシン]]は十分防ぐことが出来る。なお、この設定は後に血液中に投与される[[マイクロマシン]]に改められた。\n: 大戦後はブリアレオスと共に廃墟生活を送っていたが、2127年オリュンポスに移住、市警SWATを経て行政院ESWATに入隊する。\n: 性格は、戦いを楽しむところが無いではないが、戦闘狂というほどでもなく、仕事の達成を第一とするプロ意識の持ち主。恋愛に頬を染めるなど情緒が無いわけでもなく、むしろ晩生。ブリアレオスに父の部下時代から思いを寄せているが、作品中途まで子ども扱いされたりしている。\n: 作った料理は「料理名以外では区別がつかない」らしい。\n; ブリアレオス・ヘカトンケイレス\n: 2096年誕生。男性。ほぼ全身を機械化し、戦闘用の装甲とセンサーを備えた、戦闘[[サイボーグ]]。「データブック」ではデュナンと出会った頃とサイボーグ化直前の素顔が描かれている。映画版では漫画と設定(黒人)が異なり白人であり、彼の遺伝子を持つクローンもエクスマキナに登場する。またヘカトンケイル・システム（後述）に適応できたただ一人の人物とされているが後にカイニスもヘカトンケイルシステムへ移行したと思われる描写がある。\n: 幼いころに[[ソ連国家保安委員会|ソ連KGB]]にスカウトされるが作戦将校を殺害したため逃亡、フリーになる。2116年にデュナンと出会い、カール・ナッツのチームに入隊する。その6年後爆発事故によりサイボーグ化を始めた。外見的にはロボットのようだが、高度な技術による人工筋肉・人工皮膚を使用している有機質サイボーグ。また人間として残っている[[臓器]]もある。この時代では脳機能がかなり解明されており、人工脳組織による脳の増量も行っている。これは彼の装備している高性能センサー類の情報処理の為らしい。センサー情報などは意識への概念伝達である。\n: 表皮の体温はコントロール可能だが、彼は普通の人間に近い温度を保っている。これは恋人（デュナン）のためである。オリュンポス移住後に巻き込まれたテロで重傷を負い、それ以降に同都市の先進的な医療・サイボーグ技術による柔軟な素材を多用したボディへと段階的に変化していくが、ガイア事件までは腕に機械式の内装火器を組み込むなど、無機的な装備も多かった。ベゼクリク事件の頃までには皮膚は柔らかくはないが弾力がある体と成った。彼は[[ヘカトンケイル]]システムによって全身を制御するという、珍しいタイプのサイボーグであり、ヘカトンケイルシステムの能力でブリアレオスが損傷した場合も部品交換が早く、また追加装備により4本の腕を同時に操作したり、[[空母]]を丸ごと制御する事も可能と言われている。そのため、目が8個(顔面に4個、うなじに2個、兎耳のようなセンサーユニット先端に2個)あるなど装備品が多い。\n: 大戦後はデュナンと共に廃墟生活を送っていたが、2127年オリュンポスに移住、ESWATに入隊する。\n: オリュンポス移住後はボディの改造・改良をたびたび行っている。\n; ヒトミ（人美）\n: 2074年誕生の[[デザイナーベビー|バイオロイド]]で総合管理局・立法院に所属し秘書のような役職にある。女性。バイオロイドにとっては一般的なことだが外見と実年齢は伴っていない。物語登場時にすでに50歳を超えているが、これはバイオロイドは長寿命が可能なように設計されており、生後、延命処理という処置を行い続けることで年齢を重ねても肉体的には若さを保っているため。彼女の場合は加えて性格も子供っぽい。彼女のDNA情報にはオリュンポスメインコンピュータ・ガイアの停止のキー情報が含まれているのでオリュンポス全体にとって彼女は重要人物でもある。現在はアルテミスの子供たちに振り回されている。\n; 宮本 義経\n: 年齢不詳のバイオロイドで人美のボーイフレンド。明智モータースに勤めていてギュゲスやランドメイトの整備等を行っているが基本的な社会的地位は一般市民である。日系の血が流れている。技術の腕は並程度。重度のメカマニアで、オリュンポスの人間が現代人のオートバイの感覚で利用する民生用ランドメイトやスーツを多数所有、休日などは心行くまで弄り回している模様。ガイア事件の折に立法院の手駒として隠密活動する羽目になった人美に所有するパワードスーツを貸し、自身も同行したりもしている。\n; ドクトル・マシュー\n: 2062年誕生の老[[医師|医者]]、サイボーグ医師。この世界ではサイボーグ化手術やケアは特別な医師が行っており、ドクトルマシューはその一人である。腕は確かだが[[マッドサイエンティスト]]的性格で、患者をやたらサイボーグ化したがる性向がある。第一巻では負傷したブリアレオスを手術した。またベナンダンティ作戦にも同行するなどESWATとの関わりもあるらしい。士郎作品では例外的な[[スター・システム (小説・アニメ・漫画)|スターシステム]]的キャラで、アップルシード以外の作品にも度々登場している。\n; アテナ・アレイアス\n: バイオロイドでオリュンポス総合管理局行政総監の行政院の最高責任者。立法院とは対立関係にあった。\n: [[内務省]]・内務大臣ニケ参長とは深い信頼関係がある。優れた能力を有し、真剣に人類の未来を考えている。実質ESWATのトップである。\n; ニケ\n: バイオロイドでオリュンポス総合管理局行政院内務省の内務大臣。ESWATとポリスを管轄している。行政総監アテナの右腕として能力を発揮している。ESWAT指揮官ランス班長に、直に命令を出すときもある。\n; アレス\n: エアポリス指揮官。実力は不明だが如何せん劇中のエアポリスがやられ役なので見せ場が無く、索敵・追跡などの地味な作業ばかりで犯人制圧などの華をESWATに持っていかれ、悔しがっている。\n; 局長\n: オリュンポス総合管理局司法省の長でオリュンポスの最高責任者の一人。冷静沈着な人物。管轄組織のFBIの指揮権を有する。\n; アルゲス（ヴェルンド）\n: ブリアレオスの旧友。表向きは局長直属の部下でFBIの現場指揮官だが、実はオリュンポスを建設した「都市企画班」が、人間側のオリュンポスの安全弁として送り込んでいる一人らしい。\n; アルテミス\n: 都市企画班から重要情報を携えて送り込まれたバイオロイド。ヒトと猫のハイブリッド体。戦闘用サイボーグではないものの、それに匹敵する戦闘能力を持つことと、小柄な割に体重があるため、天然の生物とは違う素材で身体が構成されているフラクチュエイター・タイプの可能性がある。単為生殖により3人の子供（クローン）を産む。その行動には不可解な点が多く、彼女にはまだ謎があるようだ。高い戦闘能力を持つが、常識はなく、加えて良心も無いため廃墟隠遁生活の折には食人すら厭わず生き延びていた。知能は高く短時間のうちにコンピュータを使いこなすなどしているが、言葉は喋れない。\n; ドリス（吉野）\n: 企業集合体国家ポセイドン&lt;!-- 本社は日本近海の人工島であり、未来の日本と同一の国家であるかは明示されていない（未来の日本）--&gt;のスパイ。と思わせるが二重スパイなようだ。アルゲスやランスと繋がりがある。\n; 双角\n: ベナンダンティ作戦時にESWATに投降したサイボーグ[[傭兵]]の一人。[[SAS (イギリス陸軍)|SAS]]のサイボーグ部隊出身で爆発物のスペシャリストだが、曰く「爆発物は趣味」とするなどプロ意識は低く、自身の趣味性で行動している。4巻ではA-10やデュナンを翻弄して礼金をせしめるもデュナンに報復されて車ごと爆破されボロボロに。5巻に登場する女サイボーグ達とは過去に因縁があるらしい。\n; カール・ナッツ\n: デュナンの父親で[[SWAT]]の指揮官。デュナンに幼少時からSWATの枠を超えた戦闘・サバイバル訓練を施していた事から、大戦勃発を予想・察知していたと見られる。オリュンポス計画にも関わっていたらしく、バイオロイドにも彼の遺伝情報を持つ者が多い。消息不明。本編では回想シーンで遠景で登場したのみだが、「データブック」で顔が描かれている。",&charaList);
		//charaList = [14]({yomi="デュナン・ナッツ" name="デュナン・ナッツ" title="アップルシード" ...},{yomi="ブリアレオス・ヘカトンケイレス" name="ブリアレオス・ヘカトンケイレス" title="アップルシード" ...},{yomi="ヒトミ" name="人美" title="アップルシード" ...},{yomi="" name="宮本義経" title="アップルシード" ...},{yomi="ドクトル・マシュー" name="ドクトル・マシュー" title="アップ...
		assert(charaList.size() == 14);
		assert(charaList[0].name == L"デュナン・ナッツ");
		assert(charaList[0].yomi == L"デュナンナッツ");
		assert(charaList[0].actor1 == L"");
	}
	{
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"加治隆介の議",L"\n== 登場人物 ==\n;クリントン\n:[[アメリカ合衆国大統領]]。北朝鮮問題では経済制裁をして緊迫化するも、今後の核開発禁止を引き換えに過去の核の不問と日韓負担による核施設建設で決着させたり、フランスの核実験に対する日本政府の対応について、強硬姿勢を控えるようテレックスを送ったり、ひので丸シージャック事件ではプルトニウムが北朝鮮にいくのを阻止するために強攻策を取るなど、作中で安全保障について様々な対応をしている。\n;ポラック\n:フランス大統領。フランス至上主義者であり、フランスの核実験を再開する。",&charaList);
		assert(charaList.size() == 2);
		assert(charaList[0].name == L"クリントン");
		assert(charaList[0].yomi == L"クリントン");
		assert(charaList[0].actor1 == L"");
		assert(charaList[0].actor2 == L"");
		assert(charaList[0].actor3 == L"");

		assert(charaList[1].name == L"ポラック");
		assert(charaList[1].yomi == L"ポラック");
		assert(charaList[1].actor1 == L"");
		assert(charaList[1].actor2 == L"");
		assert(charaList[1].actor3 == L"");
	}
	{
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"君が望む永遠",L"\n== 登場人物 ==\n[[声優]]はゲーム/アニメの順。ただし、共通の場合は省略する。記事については、個別ルートに準拠する。\n=== 第一章より登場 ===\n; 鳴海 孝之 （なるみ たかゆき）\n: 声 - なし、[[谷山紀章]]（ファンディスク、TVアニメ、OVA）、[[杉崎和哉]]（ドラマCD）\n: 主人公。白陵大付属柊学園の3年生。涼宮遙に告白され交際を始めるがうまくいかず、一度は別れを考えるが、速瀬水月のサポートもあり、遙に告白し直し改めて付き合うようになった。遙と同じ白陵大学に進学することに決め、幸せな日々を送っていたが、遙が交通事故に遭い昏睡状態となってしまう。遙の両親から別れを宣告され、失意から[[引きこもり]]を続けていた。\n: 第二章開始時は、水月の献身的な支えにより社会復帰し、[[ファミリーレストラン]]「すかいてんぷる」でアルバイトをしながら生活している。水月とは半同棲状態にあり、水月が結婚も考えていることには満更でもない様子。だが、遙が目を覚ましたことにより過去と向き合うことになる。以後、遙のお見舞いを繰り返しながらバイトする日々を送る。\n",&charaList);
		assert(charaList.size() == 1);
		assert(charaList[0].name == L"鳴海孝之");
		assert(charaList[0].yomi == L"なるみたかゆき");
		assert(charaList[0].actor1 == L"谷山紀章");
		assert(charaList[0].actor2 == L"杉崎和哉");
		assert(charaList[0].actor3 == L"");
	}
	{
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"To LOVEる -とらぶる-の登場人物",L"\n== 主要登場人物 ==\n; {{Anchor|結城梨斗|結城 梨斗（ゆうき りと）}}\n: 声 - [[渡辺明乃]]\n: '''概要'''\n: 『TL』『ダークネス』共通の主人公。作中では名前を「'''リト'''」と片仮名で表記されることが多い。\n: 彩南高校に通う男子高校生。高校のクラスは1年A組（『TL』第1話 - 第48話）→2年A組（『TL』第49話 - 現在）。[[10月16日]]生まれ。血液型は[[ABO式血液型|O型]]。\n; {{Anchor|ララ|ララ・サタリン・デビルーク}}\n: 声 - [[戸松遥]]\n: '''概要'''\n: 『TL』のメインヒロインの1人。『ダークネス』でも引き続き登場する。",&charaList);

		assert(charaList.size() == 2);
		assert(charaList[0].name == L"結城梨斗");
		assert(charaList[0].yomi == L"ゆうきりと");
		assert(charaList[0].actor1 == L"渡辺明乃");
		assert(charaList[0].actor2 == L"");

		assert(charaList[1].name == L"ララ・サタリン・デビルーク");
		assert(charaList[1].yomi == L"ララサタリンデビルーク");
		assert(charaList[1].actor1 == L"戸松遥");
		assert(charaList[1].actor2 == L"");
	}
	{
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"CSIの登場人物",L">\n\n== 登場人物 ==\n; D・B・ラッセル(D.B. Russell)/ 第12シーズン第1話0\n: 演：[[テッド・ダンソン]]([[:en:Ted Danson|Ted Danson]])、声：[[樋浦勉]]\n: CSI捜査官レベル3、夜番主任\n: ラングストンの後任として、そして降格処分を受けたキャサリンの後任としてシアトルCSIからやってきた新主任。規則を無視して暴走しがちのCSIチームを立て直すために上層部が",&charaList);

		assert(charaList.size() == 1);
		assert(charaList[0].name == L"D.B.Russell");
		assert(charaList[0].yomi == L"DBラッセル");
		assert(charaList[0].actor1 == L"テッド・ダンソン");
		assert(charaList[0].actor2 == L"Ted Danson");	//actor1の英語の情報がある場合はどうするか、、まあいいか。
		assert(charaList[0].actor3 == L"樋浦勉");
	}
	{
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"ドラゴンボールの登場人物",L">\n\n== 登場人物 ==\n;\n; [[フリーザ]]\n: 声 - 中尾隆聖\n: 宇宙の支配者で、星の地上げなどの活動を行っているフリーザ一味の統括者。ナメック星で悟空と激戦を繰り広げる。",&charaList);

		assert(charaList.size() == 1);
		assert(charaList[0].name == L"フリーザ");
		assert(charaList[0].yomi == L"フリーザ");
		assert(charaList[0].actor1 == L"中尾隆聖");
	}
	{
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"バトル・ロワイアル 天使たちの国境",L"\n== 登場人物 ==\n=== 城岩中学校3年B組女子 ===\n; 谷沢 はるか（たにざわ はるか）\n: 「Episode.1」の主人公。女子12番。髪をショートカットにしたボーイッシュな少女。長身で、[[バレーボール]]部ではアタッカーを務める。内海幸枝に片想いしているが、その気持ちは隠している。支給武器は[[槌|トンカチ]]。\n; 松井 知里（まつい ちさと）\n: 「Episode.2」の主人公。女子19番。大人しい性格であまり目立たない。外国に興味があり英語を熱心に勉強している。共和国防衛婦人会の習い事に通いながら文化使節団として外国に行くことを目指している。\n; 内海 幸枝（うつみ ゆきえ）\n: 女子02番。クラス委員長。黒い髪を三つ編みにした少女。谷沢はるかの想い人だが、彼女の気持ちには気付いていない。父親は共和国防衛軍の軍人だが、学校ではそのことを知っている者はほとんどいない。はるかとは小学校の頃からのバレーボールのチームメイトで、[[セッター (バレーボール)|セッター]]を務めていた。\n; 榊 祐子（さかき ゆうこ）\n: 女子09番。気弱でおとなしい女の子。七原秋也が大木立道（男子03番）の死体の側に立ち、血まみれの鉈を手にしているところを目撃してしまう。支給武器は警棒と毒薬。\n; 中川 有香（なかがわ ゆか）\n: 女子16番。榊祐子が毒を入れたクリームシチューを口にし、死亡。灯台に立て籠もった少女達の中で最初の死者。\n; 野田 聡美（のだ さとみ）\n: 女子17番。眼鏡をかけた少女。中川有香の死をきっかけに、仲間の中に裏切り者がいるという疑心暗鬼で[[UZI (SMG)|ウージー]]を振りかざす。\n",&charaList);
		assert(charaList.size() == 6);
		assert(charaList[5].name == L"野田聡美");
		assert(charaList[5].yomi == L"のださとみ");
		assert(charaList[5].actor1 == L"");

		assert(charaList[4].name == L"中川有香");
		assert(charaList[4].yomi == L"なかがわゆか");
		assert(charaList[4].actor1 == L"");

		assert(charaList[3].name == L"榊祐子");
		assert(charaList[3].yomi == L"さかきゆうこ");
		assert(charaList[3].actor1 == L"");

		assert(charaList[2].name == L"内海幸枝");
		assert(charaList[2].yomi == L"うつみゆきえ");
		assert(charaList[2].actor1 == L"");

		assert(charaList[1].name == L"松井知里");
		assert(charaList[1].yomi == L"まついちさと");
		assert(charaList[1].actor1 == L"");

		assert(charaList[0].name == L"谷沢はるか");
		assert(charaList[0].yomi == L"たにざわはるか");
		assert(charaList[0].actor1 == L"");
	}
	{
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"DRAGONBALL EVOLUTION",L"\n== 登場人物 ==\n=== 仲間 ===\n; [[孫悟空 (ドラゴンボール)|孫悟空]]（[[ジャスティン・チャットウィン]]、日本語吹き替え - [[山口勝平]]）\n: 高校3年生。おとなしく冴えない少年。幼き頃から祖父の悟飯のもとで武術の鍛錬を受けている。同級生のチチに憧れを抱いているが積極的になれずに内気である。祖父の遺言でピッコロの野望を阻止するためドラゴンボールを探す旅に出る。\n",&charaList);
		assert(charaList.size() == 1);
		assert(charaList[0].yomi == L"");
		assert(charaList[0].name == L"孫悟空");
		assert(charaList[0].actor1 == L"ジャスティン・チャットウィン");
		assert(charaList[0].actor2 == L"山口勝平");
		assert(charaList[0].actor3 == L"");
	}
	{
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"ガルーダの騎士ビマX",L"\n== 登場人物 ==\n=== ビマとその仲間たち ===\n; レイ・ブラマサクティ (Ray Bramasakti)\n: [[俳優|演]] ： [[クリスティアン・ロホ]] / Christian Loho\n* スーパーヒーローの'''ビマ・サトリア・ガルーダ'''に変身する[[主人公]]",&charaList);
		assert(charaList.size() == 1);
		assert(charaList[0].name == L"RayBramasakti");
		assert(charaList[0].yomi == L"レイブラマサクティ");
		assert(charaList[0].actor1 == L"クリスティアン・ロホ");
		assert(charaList[0].actor2 == L"Christian Loho");
		assert(charaList[0].actor3 == L"");
	}
	{
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"科捜研の女の登場人物",L"\n== 主要人物 ==\n; 榊 マリコ（さかき まりこ）\n: 演 - [[沢口靖子]]（主人公）\n: 科学捜査研究所の法医学研究員。血液型はB型&lt;ref name=&quot;第4シリーズFile5&quot;&gt;第4シリーズFile5エピソードより。\n\n; 土門 薫（どもん かおる）\n: 演 - [[内藤剛志]]（第5シリーズ - ）\n",&charaList);
		assert(charaList.size() == 2);
		assert(charaList[0].name == L"榊マリコ");
		assert(charaList[0].yomi == L"さかきまりこ");
		assert(charaList[0].actor1 == L"沢口靖子");
		assert(charaList[0].actor2 == L"");

		assert(charaList[1].name == L"土門薫");
		assert(charaList[1].yomi == L"どもんかおる");
		assert(charaList[1].actor1 == L"内藤剛志");
		assert(charaList[1].actor2 == L"");
	}

	{
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"バーン・ノーティス",L"\n\n== 登場人物 ==\n; マイケル・アレン・ウェスティン - [[ジェフリー・ドノヴァン]]（日本語吹替版：[[栗田貫一]]）\n: 主人公。世界をまたにかけて活躍していた凄腕の元[[CIA]]スパイ。[[ナイジェリア]]での任務途中で突然解雇され、経歴や財産も奪われて命からがら逃れる。気がついたら、故郷である[[フロリダ州]][[マイアミ]]に送られ、政府のブラックリスト入りし、マイアミを出ないようFBIから監視されることとなる。自身が解雇された真相について調査を始めるが、近所の困っている人から問題解決を依頼されるようになり、人助けのためにスパイ技術の知識を生かして、私立探偵のようなことをするようになる。\n: 空手歴30年。機器に強く、自作の武器や盗聴器を用いてあらゆる事件を解決する。\n: 愛車は父の形見でもある[[ダッジ・チャージャー]]73年モデル。",&charaList);
		assert(charaList.size() == 1);
		assert(charaList[0].name == L"マイケル・アレン・ウェスティン");
		assert(charaList[0].yomi == L"マイケルアレンウェスティン");
		assert(charaList[0].actor1 == L"ジェフリー・ドノヴァン");
		assert(charaList[0].actor2 == L"栗田貫一");
	}



	{
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"恋愛ラボ",L">\n== 登場人物 ==\n[[声優]]はドラマCD / テレビアニメの順。声優が1人だけの表記の場合は、テレビアニメ版のみの登場であることを示す。\n\n=== 藤女生徒会執行部メンバー ===\n; 倉橋 莉子（くらはし りこ）\n: 声 - [[釘宮理恵]] / [[沼倉愛美]]\n: 通称「'''リコ'''」。本作の主人公。2年3組の女子生徒。生徒会会長補佐。[[4月3日]]生まれ。身長155cm。血液型はO型。[[座右の銘]]「なせばなる!」<ref name=\"ガイドブック\">{{Cite journal|和書|journal=恋愛ラボ公式ガイドブック|issue=まんがタイム2013年8月号増刊|publisher=[[芳文社]]|date=2013-06-17}}</ref>。\n: 公立の小学校出身。奇人変人ばかりの生徒会執行部内では一番の常識人。小学5年まで男子に混じって[[サッカー]]クラブに所属するなど、運動神経は抜群。一方で中学受験で藤女に",&charaList);
		assert(charaList.size() == 1);
		assert(charaList[0].name == L"倉橋莉子");
		assert(charaList[0].yomi == L"くらはしりこ");
		assert(charaList[0].actor1 == L"釘宮理恵");
		assert(charaList[0].actor2 == L"沼倉愛美");
	}
	{
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"魔法少女リリカルなのはシリーズの登場人物",L"\n== 主要登場人物 ==\nそれぞれのシリーズにおいての主要登場人物達。\n\n\n=== なのはシリーズの主要登場人物 ===\n全シリーズ通しての主要登場人物。\n\n; 高町 なのは（たかまち なのは）\n: [[声優|声]] - [[田村ゆかり]]\n: {{see|高町なのは}}\n: 第3期『StrikerS』までは主人公を務めた。\n; フェイト・テスタロッサ → フェイト・T・ハラオウン\n: 声 - [[水樹奈々]]\n: {{see|フェイト・テスタロッサ}}\n; 八神 はやて（やがみ はやて）\n: 声 - [[植田佳奈]]\n: {{see|八神はやて}}\n: 第2期『A's』からの登場人物で、第1期には未登場。\n",&charaList);
		assert(charaList.size() == 3);

		assert(charaList[2].name == L"八神はやて");
		assert(charaList[2].yomi == L"やがみはやて");
		assert(charaList[2].actor1 == L"植田佳奈");

		assert(charaList[1].name == L"フェイト・テスタロッサ");
		assert(charaList[1].yomi == L"フェイトテスタロッサ");
		assert(charaList[1].actor1 == L"水樹奈々");

		assert(charaList[0].name == L"高町なのは");
		assert(charaList[0].yomi == L"たかまちなのは");
		assert(charaList[0].actor1 == L"田村ゆかり");
	}

	{
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"大戦隊ゴーグルファイブ",L"\n== 登場人物 ==\n=== 大戦隊ゴーグルファイブ ===\n=== 暗黒科学帝国デスダーク ===\n大昔から科学技術を悪用した「悪魔の科学」を用いて人類に多くの災いをもたらし、過去にいくつもの文明を滅ぼしたと言われる暗黒科学者の集団。その歴史は5000年に及ぶと言われ、総統タブーと呼ばれる謎の支配者の下、1980年代になって本格的な世界征服に乗り出している。暗黒科学の技術を結集して造られた浮遊要塞「暗黒巨大城デストピア」を本拠地とし、モズーと呼ばれる合成怪獣やコングと呼ばれる巨大ロボット、戦闘機デスファイターを繰り出す。\n\n; {{Visible anchor|総統タブー}}\n: デスダークの支配者。太古の昔から群雄割拠していた暗黒科学者たちを統一して5000年前にデスダークを築き上げた存在と言われるが、その真の姿を見た者は最終決戦の時まで誰もいなかった。デストピアの内部にある謁見の間の曇りガラスの奥から黒い[[シルエット]]だけを見せて指令を下し、赤い巨大な一つ目を光らせて幹部たちを威圧する。その正体は、暗黒科学が生み出した「究極にして最強の遺伝子」の集合体であった。\n:; 巨大タブー\n:: 最終回、全ての部下を失い、デストピアを破壊された総統タブーが、ハイトロンエネルギーにより巨大化した姿。ゴーグルファイブの劇中では唯一自ら巨大化した敵である&lt;ref group=&quot;注&quot;&gt;デスダークのメンバー達は通常巨大化せず、コングと呼ばれる巨大ロボットに乗り込んでゴーグルロボと闘っていた。&lt;/ref&gt;。ゴーグルロボと最後の対決を行い、巨大な一つ目からのハイトロンビームでゴーグルロボを圧倒。電子銀河斬りで胴体を切られながらも上半身だけで襲い掛かるが、最後には唯一の弱点である一つ目に地球剣電子銀河ミサイルを食らい、爆発四散した。\n:\n; {{Visible anchor|デスギラー将軍}}（第1 - 49話）\n: [[髑髏|ドクロ]]を模した[[兜]]を被ったデスダークの指揮官。作戦立案を行うだけでなく、自らも前線に立って戦闘を行う。[[剣]]の達人で、ゴーグルレッドをライバル視している。短気で冷酷だが、デスダークへの忠誠心は厚く、武人としての一面も持つ。物語終盤、マズルカの自爆に巻き込まれ傷を負うが、赤間の「命の尊さに気付いてほしい」という願いからゴーグルファイブに手当てを受ける。その後、クマモズーを退けてゴーグルファイブへの借りを返した上で、自らクマコングに乗り込み、最後の勝負を挑む。激闘の末、電子銀河斬りによりコング共々壮烈な最期を遂げた。\n; {{Visible anchor|マズルカ}}（第1 - 48話）\n: デスダークの女[[スパイ]]。変装が得意で戦闘以外にも情報収集や[[誘拐]]など幅広い任務をこなす。第4話と第39話では変装して活動していた際に子供におばさん呼ばわりされて怒ったこともあった。爆撃や麻酔ガスを放つスティックが武器。第48話でハイトロンエネルギーの力で透明になって未来科学研究所に潜入するが、その体にはデスマルク大元帥の手によって[[爆弾]]が埋め込まれていた。衰弱していたマズルカは組織から切り捨てられたと知るや自暴自棄になり、デスギラー将軍から爆弾のコントローラーを奪い取り、将軍を道連れにするつもりで研究所と共に自爆した。イガアナ博士とザゾリヤ博士の存命中は2人よりも格が下であったが、第14話では不甲斐ない2人に皮肉を浴びせたこともあった。\n; {{Visible anchor|イガアナ博士}}（第1 - 15話）\n: 豪快な性格の科学者。自ら[[工具]]を取って動物型の巨大ロボットを作る。度重なる作戦の失敗により総統タブーの怒りを買い、デスマルク大元帥から「役立たず」と判断されて、第15話でザゾリヤ博士と共に処刑された。",&charaList);
		assert(charaList.size() == 4);
	}

	{
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"超電子バイオマン",L">\n\n== 登場人物 ==\n=== 超電子バイオマン ===\n; {{Visible anchor|郷 史朗（ごう しろう） / レッドワン}}\n: バイオマンのリーダー。24歳。劇中世界では日本人初の[[スペースシャトル]]パイロットである。やや短気ですぐに怒鳴るのが欠点だが、強い責任感と熱い心で仲間を引っ張っていくという絵に描いたようなリーダータイプの好青年。第1話でバイオ粒子を直接浴びた影響か[[動物]]との意思疎通能力を持っており、犬や[[猫]]、[[鳩]]などから情報収集を行っていたが、第10話では反バイオ粒子を",&charaList);
		assert(charaList.size() == 1);
		assert(charaList[0].name == L"郷史朗");
		assert(charaList[0].yomi == L"ごうしろう");
		assert(charaList[0].actor1 == L"");
	}


	{
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"花の魔法使いマリーベル",L"\n== 登場人物 ==\n=== 花の魔法使い ===\n;マリーベル・フォン・デカッセ - [[本多知恵子]]\n:ユーリとケンの願いを聞き届け、花魔法界から人間界へやってきた魔法使いの女の子。本作の主人公。&lt;/br&gt;魔法の花びらに働きかけて発動する花魔法の使い手で、肩書きは「花の魔法使い」。実年齢50万歳。人間年齢換算5歳。&lt;ref&gt;ケイブンシャの大百科521　『魔法のヒロインひみつ大百科』 ケイブンシャ刊 117頁　&lt;/ref&gt;口癖は「マリーベルにお任せよ!」",&charaList);
		//[0] = {yomi="マリーベル・フォン・デカッセ" name="マリーベル・フォン・デカッセ" title="花の魔法使いマリーベル" ...}
		assert(charaList.size() == 1);
		assert(charaList[0].name == L"マリーベル・フォン・デカッセ");
		assert(charaList[0].yomi == L"マリーベルフォンデカッセ");
		assert(charaList[0].actor1 == L"本多知恵子");
	}

	{
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"ナイトライダー",L">>\n== 登場人物 ==\n=== 主要人物 ===\n; マイケル・ナイト -'''Michael Knight'''-\n: 演：[[デビッド・ハッセルホフ]] [[:en:David Hasselhoff|David Hasselhoff]]、吹替：[[ささきいさお|佐々木功（現：ささきいさお）]]\n: 本作の主人公。ナイト財団の最重要顧問としてウィルトン・ナイトの遺志を継ぐ。K.I.T.T.とコンビを組み、数々の難事件に立ち向かった。元々はマイケル・アーサー・ロング（この時点ではハッセルホフとは全く別の俳優'''ラリー・アンダーソン'''がロング役。ただし車を降りて顔が逆光でシルエットになる場面（＝タニヤに撃たれる直前）からはハッセルホフが演じている）という名の敏腕刑事（階級は警部）であったが、産業スパイ事件捜査中に敵組織のボス・タニヤによって顔を撃たれて瀕死の重傷を負い、ウィルトンにより救出される。陸軍の軍歴があり、戦傷を受けた際、額に金属板を埋め込む手術を受けていたため、銃弾がこの板に跳ね返され致命傷を免れていた（ただしそれにより顔は破壊されてしまった）。一命を取り留めた後は、負わされた怪我の治療と合わせてウィルトンの息子ガースをモデルに顔の整形手術も行われ（整形後のマイケルの顔を見たデボンは、若き日のウィルトンにそっくりそのままだと述べている）、更にウィルトンの養子として名前もマイケル・ナイトに改められる。後にウィルトンの実子であるガースと対面した際マイケルはデボンに「（ガースが）俺にそっくりなのは一体どういうことだ!?」と問い詰める場面があった。ナイトとしてのプロフィールは創作されたものであるため、[[コネ]]を使って経歴を調べた悪党達に「3年前には存在さえしていなかった男だ」と驚かれることもしばしばである。\n: シーズン2第11話「激闘!魔の巨大ダム捜査・ナイト2000決死のターボジャンプ（原題:KNIGHTMARES ）」では、敵が投げた手榴弾の爆風でコンクリートの壁に叩き付けられ、そのショックでマイケル・ナイトとしての記憶を一時的に喪失し、マイケル・ロングに戻ってしまったことがある。ロングに戻ってしまったマイケルに「ナイトはどんな奴か」と尋ねられたK.I.T.T.は、「聡明で機敏で論理的だった反面、頑固で短気で若い女性の誘惑にもろく、自分のセンサーがショートしそうなおぞましい音楽（ロック）を好んで聴いている」と答えている。\n; K.I.T.T.（キット）-'''Knight Industries Two Thousand'''-\n: 声：[[ウィリアム・ダニエルズ]] [[:en:William Daniels|William Daniels]]、吹替：[[野島昭生]]\n: ナイト2000に搭載された人工知能で、マイケルの性格を良くも悪くもいちばん理解している相棒。離れた場所にいるマイケルとは、マイケルが左腕につけている'''コムリンク'''と呼ばれる腕時計型の通信機を通して会話する。実直で真面目な性格で、ユーモアにも長けている。一度路肩の石が車体の底に当たったショックでK.I.T.T.の回線が切れた（言葉のプログラムが、粗暴なブルックリン訛りに切り替わった）時に、マイケルを「マイキー」と呼んだこともある。ちなみにマイケルはその喋り方を我慢できず「頼むからその喋り方だけは勘弁してくれ」という程で、[[ダッシュボード (自動車)|インパネ]]を叩く（ショック療法）などし、さらに終盤では本部に戻る前にわざわざボニーを呼んでまで修理をさせた。また、ジャガーノートの攻撃で破壊され再プログラミングの際にも、ガレージで作業しているRC3達の会話を拾ったため、同様にマイケルのことを「兄弟」と呼んでいる。\n\n=== ナイト財団 ===\n==== 責任者 ====\n; デボン・マイルズ -'''Devon (Shire) Miles'''-（パイロット版及び日本語版ではデボン・'''シャイアー'''）\n: 演：[[エドワード・マルヘアー]] [[:en:Edward Mulhare|Edward Mulhare]]、吹替：[[中村正 (声優)|中村正]]\n: 財団の責任者でイギリス人。在りし日のウィルトンとは戦友であり、紳士的且つ温厚な性格。マイケルと意見が対立することもしばしばあるものの、彼の行動を全面的にサポートする良き理解者。軍を始めとして非常に広い人脈を持ち、大統領とも面識がある。事務方に見えるが実は博士号を持つ科学者であり、ナイト2000のボディを覆う分子結合殻の構造式を知る3人の人物の1人。また、過激な過去を持つという逸話もある（[[第二次世界大戦]]中は従軍し、[[ゲシュタポ]]の捕虜収容所から3度も脱走に成功した、ドイツ軍占領下のフランスにナイフ投げの芸人として潜入した、夜バイクに乗って司令部に行く途中敵機の夜間爆撃に遭い、バイクは破壊されたものの助かった、若い頃バイクレースのチャンピオンだった（マイケルが「デボンのヘルメットにゴーグル姿なんて想像もつかないね」と評したため「おいおい、それは偏見だぞ。私にだって若い頃はあったんだ、何なら[[トロフィー]]を見せてやろうか？」と言い返した）、等（[[SAS (イギリス陸軍)|SAS]]にいたこともあるとか）。ガースからは「親父（ウィルトン）によく似てやがる、だからお前が嫌いだ!!」と言われた。",&charaList);
		//charaList = [3]({yomi="マイケル・ナイト" name="MichaelKnight" title="ナイトライダー" ...},{yomi="キット" name="K.I.T.T." title="ナイトライダー" ...},{yomi="デボン・マイルズ" name="Devon" title="ナイトライダー" ...})
		assert(charaList.size() == 3);
	}
	{
		std::vector<FantasicCharaSt> charaList;
		bool r = findFantasicChara(L"ゆるゆり",L">\n\n== 登場人物 ==\n; 杉浦 綾乃（すぎうら あやの）\n: 声 - [[藤田咲]]\n: 14歳の中学2年生。赤紫色のロングヘアをポニーテールにしている。目の色は赤系。生徒会副会長。京子・結衣・千歳たちと同じクラス。[[1月20日]]生まれ、身長159センチメート",&charaList);

		assert(charaList.size() == 1);
		assert(charaList[0].name == L"杉浦綾乃");
		assert(charaList[0].yomi == L"すぎうらあやの");
		assert(charaList[0].actor1 == L"藤田咲");
		assert(charaList[0].actor2 == L"");
	}
}