#pragma once
#include <iomanip>
#include "picosha2.h"

unsigned __int64 sha256(const char* str, unsigned int str_len);//返回长整数

std::string sha256Str_buf(std::string sha256_16byte);
std::string sha256Str(const char* str, unsigned int str_len);//返回sha256文本8位

std::string sha256_to_str(const char* str, unsigned int str_len);//返回string

std::string sha256_get16byte(const char* str, unsigned int str_len);//返回前16个字节

std::string sha256_Substr16byte(std::string str);//截取16位

std::string sha256_ReverseStr(std::string hex_string);//反转

unsigned __int64 sha256_to_I64u(std::string sha256_16byte);//转长整数

unsigned __int64 sha256(const char* str, unsigned int str_len)
{
	std::string _sha256 = sha256_to_str(str, str_len);
	std::string _sha256_16byte = sha256_ReverseStr(sha256_Substr16byte(_sha256));
	unsigned __int64 _sha256_ulong = sha256_to_I64u(_sha256_16byte);
	return _sha256_ulong;
}

std::string sha256Str_buf(std::string sha256_16byte)
{
	std::stringstream stsm;
	std::string sha256;
	stsm << std::setfill('0') << std::setw(16) << std::hex << sha256_16byte;
	stsm >> sha256;
	return sha256;
}

std::string sha256Str(const char* str, unsigned int str_len)
{
	std::string _sha256 = sha256_to_str(str, str_len);
	//cout << _sha256 << "\n";
	std::string _sha256_16byte = sha256_ReverseStr(sha256_Substr16byte(_sha256));
	//cout << _sha256数据8位 << "\n";
	std::string _sha256_str = sha256Str_buf(_sha256_16byte);
	return _sha256_str;
}

std::string sha256_to_str(const char* str, unsigned int str_len)
{
	std::string retStr;
	picosha2::hash256_hex_string(str, str + str_len, retStr);
	return retStr;
}

std::string sha256_get16byte(const char* str, unsigned int str_len)
{
	std::string strRet;
	picosha2::hash256_hex_string(str, str + str_len, strRet);
	return sha256_Substr16byte(strRet);
}

std::string sha256_Substr16byte(std::string str)
{
	if (str.size() != 0)
	{
		return str.substr(0, 16);
	}
	return "";
}

std::string sha256_ReverseStr(std::string hex_string)
{
	if (hex_string.size() == 16)
	{
		std::string ret = hex_string.substr(14, 2) + hex_string.substr(12, 2)
			+ hex_string.substr(10, 2) + hex_string.substr(8, 2) + hex_string.substr(6, 2)
			+ hex_string.substr(4, 2) + hex_string.substr(2, 2) + hex_string.substr(0, 2);
		return ret;
	}
	return "";
}

unsigned __int64 sha256_to_I64u(std::string sha256_16byte)
{
	std::stringstream stsm;
	unsigned __int64 sha256;
	stsm << std::hex << sha256_16byte;
	stsm >> sha256;
	return sha256;
}
