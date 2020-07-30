#ifndef CHUNKDECRYPTER_H
#define CHUNKDECRYPTER_H

#include <vector>
#include <string>

class ChunkDecrypter
{
public:
	ChunkDecrypter(const std::string& chunkKeyPatternFilePath="./keySequence.bin");
	void DecryptChunk(std::vector<uint8_t>& data, const int dictionaryCount);
private:
	std::vector<uint8_t> chunkKeyPattern;
	std::vector<std::vector<uint8_t>> chunkKeys;
};

#endif
