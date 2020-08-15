#include "oo2core_loader.h"

oo2core_loader::oo2core_loader(const std::string& oo2coreFilePath) :
	libraryInstance(nullptr),
	g_OodleCompressFunc(nullptr),
	g_OodleDecompressFunc(nullptr)
{
	std::wstring wtemp(oo2coreFilePath.begin(), oo2coreFilePath.end());
	LPCWSTR LPCWSTRtemp = wtemp.c_str();
	libraryInstance = LoadLibrary(LPCWSTRtemp);
	if (libraryInstance == nullptr)
		return;

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
	int decompressedCount = g_OodleDecompressFunc(buffer, size, outputBuffer, uncompressedSize, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3);

	return decompressedCount;
}

bool oo2core_loader::is_oo2core_8_win64_legit()
{
	return true;
}
