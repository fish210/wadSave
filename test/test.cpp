// test.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <iostream>
#include <windows.h>
#include "../wadSave/head file/wadSave.h"
#pragma comment(lib, "../Debug/wadSave.lib")
using namespace std;

int main()
{
    string directory = "directory";
    fileSearch(directory, directory.length() + 1, "", &wad_struct);//历遍文件, 包括子目录
    write_EntryData("new.wad.client", 1);//写条目数据, 0表示不压缩 1表示zstd压缩
    write_EntryInfo("new.wad.client", 1);//写条目信息, 0表示不压缩 1表示zstd压缩

    int a;
    cin >> a;
}


