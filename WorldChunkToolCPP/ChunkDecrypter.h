#ifndef CHUNKDECRYPTER_H
#define CHUNKDECRYPTER_H

#include <vector>
#include <string>

#include "Utils.h"

class ChunkDecrypter
{
public:
    ChunkDecrypter(const std::string& chunkKeyPatternFilePath = CHUNK_KEY_SEQUENCE_FILE_NAME);
    static void DecryptChunk(uint8_t* data, const int length, const size_t dictionaryCount);
private:
    static std::vector<uint8_t> chunkKeyPattern;
    static std::vector<std::vector<uint8_t>> chunkKeys;
};

#endif
