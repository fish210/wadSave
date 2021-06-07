#include "wadSave.h"

//zstd函数声明-动态加载
typedef size_t(*Bound)(size_t srcSize);
typedef size_t(*compress)(void* dst, size_t dstCapacity, const void* src, size_t srcSize, int compressionLevel);
Bound ZSTD_compressBound;
compress ZSTD_compress;

int main()
{
	//*********************************************************** Init
	HMODULE hdll;
	hdll = LoadLibraryA("libzstd.dll");
	if (hdll == 0)
	{
		cout << "load error, not libzstd.dll ; 模块加载失败 libzstd.dll\n" << endl;
		return 0;
	}
	ZSTD_compressBound = (Bound)GetProcAddress(hdll, "ZSTD_compressBound");
	if (ZSTD_compressBound == 0)
	{
		FreeLibrary(hdll);
		cout << "load error not ZSTD_compressBound ; 找不到函数 ZSTD_compressBound\n" << endl;
		return 0;
	}
	ZSTD_compress = (compress)GetProcAddress(hdll, "ZSTD_compress");//ZSTD_decompress
	if (ZSTD_compress == 0)
	{
		FreeLibrary(hdll);
		cout << "load error not ZSTD_compress ; 找不到函数 ZSTD_compress\n" << endl;
		return 0;
	}
	//*********************************************************** Start
	string directory = "directory";//文件目录
	bool Compress = true;//是否压缩
	const char* FileName = "new.wad.client";

	fileSearch(directory, directory.length() + 1, "", &wad_struct);//历遍文件
	int err = write_EntryData(FileName, Compress);//写条目数据
	if (err == 0)
	{
		int err2 = write_EntryInfo(FileName, Compress);//写条目信息
		if (err2 == 0)
		{
			cout << "save success ; 保存文件成功 " << err << endl;
		}
		else
		{
			cout << "write_EntryInfo err ; " << err << endl;
		}		
		return 0;
	}
	cout << "write_EntryData err; " << err << endl;
	return 0;
}
int fileSearch(string directory, int directory_length, string filespec, Wad_Struct* p)
{
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

				unsigned __int64 xxHash_ulong = XXH64(EntryName.c_str(), EntryName.length(), 0);
				p->xxHash_ulong = xxHash_ulong; //存入条目信息->长整数哈希
				cout << "xxHash_ulong 长整数哈希: " << xxHash_ulong << endl;

				string Extension = PathFindExtension(FilePath.c_str());
				p->Extension = Extension;//存入条目信息, 拓展名	.dds
				cout << "Extension 拓展名: " << xxHash_ulong << endl;

				vec.push_back(*p);
			}
		}
	}
	_findclose(Handle);//关闭句柄
	return 0;
}
int write_EntryData(const char* out_fileName, bool Compressed)
{
	if (vec.empty()) { return 1; }
	ofstream head(out_fileName, ios::binary);
	if (!head.is_open())
	{
		cout << "open error ; 文件被占用" << endl;
		return 2;
	}
	unsigned int entry_count = vec.size();
	head.write(wad_Header_31, sizeof(wad_Header_31));//填充头部
	head.write((char*)&entry_count, sizeof(entry_count));//写入文件数量
	char* entry_buf = new char[entry_count * 32]();
	head.write(entry_buf, (__int64)entry_count * 32);//填充条目信息
	delete[] entry_buf;//释放内存
	head.close();

	for (auto it = vec.begin(); it != vec.end(); it++)
	{
		ifstream in(it->FilePath, ios::binary);
		char* c_buf = new char[it->UncompressedSize]();//申请内存
		in.read(c_buf, it->UncompressedSize);//读入文件
		in.close();//关闭文件

		ofstream out(out_fileName, ios::app | ios::binary);//写出文件
		size_t decSize = ZSTD_compressBound(it->UncompressedSize);
		char* decBuf = new char[decSize]();
		size_t retSize = ZSTD_compress(decBuf, decSize, c_buf, it->UncompressedSize, 1);//压缩数据
		
		if (Compressed)//压缩
		{
			if (str_to_Lower(it->Extension) == ".wpk")//不压缩wpk文件 XXH3_64bits
			{				
				out.write(c_buf, it->UncompressedSize);
				it->XXH3 = XXH3_64bits(c_buf, it->UncompressedSize);//存入条目信息-XXH3				
				it->CompressedSize = it->UncompressedSize;//存入条目信息-未压缩大小
				it->DataOffset = (unsigned int)out.tellp() - it->UncompressedSize;//存入条目信息-数据偏移
			}
			else
			{
				out.write(decBuf, retSize);
				it->XXH3 = XXH3_64bits(decBuf, retSize);//存入条目信息-XXH3				
				it->CompressedSize = retSize;//存入条目信息-压缩后大小
				it->DataOffset = (unsigned int)out.tellp() - retSize;//存入条目信息-数据偏移
			}
		}
		else//不压缩
		{
			out.write(c_buf, it->UncompressedSize);
			it->XXH3 = XXH3_64bits(c_buf, it->UncompressedSize);//存入条目信息-XXH3				
			it->CompressedSize = it->UncompressedSize;//存入条目信息-未压缩大小
			it->DataOffset = (unsigned int)out.tellp() - it->UncompressedSize;//存入条目信息-数据偏移
		}
		out.close();//关闭写文件
		delete[] c_buf;//释放内存
		c_buf = 0;
		delete[] decBuf;//释放内存
		decBuf = 0;
	}
	return 0;
}
int write_EntryInfo(const char* out_fileName, bool Compressed)
{
	if (vec.empty()) { return 1; }
	ofstream out(out_fileName, ios::in | ios::out | ios::binary);
	if (!out.is_open())
	{
		cout << "open error ; 文件被占用" << endl;
		return 2;
	}
	int infoOffset = 272;
	out.seekp(infoOffset, ios::beg);//beg cur end

	for (auto it = vec.begin(); it != vec.end(); it++)
	{
		string extension = str_to_Lower(it->Extension);
		out.write((char*)&it->xxHash_ulong, 8);
		out.write((char*)&it->DataOffset, 4);

		if (Compressed)
		{
			if (extension == ".wpk")//强制不压缩wpk文件
			{
				out.write((char*)&it->UncompressedSize, 4);//压缩大小
			}
			else
			{
				out.write((char*)&it->CompressedSize, 4);
			}
		}
		else
		{
			out.write((char*)&it->UncompressedSize, 4);
		}
		out.write((char*)&it->UncompressedSize, 4);//未压缩大小

		if (Compressed)
		{
			if (extension == ".wpk")//强制不压缩wpk文件
			{
				char Type = 0x00;
				out.write((char*)&Type, 1);//压缩类型
			}
			else
			{
				char Type = 0x03;
				out.write((char*)&Type, 1);
			}
		}
		else
		{
			char Type = 0x00;
			out.write((char*)&Type, 1);
		}

		char Reserved_01 = 0x00;
		out.write((char*)&Reserved_01, 1);//保留1

		unsigned short Reserved_02 = 0x00;
		out.write((char*)&Reserved_02, 2);//保留2

		out.write((char*)&it->XXH3, 8);
	}
	out.close();
	return 0;
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
