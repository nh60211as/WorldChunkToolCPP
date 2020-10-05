#include "ChunkOTF.h"

#include <filesystem>
#include <array>

#include "Utils.h"
#include "ChunkDecrypter.h"

namespace fs = std::filesystem;

ChunkOTF::ChunkOTF(std::shared_ptr<oo2core_loader> oo2coreInstance_) noexcept :
    oo2coreInstance(oo2coreInstance_),
    DictCount(0),
    cur_index(0),
    cur_pointer(0)
{
}

std::list<std::shared_ptr<FileNode>> ChunkOTF::AnalyzeChunk(const std::string& FileInput, std::list<std::shared_ptr<FileNode>>& inputFileList, bool FlagBaseGame) noexcept
{
    fileinput = FileInput; // store the file input

    ChunkCache = std::map<int, std::shared_ptr<chunk_t>>();

    std::list<std::shared_ptr<FileNode>> filelist = inputFileList;
    MetaChunk = std::map<int64_t, int64_t>();
    ChunkOffsetDict = std::map<int, int64_t>();
    std::string NamePKG = fs::path(FileInput).replace_extension(".pkg").string(); // finally something good out of C++17
    Reader = std::ifstream(FileInput, std::ios::in | std::ios::binary);

    // Read header
    Reader.seekg(4, std::ios_base::beg); // skipping the MagicChunk of size int
//int ChunkCount = Reader.ReadInt32(); int ChunkPadding = ChunkCount.ToString().Length;
    int ChunkCount;
    Reader.read(reinterpret_cast<char*>(&ChunkCount), sizeof(ChunkCount));
    //size_t ChunkPadding = std::to_string(ChunkCount).size();

    double DiskSpace = (int64_t)ChunkCount * (int64_t)0x40000 * 1e-9;
    Utils::PrintWithSeparationLine(std::to_string(ChunkCount) + " subchunks detected. Requires at least: " + std::to_string(std::round(DiskSpace * 100) / 100) + " GB.", PRINT_ORDER::AFTER);

    // Read file list
    DictCount = 0;
    int64_t totalChunkSize = 0;
    std::array<uint8_t, 8> ArrayChunkSize = { 1,1,1,0,0,0,0,0 }; // zero initialize the array. Is there a better way?
    std::array<uint8_t, 8> ArrayChunkOffset = { 1,1,1,1,1,0,0,0 }; // the ones are just a note to me that they will be modified
    for (int i = 0; i < ChunkCount; i++)
    {
        // Process file size
        //byte[] ArrayChunkSize = Reader.ReadBytes(3);
        Reader.read(reinterpret_cast<char*>(ArrayChunkSize.data()), 3); // read 3 bytes

        //int Low = ArrayChunkSize[0] & 0x0F;

        //int High = ArrayChunkSize[0] >> 4;
        //ArrayChunkSize[0] = BitConverter.GetBytes(High)[0];
        ArrayChunkSize[0] = (ArrayChunkSize[0] >> 4);
        //Array.Copy(ArrayChunkSize, ArrayTmp1, ArrayChunkSize.Length);
        //long ChunkSize = BitConverter.ToInt64(ArrayTmp1, 0); // why are C# examples so over-engineered
        int64_t ChunkSize = *(int64_t*)(ArrayChunkSize.data()); // type punning. I only know it probably works on GCC.
        ChunkSize = (ChunkSize >> 4) + (ChunkSize & 0xF);
        totalChunkSize += ChunkSize;

        // Process offset
        //byte[] ArrayChunkOffset = Reader.ReadBytes(5);
        Reader.read(reinterpret_cast<char*>(ArrayChunkOffset.data()), 5); // read 5 bytes
        //Array.Copy(ArrayChunkOffset, ArrayTmp2, ArrayChunkOffset.Length);
        //long ChunkOffset = BitConverter.ToInt64(ArrayTmp2, 0);
        int64_t ChunkOffset = *(int64_t*)(ArrayChunkOffset.data());

        MetaChunk.emplace(ChunkOffset, ChunkSize);
        ChunkOffsetDict.emplace(i, ChunkOffset);
        //DictCount = i + 1; // why does it need to be updated every loop
    }
    DictCount = ChunkCount;
    cur_index = 0;
    int64_t cur_offset = ChunkOffsetDict[cur_index];
    int64_t cur_size = MetaChunk[cur_offset];

    ChunkDecompressed = getDecompressedChunk(cur_offset, cur_size, Reader, FlagBaseGame, cur_index);
    if (cur_index + 1 < DictCount)
    {
        NextChunkDecompressed = getDecompressedChunk(ChunkOffsetDict[cur_index + 1],
            MetaChunk[ChunkOffsetDict[cur_index + 1]],
            Reader, FlagBaseGame, static_cast<size_t>(cur_index) + 1);
    }
    else
    {
        NextChunkDecompressed->clear();
    }
    cur_pointer = 0x0C;
    int TotalParentCount = *(int*)(ChunkDecompressed->data() + cur_pointer);
    cur_pointer += 4;
    //int TotalChildrenCount = *(int*)(ChunkDecompressed.data() + cur_pointer);
    cur_pointer = 0x100;
    std::shared_ptr<FileNode> root_node = nullptr;
    for (int i = 0; i < TotalParentCount; i++)
    {
        //std::cout << i << ": " << std::endl;

        std::string StringNameParent = getName(0x3C, FlagBaseGame);
        int64_t FileSize = getInt64(FlagBaseGame);
        int64_t FileOffset = getInt64(FlagBaseGame);
        int EntryType = getInt32(FlagBaseGame);
        int CountChildren = getInt32(FlagBaseGame);

        if (filelist.size() == 0)
        {
            root_node = std::make_shared<FileNode>(StringNameParent, false, FileInput);
            root_node->EntireName = root_node->Name;
            filelist.emplace_back(root_node);
        }
        else
        {
            root_node = filelist.front();
            root_node->FromChunk = fileinput;
            root_node->FromChunkName = Utils::getFileNameWithoutExtension(FileInput);
        }
        //std::cout << root_node->EntireName << std::endl;

        for (int j = 0; j < CountChildren; j++)
        {
            //std::cout << "j: " << j << std::endl;

            int origin_pointer = cur_pointer;
            int origin_loc = cur_index;
            if (ChunkCache.find(cur_index) == ChunkCache.end()) ChunkCache.emplace(cur_index, ChunkDecompressed);
            if (ChunkCache.find(cur_index + 1) == ChunkCache.end()) ChunkCache.emplace(cur_index + 1, NextChunkDecompressed);

            std::string StringNameChild = getName(0xA0, FlagBaseGame);
            FileSize = getInt64(FlagBaseGame);
            FileOffset = getInt64(FlagBaseGame);
            EntryType = getInt32(FlagBaseGame);
            int Unknown = getInt32(FlagBaseGame); // function with side effect, keep it here

            if (EntryType == 0x02)
            {
                cur_pointer = origin_pointer;
                if (cur_index != origin_loc)
                {
                    cur_index = origin_loc;
                    ChunkDecompressed = ChunkCache[cur_index];
                    NextChunkDecompressed = ChunkCache[cur_index + 1];
                    ChunkCache.erase(cur_index);
                    ChunkCache.erase(cur_index + 1);
                }
                StringNameChild = getName(0x50, FlagBaseGame);
                std::vector<uint8_t> temp(0x68);
                getOnLength(temp.size(), temp.data(), 0, FlagBaseGame);
            }
            // https://stackoverflow.com/questions/4025482/cant-escape-the-backslash-with-regex
            // split by regular expression. "\\\\" will be interpreted as "\\"
            std::vector<std::string> fathernodes = Utils::stringSplit(StringNameChild, "\\\\");
            bool isFile = false;
            if (EntryType == 0x02 || EntryType == 0x00) isFile = true;
            std::shared_ptr<FileNode> child_node = std::make_shared<FileNode>(fathernodes.back(), isFile, FileInput);
            if (isFile)
            {
                child_node->Size = FileSize;
                child_node->Offset = FileOffset;
                child_node->ChunkIndex = (int)(FileOffset / 0x40000);
                child_node->ChunkPointer = (int)(FileOffset % 0x40000);
            }
            child_node->EntireName = StringNameChild;
            std::shared_ptr<FileNode> target_node = root_node;
            //std::cout << j << ": " << target_node->EntireName << std::endl;
            for (const std::string& node_name : fathernodes)
            {
                if (node_name.empty()) continue;
                for (const std::shared_ptr<FileNode>& node : target_node->Childern)
                {
                    //std::cout << node->Name.size() << "==" << node_name.size() << std::endl;
                    //std::cout << node->Name << "==" << node_name << std::endl;
                    //std::cout << (node->Name.compare(node_name) == 0) << std::endl;
                    if (node->Name.compare(node_name) == 0)
                    {
                        if (node->Name == child_node->Name) { break; }
                        target_node = node;
                        break;
                    }
                }
            }
            //std::cout << j << ": " << target_node->EntireName << std::endl;
            bool need_add = true;
            for (const std::shared_ptr<FileNode>& tmp_node : target_node->Childern)
            {
                if (tmp_node->Name == child_node->Name)
                {
                    if (child_node->IsFile)
                    {
                        //std::cout << target_node->Childern.size() << std::endl;
                        target_node->Childern.remove(tmp_node);
                        //std::cout << target_node->Childern.size() << std::endl;
                    }
                    else
                    {
                        tmp_node->FromChunk = child_node->FromChunk;
                        tmp_node->FromChunkName = child_node->FromChunkName;
                        need_add = false;
                    }
                    break;
                }
            }
            if (need_add)
                target_node->Childern.emplace_back(child_node);
        }
    }
    // the debug build extremely slow
    ChunkCache.clear();
    if (filelist.size() > 0)
    {
        filelist.front()->getSize();
    }
    return filelist;
}

