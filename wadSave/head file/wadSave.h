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

struct Wad_Struct wad_struct = { "", "", "", NULL, NULL, NULL, NULL, ZStandardCompressed , NULL, NULL, NULL, "" };//初始化结构体

//自定义类型数组
vector<Wad_Struct> v;

//功能函数
int fileSearch(string directory, int directory_length, string filespec, Wad_Struct* p);//文件搜索
unsigned __int64 xxhash64_to_i64u(string str, int str_length);//计算长整数哈希
string xxhash64_to_str(string str, int str_length);//计算文本哈希
string str_to_Lower(string str);//转小写
string str_replace_all(string str, string replace_source, string replace_newStr);//批量替换子串
size_t zstd_compress(const char* src, size_t src_size, std::string& retdata);//压缩
int write_EntryData(const char *out_fileName, bool Compressed);//写条目数据
int write_EntryInfo(const char* out_fileName, bool Compressed);//写条目信息
