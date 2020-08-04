#ifndef CHUNKOTF_H
#define CHUNKOTF_H

#include <map>
#include <fstream>
#include <vector>
#include <memory>

#include "FileNode.h"
#include "oo2core_loader.h"
#include "ChunkDecrypter.h"

// on the fly chunk processor
class ChunkOTF
{
public:
	ChunkOTF(std::shared_ptr<oo2core_loader> oo2coreInstance_);
	std::list<std::shared_ptr<FileNode>> AnalyzeChunk(const std::string& FileInput, std::list<std::shared_ptr<FileNode>> & inputFileList, bool FlagBaseGame);
	void createSelectedFolder(const std::list<std::shared_ptr<FileNode>>& itemlist, const std::string& BaseLocation);
    int ExtractSelected(std::list<std::shared_ptr<FileNode>> & itemlist, const std::string& BaseLocation, bool FlagBaseGame);
private:
	std::map<int64_t, int64_t> MetaChunk;
	std::map<int, int64_t> ChunkOffsetDict;
	std::ifstream Reader;
	std::vector<uint8_t> ChunkDecompressed;
	std::vector<uint8_t> NextChunkDecompressed;
	int cur_pointer;
	int cur_index;
	int DictCount;
	std::string fileinput;
	std::map<int, std::vector<uint8_t>> ChunkCache;

	// 
	const std::shared_ptr<oo2core_loader> oo2coreInstance;

	std::vector<uint8_t> getDecompressedChunk(int64_t offset, int64_t size, std::ifstream& reader, bool FlagBaseGame, int chunkNum);
	//To read an ASCII string from chunk bytes
	std::string getName(int targetlength, bool FlagBaseGame);
	//To read a byte array at length of targetlength
	void getOnLength(int64_t targetlength, uint8_t * tmp, int64_t startAddr, bool FlagBaseGame);
	//To read int64 from chunk bytes
	int64_t getInt64(bool FlagBaseGame);
	//To read int32 from chunk bytes
	int getInt32(bool FlagBaseGame);
};

#endif
