
#include "common.h"
#include "wiki2yomi.h"
#include "wiki2ruigi.h"
#include "wiki2eng.h"
#include "wiki2chara.h"
#include "wiki2category.h"
#include "cmudict2yomi.h"
#include "option.h"

#if _MSC_VER
#else
#include <signal.h>
void sigterm_to_log(int sig) 
{
//	ERRORLOG("signal:" << sig << "を受信しました。終了させます。");
//	MainWindow::m()->Shutdown(EXITCODE_LEVEL_ERROR,true); //exit()
	exit(2);
}
#endif


void usage()
{
	wprintf(L"wikipedia to dic    2014 rti\n");
	wprintf(L"--usage--\n");
	wprintf(L"wiki2dic yomi|ruigi|eng filename  [option]\n");
	wprintf(L"\n");
	wprintf(L"Ex.\n");
	wprintf(L"wiki2dic yomi jawiki-latest-pages.xml\n");
	wprintf(L"  読みを取得します。\n");
	wprintf(L"  出力形式は、 読み[TAB]タイトル[ENTER] です。\n");
	wprintf(L"\n");
	wprintf(L"wiki2dic ruigi jawiki-latest-pages.xml\n");
	wprintf(L"  類義語を取得します。\n");
	wprintf(L"  出力形式は、 タイトル[TAB]類義...[ENTER] です。\n");
	wprintf(L"\n");
	wprintf(L"wiki2dic eng jawiki-latest-pages.xml\n");
	wprintf(L"  wikipediaの読みと英語単語から、英単語の日本語読みを取ります。\n");
	wprintf(L"  出力形式は、 読み[TAB]単語[ENTER] です。\n");
	wprintf(L"\n");
	wprintf(L"wiki2dic chara jawiki-latest-pages.xml\n");
	wprintf(L"  wikipediaから架空のキャラクターたちの読みを取得します。\n");
	wprintf(L"  出力形式は、 読み[TAB]キャラ名[TAB]タイトル[TAB]役者1[TAB]役者2[TAB]役者3[ENTER] です。\n");
	wprintf(L"\n");
	wprintf(L"wiki2dic category jawiki-latest-pages.xml\n");
	wprintf(L"  wikipediaからページのカテゴリータグだけを抜き出して列挙します。\n");
	wprintf(L"  出力形式は、 タイトル[TAB]カテゴリ...[ENTER] です。\n");
	wprintf(L"\n");
	wprintf(L"--wikipediaデータ--\n");
	wprintf(L" http://dumps.wikimedia.org/jawiki/latest/　から\n");
	wprintf(L" jawiki-latest-pages-meta-current.xml.bz2 をDLして展開すること\n");
	wprintf(L"\n");
	wprintf(L"--option--\n");
	wprintf(L"--case	よみがな文字列の変換\n");
	wprintf(L"\t--case=kata\t[ディフォルト]カタカナに変換して出力\n" );
	wprintf(L"\t--case=kana\tひらがなに変換して出力\n" );
	wprintf(L"\t--case=none\t何もしない。取れたままを出力\n" );
	wprintf(L"--show	表示項目\n");
	wprintf(L"\t--show=noempty\t[ディフォルト]読めたものだけ表示する\n" );
	wprintf(L"\t--show=all\t全部表示する\n" );
	wprintf(L"\t--show=empty\t読めたものだけ表示する\n" );
	wprintf(L"--aimai	曖昧さの解決の()\n");
	wprintf(L"\t--aimai=del\t[ディフォルト]消す\n" );
	wprintf(L"\t--aimai=keep\t消さない\n" );
	wprintf(L"\n");
	wprintf(L"--おまけ--(試験的なルーチンです)\n");
	wprintf(L"wiki2dic cmudict cmudict.0.7a\n");
	wprintf(L"  英語の発音辞書から強引に日本語読みに変換します。\n");
	wprintf(L"  出力形式は、 読み[TAB]タイトル[ENTER] です。\n");
	wprintf(L"--cmudictデータ--\n");
	wprintf(L" http://svn.code.sf.net/p/cmusphinx/code/trunk/cmudict/　から\n");
	wprintf(L" cmudict.0.7a をDLすること\n");
	wprintf(L"\n");
}

//オプションを配列にマップ



int main(int argc, const char **argv)
{
	srand((unsigned int)time(NULL));
#if _MSC_VER
	setlocale(LC_ALL, "");
#else
	std::setlocale(LC_ALL, "");
	fwide(stdout,1);
	fwide(stderr,1);
#endif

	//デバッグビルドだと毎回テストを実行します. 慈悲はない.
	SEXYTEST_RUNNER();

	//オプションを配列にマップ
	if (! Option::m()->Create(argc,argv) )
	{
		wprintf(L"\n");
		usage();
		return 0;
	}

	if (argc <= 2)
	{
//		wiki2yomiConvert("jawiki-latest-pages.xml");
//		wiki2ruigiConvert("jawiki-latest-pages.xml");
//		wiki2engConvert("jawiki-latest-pages.xml");
//		cmudic2yomiConvert("cmudict.0.7a");
//		wiki2charaConvert("jawiki-latest-pages.xml");
//		wiki2categoryConvert("jawiki-latest-pages.xml");
		usage();
		return 0;
	}

	//モードによって振り分け
	if ( strcmp(argv[1],"yomi") == 0)
	{
		wiki2yomiConvert(argv[2] );
	}
	else if ( strcmp(argv[1],"ruigi") == 0)
	{
		wiki2ruigiConvert(argv[2] );
	}
	else if ( strcmp(argv[1],"eng") == 0)
	{
		wiki2engConvert(argv[2] );
	}
	else if ( strcmp(argv[1],"chara") == 0)
	{
		wiki2charaConvert(argv[2] );
	}
	else if ( strcmp(argv[1],"category") == 0)
	{
		wiki2categoryConvert(argv[2] );
	}
	else if ( strcmp(argv[1],"cmudict") == 0)
	{
		cmudic2yomiConvert(argv[2] );
	}
	else
	{
		wprintf(L"パラメタ(%ls)は存在しません。死んで詫びよ。\n",_A2W(argv[1]).c_str() );
		wprintf(L"\n");
		usage();
		return 0;
	}

	return 0;
}
