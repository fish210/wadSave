#pragma once
#include <Windows.h>
#include <fstream>
#include <io.h>
#include <algorithm>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include "wadStruct.h"
#include "xxhash/xxhash.h"
#include <iostream>
using namespace std;

//��ʼ���ṹ��
struct Wad_Struct wad_struct = { "", "", NULL, NULL, NULL, NULL, 0x03 , NULL, "" };

//�Զ�����������
vector<Wad_Struct> vec;

//���ܺ���

//�ļ�����
int fileSearch(string directory, int directory_length, string filespec, Wad_Struct* p);
//д��Ŀ����
int write_EntryData(const char* out_fileName, bool Compressed);
//д��Ŀ��Ϣ
int write_EntryInfo(const char* out_fileName, bool Compressed);
//�����滻�Ӵ�
string str_replace_all(string str, string replace_source, string replace_newStr);
//תСд
string str_to_Lower(string str);