std::shared_ptr<chunk_t> ChunkOTF::getDecompressedChunk(int64_t offset, int64_t size, std::ifstream& reader, bool FlagBaseGame, size_t chunkNum) noexcept
{
    std::shared_ptr<chunk_t> ChunkDecompressed_ = std::make_shared<chunk_t>(0x40000);
    if (size != 0)
    {
        reader.seekg(offset, std::ios_base::beg);

        ChunkCompressedGlobalBuffer.resize(size);
        reader.read(reinterpret_cast<char*>(ChunkCompressedGlobalBuffer.data()), size);

        // nothing more i can do here
        size_t actualSize = oo2coreInstance->Decompress(
            ChunkCompressedGlobalBuffer.data(),
            static_cast<int>(ChunkCompressedGlobalBuffer.size()),
            ChunkDecompressed_->data(),
            static_cast<int>(ChunkDecompressed_->size()));
        ChunkDecompressed_->resize(actualSize);
    }
    else
    {
        reader.seekg(offset, std::ios_base::beg);
        reader.read(reinterpret_cast<char*>(ChunkDecompressed_->data()), ChunkDecompressed_->size());
    }

    if (!FlagBaseGame)
        ChunkDecrypter::DecryptChunk(ChunkDecompressed_->data(), static_cast<int>(ChunkDecompressed_->size()), chunkNum);

    return ChunkDecompressed_;
}

