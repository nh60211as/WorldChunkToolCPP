#include "Chunk.h"

#include <fstream>
#include <filesystem>
#include <map>
#include <array>

#include "Utils.h"
#include "ChunkDecrypter.h"
#include "PKG.h"

namespace fs = std::filesystem;

void Chunk::DecompressChunks(const std::string& FileInput, const flags currentFlag, std::shared_ptr<oo2core_loader> oo2coreInstance)
{
    std::string NamePKG = fs::current_path().string() + "\\" + fs::path(FileInput).filename().replace_extension(".pkg").string(); // finally something good out of C++17
    std::ifstream Reader(FileInput, std::ios::in | std::ios::binary);

    // Key = ChunkOffset, Value = ChunkSize
    // C# Dictionary is implemented as a hash table
    // https://docs.microsoft.com/en-us/dotnet/api/system.collections.generic.dictionary-2?view=netcore-3.1
    // Dictionary<long, long> MetaChunk = new Dictionary<long, long>();
    std::vector<std::pair<int64_t, int64_t>> MetaChunk;

    // Read header
    //Reader.BaseStream.Seek(4, SeekOrigin.Begin);
    Reader.seekg(4, std::ios::beg); // skipping the MagicChunk of size int
    //int ChunkCount = Reader.ReadInt32(); int ChunkPadding = ChunkCount.ToString().Length;
    int ChunkCount;
    Reader.read(reinterpret_cast<char*>(&ChunkCount), sizeof(ChunkCount));
    std::streamsize ChunkPadding = static_cast<std::streamsize>(std::to_string(ChunkCount).size());

    double DiskSpace = static_cast<double>((int64_t)ChunkCount * (int64_t)0x40000 * 1e-9);
    Utils::PrintWithSeparationLine(std::to_string(ChunkCount) + " subchunks detected. Requires at least: " + std::to_string(std::round(DiskSpace * 100) / 100) + " GB.", PRINT_ORDER::AFTER);

    // Read file list
    int64_t totalChunkSize = 0;
    std::array<uint8_t, 8> ArrayChunkSize = { 1,1,1,0,0,0,0,0 }; // zero initialize the array. Is there a better way?
    std::array<uint8_t, 8> ArrayChunkOffset = { 1,1,1,1,1,0,0,0 }; // the ones are just a note to me that they will be modified
    for (int i = 0; i < ChunkCount; i++)
    {
        // Process file size
        //byte[] ArrayChunkSize = Reader.ReadBytes(3);
        Reader.read(reinterpret_cast<char*>(ArrayChunkSize.data()), 3); // read 3 bytes

        //int Low = ArrayChunkSize[0] & 0x0F;

        //int High = ArrayChunkSize[0] >> 4;
        //ArrayChunkSize[0] = BitConverter.GetBytes(High)[0];
        ArrayChunkSize[0] = static_cast<uint8_t>(ArrayChunkSize[0] >> 4);
        //Array.Copy(ArrayChunkSize, ArrayTmp1, ArrayChunkSize.Length);
        //long ChunkSize = BitConverter.ToInt64(ArrayTmp1, 0); // why are C# examples so over-engineered
        int64_t ChunkSize = *(int64_t*)(ArrayChunkSize.data()); // type punning. I only know it probably works on GCC.
        ChunkSize = (ChunkSize >> 4) + (ChunkSize & 0xF);
        totalChunkSize += ChunkSize;

        // Process offset
        //byte[] ArrayChunkOffset = Reader.ReadBytes(5);
        Reader.read(reinterpret_cast<char*>(ArrayChunkOffset.data()), 5); // read 5 bytes
        //Array.Copy(ArrayChunkOffset, ArrayTmp2, ArrayChunkOffset.Length);
        //long ChunkOffset = BitConverter.ToInt64(ArrayTmp2, 0);
        int64_t ChunkOffset = *(int64_t*)(ArrayChunkOffset.data());

        MetaChunk.emplace_back(ChunkOffset, ChunkSize);
    }

    // Write decompressed chunks to pkg
    //BinaryWriter Writer = new BinaryWriter(File.Create(NamePKG));
    std::ofstream Writer(NamePKG, std::ios::out | std::ios::binary);
    size_t DictCount = 1;

    for (const std::pair<int64_t, int64_t>& Entry : MetaChunk)
    {
        int64_t Key = Entry.first;
        int64_t Value = Entry.second;
        std::vector<uint8_t> ChunkDecompressed(0x40000);

        //std::cout << "\rProcessing " << DictCount << "/" << ChunkCount << "\n";
        std::cout << "\rProcessing " << std::right << std::setfill(' ') << std::setw(ChunkPadding) << std::to_string(DictCount) << " / " << ChunkCount << "..." << std::flush;
        if (Value != 0)
        {
            Reader.seekg(Key, std::ios_base::beg); // skipping the MagicChunk of size int
            std::vector<uint8_t> ChunkCompressed(static_cast<size_t>(Value));
            Reader.read(reinterpret_cast<char*>(ChunkCompressed.data()), Value);
            // nothing more i can do here
            size_t actualSize = oo2coreInstance->Decompress(ChunkCompressed.data(), static_cast<int>(ChunkCompressed.size()),
                ChunkDecompressed.data(), static_cast<int>(ChunkDecompressed.size()));
            ChunkDecompressed.resize(actualSize);
        }
        else
        {
            Reader.seekg(Key, std::ios_base::beg);
            Reader.read(reinterpret_cast<char*>(ChunkDecompressed.data()), static_cast<std::streamsize>(ChunkDecompressed.size()));
        }
        if (!currentFlag.FlagBaseGame)
            ChunkDecrypter::DecryptChunk(ChunkDecompressed.data(), static_cast<int>(ChunkDecompressed.size()), DictCount - 1u);

        Writer.write(reinterpret_cast<char*> (ChunkDecompressed.data()), static_cast<std::streamsize>(ChunkDecompressed.size()));
        DictCount++;
    }
    Reader.close();
    Writer.close();

    Utils::PrintWithSeparationLine("Finished.", PRINT_ORDER::BEFORE);
    Utils::PrintWithSeparationLine("Output at: " + NamePKG, PRINT_ORDER::AFTER);

    //// Write csv
    Utils::PrintWithSeparationLine("Writing file list.", PRINT_ORDER::AFTER);
    PKG::ExtractPKG(NamePKG, currentFlag.FlagAutoConfirm, currentFlag.FlagUnpackAll, true);
}
