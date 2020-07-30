#include "oo2core_loader.h"
#include <vector>

oo2core_loader::oo2core_loader()
{
    libraryInstance = LoadLibrary(TEXT(OO2CORE_FILE_NAME));

    g_OodleCompressFunc = (OodleLZ_Compress_Func*)GetProcAddress(libraryInstance, "OodleLZ_Compress");
    g_OodleDecompressFunc = (OodleLZ_Decompress_Func*)GetProcAddress(libraryInstance, "OodleLZ_Decompress");
}

oo2core_loader::~oo2core_loader()
{
    if (libraryInstance != nullptr)
        FreeLibrary(libraryInstance);
}

std::vector<uint8_t> oo2core_loader::Decompress(std::vector<uint8_t> & buffer, int size, int uncompressedSize) const
{
    std::vector<uint8_t> decompressedBuffer(uncompressedSize);
    int decompressedCount = g_OodleDecompressFunc(buffer.data(), size, decompressedBuffer.data(), uncompressedSize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3);
    
    if (decompressedCount == uncompressedSize)
        return decompressedBuffer;
    if (decompressedCount < uncompressedSize)
    {
        decompressedBuffer.resize(decompressedCount);
        return decompressedBuffer;
    }
    return std::vector<uint8_t>();
}

bool oo2core_loader::is_oo2core_8_win64_legit()
{
    return true;
}
