#pragma once
#include "pch.h"



namespace StringUtils
{
	// turn non ASCII characters into '?'
	inline std::string clean_wstring(const std::wstring& wStr)
	{
		std::wstring cleaned_wstr = wStr;

		for (int i = 0; i < wStr.size(); i++)
		{
			if (wStr[i] > 127)
			{
				cleaned_wstr[i] = 63;
			}
		}

		return std::string(wStr.begin(), wStr.end());
	}

	// turn non ASCII characters into '?'
	inline std::string clean_string(const std::string& str)
	{
		std::string cleaned_str = str;

		for (int i = 0; i < str.size(); i++)
		{
			if (str[i] > 127)
			{
				cleaned_str[i] = 63;
			}
		}

		return cleaned_str;
	}
}


namespace Files
{
	json get_json(const fs::path& file_path);
	bool write_json(const fs::path& file_path, const json& j, int indentation = 4);

	std::string get_text_content(const fs::path& file_path);
	void write_text_content(const fs::path file_path, const std::string& content);
}