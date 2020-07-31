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

namespace fs = std::filesystem;

int printHelpInfo();

void setFlag(const std::vector<std::string>& args, const std::string& argument, bool& flag, const std::string& printMessage);
int ProcessFile(const std::string& FileInput, const flags currentFlag, const std::shared_ptr<oo2core_loader> & oo2coreInstance);


// current testing argument: "C:/SteamLibrary/steamapps/common/Monster Hunter World/chunk/chunkG4.bin" -UnpackAll -AutoConfirm
int main(int argc, char* argv[])
{
	std::cout << "==============================" << std::endl;
	Utils::Print("WorldChunkTool v1.2.2 by MHVuze", PRINT_ORDER::AFTER);
	Utils::Print("C++ implementation by nh60211as", PRINT_ORDER::AFTER);

	if (Utils::IsBigEndian())
	{
		std::cout << "Big endian machine is not supported now." << std::endl;
		return 0;
	}

	if (!Utils::checkNeededFiles())
	{
		std::cout << "Needed file(s) missing." << std::endl;
		return 0;
	}

	if (argc == 1) // if there is no other input argument
		return printHelpInfo();

	// load oo2core_8_win64.dll
	std::shared_ptr<oo2core_loader> oo2coreInstance = std::make_shared<oo2core_loader>();
	if (!oo2coreInstance) // if the library is not loaded
	{
		std::cout << OO2CORE_FILE_NAME << " not found." << std::endl;
		std::cout << "Place it at the same folder as WorldChunkTool.exe." << std::endl;
		return 2;
	}

	if (!oo2coreInstance->is_oo2core_8_win64_legit())
	{
		std::cout << OO2CORE_FILE_NAME << " is not legit." << std::endl;
		std::cout << "Download the file from Warframe or something." << std::endl;
		std::cout << "You can also check the hash of " << OO2CORE_FILE_NAME << ": " << std::endl;
		std::cout << "SHA256: " << oo2coreSHA256 << std::endl;
		return 2;
	}



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

	// Determine action based on file magic
	if (currentFlag.FlagUnpackAll && Utils::isDirectory(FileInput))
	{
		const std::regex wordRegex = currentFlag.FlagBaseGame ? std::regex("chunk([0-9]+).bin") : std::regex("chunkG([0-9]+).bin"); // no wonder it never worked after iceborne
		std::vector<std::string> ChunkFileList = Utils::sortFileByChunkName(FileInput, wordRegex);
		for (const std::string ChunkFile : ChunkFileList)
		{
			std::cout << "Processing " << ChunkFile << "." << std::endl;
			ProcessFile(ChunkFile, currentFlag, oo2coreInstance);
		}
	}
	else
		ProcessFile(FileInput, currentFlag, oo2coreInstance);

	if (currentFlag.FlagUnpackAll)
	{
		std::cout << "Output at: " << fs::current_path().string() << "\\chunk_combined" << std::endl;
		Utils::pause(currentFlag.FlagAutoConfirm);
	}

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

int ProcessFile(const std::string& FileInput, const flags currentFlag, const std::shared_ptr<oo2core_loader> & oo2coreInstance)
{
	if (!fs::exists(FileInput))
	{
		std::cout << "ERROR: Specified file doesn't exist." << std::endl;
		return 1;
	}

	int MagicInputFile = Utils::getFileMagicNumber(FileInput);
	if (MagicInputFile == MagicChunk)
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
			std::list<std::shared_ptr<FileNode>> FileCatalog;
			std::string FilePath = fs::current_path().string() + "\\" + Utils::removeExtension(FileInput);
			if (currentFlag.FlagUnpackAll)
				FilePath = fs::current_path().string() + "\\chunk_combined";
			FileCatalog = ChunkOtfInst.AnalyzeChunk(FileInput, FileCatalog, currentFlag.FlagBaseGame);
			std::cout << "Extracting chunk file, please wait." << std::endl;
			ChunkOtfInst.ExtractSelected(FileCatalog, FilePath, currentFlag.FlagBaseGame);
			Utils::Print("\nFinished.", PRINT_ORDER::AFTER);
			if (!currentFlag.FlagUnpackAll) { Utils::Print("Output at: " + FilePath, PRINT_ORDER::AFTER); }
			Utils::pause(currentFlag.FlagAutoConfirm);
		}
		return 0;
	}
	else if (MagicInputFile == MagicPKG)
	{
		std::cout << "PKG file detected." << std::endl;
		// TODO:
		//PKG.ExtractPKG(FileInput, FlagAutoConfirm, FlagUnpackAll, false);
		Utils::pause(currentFlag.FlagAutoConfirm);
		return 0;
	}
	else
	{
		std::cout << "ERROR: Invalid magic " << std::hex << MagicInputFile << "." << std::endl;
		Utils::pause(currentFlag.FlagAutoConfirm);
		return 0;
	}

	return 0;
}
