#include "wadSave.h"

//zstd��������-��̬����
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
		cout << "load error, not libzstd.dll ; ģ�����ʧ�� libzstd.dll\n" << endl;
		return 0;
	}
	ZSTD_compressBound = (Bound)GetProcAddress(hdll, "ZSTD_compressBound");
	if (ZSTD_compressBound == 0)
	{
		FreeLibrary(hdll);
		cout << "load error not ZSTD_compressBound ; �Ҳ������� ZSTD_compressBound\n" << endl;
		return 0;
	}
	ZSTD_compress = (compress)GetProcAddress(hdll, "ZSTD_compress");//ZSTD_decompress
	if (ZSTD_compress == 0)
	{
		FreeLibrary(hdll);
		cout << "load error not ZSTD_compress ; �Ҳ������� ZSTD_compress\n" << endl;
		return 0;
	}
	//*********************************************************** Start
	string directory = "directory";//�ļ�Ŀ¼
	bool Compress = true;//�Ƿ�ѹ��
	const char* FileName = "new.wad.client";

	fileSearch(directory, directory.length() + 1, "", &wad_struct);//�����ļ�
	int err = write_EntryData(FileName, Compress);//д��Ŀ����
	if (err == 0)
	{
		int err2 = write_EntryInfo(FileName, Compress);//д��Ŀ��Ϣ
		if (err2 == 0)
		{
			cout << "save success ; �����ļ��ɹ� " << err << endl;
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

				unsigned __int64 xxHash_ulong = XXH64(EntryName.c_str(), EntryName.length(), 0);
				p->xxHash_ulong = xxHash_ulong; //������Ŀ��Ϣ->��������ϣ
				cout << "xxHash_ulong ��������ϣ: " << xxHash_ulong << endl;

				string Extension = PathFindExtension(FilePath.c_str());
				p->Extension = Extension;//������Ŀ��Ϣ, ��չ��	.dds
				cout << "Extension ��չ��: " << xxHash_ulong << endl;

				vec.push_back(*p);
			}
		}
	}
	_findclose(Handle);//�رվ��
	return 0;
}
int write_EntryData(const char* out_fileName, bool Compressed)
{
	if (vec.empty()) { return 1; }
	ofstream head(out_fileName, ios::binary);
	if (!head.is_open())
	{
		cout << "open error ; �ļ���ռ��" << endl;
		return 2;
	}
	unsigned int entry_count = vec.size();
	head.write(wad_Header_31, sizeof(wad_Header_31));//���ͷ��
	head.write((char*)&entry_count, sizeof(entry_count));//д���ļ�����
	char* entry_buf = new char[entry_count * 32]();
	head.write(entry_buf, (__int64)entry_count * 32);//�����Ŀ��Ϣ
	delete[] entry_buf;//�ͷ��ڴ�
	head.close();

	for (auto it = vec.begin(); it != vec.end(); it++)
	{
		ifstream in(it->FilePath, ios::binary);
		char* c_buf = new char[it->UncompressedSize]();//�����ڴ�
		in.read(c_buf, it->UncompressedSize);//�����ļ�
		in.close();//�ر��ļ�

		ofstream out(out_fileName, ios::app | ios::binary);//д���ļ�
		size_t decSize = ZSTD_compressBound(it->UncompressedSize);
		char* decBuf = new char[decSize]();
		size_t retSize = ZSTD_compress(decBuf, decSize, c_buf, it->UncompressedSize, 1);//ѹ������
		
		if (Compressed)//ѹ��
		{
			if (str_to_Lower(it->Extension) == ".wpk")//��ѹ��wpk�ļ� XXH3_64bits
			{				
				out.write(c_buf, it->UncompressedSize);
				it->XXH3 = XXH3_64bits(c_buf, it->UncompressedSize);//������Ŀ��Ϣ-XXH3				
				it->CompressedSize = it->UncompressedSize;//������Ŀ��Ϣ-δѹ����С
				it->DataOffset = (unsigned int)out.tellp() - it->UncompressedSize;//������Ŀ��Ϣ-����ƫ��
			}
			else
			{
				out.write(decBuf, retSize);
				it->XXH3 = XXH3_64bits(decBuf, retSize);//������Ŀ��Ϣ-XXH3				
				it->CompressedSize = retSize;//������Ŀ��Ϣ-ѹ�����С
				it->DataOffset = (unsigned int)out.tellp() - retSize;//������Ŀ��Ϣ-����ƫ��
			}
		}
		else//��ѹ��
		{
			out.write(c_buf, it->UncompressedSize);
			it->XXH3 = XXH3_64bits(c_buf, it->UncompressedSize);//������Ŀ��Ϣ-XXH3				
			it->CompressedSize = it->UncompressedSize;//������Ŀ��Ϣ-δѹ����С
			it->DataOffset = (unsigned int)out.tellp() - it->UncompressedSize;//������Ŀ��Ϣ-����ƫ��
		}
		out.close();//�ر�д�ļ�
		delete[] c_buf;//�ͷ��ڴ�
		c_buf = 0;
		delete[] decBuf;//�ͷ��ڴ�
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
		cout << "open error ; �ļ���ռ��" << endl;
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
			if (extension == ".wpk")//ǿ�Ʋ�ѹ��wpk�ļ�
			{
				out.write((char*)&it->UncompressedSize, 4);//ѹ����С
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
		out.write((char*)&it->UncompressedSize, 4);//δѹ����С

		if (Compressed)
		{
			if (extension == ".wpk")//ǿ�Ʋ�ѹ��wpk�ļ�
			{
				char Type = 0x00;
				out.write((char*)&Type, 1);//ѹ������
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
		out.write((char*)&Reserved_01, 1);//����1

		unsigned short Reserved_02 = 0x00;
		out.write((char*)&Reserved_02, 2);//����2

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
		str.replace(offindex, nlen, replace_newStr);//����λ�� ���� ����
		offindex += 1;
		offindex = str.find(replace_source, offindex);
	}
	string strRet = str;
	return strRet;
}
