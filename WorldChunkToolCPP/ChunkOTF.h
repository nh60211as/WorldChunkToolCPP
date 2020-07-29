#ifndef CHUNKOTF_H
#define CHUNKOTF_H

#include <map>
#include <fstream>
#include <vector>

#include "FileNode.h"
#include "oo2core_loader.h"

// on the fly chunk processor
class ChunkOTF
{
public:
	ChunkOTF(const oo2core_loader* oo2coreInstance_);
	std::vector<FileNode> AnalyzeChunk(const std::string& FileInput, const std::vector<FileNode>& inputFileList, bool FlagBaseGame);
private:
	const oo2core_loader* oo2coreInstance;
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

	std::vector<uint8_t> getDecompressedChunk(int64_t offset, int64_t size, std::ifstream& reader, bool FlagBaseGame, int chunkNum);
	//To read an ASCII string from chunk bytes
	std::string getName(int targetlength, bool FlagBaseGame);
	//To read a byte array at length of targetlength
	std::vector<uint8_t> getOnLength(int64_t targetlength, std::vector<uint8_t>& tmp, int64_t startAddr, bool FlagBaseGame);
	//To read int64 from chunk bytes
	int64_t getInt64(bool FlagBaseGame);
	//To read int32 from chunk bytes
	int getInt32(bool FlagBaseGame);
};

#endif