std::string ChunkOTF::getName(int targetlength, bool FlagBaseGame) noexcept
{
    std::vector<uint8_t> tmp(targetlength);
    getOnLength(targetlength, tmp.data(), 0, FlagBaseGame);

    // find the index of value 0
    std::vector<uint8_t>::iterator stringEnd = std::find(tmp.begin(), tmp.end(), 0);

    std::string ans;
    ans.reserve(std::distance(std::begin(tmp), stringEnd));
    ans.append(std::begin(tmp), stringEnd);
    ans.shrink_to_fit();
    return ans;
}

int64_t ChunkOTF::getInt64(bool FlagBaseGame) noexcept
{
    std::array<uint8_t, 8> tmp = { 0,0,0,0,0,0,0,0 };
    getOnLength(8, tmp.data(), 0, FlagBaseGame);

    return *(int64_t*)(tmp.data());
}

int ChunkOTF::getInt32(bool FlagBaseGame) noexcept
{
    std::array<uint8_t, 4> tmp = { 0,0,0,0 }; // do I seriously have to initialize every array?
    getOnLength(4, tmp.data(), 0, FlagBaseGame);

    return *(int*)(tmp.data());
}

void ChunkOTF::getOnLength(int64_t targetlength, uint8_t* tmpPtr, int64_t startAddr, bool FlagBaseGame) noexcept
{
    if (cur_pointer + targetlength < 0x40000)
    {
        std::copy(std::begin(*ChunkDecompressed) + cur_pointer, std::begin(*ChunkDecompressed) + cur_pointer + targetlength, tmpPtr + startAddr);
        cur_pointer += (int)targetlength;
    }
    else
    {
        int tmp_can_read_length = 0x40000 - cur_pointer;
        int64_t tmp_remain_length = targetlength - tmp_can_read_length;
        std::copy(std::begin(*ChunkDecompressed) + cur_pointer, std::begin(*ChunkDecompressed) + cur_pointer + tmp_can_read_length, tmpPtr + startAddr);
        cur_pointer = 0;
        ChunkDecompressed = NextChunkDecompressed;
        cur_index += 1;
        if (cur_index + 1 < DictCount)
        {
            NextChunkDecompressed = getDecompressedChunk(ChunkOffsetDict[cur_index + 1],
                MetaChunk[ChunkOffsetDict[cur_index + 1]],
                Reader, FlagBaseGame, static_cast<size_t>(cur_index) + 1);
        }
        else
        {
            NextChunkDecompressed->clear();
        }
        getOnLength(tmp_remain_length, tmpPtr, startAddr + tmp_can_read_length, FlagBaseGame);
    }
}

