#ifndef CHUNKDECRYPTER_H
#define CHUNKDECRYPTER_H

#include <vector>
#include <string>

#include "Utils.h"

class ChunkDecrypter
{
public:
	ChunkDecrypter(const std::string& chunkKeyPatternFilePath=CHUNK_KEY_SEQUENCE_FILE_NAME);
	void DecryptChunk(uint8_t* data, const int length, const int dictionaryCount) const;
private:
	std::vector<uint8_t> chunkKeyPattern;
	std::vector<std::vector<uint8_t>> chunkKeys;
};

#endif
