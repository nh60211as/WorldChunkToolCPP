#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <regex>

// necessary file names
static constexpr const char* OO2CORE_FILE_NAME = "./oo2core_8_win64.dll";
static constexpr const char* CHUNK_KEY_SEQUENCE_FILE_NAME = "./keySequence.bin";

// file hash of OO2CORE_FILE_NAME
static constexpr const char* OO2CORE_SHA256 = "D5440627BBDE1EF2D9730976DD90C5189D874FB7BB808669734E598CDFDBA8D9";

// flags for user input
struct flags
{
    bool FlagBuildPkg;
    bool FlagBaseGame;
    bool FlagAutoConfirm;
    bool FlagUnpackAll;

    flags() : FlagBuildPkg(false), FlagBaseGame(false), FlagAutoConfirm(false), FlagUnpackAll(false)
    {
    };
};

// print order for Util::PrintWithSeparationLine()
enum class PRINT_ORDER : bool
{
    BEFORE = false,
    AFTER = true
};


// magic numbers of chunk and pkg
static constexpr int MagicChunk = 0x00504D43;
static constexpr int MagicPKG = 0x20474B50;

static constexpr size_t defaultChunkSize = 0x40000;

// all of the functions here are reported as C4505
// https://docs.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-4-c4505?view=vs-2019
// is this because they are inlined to other functions?
// add inline keyword fixed this issue
namespace Utils
{
    static inline int IsBigEndian()
    {
        int i = 1;
        return !*((char*)&i);
    }

    static inline std::vector<std::string> fetchMissingFileList()
    {
        std::vector<std::string> missingFileList;
        if (!std::filesystem::exists(OO2CORE_FILE_NAME))
            missingFileList.emplace_back(std::filesystem::path(OO2CORE_FILE_NAME).filename().string());
        if (!std::filesystem::exists(CHUNK_KEY_SEQUENCE_FILE_NAME))
            missingFileList.emplace_back(std::filesystem::path(CHUNK_KEY_SEQUENCE_FILE_NAME).filename().string());

        return missingFileList;
    }

    static inline void PrintWithSeparationLine(const std::string& Input, PRINT_ORDER order)
    {
        if (order == PRINT_ORDER::AFTER)
        {
            std::cout << Input << "\n";
            std::cout << "==============================\n";
        }
        else
        {
            std::cout << "\n==============================\n";
            std::cout << Input << "\n";
        }
    }

    // https://stackoverflow.com/questions/1488775/c-remove-new-line-from-multiline-string
    static inline void sanitizeNewLine(std::string& str)
    {
        str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
    }

    template <class T> static bool contains(const std::vector<T>& v, const T& target)
    {
        if (std::find(std::begin(v), std::end(v), target) != std::end(v))
            return true;

        return false;
    }

    // from https://thispointer.com/c-check-if-given-path-is-a-file-or-directory-using-boost-c17-filesystem-library/
    static inline bool isDirectory(const std::string& filePath)
    {
        if (std::filesystem::exists(filePath) && std::filesystem::is_directory(filePath))
            return true;

        return false;
    }

    static inline std::string getUpperDirectory(const std::string& filePath)
    {
        return std::filesystem::path(filePath).parent_path().string();
    }

    static inline int getFileMagicNumber(const std::string& FileInput)
    {
        std::ifstream Reader(FileInput, std::ios::in | std::ios::binary);
        int MagicInputFile = 0;
        Reader.read(reinterpret_cast<char*>(&MagicInputFile), sizeof(MagicInputFile));
        Reader.close();

        return MagicInputFile;
    }

    static inline std::vector<std::string> sortFileByChunkName(const std::string& FileInput, const std::regex& wordRegex)
    {
        std::vector<std::pair<int, std::string>> chunkNamePairs; // .../chunk/chunkG2.bin, first:2 , second: .../chunk/chunkG2.bin

        std::smatch chunkMatch;
        for (const std::filesystem::directory_entry& file : std::filesystem::directory_iterator(FileInput))
        {
            std::string ChunkFile = std::filesystem::path(file).filename().string(); // get the file name
            if (std::regex_search(ChunkFile, chunkMatch, wordRegex))
            {
                if (chunkMatch.size() == 2) // only when the matched patteren is exactly 1
                    chunkNamePairs.emplace_back(std::stoi(chunkMatch[1].str()), std::filesystem::path(file).string());
            }
        }

        // sort by chunk number
        std::sort(std::begin(chunkNamePairs), std::end(chunkNamePairs),
            [](const std::pair<int, std::string>& a, const std::pair<int, std::string>& b) {return a.first < b.first; }
        );

        std::vector<std::string> ans;
        ans.reserve(chunkNamePairs.size());
        for (const std::pair<int, std::string>& p : chunkNamePairs)
            ans.emplace_back(p.second);

        return ans;
    }

    // https://cloud.tencent.com/developer/article/1433558
    static inline std::vector<std::string> stringSplit(const std::string& in, const std::string& delim)
    {
        std::regex re{ delim };
        return std::vector<std::string> {
            std::sregex_token_iterator(in.begin(), in.end(), re, -1),
                std::sregex_token_iterator()
        };
    }

    static inline std::string removeExtension(const std::string& fileInput)
    {
        return std::filesystem::path(fileInput).replace_extension("").string();
    }

    static inline std::string getFileNameWithoutExtension(const std::string& fileInput)
    {
        return 	Utils::removeExtension(std::filesystem::path(fileInput).filename().string());
    }

    static inline void pause(bool autoConfirm)
    {
        if (!autoConfirm)
        {
            std::cout << "Press any key to continue...";
            std::cin.get();
        }
    }

    static inline std::string stringRemove(const std::string& input, const char target, bool isLast)
    {
        std::string output = input;

        // I don't even know if this will work
        // It actually works
        if (isLast)
        {
            std::string::const_reverse_iterator stringEnd = std::find(input.rbegin(), input.rend(), target);
            ptrdiff_t endOfString = std::distance(stringEnd, input.rend());
            output.erase(output.begin() + endOfString, output.end());
        }
        else
        {
            std::string::const_iterator stringEnd = std::find(input.begin(), input.end(), target);
            output.erase(stringEnd + 1, output.end());
        }

        return output;
    }
};

#endif
