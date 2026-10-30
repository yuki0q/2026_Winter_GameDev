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
	try
	{
		return std::stoi(str);
	}
	catch (...)
	{
		return defaultValue;
	}
}

float CsvLoader::ToFloat(const std::string& str, float defaultValue)
{
	try
	{
		return std::stof(str);
	}
	catch (...)
	{
		return defaultValue;
	}
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