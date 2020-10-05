#ifndef CHUNK_H
#define CHUNK_H

#include <string>
#include <memory>

#include "oo2core_loader.h"

class Chunk
{
public:
    static void DecompressChunks(const std::string& FileInput, const flags currentFlag, std::shared_ptr<oo2core_loader> oo2coreInstance);
};

#endif
