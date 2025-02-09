#include "pch.h"
#include "Utils.hpp"



namespace Files
{
	json get_json(const fs::path& file_path)
	{
		json j;

		if (!fs::exists(file_path))
		{
			LOG("[ERROR] File doesn't exist: '{}'", file_path.string());
			return j;
		}

		try
		{
			std::ifstream file(file_path);
			file >> j;
		}
		catch (const std::exception& e)
		{
			LOG("[ERROR] Unable to read '{}': {}", file_path.filename().string(), e.what());
		}

		return j;
	}

	bool write_json(const fs::path& file_path, const json& j, int indentation)
	{
		try
		{
			std::ofstream file(file_path);

			if (file.is_open())
			{
				file << j.dump(indentation);
				file.close();
			}
			else {
				LOG("[ERROR] Couldn't open file for writing: {}", file_path.string());
			}

			return true;
		}
		catch (const std::exception& e)
		{
			LOG("[ERROR] Unable to write to '{}': {}", file_path.filename().string(), e.what());
			return false;
		}
	}


	std::string get_text_content(const fs::path& file_path)
	{
		std::ifstream Temp(file_path);
		std::stringstream Buffer;
		Buffer << Temp.rdbuf();
		return Buffer.str();
	}

	void write_text_content(const fs::path file_path, const std::string& content)
	{
		std::ofstream File(file_path, std::ofstream::trunc);
		File << content;
		File.close();
	}
}