#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <regex>

struct flags
{
	bool FlagBuildPkg;
	bool FlagBaseGame;
	bool FlagAutoConfirm;
	bool FlagUnpackAll;

	flags()
	{
		FlagBuildPkg = false;
		FlagBaseGame = false;
		FlagAutoConfirm = false;
		FlagUnpackAll = false;
	};
};

enum class PRINT_ORDER
{
	BEFORE,
	AFTER
};


static constexpr int MagicChunk = 0x00504D43;
static constexpr int MagicPKG = 0x20474B50;

namespace Utils
{
	static std::vector<uint8_t> chunkKeyPattern; // it should be initialized in main()

	static int IsBigEndian()
	{
		int i = 1;
		return !*((char*)&i);
	}

	static bool checkNeededFiles()
	{
		return true;
	}

	static void Print(const std::string& Input, PRINT_ORDER order)
	{
		if (order == PRINT_ORDER::AFTER)
		{
			std::cout << Input << std::endl;
			std::cout << "==============================" << std::endl;
		}
		else
		{
			std::cout << "\n==============================" << std::endl;
			std::cout << Input << std::endl;
		}
	}

	template <class T> static bool contains(const std::vector<T>& v, const T& target)
	{
		if (std::find(std::begin(v), std::end(v), target) != std::end(v))
			return true;

		return false;
	}

	// from https://thispointer.com/c-check-if-given-path-is-a-file-or-directory-using-boost-c17-filesystem-library/
	static bool isDirectory(const std::string& filePath)
	{
		if (std::filesystem::exists(filePath) && std::filesystem::is_directory(filePath))
			return true;

		return false;
	}

	static std::string getUpperDirectory(const std::string& filePath)
	{
		return std::filesystem::path(filePath).parent_path().string();
	}

	static int getFileMagicNumber(const std::string& FileInput)
	{
		std::ifstream Reader(FileInput, std::ios::in | std::ios::binary);
		int MagicInputFile = 0;
		Reader.read(reinterpret_cast<char*>(&MagicInputFile), sizeof(MagicInputFile));
		Reader.close();

		return MagicInputFile;
	}

	// https://cloud.tencent.com/developer/article/1433558
	static std::vector<std::string> stringSplit(const std::string& in, const std::string& delim)
	{
		std::regex re{ delim };
		return std::vector<std::string> {
			std::sregex_token_iterator(in.begin(), in.end(), re, -1),
				std::sregex_token_iterator()
		};
	}

	static std::string removeExtension(const std::string& fileInput)
	{
		return std::filesystem::path(fileInput).replace_extension("").string();
	}

	static std::string getFileNameWithoutExtension(const std::string& fileInput)
	{
		return 	Utils::removeExtension(std::filesystem::path(fileInput).filename().string());
	}

	static void pause()
	{
		std::cout << "Press any key to continue...";
		std::cin.get();
	}
};

#endif
