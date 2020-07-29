#include "Chunk.h"

#include <fstream>
#include <filesystem>
#include <map>

#include "Utils.h"

namespace fs = std::filesystem;

void Chunk::DecompressChunks(const std::string& FileInput, const flags currentFlag)
{
    std::string NamePKG = fs::path(FileInput).replace_extension(".pkg").string(); // finally something good out of C++17
    std::ifstream Reader(FileInput, std::ios::binary);

    // Key = ChunkOffset, Value = ChunkSize
    // C# Dictionary is implemented as a hash table
    // https://docs.microsoft.com/en-us/dotnet/api/system.collections.generic.dictionary-2?view=netcore-3.1
    // Dictionary<long, long> MetaChunk = new Dictionary<long, long>();
    std::map<int64_t, int64_t> MetaChunk;

    // Read header
    //Reader.BaseStream.Seek(4, SeekOrigin.Begin);
    Reader.seekg(4, std::ios_base::beg); // skipping the MagicChunk of size int
    //int ChunkCount = Reader.ReadInt32(); int ChunkPadding = ChunkCount.ToString().Length;
    int ChunkCount;
    Reader.read(reinterpret_cast<char*>(&ChunkCount), sizeof(ChunkCount));
    int ChunkPadding = std::to_string(ChunkCount).size();

    double DiskSpace = (int64_t)ChunkCount * (int64_t)0x40000 * 1e-9;

    // TODO: actually parse the string
    Utils::Print("{ChunkCount} chunks in this file. Requires at least: {Math.Round(DiskSpace, 2)} GB.", PRINT_ORDER::AFTER);

    // Read file list
    int64_t totalChunkSize = 0;
    for (int i = 0; i < ChunkCount; i++)
    {
        // Process file size
        uint8_t ArrayTmp1[8];
        //byte[] ArrayChunkSize = Reader.ReadBytes(3);
        uint8_t ArrayChunkSize[3];
        Reader.read(reinterpret_cast<char*>(&ArrayChunkSize), sizeof(ArrayChunkSize));

        //int Low = ArrayChunkSize[0] & 0x0F;

        //int High = ArrayChunkSize[0] >> 4;
        //ArrayChunkSize[0] = BitConverter.GetBytes(High)[0];
        ArrayChunkSize[0] = *(uint8_t*)(ArrayChunkSize[0] >> 4); // does the endianess need a check
        //Array.Copy(ArrayChunkSize, ArrayTmp1, ArrayChunkSize.Length);
        std::copy(std::begin(ArrayChunkSize), std::end(ArrayChunkSize), std::begin(ArrayTmp1));
        //long ChunkSize = BitConverter.ToInt64(ArrayTmp1, 0); // why are C# examples so over-engineered
        int64_t ChunkSize = *(int64_t*)(ArrayTmp1[0]);
        ChunkSize = (ChunkSize >> 4) + (ChunkSize & 0xF);
        totalChunkSize += ChunkSize;

        // Process offset
        uint8_t ArrayTmp2[8];
        //byte[] ArrayChunkOffset = Reader.ReadBytes(5);
        uint8_t ArrayChunkOffset[5];
        Reader.read(reinterpret_cast<char*>(&ArrayChunkOffset), sizeof(ArrayChunkOffset));
        //Array.Copy(ArrayChunkOffset, ArrayTmp2, ArrayChunkOffset.Length);
        std::copy(std::begin(ArrayChunkOffset), std::end(ArrayChunkOffset), std::begin(ArrayTmp2));
        //long ChunkOffset = BitConverter.ToInt64(ArrayTmp2, 0);
        int64_t ChunkOffset = *(int64_t*)(ArrayTmp2[0]);

        MetaChunk.emplace(ChunkOffset, ChunkSize);
    }

    // Write decompressed chunks to pkg
    //BinaryWriter Writer = new BinaryWriter(File.Create(NamePKG));
    //int DictCount = 1;

    //foreach(KeyValuePair<long, long> Entry in MetaChunk)
    //{
    //    Console.Write($"\rProcessing {DictCount.ToString().PadLeft(ChunkPadding)} / {ChunkCount}...");
    //    if (Entry.Value != 0)
    //    {
    //        Reader.BaseStream.Seek(Entry.Key, SeekOrigin.Begin);
    //        byte[] ChunkCompressed = Reader.ReadBytes((int)Entry.Value); // Unsafe cast
    //        byte[] ChunkDecompressed = Utils.Decompress(ChunkCompressed, ChunkCompressed.Length, 0x40000);
    //        if (!FlagBaseGame) { Utils.DecryptChunk(ChunkDecompressed, Utils.GetChunkKey(DictCount - 1)); }
    //        Writer.Write(ChunkDecompressed);
    //    }
    //    else
    //    {
    //        Reader.BaseStream.Seek(Entry.Key, SeekOrigin.Begin);
    //        byte[] ChunkDecompressed = Reader.ReadBytes(0x40000);
    //        if (!FlagBaseGame) { Utils.DecryptChunk(ChunkDecompressed, Utils.GetChunkKey(DictCount - 1)); }
    //        Writer.Write(ChunkDecompressed);
    //    }
    //    DictCount++;
    //}
    //Reader.Close();
    //Writer.Close();

    //Utils.Print("Finished.", true);
    //Utils.Print($"Output at: {NamePKG}", false);

    //// Write csv
    //Utils.Print("Writing file list.", false);
    //PKG.ExtractPKG(NamePKG, FlagAutoConfirm, FlagUnpackAll, true);
}