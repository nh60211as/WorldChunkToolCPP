#include "PKG.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <iterator>

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

    int TotalParentCount = 0;
    Reader.read(reinterpret_cast<char*>(&TotalParentCount),sizeof(TotalParentCount));
    int ParentPadding = std::to_string(TotalParentCount).size();
    int TotalChildrenCount = 0;
    Reader.read(reinterpret_cast<char*>(&TotalChildrenCount), sizeof(TotalChildrenCount));
    if (!OnlyLog) 
        Utils::Print("PKG file has " + std::to_string(TotalParentCount) + " parent entries with " + std::to_string(TotalChildrenCount) + " children entries.", PRINT_ORDER::BEFORE);

    Reader.seekg(0x100, std::ios::beg);
    for (int i = 0; i < TotalParentCount; i++)
    {
        // doesn't seem to be used here, just seek 0x3C instead
        //std::vector<uint8_t> tmp(0x3C);
        //Reader.read(reinterpret_cast<char*>(tmp.data()),tmp.size());
        //byte[] ArrayNameParent = Reader.ReadBytes(0x3C).Where(b = > b != 0x00).ToArray();
        std::string StringNameParent;
        Reader.seekg(0x3C, std::ios::cur);
        int64_t FileSize = 0u;
        int64_t FileOffset = 0u;
        int EntryType = 0;
        int CountChildren = 0;
        Reader.read(reinterpret_cast<char*>(&FileSize), sizeof(FileSize));
        Reader.read(reinterpret_cast<char*>(&FileOffset), sizeof(FileOffset));
        Reader.read(reinterpret_cast<char*>(&EntryType), sizeof(EntryType));
        Reader.read(reinterpret_cast<char*>(&CountChildren), sizeof(CountChildren));

        for (int j = 0; j < CountChildren; j++)
        {
            if (!OnlyLog) 
                std::cout << "\rParent entry " << std::right << std::setfill(' ') << std::setw(ParentPadding) << std::to_string(i + 1) <<
                "/" << std::to_string(TotalParentCount) << 
                ". Processing child entry" << std::right << std::setfill(' ') << std::setw(4) << std::to_string(j + 1) <<
                "/" << std::right << std::setfill(' ') << std::setw(4) << std::to_string(CountChildren) << "..." << std::flush;

            int64_t ReaderPositionSubFile = Reader.tellg();
            std::vector<uint8_t> ArrayNameChild(0xA0);
            Reader.read(reinterpret_cast<char*>(ArrayNameChild.data()), ArrayNameChild.size());
            // find the index of first zero in the vector
            // calculate the distance between the index and the beginning of the vector
            // resize it accordingly
            // basically the same thing
            ArrayNameChild.resize(std::distance(ArrayNameChild.begin(),std::find(ArrayNameChild.begin(), ArrayNameChild.end(), 0)));
            //byte[] ArrayNameChild = Reader.ReadBytes(0xA0).Where(b = > b != 0x00).ToArray();
            int Unknown = 0;
            Reader.read(reinterpret_cast<char*>(&FileSize), sizeof(FileSize));
            Reader.read(reinterpret_cast<char*>(&FileOffset), sizeof(FileOffset));
            Reader.read(reinterpret_cast<char*>(&EntryType), sizeof(EntryType));
            Reader.read(reinterpret_cast<char*>(&Unknown), sizeof(Unknown));

            // Proper up remapped files
            if (EntryType == 0x02)
            {
                Reader.seekg(ReaderPositionSubFile, std::ios::beg);
                ArrayNameChild.resize(0x50);
                Reader.read(reinterpret_cast<char*>(ArrayNameChild.data()), ArrayNameChild.size());
                ArrayNameChild.resize(std::distance(ArrayNameChild.begin(), std::find(ArrayNameChild.begin(), ArrayNameChild.end(), 0)));
                Reader.seekg(0x68, std::ios::cur);
            }
            StringNameParent = std::string(std::begin(ArrayNameChild), std::end(ArrayNameChild));

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
                std::stringstream sstream;
                sstream << std::setfill('0') << std::setw(16) << std::hex << std::uppercase << FileOffset;

                std::string::reverse_iterator stringEnd = std::find(StringNameParent.rbegin(), StringNameParent.rend(), '\\');
                size_t lastIndex = std::distance(stringEnd, StringNameParent.rend());

                std::string::iterator stringStart = std::find(StringNameParent.begin(), StringNameParent.end(), '.');
                size_t firstIndex = std::distance(StringNameParent.begin(), stringStart);

                LogWriter << 
                    i << ',' <<
                    sstream.str() << ',' <<
                    FileSize << ',' <<
                    EntryType << ',' <<
                    Unknown << ',' <<
                    StringNameParent << ',' <<
                    Utils::stringRemove(StringNameParent,'\\',true) << ',' <<
                    StringNameParent.substr(lastIndex) << ',' <<
                    StringNameParent.substr(firstIndex + 1) << "\n";
            }
        }
    }
    Reader.close();
    LogWriter.close();

    if (!OnlyLog) Utils::Print("Finished.", PRINT_ORDER::BEFORE);
    if (!OnlyLog) Utils::Print("Output at: " + OutputDirectory, PRINT_ORDER::AFTER);
    Utils::pause(FlagAutoConfirm);

}
