#pragma once
#include <Windows.h>
#include <iostream>
#include <io.h>
#include <algorithm>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "wadStruct.h"
#include "../xxhash/xxhash.h"
#include "../sha256/picosha2.hpp"

#include "../zstd/zstd.h"
#pragma comment(lib, "libzstd.lib")

using namespace std;

struct Wad_Struct wad_struct = { "", "", "", NULL, NULL, NULL, NULL, ZStandardCompressed , NULL, NULL, NULL, "" };//��ʼ���ṹ��

//�Զ�����������
vector<Wad_Struct> v;

//���ܺ���
int fileSearch(string directory, int directory_length, string filespec, Wad_Struct* p);//�ļ�����
unsigned __int64 xxhash64_to_i64u(string str, int str_length);//���㳤������ϣ
string xxhash64_to_str(string str, int str_length);//�����ı���ϣ
string str_to_Lower(string str);//תСд
string str_replace_all(string str, string replace_source, string replace_newStr);//�����滻�Ӵ�
size_t zstd_compress(const char* src, size_t src_size, std::string& retdata);//ѹ��
int write_EntryData(const char *out_fileName, bool Compressed);//д��Ŀ����
int write_EntryInfo(const char* out_fileName, bool Compressed);//д��Ŀ��Ϣ
