#ifndef CHUNK_H
#define CHUNK_H

#include <string>
#include <fstream>
#include <filesystem>
#include <map>

#include "Utils.h"

namespace fs = std::filesystem;

class Chunk
{
public:
    static void DecompressChunks(const std::string& FileInput, const flags currentFlag);
};

#endif
