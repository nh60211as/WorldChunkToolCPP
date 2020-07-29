#ifndef CHUNK_H
#define CHUNK_H

#include <string>

class Chunk
{
public:
    static void DecompressChunks(const std::string& FileInput, const flags currentFlag);
};

#endif
