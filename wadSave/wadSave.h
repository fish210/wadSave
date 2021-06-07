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

//初始化结构体
struct Wad_Struct wad_struct = { "", "", NULL, NULL, NULL, NULL, 0x03 , NULL, "" };

//自定义类型数组
vector<Wad_Struct> vec;

//功能函数

//文件搜索
int fileSearch(string directory, int directory_length, string filespec, Wad_Struct* p);
//写条目数据
int write_EntryData(const char* out_fileName, bool Compressed);
//写条目信息
int write_EntryInfo(const char* out_fileName, bool Compressed);
//批量替换子串
string str_replace_all(string str, string replace_source, string replace_newStr);
//转小写
string str_to_Lower(string str);