int ChunkOTF::ExtractSelected(std::list<std::shared_ptr<FileNode>>& itemlist, const std::string& BaseLocation, bool FlagBaseGame) noexcept
{
    // this will call
    // int ChunkOTF::ExtractSelected(std::list<std::shared_ptr<FileNode>>& itemlist, std::string&& BaseLocation, bool FlagBaseGame)
    // because std::string(BaseLocation) is an rvalue
    return ExtractSelected(itemlist, std::string(BaseLocation), FlagBaseGame);
}

int ChunkOTF::ExtractSelected(std::list<std::shared_ptr<FileNode>>& itemlist, std::string&& BaseLocation, bool FlagBaseGame) noexcept
{
    int failed = 0;
    for (std::shared_ptr<FileNode>& node : itemlist)
    {
        if (!node->Childern.empty())
        {
            failed += ExtractSelected(node->Childern, std::move(BaseLocation), FlagBaseGame);
        }
        else if (node->IsSelected())
        {
            ChunkOTF* CurNodeChunk = this;
            CurNodeChunk->cur_index = node->ChunkIndex;
            CurNodeChunk->cur_pointer = node->ChunkPointer;
            int64_t size = node->Size;
            if (CurNodeChunk->ChunkCache.find(CurNodeChunk->cur_index) != CurNodeChunk->ChunkCache.end())
            {
                CurNodeChunk->ChunkDecompressed = CurNodeChunk->ChunkCache[CurNodeChunk->cur_index];
            }
            else
            {
                if (CurNodeChunk->ChunkCache.size() > 20) CurNodeChunk->ChunkCache.clear();
                CurNodeChunk->ChunkDecompressed = CurNodeChunk->getDecompressedChunk(
                    CurNodeChunk->ChunkOffsetDict[CurNodeChunk->cur_index],
                    CurNodeChunk->MetaChunk[CurNodeChunk->ChunkOffsetDict[CurNodeChunk->cur_index]],
                    CurNodeChunk->Reader,
                    FlagBaseGame,
                    CurNodeChunk->cur_index);
                CurNodeChunk->ChunkCache.emplace(CurNodeChunk->cur_index, CurNodeChunk->ChunkDecompressed);
            }
            if (CurNodeChunk->ChunkCache.find(CurNodeChunk->cur_index + 1) != CurNodeChunk->ChunkCache.end())
            {
                CurNodeChunk->NextChunkDecompressed = CurNodeChunk->ChunkCache[CurNodeChunk->cur_index + 1];
            }
            else
            {
                if (CurNodeChunk->ChunkCache.size() > 20) CurNodeChunk->ChunkCache.clear();
                if (CurNodeChunk->cur_index + 1 < CurNodeChunk->DictCount)
                {
                    CurNodeChunk->NextChunkDecompressed = CurNodeChunk->getDecompressedChunk(
                        CurNodeChunk->ChunkOffsetDict[CurNodeChunk->cur_index + 1],
                        CurNodeChunk->MetaChunk[CurNodeChunk->ChunkOffsetDict[CurNodeChunk->cur_index + 1]],
                        CurNodeChunk->Reader,
                        FlagBaseGame,
                        static_cast<size_t>(CurNodeChunk->cur_index) + 1);
                }
                else { CurNodeChunk->NextChunkDecompressed->clear(); }
                CurNodeChunk->ChunkCache.emplace(CurNodeChunk->cur_index + 1, CurNodeChunk->NextChunkDecompressed);
            }
            //if (!node->IsFile)
            //	fs::create_directories(BaseLocation + node->EntireName + "\\");
            //else
            //{
            //	fs::create_directories(Utils::getUpperDirectory(BaseLocation + node->EntireName)); // ok seriously how did c# do it
            //}
            // what does this part mean???
            if (node->IsFile)
            {
                std::cout << "Extracting " << node->EntireName << " ...                          \r";
                //fs::create_directories(Utils::getUpperDirectory(BaseLocation + node->EntireName)); // ok seriously how did c# do it
                //fs::path filePath(BaseLocation);
                //filePath += node->EntireName;
                Writer.open(BaseLocation + node->EntireName, std::ios::out | std::ios::binary);
                std::vector<uint8_t> temp(size);
                CurNodeChunk->getOnLength(temp.size(), temp.data(), 0, FlagBaseGame);
                Writer.write(reinterpret_cast<char*> (temp.data()), temp.size());
                Writer.close();
            }
        }
    }
    return failed;
}

void ChunkOTF::createSelectedFolder(const std::list<std::shared_ptr<FileNode>>& itemlist, const std::string& BaseLocation) noexcept
{
    createSelectedFolder(itemlist, fs::path(BaseLocation));
}

void ChunkOTF::createSelectedFolder(const std::list<std::shared_ptr<FileNode>>& itemlist, const std::filesystem::path& BaseLocation) noexcept
{
    for (const std::shared_ptr<FileNode>& node : itemlist)
    {
        if (!node->Childern.empty())
            createSelectedFolder(node->Childern, BaseLocation);
        else if (node->IsSelected())
        {
            fs::path dirPath(BaseLocation);
            dirPath += node->EntireName;
            if (!node->IsFile)
            {
                if (!fs::exists(dirPath))
                    fs::create_directories(dirPath);
            }
            else
            {
                if (!fs::exists(dirPath.parent_path()))
                    fs::create_directories(dirPath.parent_path()); // ok seriously how did c# do it
            }
        }
    }
}
