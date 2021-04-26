#include "../head file/wadSave.h"
#include "../head file/wadStruct.h"

string xxhash64_to_str(string str, int str_length)
{
	unsigned __int64 retI64u = XXH64(str.c_str(), str_length, 0);
	std::stringstream stsm;
	std::string sha256;
	stsm << std::setfill('0') << std::setw(16) << std::hex << retI64u;
	stsm >> sha256;
	return sha256;
}

string str_to_Lower(string str)
{
	transform(str.begin(), str.end(), str.begin(), tolower);
	string str2 = str;
	return str2;
}

string str_replace_all(string str, string replace_source, string replace_newStr)
{
	int offindex = str.find(replace_source, 0);
	while (offindex != string::npos)
	{
		int nlen = replace_source.length();
		str.replace(offindex, nlen, replace_newStr);//出现位置 长度 内容
		offindex += 1;
		offindex = str.find(replace_source, offindex);
	}
	string strRet = str;
	return strRet;
}

unsigned __int64 xxhash64_to_i64u(string str, int str_length)
{
	unsigned __int64 retI64u = XXH64(str.c_str(), str_length, 0);
	return retI64u;
}

size_t zstd_compress(const char* src, size_t src_size, std::string& retdata)
{
	size_t const BuffSize = ZSTD_compressBound(src_size);
	retdata.resize(BuffSize);
	auto dstp = const_cast<void*>(static_cast<const void*>(retdata.c_str()));
	size_t const Size = ZSTD_compress(dstp, BuffSize, src, src_size, 3);//Compression level 3
	if (ZSTD_isError(Size))
	{
		return 0;
	}
	return Size;
}

int fileSearch(string directory, int directory_length, string filespec, Wad_Struct* p)
{
	if (directory == "") { return 1; }
	if (directory.substr(directory.size() - 1, 1) == "\\" || directory.substr(directory.size() - 1, 1) == "/") { return 2; }
	if (filespec == "") { filespec = "*.*"; }

	int Handle;//句柄
	string CurrentDirectory;//当前目录
	CurrentDirectory = directory + "\\" + filespec;

	struct _finddatai64_t filefind;//存储文件信息的结构体	
	if ((Handle = _findfirsti64(CurrentDirectory.c_str(), &filefind)) == -1) { return 3; }//初始化文件句柄

	while (_findnexti64(Handle, &filefind) != -1)//遍历当前文件夹下所有文件
	{

		if (strcmp(filefind.name, "..") == 0) { continue; }//跳过上层目录
		if ((_A_SUBDIR == filefind.attrib))//文件属性是否文件夹
		{
			CurrentDirectory = directory + "\\" + filefind.name;
			fileSearch(CurrentDirectory, directory_length, filespec, p);//查找子目录
		}
		else
		{
			if (filefind.size != 0 && filefind.size <= 1024 * 1024 * 300)//一般单文件都不会超过300mb
			{
				CurrentDirectory = directory + "\\" + filefind.name;
				string FilePath_buf = str_replace_all(CurrentDirectory, "/", "\\");//转换斜杠
				string FilePath = str_to_Lower(FilePath_buf);//转换小写	
				if (FilePath.size() > 255) { continue; }

				//************************************************************************************ 
				p->FilePath = FilePath;//存入条目信息 -> 文件路径
				cout << "FilePath 文件路径: " << FilePath << endl;

				p->UncompressedSize = (unsigned int)filefind.size;//存入条目信息 -> 未压缩大小
				cout << "UncompressedSize 未压缩大小: " << filefind.size << endl;

				string EntryName_buf = FilePath.erase(0, directory_length);//取出条目路径
				string EntryName = str_replace_all(EntryName_buf, "\\", "/");//斜杠转换				
				p->EntryName = EntryName;//存入条目信息 -> 条目名
				cout << "EntryName 条目名: " << EntryName << endl;

				unsigned __int64 xxHash_ulong = xxhash64_to_i64u(EntryName.c_str(), EntryName.length());
				p->xxHash_ulong = xxHash_ulong; //存入条目信息->长整数哈希
				cout << "xxHash_ulong 长整数哈希: " << xxHash_ulong << endl;

				string xxHash_str = xxhash64_to_str(EntryName.c_str(), EntryName.length());
				p->xxHash_str = xxHash_str;//存入条目信息 -> 文本哈希
				cout << "xxHash_str 文本哈希: " << xxHash_str << endl;

				string Extension = PathFindExtension(FilePath.c_str());
				p->Extension = Extension;//存入条目信息, 拓展名	.dds

				v.push_back(*p);
			}
		}
	}
	_findclose(Handle);//关闭句柄
	return 0;
}

