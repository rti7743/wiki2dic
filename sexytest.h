
#pragma once

#define SEXYTEST_LINE_STRCAT(x,y) SEXYTEST_LINE_STRCAT_(x,y)
#define SEXYTEST_LINE_STRCAT_(x,y) x##y

#ifdef _DEBUG

	#ifdef _MSC_VER
		//msvc
		#define SEXYTEST_BREAKPOINT()			{ \
				MSG msg;	\
				BOOL bQuit = PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);	\
				if (bQuit)	PostQuitMessage((int)msg.wParam);	\
				DebugBreak(); \
			} 
	#else
		#define SEXYTEST_BREAKPOINT()	{ asm ("int $3") ; }
	#endif

	#ifdef _MSC_VER
		#define SEXYTEST_TRACE(msg)			{ OutputDebugString(msg); } 
	#else
		#define SEXYTEST_TRACE(msg)			{ puts(msg); }
	#endif
	
	//assertを上書きする
	#ifdef SEXYTEST_ASSERT_OVERRAIDE
		#ifdef assert
		#undef assert
		#endif
		#define assert(X)	SEXYTEST_ASSERT(X)
	#endif

	#ifndef SEXYTEST_FAIL_DO
		//テストが失敗したと起動するか？
		//
		//ログとブレークポイント
		#define SEXYTEST_FAIL_DO(msg)	{ SEXYTEST_TRACE(msg); SEXYTEST_BREAKPOINT(); }
		//ログと無視
		//#define SEXYTEST_FAIL_DO(msg)	{ SEXYTEST_TRACE(msg); }
		//無視
		//#define SEXYTEST_FAIL_DO(msg)  
		//ログと強制終了
		//#define SEXYTEST_FAIL_DO(msg) { SEXYTEST_TRACE(msg); exit(-1); }
	#endif


	//値ダンプ系 (stlコンテナに対してサービスする)
	template < typename _T >
	std::string SexyTestDump(const _T& s)
	{
		std::stringstream out;
		out << s;
		return out.str();
	}
	#ifdef _IS_WINDOWS_ENCODING_LOAD
		static std::string SexyTestDump(const std::wstring& s)
		{
			std::wstringstream out;
			out << s;
			return _W2A(out.str());
		}
	#endif

	#ifdef _MSC_VER
		#ifdef _VECTOR_
			#define SEXYTEST_VECTOR_DUMP_FLAG
		#endif 
		#ifdef _LIST_
			#define SEXYTEST_LIST_DUMP_FLAG
		#endif 
		#ifdef _DEQUE_
			#define SEXYTEST_DEQUE_DUMP_FLAG
		#endif 
		#ifdef _MAP_
			#define SEXYTEST_MAP_DUMP_FLAG
		#endif 
	#else
		#ifdef _GLIBCXX_VECTOR
			#define SEXYTEST_VECTOR_DUMP_FLAG
		#endif 
		#ifdef _GLIBCXX_LIST
			#define SEXYTEST_LIST_DUMP_FLAG
		#endif 
		#ifdef _GLIBCXX_DEQUE
			#define SEXYTEST_DEQUE_DUMP_FLAG
		#endif 
		#ifdef _GLIBCXX_MAP
			#define SEXYTEST_MAP_DUMP_FLAG
		#endif 
	#endif

	//拙者のC++力ではこれか限界でごさった。 誰か直して欲しいでござる.
	#ifdef SEXYTEST_VECTOR_DUMP_FLAG
		template < class _T > std::string SexyTestDump(const std::vector<_T>& stl){ return SexyTestDumpSTL(stl.begin(),stl.end()); }
	#endif
	#ifdef SEXYTEST_LIST_DUMP_FLAG
		template < class _T > std::string SexyTestDump(const std::list<_T>& stl){ return SexyTestDumpSTL(stl.begin(),stl.end()); }
	#endif

	#ifdef SEXYTEST_DEQUE_DUMP_FLAG
		template < class _T > std::string SexyTestDump(const std::deque<_T>& stl){ return SexyTestDumpSTL(stl.begin(),stl.end()); }
	#endif

	#ifdef SEXYTEST_MAP_DUMP_FLAG
		template < class _T,class _V > std::string SexyTestDump(const std::map<_T,_V>& stl){ return SexyTestDumpPairSTL(stl.begin(),stl.end()); }
	#endif

	template < typename _B,typename _T >
	std::string SexyTestDumpSTL(const _B& begin ,const  _T& end)
	{
		_B it = begin;
		std::stringstream out;
		if (end == it)
		{
			return "[]";
		}
		out << "[" << SexyTestDump(*it) ; ++it;

		for(; it != end ; ++it )
		{
			out << "," << SexyTestDump(*it) ;
		}
		out << "]";
		return out.str();
	}

	template < typename _B,typename _T >
	std::string SexyTestDumpPairSTL(const _B& begin ,const  _T& end)
	{
		_B it = begin;
		std::stringstream out;
		if (end == it)
		{
			return "[]";
		}
		out << "[" << SexyTestDump(it->first) << "=>" << SexyTestDump(it->second) ; ++it;

		for(; it != end ; ++it )
		{
			out << "," << SexyTestDump(it->first) << "=>" << SexyTestDump(it->second) ;
		}
		out << "]";
		return out.str();
	}

	//各種チェック用
	#define SEXYTEST_EQ(x,y) {\
			if ( (x) != (y) ){\
				std::stringstream out;\
				out << __FILE__ << "(" << __LINE__ << "):"  << " "<< #x << "(" << SexyTestDump(x) << ") == " << #y << "(" << SexyTestDump(y) << ")"  << std::endl;\
				SEXYTEST_FAIL_DO(out.str().c_str());\
			}\
		}
	#define SEXYTEST_ASSERT(x) {\
			if ( ! (x) ){\
				std::stringstream out;\
				out << __FILE__ << "(" << __LINE__ << "):"  << " "<< #x << "(" << SexyTestDump(x) << ")" << std::endl;\
				SEXYTEST_FAIL_DO(out.str().c_str());\
			}\
		}


	#define _SEXYTEST_LINE_STRCAT(x,y,z) _SEXYTEST_LINE_STRCAT_(x,y,z)
	#define _SEXYTEST_LINE_STRCAT_(x,y,z) x##y##z

	typedef void( *SEXYTEST_TESTFUNCTION_TYPE )();

	class SEXYTEST_DATASTORE
	{
	public:
		static SEXYTEST_DATASTORE* s()
		{
			static SEXYTEST_DATASTORE s;
			return &s;
		}
		std::list< SEXYTEST_TESTFUNCTION_TYPE > testStore;
	};

	class SexyTestUnitJoin
	{
	public:
		SexyTestUnitJoin( SEXYTEST_TESTFUNCTION_TYPE testfunc ,bool isQuick)
		{
			if (isQuick)	SEXYTEST_DATASTORE::s()->testStore.push_front(testfunc);
			else			SEXYTEST_DATASTORE::s()->testStore.push_back(testfunc);
		}
	};




	static void SEXYTEST_RUNNER()
	{
		int ie =  SEXYTEST_DATASTORE::s()->testStore.size();
		for(auto it = SEXYTEST_DATASTORE::s()->testStore.begin() ; it != SEXYTEST_DATASTORE::s()->testStore.end() ; ++it )
		{
			(*it)();
		}
	}


	//テストを書く
	#define SEXYTEST()  \
		static void SEXYTEST_LINE_STRCAT(SexyTestUnitTest,__LINE__)(); \
		static SexyTestUnitJoin SEXYTEST_LINE_STRCAT(SexyTestUnitJoin,__LINE__)(SEXYTEST_LINE_STRCAT(SexyTestUnitTest,__LINE__),false); \
		static void SEXYTEST_LINE_STRCAT(SexyTestUnitTest,__LINE__)()


	//今、デバッグしているところのテストを最優先で実行する.(依存の解説ではなく、デバッガーのブレークポイントの理由で使うよ。デバッグ終わったら、元のSEXYTEST()に戻してね)
	#define SEXYTEST_NOW()  \
		static void SEXYTEST_LINE_STRCAT(SexyTestUnitTest,__LINE__)(); \
		static SexyTestUnitJoin SEXYTEST_LINE_STRCAT(SexyTestUnitJoin,__LINE__)(SEXYTEST_LINE_STRCAT(SexyTestUnitTest,__LINE__),true); \
		static void SEXYTEST_LINE_STRCAT(SexyTestUnitTest,__LINE__)()

#else  //_DEBUG
	#define SEXYTEST_BREAKPOINT()
	#define SEXYTEST_TRACE(msg)

	//各種チェック用
	#define SEXYTEST_EQ(x,y)
	#define SEXYTEST_ASSERT(X)

	#define SEXYTEST_RUNNER()
	#define SEXYTEST()  \
		static void SEXYTEST_LINE_STRCAT(SexyTestUnitTest,__LINE__)()

	#define SEXYTEST_NOW()  \
		static void SEXYTEST_LINE_STRCAT(SexyTestUnitTest,__LINE__)()

#endif //_DEBUG
