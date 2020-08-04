#include "PKG.h"

#include <filesystem>
#include <fstream>

#include "Utils.h"

namespace fs = std::filesystem;

void PKG::ExtractPKG(const std::string& FileInput, bool FlagAutoConfirm, bool FlagUnpackAll, bool OnlyLog)
{
    std::string OutputDirectory = Utils::removeExtension(FileInput);
    if (FlagUnpackAll) OutputDirectory = fs::current_path().string() + "\\chunk";
    std::ifstream Reader(FileInput,std::ios::in | std::ios::binary);
    std::ofstream LogWriter(fs::path(FileInput).replace_extension(".csv").string(), std::ios::out);
    LogWriter << "Index,Offset,Size,EntryType,Unk,FullPath,Directory,FileName,FileType\n";

    Reader.seekg(0x0C, std::ios::beg);

    int TotalParentCount;
    Reader.read(reinterpret_cast<char*>(TotalParentCount),sizeof(TotalParentCount));
    int ParentPadding = std::to_string(TotalParentCount).size();
    int TotalChildrenCount;
    Reader.read(reinterpret_cast<char*>(TotalChildrenCount), sizeof(TotalChildrenCount));
    if (!OnlyLog) 
        Utils::Print("PKG file has " + std::to_string(TotalParentCount) + " parent entries with " + std::to_string(TotalChildrenCount) + " children entries.", PRINT_ORDER::AFTER);

    Reader.seekg(0x100, std::ios::beg);
    for (int i = 0; i < TotalParentCount; i++)
    {
        // doesn't seem to be used here, just seek 0x3C instead
        //std::vector<uint8_t> tmp(0x3C);
        //Reader.read(reinterpret_cast<char*>(tmp.data()),tmp.size());
        //byte[] ArrayNameParent = Reader.ReadBytes(0x3C).Where(b = > b != 0x00).ToArray();
        std::string StringNameParent;
        Reader.seekg(0x3C, std::ios::cur);
        int64_t FileSize;
        int64_t FileOffset;
        int EntryType;
        int CountChildren;
        Reader.read(reinterpret_cast<char*>(FileSize), sizeof(FileSize));
        Reader.read(reinterpret_cast<char*>(FileOffset), sizeof(FileOffset));
        Reader.read(reinterpret_cast<char*>(EntryType), sizeof(EntryType));
        Reader.read(reinterpret_cast<char*>(CountChildren), sizeof(CountChildren));

        for (int j = 0; j < CountChildren; j++)
        {
            if (!OnlyLog) 
                std::cout << "\rParent entry " << std::left << std::setfill(' ') << std::setw(ParentPadding) << std::to_string(i + 1) <<
                "/" << std::to_string(TotalParentCount) << 
                ". Processing child entry" << std::left << std::setfill(' ') << std::setw(4) << std::to_string(j + 1) <<  
                "/" << std::left << std::setfill(' ') << std::setw(4) << std::to_string(CountChildren) << "...\n";

            int64_t ReaderPositionSubFile = Reader.tellg();
            std::vector<uint8_t> ArrayNameChild(0xA0);
            Reader.read(reinterpret_cast<char*>(ArrayNameChild.data()), ArrayNameChild.size());
            // find the index of first zero in the vector
            // calculate the distance between the index and the beginning of the vector
            // resize it accordingly
            // basically the same thing
            ArrayNameChild.resize(std::distance(ArrayNameChild.begin(),std::find(ArrayNameChild.begin(), ArrayNameChild.end(), 0)));
            //byte[] ArrayNameChild = Reader.ReadBytes(0xA0).Where(b = > b != 0x00).ToArray();
            int Unknown;
            Reader.read(reinterpret_cast<char*>(FileSize), sizeof(FileSize));
            Reader.read(reinterpret_cast<char*>(FileOffset), sizeof(FileOffset));
            Reader.read(reinterpret_cast<char*>(EntryType), sizeof(EntryType));
            Reader.read(reinterpret_cast<char*>(Unknown), sizeof(Unknown));

            // Proper up remapped files
            if (EntryType == 0x02)
            {
                Reader.seekg(ReaderPositionSubFile, std::ios::beg);
                ArrayNameChild.resize(0x50);
                Reader.read(reinterpret_cast<char*>(ArrayNameChild.data()), ArrayNameChild.size());
                ArrayNameChild.resize(std::distance(ArrayNameChild.begin(), std::find(ArrayNameChild.begin(), ArrayNameChild.end(), 0)));
                Reader.seekg(0x68, std::ios::cur);
            }
            StringNameParent.reserve(ArrayNameChild.size());
            StringNameParent.append(std::begin(ArrayNameChild), std::end(ArrayNameChild));

            // Extract remapped and regular files
            if (EntryType == 0x02 || EntryType == 0x00)
            {
                int64_t ReaderPositionBeforeEntry = Reader.tellg();
                Reader.seekg(FileOffset, std::ios::beg);
                std::vector<uint8_t> ArrayFileData(FileSize);
                Reader.read(reinterpret_cast<char*>(ArrayFileData.data()), ArrayFileData.size());

                if (!OnlyLog)
                {
                    fs::create_directories(Utils::getUpperDirectory(OutputDirectory + "\\" + StringNameParent));
                    std::ofstream writer(OutputDirectory + "\\" + StringNameParent,std::ios::out);
                    writer.write(reinterpret_cast<char*>(ArrayFileData.data()), ArrayFileData.size());
                    writer.close();
                }

                Reader.seekg(ReaderPositionBeforeEntry, std::ios::beg);
            }

            // Handle directory entries
            if (EntryType != 0x01)
            {
                LogWriter << 
                    i << ',' <<
                    FileOffset.ToString("X16") << ',' <<
                    FileSize << ',' <<
                    EntryType << ',' <<
                    Unknown << ',' <<
                    StringNameParent << ',' <<
                    StringNameParent.Remove(StringNameParent.LastIndexOf('\\') + 1) << ',' <<
                    StringNameParent.Substring(StringNameParent.LastIndexOf('\\') + 1) << ',' <<
                    StringNameParent.Substring(StringNameParent.IndexOf('.') + 1) << "\n";
            }
        }
    }
    Reader.close();
    LogWriter.close();

    if (!OnlyLog) Utils::Print("Finished.", PRINT_ORDER::BEFORE);
    if (!OnlyLog) Utils::Print("Output at: " + OutputDirectory, PRINT_ORDER::AFTER);
    Utils::pause(FlagAutoConfirm);

}