int write_EntryData(const char* out_fileName, bool Compressed)
{
	if (v.empty()) { return 1; }
	if (Compressed == 1){ Compressed = true; }
	if (Compressed == 0) { Compressed = false; }
	ofstream head(out_fileName, ios::binary);
	if (!head.is_open())
	{
		cout << "open error ; 文件被占用" << endl;
		return 2;
	}
	unsigned int entry_count = v.size();
	head.write(wad_Header_30, sizeof(wad_Header_30));//填充头部
	head.write((char*)&entry_count, sizeof(entry_count));//写入文件数量
	char* entry_buf = new char[entry_count * 32]();
	head.write(entry_buf, (__int64)entry_count * 32);//填充条目信息
	delete[] entry_buf;//释放内存
	head.close();
	
	for (auto it = v.begin(); it != v.end(); it++)
	{
		ifstream in(it->FilePath, ios::binary);
		char* c_buf = new char[it->UncompressedSize]();//申请内存
		in.read(c_buf, it->UncompressedSize);//读入文件
		in.close();//关闭文件

		ofstream out(out_fileName, ios::app | ios::binary);
		string ret_buf;
		size_t ret_size = zstd_compress(c_buf, it->UncompressedSize, ret_buf);//压缩数据
		const char* retData = ret_buf.c_str();

		if (Compressed)//压缩
		{
			if (str_to_Lower(it->Extension) == ".wpk")//不压缩wpk文件
			{
				unsigned __int64 sha256_ = sha256(c_buf, it->UncompressedSize);
				it->SHA256 = sha256_;//存入条目信息-sha256
				out.write(c_buf, it->UncompressedSize);
				it->CompressedSize = it->UncompressedSize;//存入条目信息-未压缩大小
				unsigned int 当前指针 = (unsigned int)out.tellp();
				it->DataOffset = 当前指针 - (it->UncompressedSize);//存入条目信息-数据偏移
			}
			else
			{
				unsigned __int64 sha256_ = sha256(retData, ret_size);
				it->SHA256 = sha256_;//存入条目信息-sha256
				out.write(retData, ret_size);
				it->CompressedSize = ret_size;//存入条目信息-压缩后大小
				unsigned int Current_pointer = (unsigned int)out.tellp();
				it->DataOffset = Current_pointer - ret_size;//存入条目信息-数据偏移
			}
		}
		else//不压缩
		{
			unsigned __int64 sha256_ = sha256(c_buf, it->UncompressedSize);
			it->SHA256 = sha256_;//存入条目信息-sha256
			out.write(c_buf, it->UncompressedSize);
			it->CompressedSize = it->UncompressedSize;//存入条目信息-未压缩大小
			unsigned int 当前指针 = (unsigned int)out.tellp();
			it->DataOffset = 当前指针 - (it->UncompressedSize);//存入条目信息-数据偏移
		}
		out.close();//关闭写文件
		delete[] c_buf;//释放内存
	}
	return 0;
}

int write_EntryInfo(const char* out_fileName, bool Compressed)
{
	if (v.empty()) { return 1; }
	if (Compressed == 1) { Compressed = true; }
	if (Compressed == 0) { Compressed = false; }
	
	ofstream out(out_fileName, ios::in | ios::out | ios::binary);
	if (!out.is_open())
	{ 
		cout << "open error ; 文件被占用" << endl;
		return 2;
	}
	int infoOffset = 272;
	out.seekp(infoOffset, ios::beg);//beg cur end

	for (auto it = v.begin(); it != v.end(); it++)
	{
		string extension = str_to_Lower(it->Extension);
		unsigned __int64 xxhash_ = it->xxHash_ulong;
		out.write((char*)&xxhash_, sizeof(xxhash_));

		unsigned int DataOffset = it->DataOffset;
		out.write((char*)&DataOffset, sizeof(DataOffset));
		
		if (Compressed)
		{
			if (extension == ".wpk")
			{
				unsigned int CompressedSize = it->UncompressedSize;
				out.write((char*)&CompressedSize, sizeof(CompressedSize));//压缩大小
			}
			else
			{
				unsigned int CompressedSize = it->CompressedSize;
				out.write((char*)&CompressedSize, sizeof(CompressedSize));
			}
		}
		else
		{
			unsigned int CompressedSize = it->UncompressedSize;
			out.write((char*)&CompressedSize, sizeof(CompressedSize));
		}

		unsigned int UncompressedSize = it->UncompressedSize;
		out.write((char*)&UncompressedSize, sizeof(UncompressedSize));//未压缩大小

		if (Compressed)
		{
			if (extension == ".wpk")
			{
				char Type = Uncompressed;
				out.write((char*)&Type, sizeof(Type));//压缩类型
			}
			else
			{
				char Type = ZStandardCompressed;
				out.write((char*)&Type, sizeof(Type));
			}
		}
		else
		{
			char Type = Uncompressed;
			out.write((char*)&Type, sizeof(Type));
		}

		char Duplicated = it->Duplicated;
		out.write((char*)&Duplicated, sizeof(Duplicated));//重复

		unsigned short Unknown1 = it->Unknown1;
		out.write((char*)&Unknown1, sizeof(Unknown1));//保留

		unsigned __int64 SHA256_ulong = it->SHA256;
		out.write((char*)&SHA256_ulong, sizeof(SHA256_ulong));//sha256
	}
	out.close();
	return 0;
}