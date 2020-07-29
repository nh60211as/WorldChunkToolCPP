#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <iostream>
#include <filesystem>

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

namespace Utils
{
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
};

#endif
