#include <iostream>
#include <vector>
#include <filesystem>
#include <regex>
#include <fstream>

#include "oo2core_loader.h"
#include "Utils.h"
#include "Chunk.h"
#include "ChunkOTF.h"

namespace fs = std::filesystem;

int printHelpInfo();

void setFlag(const std::vector<std::string>& args, const std::string& argument, bool& flag, const std::string& printMessage);
int ProcessFile(const std::string& FileInput, const flags currentFlag, const oo2core_loader * oo2coreInstance);

int main(int argc, char* argv[])
{
	std::cout << "==============================" << std::endl;
	Utils::Print("WorldChunkTool v1.2.2 by MHVuze", PRINT_ORDER::BEFORE);
	Utils::Print("C++ implementation by nh60211as", PRINT_ORDER::AFTER);

	if (argc == 1) // if there is no other input argument
		return printHelpInfo();

	// load oo2core_8_win64.dll
	oo2core_loader oo2coreInstance;
	if (!oo2coreInstance) // if the library is not loaded
	{
		std::cout << OO2CORE_FILE_NAME << " not found." << std::endl;
		std::cout << "Place it at the same folder as WorldChunkTool.exe." << std::endl;
		return 2;
	}

	if(!oo2coreInstance.is_oo2core_8_win64_legit())
	{
		std::cout << OO2CORE_FILE_NAME << " is not legit." << std::endl;
		std::cout << "Download the file from Warframe or something." << std::endl;
		std::cout << "You can also check the hash of " << OO2CORE_FILE_NAME << ": " << std::endl;
		std::cout << "SHA256: " << oo2coreSHA256 << std::endl;
		return 2;
	}

	// load chunkKeyPattern
	Utils::chunkKeyPattern = std::vector<uint8_t>(1000009);
	std::ifstream chunkKeyReader("./keySequence.bin",std::ios::binary);
	chunkKeyReader.read(reinterpret_cast<char*>(Utils::chunkKeyPattern.data()), Utils::chunkKeyPattern.size());

	std::string FileInput(argv[1]);
	flags currentFlage{}; // is this some C++17 initialization I have to do for every default constructor?

	// collect the command line arguments except the executable name and input directory
	std::vector<std::string> args;
	args.reserve(static_cast<size_t>(argc) - 2);
	for (int i = 2; i < argc; i++)
		args.emplace_back(argv[i]);

	// Set options
	setFlag(args,"-AutoConfirm",currentFlage.FlagAutoConfirm,"Auto confirmation turned on.");
	setFlag(args, "-UnpackAll", currentFlage.FlagUnpackAll, "Unpacking all chunk*.bin files into a single folder.");
	setFlag(args, "-BuildPKG", currentFlage.FlagBuildPkg, "Building PKG.");
	setFlag(args, "-BaseGame", currentFlage.FlagBaseGame, "Using legacy mode for MH:W base game chunks.");

	// Determine action based on file magic
	if (currentFlage.FlagUnpackAll && Utils::isDirectory(FileInput))
	{
		const std::regex wordRegex = currentFlage.FlagBaseGame ? std::regex("chunk*.bin") : std::regex("chunkG*.bin"); // no wonder it never worked after iceborne
		for (const fs::directory_entry& file : fs::directory_iterator(FileInput))
		{
			std::string ChunkFile = fs::path(file).filename().string(); // get the file name
			if (!std::regex_match(ChunkFile, wordRegex)) // if the file name doesn't match the pattern (it's harder than I thought)
				continue;
			std::cout << "Processing " << ChunkFile << "." << std::endl;
			ProcessFile(ChunkFile, currentFlage, &oo2coreInstance);
		}
	}
	else
		ProcessFile(FileInput, currentFlage, &oo2coreInstance);

	return 0;
}

int printHelpInfo()
{
	std::cout << "Usage: \tWorldChunkTool <chunk*_file|PKG_file|chunk*_dir> (options)\n" << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "\t-UnpackAll: Unpack all chunk*.bin files in the provided directory into a single folder." << std::endl;
	std::cout << "\t-AutoConfirm: No confirmations required." << std::endl;
	std::cout << "\t-BuildPKG: Build PKG file from chunks and create data sheet. No extraction. For research purposes only." << std::endl;
	std::cout << "\t-BaseGame: Switch to legacy mode for MH:W base game chunks (pre-IB update)." << std::endl;
	return 0;
}

void setFlag(const std::vector<std::string>& args, const std::string& argument, bool& flag, const std::string& printMessage)
{
	if (Utils::contains<std::string>(args, argument))
	{
		flag = true;
		Utils::Print(printMessage, PRINT_ORDER::AFTER);
	}
}

int ProcessFile(const std::string& FileInput, const flags currentFlag, const oo2core_loader* oo2coreInstance)
{
	if (!fs::exists(FileInput))
	{
		std::cout << "ERROR: Specified file doesn't exist." << std::endl;
		return 1;
	}

	if (Utils::isFileMagicChunk(FileInput))
	{
		std::cout << "Chunk file detected." << std::endl;

		// Build PKG
		if (currentFlag.FlagBuildPkg)
		{
			// TODO
			Chunk::DecompressChunks(FileInput, currentFlag);
		}
		else
		{
			ChunkOTF ChunkOtfInst(oo2coreInstance);
		}
	}

}

bool is_oo2core_8_win64_legit()
{
	return true;
}