
#pragma once

#include <locale.h>
#define _IS_WINDOWS_ENCODING_LOAD	1


#ifdef _MSC_VER
//windows でも UTF8で快適にコーディングできるようにする!!

namespace _my_encoding
{
	static std::wstring _atou16_convert(const char* inSrc)
	{
		const int srcSize = strlen(inSrc);
		std::vector<WCHAR> _buffer(srcSize * 10+1);
		
		_buffer[0] = 0;
		::MultiByteToWideChar(CP_ACP,0,inSrc,srcSize,&_buffer[0],_buffer.size() );

		return (const WCHAR*)&_buffer[0];
	}
	static std::wstring _atou16_convert(const std::string& inSrc)
	{
		const int srcSize = inSrc.size();
		std::vector<WCHAR> _buffer(srcSize * 10+1);
		
		_buffer[0] = 0;
		::MultiByteToWideChar(CP_ACP,0,inSrc.c_str(),srcSize,&_buffer[0],_buffer.size() );

		return (const WCHAR*)&_buffer[0];
	}

	static std::string _u16toa_convert(const WCHAR* inSrc)
	{
		const int srcSize = wcslen(inSrc);
		std::vector<char> _buffer(srcSize * 10+1);
		
		_buffer[0] = 0;
		::WideCharToMultiByte(CP_ACP,0,inSrc,srcSize,&_buffer[0],_buffer.size() , NULL,NULL);

		return &_buffer[0];
	}

	static std::string _u16toa_convert(const std::wstring& inSrc)
	{
		const int srcSize = inSrc.size();
		std::vector<char> _buffer(srcSize * 10+1);
		
		_buffer[0] = 0;
		::WideCharToMultiByte(CP_ACP,0,inSrc.c_str(),srcSize,&_buffer[0],_buffer.size() , NULL,NULL);

		return &_buffer[0];
	}

	static std::string _u16tou8_convert(const WCHAR* inSrc)
	{
		const int srcSize = wcslen(inSrc);
		std::vector<char> _buffer(srcSize * 10+1);

		_buffer[0] = 0;
		::WideCharToMultiByte(CP_UTF8,0,inSrc,srcSize,&_buffer[0],_buffer.size() , NULL,NULL);

		return &_buffer[0];
	}
	
	static std::string _u16tou8_convert(const std::wstring& inSrc)
	{
		const int srcSize = inSrc.size();
		std::vector<char> _buffer(srcSize * 10+1);

		_buffer[0] = 0;
		::WideCharToMultiByte(CP_UTF8,0,inSrc.c_str(),srcSize,&_buffer[0],_buffer.size() , NULL,NULL);

		return &_buffer[0];
	}

	static std::wstring _u8tou16_convert(const char* inSrc)
	{
		const int srcSize = strlen(inSrc);
		std::vector<WCHAR> _buffer(srcSize * 10+1);

		_buffer[0] = 0;
		::MultiByteToWideChar(CP_UTF8,0,inSrc,srcSize,&_buffer[0],_buffer.size() );

		return &_buffer[0];
	}
	static std::wstring _u8tou16_convert(const std::string&  inSrc)
	{
		const int srcSize = inSrc.size();
		std::vector<WCHAR> _buffer(srcSize * 10+1);

		_buffer[0] = 0;
		::MultiByteToWideChar(CP_UTF8,0,inSrc.c_str(),srcSize,&_buffer[0],_buffer.size() );

		return &_buffer[0];
	}

	static std::string _atou8_convert(const char * inSrc)
	{
		//ascii -> utf16に変換
		const std::wstring temp16str = _atou16_convert(inSrc);

		//utf16 -> utf8に変換
		return _u16tou8_convert(temp16str);
	}
	static std::string _atou8_convert(const std::string& inSrc)
	{
		//ascii -> utf16に変換
		const std::wstring temp16str = _atou16_convert(inSrc);

		//utf16 -> utf8に変換
		return _u16tou8_convert(temp16str);
	}


	static std::string _u8toa_convert(const char* inSrc)
	{
		//utf8 -> utf16に変換
		const std::wstring temp16str = _u8tou16_convert(inSrc);

		//utf16 -> asciiに変換
		return _u16toa_convert(temp16str);
	}
	
	static std::string _u8toa_convert(const std::string&  inSrc)
	{
		//utf8 -> utf16に変換
		const std::wstring temp16str = _u8tou16_convert(inSrc);

		//utf16 -> asciiに変換
		return _u16toa_convert(temp16str);
	}

};

