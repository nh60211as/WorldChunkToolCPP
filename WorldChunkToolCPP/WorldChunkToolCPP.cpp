#include <iostream>
#include <vector>
#include <filesystem>
#include <regex>
#include <fstream>
#include <memory>

#include "Utils.h"
#include "Chunk.h"
#include "ChunkDecrypter.h"
#include "ChunkOTF.h"
#include "oo2core_loader.h"
#include "PKG.h"

namespace fs = std::filesystem;

void printHelpInfo();
void setFlag(const std::vector<std::string>& args, const std::string& argument, bool& flag, const std::string& printMessage);
bool ProcessFile(const std::string& FileInput, const flags currentFlag, const std::shared_ptr<oo2core_loader>& oo2coreInstance);


// return code 0: successful extraction
//             1: input file missing
//             2: oo2core_8_win64.dll related issue
//             3: necessary file missing
//             999: other issues
int main(int argc, char* argv[])
{
	std::ios::sync_with_stdio(false);
	Utils::PrintWithSeparationLine("", PRINT_ORDER::AFTER);
	Utils::PrintWithSeparationLine("WorldChunkTool v1.2.2 by MHVuze", PRINT_ORDER::AFTER);
	Utils::PrintWithSeparationLine("C++ implementation by nh60211as", PRINT_ORDER::AFTER);

	// if there is no other input argument
	if (argc == 1)
	{
		printHelpInfo();
		return 999;
	}

	if (Utils::IsBigEndian())
	{
		std::cout << "Big endian machine is not supported now.\n";
		return 999;
	}

	std::vector<std::string> missingFileList = Utils::fetchMissingFileList();
	if (!missingFileList.empty())
	{
		for (const std::string& missingFile : missingFileList)
			std::cout << "Needed file " << missingFile << " missing.\n";
		return 3;
	}

	// load oo2core_8_win64.dll
	std::shared_ptr<oo2core_loader> oo2coreInstance = std::make_shared<oo2core_loader>();

	// this is explict operator to check if the library is loaded correctly, 
	// not checking if the oo2coreInstance pointer points to an actual address
	if (!(*oo2coreInstance))
	{
		std::cout << OO2CORE_FILE_NAME << " not loaded.\n";
		std::cout << "Place it at the same folder as WorldChunkTool.exe.\n";
		return 2;
	}

	if (!oo2coreInstance->is_oo2core_8_win64_legit())
	{
		std::cout << OO2CORE_FILE_NAME << " is not legit.\n";
		std::cout << "Download the file from Warframe or something.\n";
		std::cout << "You can also check the hash of " << OO2CORE_FILE_NAME << ": \n";
		std::cout << "SHA256: " << OO2CORE_SHA256 << "\n";
		return 2;
	}

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

	std::string FileInput(argv[1]);
	ChunkDecrypter chunkDecrypter; // make sure its static member is initialized
	if (currentFlag.FlagUnpackAll && Utils::isDirectory(FileInput))
	{
		const std::regex wordRegex = currentFlag.FlagBaseGame ? std::regex("chunk([0-9]+).bin") : std::regex("chunkG([0-9]+).bin"); // no wonder it never worked after iceborne
		std::vector<std::string> ChunkFileList = Utils::sortFileByChunkName(FileInput, wordRegex);
		for (const std::string ChunkFile : ChunkFileList)
		{
			std::cout << "Processing " << ChunkFile << ".\n";
			if (!ProcessFile(ChunkFile, currentFlag, oo2coreInstance))
				return 1;
		}
	}
	else if (fs::exists(FileInput))
	{
		if (!ProcessFile(FileInput, currentFlag, oo2coreInstance))
			return 1;
	}
	else
	{
		std::cout << "Input is not a file or did not specify -UnpackAll when input is a folder.\n";
		return 1;
	}

	if (currentFlag.FlagUnpackAll)
	{
		std::cout << "Output at: " << fs::current_path().string() << "\\chunk_combined\n";
		Utils::pause(currentFlag.FlagAutoConfirm);
	}

	return 0;
}

void printHelpInfo()
{
	std::cout << "Usage: \tWorldChunkTool <chunk*_file|PKG_file|chunk*_dir> (options)\n\n";
	std::cout << "Options:\n";
	std::cout << "\t-UnpackAll: Unpack all chunk*.bin files in the provided directory into a single folder.\n";
	std::cout << "\t-AutoConfirm: No confirmations required.\n";
	std::cout << "\t-BuildPKG: Build PKG file from chunks and create data sheet. No extraction. For research purposes only.\n";
	std::cout << "\t-BaseGame: Switch to legacy mode for MH:W base game chunks (pre-IB update).\n";
}

void setFlag(const std::vector<std::string>& args, const std::string& argument, bool& flag, const std::string& printMessage)
{
	if (Utils::contains<std::string>(args, argument))
	{
		flag = true;
		Utils::PrintWithSeparationLine(printMessage, PRINT_ORDER::AFTER);
	}
}

bool ProcessFile(const std::string& FileInput, const flags currentFlag, const std::shared_ptr<oo2core_loader>& oo2coreInstance)
{
	// this is processed by main()
	if (!fs::exists(FileInput))
	{
		std::cout << "ERROR: Specified file doesn't exist.\n";
		return false;
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
			Utils::PrintWithSeparationLine("\nFinished.", PRINT_ORDER::AFTER);
			if (!currentFlag.FlagUnpackAll) { Utils::PrintWithSeparationLine("Output at: " + FilePath, PRINT_ORDER::AFTER); }
			Utils::pause(currentFlag.FlagAutoConfirm);
		}
		return true;
	}
	else if (MagicInputFile == MagicPKG)
	{
		std::cout << "PKG file detected.\n";
		PKG::ExtractPKG(FileInput, currentFlag.FlagAutoConfirm, currentFlag.FlagUnpackAll, false);
		Utils::pause(currentFlag.FlagAutoConfirm);
		return true;
	}
	else
	{
		std::cout << "ERROR: Invalid magic " << std::hex << MagicInputFile << ".\n";
		Utils::pause(currentFlag.FlagAutoConfirm);
		return false;
	}

	return true;
}
