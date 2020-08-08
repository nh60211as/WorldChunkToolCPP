#include <iostream>
#include <vector>
#include <filesystem>
#include <regex>
#include <fstream>
#include <memory>
#include <chrono>

#include "oo2core_loader.h"
#include "Utils.h"
#include "Chunk.h"
#include "ChunkOTF.h"
#include "ChunkDecrypter.h"
#include "PKG.h"

namespace fs = std::filesystem;

int printHelpInfo();

void setFlag(const std::vector<std::string>& args, const std::string& argument, bool& flag, const std::string& printMessage);
int ProcessFile(const std::string& FileInput, const flags currentFlag, const std::shared_ptr<oo2core_loader>& oo2coreInstance);


// current testing argument: "C:/SteamLibrary/steamapps/common/Monster Hunter World/chunk/chunkG4.bin" -BuildPKG -AutoConfirm
int main(int argc, char* argv[])
{
	std::ios::sync_with_stdio(false);
	std::cout << "==============================\n";
	Utils::Print("WorldChunkTool v1.2.2 by MHVuze", PRINT_ORDER::AFTER);
	Utils::Print("C++ implementation by nh60211as", PRINT_ORDER::AFTER);

	if (Utils::IsBigEndian())
	{
		std::cout << "Big endian machine is not supported now.\n";
		return 0;
	}

	std::vector<std::string> missingFileList = Utils::fetchMissingFileList();
	if (!missingFileList.empty())
	{
		for (const std::string& missingFile : missingFileList)
			std::cout << "Needed file " << missingFile << " missing.\n";
		return 0;
	}

	if (argc == 1) // if there is no other input argument
	{
		printHelpInfo();
		return 0;
	}

	// load oo2core_8_win64.dll
	std::shared_ptr<oo2core_loader> oo2coreInstance = std::make_shared<oo2core_loader>();
	if (!oo2coreInstance) // if the library is not loaded
	{
		std::cout << OO2CORE_FILE_NAME << " not found.\n";
		std::cout << "Place it at the same folder as WorldChunkTool.exe.\n";
		return 2;
	}

	if (!oo2coreInstance->is_oo2core_8_win64_legit())
	{
		std::cout << OO2CORE_FILE_NAME << " is not legit.\n";
		std::cout << "Download the file from Warframe or something.\n";
		std::cout << "You can also check the hash of " << OO2CORE_FILE_NAME << ": \n";
		std::cout << "SHA256: " << oo2coreSHA256 << "\n";
		return 2;
	}

	ChunkDecrypter chunkDecrypter; // make sure its static member is initialized

	std::string FileInput(argv[1]);
	flags currentFlag{}; // is this some C++17 initialization I have to do for every default constructor?

	// collect the command line arguments except the executable name and input directory
	std::vector<std::string> args;
	args.reserve(static_cast<size_t>(argc) - 2);
	for (int i = 2; i < argc; i++)
	{
		std::string argument = argv[i];
		Utils::sanitizeNewLine(argument);
		args.emplace_back(argument);
	}

	// Set options
	setFlag(args, "-AutoConfirm", currentFlag.FlagAutoConfirm, "Auto confirmation turned on.");
	setFlag(args, "-UnpackAll", currentFlag.FlagUnpackAll, "Unpacking all chunk*.bin files into a single folder.");
	setFlag(args, "-BuildPKG", currentFlag.FlagBuildPkg, "Building PKG.");
	setFlag(args, "-BaseGame", currentFlag.FlagBaseGame, "Using legacy mode for MH:W base game chunks.");


	if (currentFlag.FlagUnpackAll && Utils::isDirectory(FileInput))
	{
		const std::regex wordRegex = currentFlag.FlagBaseGame ? std::regex("chunk([0-9]+).bin") : std::regex("chunkG([0-9]+).bin"); // no wonder it never worked after iceborne
		std::vector<std::string> ChunkFileList = Utils::sortFileByChunkName(FileInput, wordRegex);
		for (const std::string ChunkFile : ChunkFileList)
		{
			std::cout << "Processing " << ChunkFile << ".\n";
			ProcessFile(ChunkFile, currentFlag, oo2coreInstance);
		}
	}
	else if (fs::exists(FileInput))
	{
		ProcessFile(FileInput, currentFlag, oo2coreInstance);
	}
	else
	{
		std::cout << "Input is not a file or did not specify -UnpackAll when input is a folder.\n";
		return 0;
	}

	if (currentFlag.FlagUnpackAll)
	{
		std::cout << "Output at: " << fs::current_path().string() << "\\chunk_combined\n";
		Utils::pause(currentFlag.FlagAutoConfirm);
	}

	return 0;
}

int printHelpInfo()
{
	std::cout << "Usage: \tWorldChunkTool <chunk*_file|PKG_file|chunk*_dir> (options)\n\n";
	std::cout << "Options:\n";
	std::cout << "\t-UnpackAll: Unpack all chunk*.bin files in the provided directory into a single folder.\n";
	std::cout << "\t-AutoConfirm: No confirmations required.\n";
	std::cout << "\t-BuildPKG: Build PKG file from chunks and create data sheet. No extraction. For research purposes only.\n";
	std::cout << "\t-BaseGame: Switch to legacy mode for MH:W base game chunks (pre-IB update).\n";
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

int ProcessFile(const std::string& FileInput, const flags currentFlag, const std::shared_ptr<oo2core_loader>& oo2coreInstance)
{
	// this is processed by main()
	if (!fs::exists(FileInput))
	{
		std::cout << "ERROR: Specified file doesn't exist.\n";
		return 1;
	}

	int MagicInputFile = Utils::getFileMagicNumber(FileInput);
	if (MagicInputFile == MagicChunk)
	{
		std::cout << "Chunk file detected.\n";

		// Build PKG
		if (currentFlag.FlagBuildPkg)
		{
			Chunk::DecompressChunks(FileInput, currentFlag, oo2coreInstance);
		}
		else
		{
			ChunkOTF ChunkOtfInst(oo2coreInstance);
			std::list<std::shared_ptr<FileNode>> FileCatalog;
			std::string FilePath = fs::current_path().string() + "\\" + Utils::removeExtension(FileInput);
			if (currentFlag.FlagUnpackAll)
				FilePath = fs::current_path().string() + "\\chunk_combined";
			FileCatalog = ChunkOtfInst.AnalyzeChunk(FileInput, FileCatalog, currentFlag.FlagBaseGame);
			std::cout << "Creating folders, please wait.\n";
			ChunkOtfInst.createSelectedFolder(FileCatalog, FilePath);
			std::cout << "Extracting chunk file, please wait.\n";
			ChunkOtfInst.ExtractSelected(FileCatalog, FilePath, currentFlag.FlagBaseGame);
			Utils::Print("\nFinished.", PRINT_ORDER::AFTER);
			if (!currentFlag.FlagUnpackAll) { Utils::Print("Output at: " + FilePath, PRINT_ORDER::AFTER); }
			Utils::pause(currentFlag.FlagAutoConfirm);
		}
		return 0;
	}
	else if (MagicInputFile == MagicPKG)
	{
		std::cout << "PKG file detected.\n";
		// TODO:
		PKG::ExtractPKG(FileInput, currentFlag.FlagAutoConfirm, currentFlag.FlagUnpackAll, false);
		Utils::pause(currentFlag.FlagAutoConfirm);
		return 0;
	}
	else
	{
		std::cout << "ERROR: Invalid magic " << std::hex << MagicInputFile << ".\n";
		Utils::pause(currentFlag.FlagAutoConfirm);
		return 0;
	}

	return 0;
}
