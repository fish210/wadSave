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
		str.replace(offindex, nlen, replace_newStr);//����λ�� ���� ����
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

	int Handle;//���
	string CurrentDirectory;//��ǰĿ¼
	CurrentDirectory = directory + "\\" + filespec;

	struct _finddatai64_t filefind;//�洢�ļ���Ϣ�Ľṹ��	
	if ((Handle = _findfirsti64(CurrentDirectory.c_str(), &filefind)) == -1) { return 3; }//��ʼ���ļ����

	while (_findnexti64(Handle, &filefind) != -1)//������ǰ�ļ����������ļ�
	{

		if (strcmp(filefind.name, "..") == 0) { continue; }//�����ϲ�Ŀ¼
		if ((_A_SUBDIR == filefind.attrib))//�ļ������Ƿ��ļ���
		{
			CurrentDirectory = directory + "\\" + filefind.name;
			fileSearch(CurrentDirectory, directory_length, filespec, p);//������Ŀ¼
		}
		else
		{
			if (filefind.size != 0 && filefind.size <= 1024 * 1024 * 300)//һ�㵥�ļ������ᳬ��300mb
			{
				CurrentDirectory = directory + "\\" + filefind.name;
				string FilePath_buf = str_replace_all(CurrentDirectory, "/", "\\");//ת��б��
				string FilePath = str_to_Lower(FilePath_buf);//ת��Сд	
				if (FilePath.size() > 255) { continue; }

				//************************************************************************************ 
				p->FilePath = FilePath;//������Ŀ��Ϣ -> �ļ�·��
				cout << "FilePath �ļ�·��: " << FilePath << endl;

				p->UncompressedSize = (unsigned int)filefind.size;//������Ŀ��Ϣ -> δѹ����С
				cout << "UncompressedSize δѹ����С: " << filefind.size << endl;

				string EntryName_buf = FilePath.erase(0, directory_length);//ȡ����Ŀ·��
				string EntryName = str_replace_all(EntryName_buf, "\\", "/");//б��ת��				
				p->EntryName = EntryName;//������Ŀ��Ϣ -> ��Ŀ��
				cout << "EntryName ��Ŀ��: " << EntryName << endl;

				unsigned __int64 xxHash_ulong = xxhash64_to_i64u(EntryName.c_str(), EntryName.length());
				p->xxHash_ulong = xxHash_ulong; //������Ŀ��Ϣ->��������ϣ
				cout << "xxHash_ulong ��������ϣ: " << xxHash_ulong << endl;

				string xxHash_str = xxhash64_to_str(EntryName.c_str(), EntryName.length());
				p->xxHash_str = xxHash_str;//������Ŀ��Ϣ -> �ı���ϣ
				cout << "xxHash_str �ı���ϣ: " << xxHash_str << endl;

				string Extension = PathFindExtension(FilePath.c_str());
				p->Extension = Extension;//������Ŀ��Ϣ, ��չ��	.dds

				v.push_back(*p);
			}
		}
	}
	_findclose(Handle);//�رվ��
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
		cout << "open error ; �ļ���ռ��" << endl;
		return 2;
	}
	unsigned int entry_count = v.size();
	head.write(wad_Header_30, sizeof(wad_Header_30));//���ͷ��
	head.write((char*)&entry_count, sizeof(entry_count));//д���ļ�����
	char* entry_buf = new char[entry_count * 32]();
	head.write(entry_buf, (__int64)entry_count * 32);//�����Ŀ��Ϣ
	delete[] entry_buf;//�ͷ��ڴ�
	head.close();
	
	for (auto it = v.begin(); it != v.end(); it++)
	{
		ifstream in(it->FilePath, ios::binary);
		char* c_buf = new char[it->UncompressedSize]();//�����ڴ�
		in.read(c_buf, it->UncompressedSize);//�����ļ�
		in.close();//�ر��ļ�

		ofstream out(out_fileName, ios::app | ios::binary);
		string ret_buf;
		size_t ret_size = zstd_compress(c_buf, it->UncompressedSize, ret_buf);//ѹ������
		const char* retData = ret_buf.c_str();

		if (Compressed)//ѹ��
		{
			if (str_to_Lower(it->Extension) == ".wpk")//��ѹ��wpk�ļ�
			{
				unsigned __int64 sha256_ = sha256(c_buf, it->UncompressedSize);
				it->SHA256 = sha256_;//������Ŀ��Ϣ-sha256
				out.write(c_buf, it->UncompressedSize);
				it->CompressedSize = it->UncompressedSize;//������Ŀ��Ϣ-δѹ����С
				unsigned int ��ǰָ�� = (unsigned int)out.tellp();
				it->DataOffset = ��ǰָ�� - (it->UncompressedSize);//������Ŀ��Ϣ-����ƫ��
			}
			else
			{
				unsigned __int64 sha256_ = sha256(retData, ret_size);
				it->SHA256 = sha256_;//������Ŀ��Ϣ-sha256
				out.write(retData, ret_size);
				it->CompressedSize = ret_size;//������Ŀ��Ϣ-ѹ�����С
				unsigned int Current_pointer = (unsigned int)out.tellp();
				it->DataOffset = Current_pointer - ret_size;//������Ŀ��Ϣ-����ƫ��
			}
		}
		else//��ѹ��
		{
			unsigned __int64 sha256_ = sha256(c_buf, it->UncompressedSize);
			it->SHA256 = sha256_;//������Ŀ��Ϣ-sha256
			out.write(c_buf, it->UncompressedSize);
			it->CompressedSize = it->UncompressedSize;//������Ŀ��Ϣ-δѹ����С
			unsigned int ��ǰָ�� = (unsigned int)out.tellp();
			it->DataOffset = ��ǰָ�� - (it->UncompressedSize);//������Ŀ��Ϣ-����ƫ��
		}
		out.close();//�ر�д�ļ�
		delete[] c_buf;//�ͷ��ڴ�
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
		cout << "open error ; �ļ���ռ��" << endl;
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
				out.write((char*)&CompressedSize, sizeof(CompressedSize));//ѹ����С
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
		out.write((char*)&UncompressedSize, sizeof(UncompressedSize));//δѹ����С

		if (Compressed)
		{
			if (extension == ".wpk")
			{
				char Type = Uncompressed;
				out.write((char*)&Type, sizeof(Type));//ѹ������
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
		out.write((char*)&Duplicated, sizeof(Duplicated));//�ظ�

		unsigned short Unknown1 = it->Unknown1;
		out.write((char*)&Unknown1, sizeof(Unknown1));//����

		unsigned __int64 SHA256_ulong = it->SHA256;
		out.write((char*)&SHA256_ulong, sizeof(SHA256_ulong));//sha256
	}
	out.close();
	return 0;
}