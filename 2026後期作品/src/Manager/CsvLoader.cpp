#include <cerrno>
#include <climits>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include "CsvLoader.h"

std::vector<CsvLoader::Row> CsvLoader::Load(const std::string& path, bool skipHeader)
{

	std::vector<Row> rows;

	std::ifstream ifs(path);
	if (!ifs)
	{
		// ファイルが開けない
		return rows;
	}

	std::string line;
	bool isFirstLine = true;
	bool isHeader = skipHeader;

	while (std::getline(ifs, line))
	{

		// 先頭行のUTF-8 BOM(EF BB BF)を除去
		if (isFirstLine)
		{
			isFirstLine = false;
			if (line.size() >= 3 &&
				static_cast<unsigned char>(line[0]) == 0xEF &&
				static_cast<unsigned char>(line[1]) == 0xBB &&
				static_cast<unsigned char>(line[2]) == 0xBF)
			{
				line.erase(0, 3);
			}
		}

		// 改行コード(CRLF)の除去
		if (!line.empty() && line.back() == '\r')
		{
			line.pop_back();
		}

		// 空行・コメント行は無視
		if (line.empty() || line[0] == '#')
		{
			continue;
		}

		// ヘッダー行は読み飛ばす
		if (isHeader)
		{
			isHeader = false;
			continue;
		}

		rows.push_back(Split(line));

	}

	return rows;

}

CsvLoader::Row CsvLoader::Split(const std::string& line)
{

	Row row;
	std::stringstream ss(line);
	std::string cell;

	while (std::getline(ss, cell, ','))
	{
		row.push_back(Trim(cell));
	}

	return row;

}

int CsvLoader::ToInt(const std::string& str, int defaultValue)
{
	// std::stoi は変換できない時に例外を投げる（デバッガ接続中は1回ごとに大きなコストがかかる）ため、
	// 例外を使わない strtol で変換する
	if (str.empty())
	{
		return defaultValue;
	}

	char* end = nullptr;
	errno = 0;
	long value = std::strtol(str.c_str(), &end, 10);

	// 1文字も数値として読めなかった、または範囲外の場合は既定値
	if (end == str.c_str() || errno == ERANGE ||
		value < INT_MIN || value > INT_MAX)
	{
		return defaultValue;
	}

	return static_cast<int>(value);
}

float CsvLoader::ToFloat(const std::string& str, float defaultValue)
{
	// ToInt と同じ理由で、例外を使わない strtof で変換する
	if (str.empty())
	{
		return defaultValue;
	}

	char* end = nullptr;
	errno = 0;
	float value = std::strtof(str.c_str(), &end);

	if (end == str.c_str() || errno == ERANGE)
	{
		return defaultValue;
	}

	return value;
}

std::string CsvLoader::Trim(const std::string& str)
{

	const char* WHITESPACE = " \t\r\n";

	size_t begin = str.find_first_not_of(WHITESPACE);
	if (begin == std::string::npos)
	{
		return "";
	}
	size_t end = str.find_last_not_of(WHITESPACE);

	return str.substr(begin, end - begin + 1);

}