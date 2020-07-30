#include "oo2core_loader.h"


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

int oo2core_loader::Decompress(uint8_t* buffer, int size, uint8_t* outputBuffer, int uncompressedSize) const
{
    return g_OodleDecompressFunc(buffer, size, outputBuffer, uncompressedSize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3);
}

bool oo2core_loader::is_oo2core_8_win64_legit()
{
    return true;
}
