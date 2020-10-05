#ifndef OO2CORE_LOADER_H
#define OO2CORE_LOADER_H

// reference: https://en.uesp.net/wiki/ESOMod:DAT_File_Format
#include <Windows.h>
#include <cstdint>
#include <string>

#include "Utils.h"

typedef int __stdcall OodleLZ_Compress_Func(unsigned int fmt, unsigned char* buffer, int bufferSize, unsigned char* outputBuffer, int level, void* unused1, void* unused2, void* unused3);
typedef int __stdcall OodleLZ_Decompress_Func(unsigned char* buffer, int bufferSize, unsigned char* outputBuffer, int outputBufferSize, int a, int b, int c, void* d, void* e, void* f, void* g, void* h, void* i, int threadModule);

class oo2core_loader
{
public:
    oo2core_loader(const std::string& oo2coreFilePath = OO2CORE_FILE_NAME) noexcept;
    ~oo2core_loader() noexcept;

    // use explict operator to check if the library is loaded
    explicit operator bool() const
    {
        if (libraryInstance == nullptr)
            return false;
        if (g_OodleCompressFunc == nullptr)
            return false;
        if (g_OodleDecompressFunc == nullptr)
            return false;

        return true;
    }

    int Decompress(uint8_t* buffer, int size, uint8_t* outputBuffer, int uncompressedSize) const noexcept;

    // TODO: actually check the file hash of oo2core_8_win64.dll
    // SHA256: D5440627BBDE1EF2D9730976DD90C5189D874FB7BB808669734E598CDFDBA8D9
    bool is_oo2core_8_win64_legit() const noexcept;

private:
    HINSTANCE libraryInstance;
    OodleLZ_Compress_Func* g_OodleCompressFunc;
    OodleLZ_Decompress_Func* g_OodleDecompressFunc;
};

#endif
