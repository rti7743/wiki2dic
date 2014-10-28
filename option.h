
#pragma once

//オプション。　めんどうなので singleton。
//singleton(ようするに隠れたグローバル変数)がきらいなプログラマー(書き手)なんていません。
//singletonが好きなプログラマー(読み手)がいないのと一緒です。
class Option
{
	//singleton
	Option()
	{
	}
public:
	virtual ~Option()
	{
	}

	enum TypeCase 
	{
		 TypeCase_None = 0	//何もしない
		,TypeCase_Kata = 1	//カタカナ(ディフォルト)
		,TypeCase_Kana = 2	//ひらがな
	};

	enum TypeShow 
	{
		 TypeShow_NoEmpty = 0	//項目が空ではないものだけ
		,TypeShow_All = 1		//全部
		,TypeShow_Empty = 2		//項目が空のものだけ
	};
	
	enum TypeAimai 
	{
		 TypeAimai_Del = 0		//曖昧さの解決の()を消す
		,TypeAimai_Keep = 1		//曖昧さの解決の()を残す
	};
	
	//シングルトンインスタンスの作成.
	static Option* m()
	{
		static Option o;
		return &o;
	}
	
	bool Create(int argc, const char **argv)
	{
		ArgsToMap(argc,argv);
		
		//よく参照する項目をintとかにでもする.

		//出力の読みはどうするか
		const std::string _case = Get("case","kata");
		if (_case == "none")
		{//何もない
			this->Case = TypeCase_None;
		}
		else if (_case == "kata")
		{//カタカナ
			this->Case = TypeCase_Kata;
		}
		else if (_case == "kana")
		{//ひらがな
			this->Case = TypeCase_Kana;
		}
		else
		{//イミフ
			wprintf(L"ちょwwwおまwww、、 --caseオプションの値 %ls とは、イミフメイすぎてうけるwww\n",_A2W(_case).c_str() );
			return false;
		}

		const std::string _show = Get("show","noempty");
		if (_show == "noempty")
		{//空ではないものだけ
			this->Show = TypeShow_NoEmpty;
		}
		else if (_show == "all")
		{//カタカナ
			this->Show = TypeShow_All;
		}
		else if (_show == "empty")
		{//空ではないものだけ
			this->Show = TypeShow_Empty;
		}
		else
		{//イミフ
			wprintf(L"ちょwwwおまwww、、 --showオプションの値 %ls とは、イミフメイすぎてうけるwww\n",_A2W(_show).c_str() );
			return false;
		}

		const std::string _aimai = Get("aimai","del");
		if (_aimai == "del")
		{//曖昧さの解決の()を消すかどうか 消す
			this->Aimai = TypeAimai_Del;
		}
		else if (_aimai == "keep")
		{//カタカナ
			this->Aimai = TypeAimai_Keep;
		}
		else
		{//イミフ
			wprintf(L"ちょwwwおまwww、、 --aimaiオプションの値 %ls とは、イミフメイすぎてうけるwww\n",_A2W(_aimai).c_str() );
			return false;
		}

		return true;
	}

	//汎用 key=valueゲット.
	std::string Get(const std::string& key,const std::string& def = "")
	{
		return mapfind(this->OptionMap,"--"+key,def);
	}
	
	
	TypeCase getCase() const
	{
		return this->Case;
	}
	TypeShow getShow() const
	{
		return this->Show;
	}
	TypeAimai getAimai() const
	{
		return this->Aimai;
	}
	
private:
	//オプションを格納する配列
	std::map<std::string,std::string> OptionMap;
	//よく使うオプションのキャッシュ
	TypeCase Case;	//読みの変換
	TypeShow Show;	//表示する項目の選択
	TypeAimai Aimai;//曖昧さの解決の()をどうするか

	void ArgsToMap(int argc, const char **argv)
	{
		this->OptionMap.clear();
		for(int i = 0 ; i < argc ; i ++ )
		{
			const char * eq = strchr(argv[i],'=');
			if (eq == NULL)
			{
				this->OptionMap[argv[i]] = "";
			}
			else
			{
				this->OptionMap[std::string(argv[i] , 0 , eq - argv[i] )] = eq + 1;
			}
		}
	}

};
