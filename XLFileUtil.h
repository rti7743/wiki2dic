
#pragma once

class XLFileUtil  
{
public:
	XLFileUtil();
	virtual ~XLFileUtil();

	//ファイルが存在するか?
	static bool Exist(const std::string & inFileName);
	//ディレクトリが存在するか?
	static bool ExistDirectory(const std::string & dir);
	//削除
	static bool del(const std::string & inFileName);
	//ローテートする
	static void rotate(const std::string & inDir,const std::string& filenameprefix ,unsigned  int limit);
	static bool copy(const std::string & inFileNameA,const std::string & inFileNameB);
	static std::string pwd();
	static std::string getTempDirectory(const std::string& projectname);

	enum findfile_orderby_enum
	{
		 findfile_orderby_name_ascending
		,findfile_orderby_name_descending
		,findfile_orderby_size_ascending
		,findfile_orderby_size_descending
		,findfile_orderby_date_ascending
		,findfile_orderby_date_descending
	};

	static bool findfile(const std::string & dir,const std::function< bool(const std::string& filename,const std::string& fullfilename) >& callback);
	static bool findfile_orderby(const std::string & dir,findfile_orderby_enum orderby,const std::function< bool(const std::string& filename,const std::string& fullfilename) >& callback);

	static bool move(const std::string & inFileNameA,const std::string & inFileNameB);
	//ファイルをすべて string に読み込む.
	static std::string cat(const std::string & inFileName);
	//ファイルをすべて std::vector<char> に読み込む.
	static std::vector<char> cat_b(const std::string & inFileName);
	//ファイルをすべて out に読み込む
	static bool cat_b(const std::string & inFileName , std::vector<char>* out);
	//ファイルの終端からN行読み込む.(あんまり効率は良くない)
	static std::string tail(const std::string & inFileName,unsigned int lines);

	//inStr を ファイルに書き込む
	static bool write(const std::string & inFileName,const std::string & inStr );
	//inBuffer を ファイルに書き込む
	static bool write(const std::string & inFileName,const std::vector<char> & inBuffer);
	//inBuffer を ファイルに書き込む
	static bool write(const std::string & inFileName,const char* data , int size);

	static  size_t getfilesize(const std::string & inFileName);

	static  time_t getfiletime(const std::string & inFileName);
	static  bool setfiletime(const std::string & inFileName,time_t now);

	static void mkdirP(const std::string &dirname);

	static void DelTree(const std::string &path);

	static std::string FindOneDirecotry(const std::string& dir ,const std::string& name);
};

