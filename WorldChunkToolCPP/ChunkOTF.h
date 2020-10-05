#ifndef CHUNKOTF_H
#define CHUNKOTF_H

#include <map>
#include <fstream>
#include <vector>
#include <memory>

#include "FileNode.h"
#include "oo2core_loader.h"
#include "DefaultInitAllocator.h"

using chunk_t = std::vector<uint8_t, DefaultInitAllocator<uint8_t>>;

// on the fly chunk processor
class ChunkOTF
{
public:
    ChunkOTF(std::shared_ptr<oo2core_loader> oo2coreInstance_) noexcept;
    std::list<std::shared_ptr<FileNode>> AnalyzeChunk(const std::string& FileInput, std::list<std::shared_ptr<FileNode>>& inputFileList, bool FlagBaseGame) noexcept;
    void createSelectedFolder(const std::list<std::shared_ptr<FileNode>>& itemlist, const std::filesystem::path& BaseLocation) noexcept;
    void createSelectedFolder(const std::list<std::shared_ptr<FileNode>>& itemlist, const std::string& BaseLocation) noexcept;
    int ExtractSelected(std::list<std::shared_ptr<FileNode>>& itemlist, const std::string& BaseLocation, bool FlagBaseGame) noexcept;
    int ExtractSelected(std::list<std::shared_ptr<FileNode>>& itemlist, std::string&& BaseLocation, bool FlagBaseGame) noexcept;

private:
    std::map<int64_t, int64_t> MetaChunk;
    std::map<int, int64_t> ChunkOffsetDict;
    std::ifstream Reader;
    std::ofstream Writer;
    chunk_t ChunkCompressedGlobalBuffer;
    std::shared_ptr<chunk_t> ChunkDecompressed;
    std::shared_ptr<chunk_t> NextChunkDecompressed;
    int cur_pointer;
    int cur_index;
    int DictCount;
    std::string fileinput;
    std::map<int, std::shared_ptr<chunk_t>> ChunkCache;

    // pointer to the oo2core_loader instance
    const std::shared_ptr<oo2core_loader> oo2coreInstance;

    std::shared_ptr<chunk_t> getDecompressedChunk(int64_t offset, int64_t size, std::ifstream& reader, bool FlagBaseGame, size_t chunkNum) noexcept;
    //To read an ASCII string from chunk bytes
    std::string getName(int targetlength, bool FlagBaseGame) noexcept;
    //To read a byte array at length of targetlength
    void getOnLength(int64_t targetlength, uint8_t* tmp, int64_t startAddr, bool FlagBaseGame) noexcept;
    //To read int64 from chunk bytes
    int64_t getInt64(bool FlagBaseGame) noexcept;
    //To read int32 from chunk bytes
    int getInt32(bool FlagBaseGame) noexcept;
};

#endif
