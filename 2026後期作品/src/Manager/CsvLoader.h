#pragma once
#include <DxLib.h>
#include <string>
#include <vector>

class CsvLoader
{
public:

	// 1行分のセル
	using Row = std::vector<std::string>;

	// CSVを読み込む
	// ・1行目はヘッダーとして読み飛ばす（skipHeader = false で無効化）
	// ・空行と「#」で始まる行は無視
	// ・UTF-8のBOMがあれば除去
	// ・ファイルが開けない場合は空の配列を返す
	static std::vector<Row> Load(const std::string& path, bool skipHeader = true);

	// 1行をカンマで分割（各セルの前後の空白は除去）
	static Row Split(const std::string& line);

	// 文字列から数値への変換（失敗した場合は defaultValue を返す）
	static int ToInt(const std::string& str, int defaultValue = 0);
	static float ToFloat(const std::string& str, float defaultValue = 0.0f);

private:

	// 前後の空白・改行を除去
	static std::string Trim(const std::string& str);
};