//ascii -> utf16
#define _A2W(lpa) _my_encoding::_atou16_convert(lpa)
//utf16 -> acsii
#define _W2A(lpa) _my_encoding::_u16toa_convert(lpa)

//ascii -> utf8
#define _A2U(lpa) _my_encoding::_atou8_convert(lpa)
//utf8 -> ascii
#define _U2A(lpa) _my_encoding::_u8toa_convert(lpa)

//utf8 -> utf16
#define _U2W(lpa) _my_encoding::_u8tou16_convert(lpa)
//utf16 -> utf8
#define _W2U(lpa) _my_encoding::_u16tou8_convert(lpa)

/*
使い方
void hoge()
{
	const char * sjis = "アスキー"; 	//SJIS
//	UTF8が必要な関数(sjis);     		//UTF8にしたい・・・

	UTF8が必要な関数(_U2A(sjis));		//UTF8に変換して利用
}


void hoge2()
{
	const char * sjis = "アスキー"; //SJIS

	UTF8が必要な関数(_A2U(sjis));		//SJIS -> UTF8に変換して利用
	UTF16が必要な関数(_A2W(sjis));		//SJIS -> UTF16変換して利用

	const char * utf8 = "UTF8な文字列";	//UTF8
	SJISが必要な関数(_U2A(sjis));		//UTF8 -> SJISに変換
	UTF16が必要な関数(_U2W(sjis));		//UTF8 -> UTF16変換して利用
}
*/

#else  //_MSC_VER
//Unix環境では、 UTF-8 がディフォルトという前提で進める. もう誰も EUC-JP とか使っていないでしょ.

/*
//ascii -> utf16
#define _A2W(lpa)  lpa
//utf16 -> acsii
#define _W2A(lpa)  lpa

//ascii -> utf8
#define _A2U(lpa)  lpa
//utf8 -> ascii
#define _U2A(lpa)  lpa

//utf8 -> utf16
#define _U2W(lpa)  lpa
//utf16 -> utf8
#define _W2U(lpa)  lpa
*/

namespace _my_encoding
{
	static std::wstring _atou16_convert(const char* inSrc)
	{
		const int srcSize = strlen(inSrc);
		std::vector<WCHAR> _buffer(srcSize * 10+1);
		
		_buffer[0] = 0;

		mbstowcs(&_buffer[0],inSrc,_buffer.size());

		return (const WCHAR*)&_buffer[0];
	}
	static std::wstring _atou16_convert(const std::string& inSrc)
	{
		const int srcSize = inSrc.size();
		std::vector<WCHAR> _buffer(srcSize * 10+1);
		
		_buffer[0] = 0;

		mbstowcs(&_buffer[0],inSrc.c_str(),_buffer.size());

		return (const WCHAR*)&_buffer[0];
	}

	static std::string _u16toa_convert(const WCHAR* inSrc)
	{
		const int srcSize = wcslen(inSrc);
		std::vector<char> _buffer(srcSize * 10+1);
		
		_buffer[0] = 0;
		wcstombs(&_buffer[0],inSrc,_buffer.size());

		return &_buffer[0];
	}

	static std::string _u16toa_convert(const std::wstring& inSrc)
	{
		const int srcSize = inSrc.size();
		std::vector<char> _buffer(srcSize * 10+1);
		
		_buffer[0] = 0;
		wcstombs(&_buffer[0],inSrc.c_str(),_buffer.size());

		return &_buffer[0];
	}

};

//ascii -> utf16
#define _A2W(lpa) _my_encoding::_atou16_convert(lpa)
//utf16 -> acsii
#define _W2A(lpa) _my_encoding::_u16toa_convert(lpa)

//ascii -> utf8
#define _A2U(lpa) lpa
//utf8 -> ascii
#define _U2A(lpa) lpa

//utf8 -> utf16
#define _U2W(lpa) _my_encoding::_atou16_convert(lpa)
//utf16 -> utf8
#define _W2U(lpa) _my_encoding::_u16toa_convert(lpa)

/*
int main()
{
	std::setlocale(LC_ALL, "");
	const char * utf8String = "UTF8な文字列";	//UTF8
	std::wstring utf16String2 = L"UTF16な文章";

	std::wstring utf16String = _A2W("UTF8な文字列" );
	std::string utf8String_ = _W2A(utf16String.c_str() );
	
	printf("%s\n",utf8String);
	printf("%s\n",utf8String_.c_str() );
	return 0;
}
*/

#endif //_MSC_VER