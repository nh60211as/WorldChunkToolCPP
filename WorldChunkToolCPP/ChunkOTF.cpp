#include "ChunkOTF.h"

#include <filesystem>
#include <cmath>

#include "Utils.h"

namespace fs = std::filesystem;

ChunkOTF::ChunkOTF(std::shared_ptr<oo2core_loader> oo2coreInstance_):
	oo2coreInstance(oo2coreInstance_)
{
}

std::list<std::shared_ptr<FileNode>> ChunkOTF::AnalyzeChunk(const std::string& FileInput, std::list<std::shared_ptr<FileNode>> & inputFileList, bool FlagBaseGame)
{
	fileinput = FileInput; // store the file input

	ChunkCache = std::map<int, std::vector<uint8_t>>();

	std::list<std::shared_ptr<FileNode>> filelist = inputFileList;
	MetaChunk = std::map<int64_t, int64_t>();
	ChunkOffsetDict = std::map<int, int64_t>();
	std::string NamePKG = fs::path(FileInput).replace_extension(".pkg").string(); // finally something good out of C++17
	Reader = std::ifstream(FileInput, std::ios::in);

	// Read header
	Reader.seekg(4, std::ios_base::beg); // skipping the MagicChunk of size int
//int ChunkCount = Reader.ReadInt32(); int ChunkPadding = ChunkCount.ToString().Length;
	int ChunkCount;
	Reader.read(reinterpret_cast<char*>(&ChunkCount), sizeof(ChunkCount));
	int ChunkPadding = std::to_string(ChunkCount).size();

	double DiskSpace = (int64_t)ChunkCount * (int64_t)0x40000 * 1e-9;
	// TODO: actually parse the string
	Utils::Print(std::to_string(ChunkCount) + " subchunks detected. Requires at least: " + std::to_string(std::round(DiskSpace*100)/100) + " GB.", PRINT_ORDER::AFTER);

	// Read file list
	DictCount = 0;
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

		DictCount = i + 1;
	}

	cur_index = 0;
	int64_t cur_offset = ChunkOffsetDict[cur_index];
	int64_t cur_size = MetaChunk[cur_offset];

	ChunkDecompressed = getDecompressedChunk(cur_offset, cur_size, Reader, FlagBaseGame, cur_index);
	if (cur_index + 1 < DictCount)
	{
		NextChunkDecompressed = getDecompressedChunk(ChunkOffsetDict[cur_index + 1], MetaChunk[ChunkOffsetDict[cur_index + 1]], Reader, FlagBaseGame, cur_index + 1);
	}
	else
	{
		NextChunkDecompressed.resize(0);
	}
	cur_pointer = 0x0C;
	int TotalParentCount = *(int*)(ChunkDecompressed.data() + cur_pointer);
	cur_pointer += 4;
	int TotalChildrenCount = *(int*)(ChunkDecompressed.data() + cur_pointer);
	cur_pointer = 0x100;
	std::shared_ptr<FileNode> root_node = nullptr;
	for (int i = 0; i < TotalParentCount; i++)
	{
		std::string StringNameParent = getName(0x3C, FlagBaseGame);
		int64_t FileSize = getInt64(FlagBaseGame);
		long FileOffset = getInt64(FlagBaseGame);
		int EntryType = getInt32(FlagBaseGame);
		int CountChildren = getInt32(FlagBaseGame);

		if (filelist.size() == 0)
		{
			root_node = std::make_shared<FileNode>(StringNameParent, false, FileInput);
			root_node->EntireName = root_node->Name;
			filelist.emplace_back(root_node);
		}
		else
		{
			root_node = filelist.front();
			root_node->FromChunk = fileinput;
			root_node->FromChunkName = fs::path(FileInput).replace_extension("").string(); // atcually removed the extension
		}

		for (int j = 0; j < CountChildren; j++)
		{
			int origin_pointer = cur_pointer;
			int origin_loc = cur_index;
			if (ChunkCache.count(cur_index) == 0) ChunkCache.emplace(cur_index, ChunkDecompressed);
			if (ChunkCache.count(cur_index + 1) == 0) ChunkCache.emplace(cur_index + 1, NextChunkDecompressed);

			std::string StringNameChild = getName(0xA0, FlagBaseGame);
			FileSize = getInt64(FlagBaseGame);
			FileOffset = getInt64(FlagBaseGame);
			EntryType = getInt32(FlagBaseGame);
			int Unknown = getInt32(FlagBaseGame);

			if (EntryType == 0x02)
			{
				cur_pointer = origin_pointer;
				if (cur_index != origin_loc)
				{
					cur_index = origin_loc;
					ChunkDecompressed = ChunkCache[cur_index];
					NextChunkDecompressed = ChunkCache[cur_index + 1];
					ChunkCache.erase(cur_index);
					ChunkCache.erase(cur_index + 1);
				}
				StringNameChild = getName(0x50, FlagBaseGame);
				std::vector<uint8_t> temp(0x68);
				getOnLength(temp.size(), temp, 0, FlagBaseGame);
			}
			std::vector<std::string> fathernodes = Utils::stringSplit(StringNameChild, "\\");
			bool isFile = false;
			if (EntryType == 0x02 || EntryType == 0x00) isFile = true;
			std::shared_ptr<FileNode> child_node = std::make_shared<FileNode>(fathernodes.back(), isFile, FileInput);
			if (isFile)
			{
				child_node->Size = FileSize;
				child_node->Offset = FileOffset;
				child_node->ChunkIndex = (int)(FileOffset / 0x40000);
				child_node->ChunkPointer = (int)(FileOffset % 0x40000);
			}
			child_node->EntireName = StringNameChild;
			std::shared_ptr<FileNode> target_node = root_node;
			for (std::string node_name : fathernodes)
			{
				if (node_name.empty()) continue;
				for (const std::shared_ptr<FileNode> & node : target_node->Childern)
				{
					if (node->Name == node_name)
					{
						if (node->Name == child_node->Name) { break; }
						target_node = node;
						break;
					}
				}
			}
			bool need_add = true;
			for (const std::shared_ptr<FileNode>& tmp_node : target_node->Childern)
			{
				if (tmp_node->Name == child_node->Name)
				{
					if (child_node->IsFile) target_node->Childern.remove(tmp_node);
					else
					{
						tmp_node->FromChunk = child_node->FromChunk;
						tmp_node->FromChunkName = child_node->FromChunkName;
						need_add = false;
					}
					break;
				}
			}
			if (need_add)
				target_node->Childern.emplace_back(child_node);
		}
	}
	ChunkCache.clear();
	if (filelist.size() > 0)
	{
		filelist.front()->getSize();
	}
	return filelist;
}

std::vector<uint8_t> ChunkOTF::getDecompressedChunk(int64_t offset, int64_t size, std::ifstream& reader, bool FlagBaseGame, int chunkNum)
{
	if (size != 0)
	{
		reader.seekg(offset, std::ios_base::beg);

		std::vector<uint8_t> ChunkCompressed(size);
		reader.read(reinterpret_cast<char*>(ChunkCompressed.data()), size);

		std::vector<uint8_t> ChunkDecompressed(0x40000);
		oo2coreInstance->Decompress(ChunkCompressed.data(), ChunkCompressed.size(),
			ChunkDecompressed.data(), ChunkDecompressed.size());

		if (!FlagBaseGame)
		{
			Utils::DecryptChunk(ChunkDecompressed, chunkNum);
		}

		return ChunkDecompressed;
	}
	else
	{
		reader.seekg(offset, std::ios_base::beg);
		std::vector<uint8_t> ChunkDecompressed(0x40000);
		reader.read(reinterpret_cast<char*>(ChunkDecompressed.data()), ChunkDecompressed.size());
		if (!FlagBaseGame)
		{
			Utils::DecryptChunk(ChunkDecompressed, chunkNum);
		}
		return ChunkDecompressed;
	}
}

std::string ChunkOTF::getName(int targetlength, bool FlagBaseGame)
{
	std::vector<uint8_t> tmp(targetlength);
	getOnLength(targetlength, tmp, 0, FlagBaseGame);

	std::string ans;
	ans.reserve(tmp.size());
	ans.append(std::begin(tmp), std::end(tmp));
	return ans;
}

int64_t ChunkOTF::getInt64(bool FlagBaseGame)
{
	std::vector<uint8_t> tmp(8);
	getOnLength(8, tmp, 0, FlagBaseGame);

	return *(int64_t*)(tmp.data());
}

int ChunkOTF::getInt32(bool FlagBaseGame)
{
	std::vector<uint8_t> tmp(4);
	getOnLength(4, tmp, 0, FlagBaseGame);

	return *(int*)(tmp.data());
}

std::vector<uint8_t> ChunkOTF::getOnLength(int64_t targetlength, std::vector<uint8_t>& tmp, int64_t startAddr, bool FlagBaseGame)
{
	if (cur_pointer + targetlength < 0x40000)
	{
		std::copy(std::begin(ChunkDecompressed) + cur_pointer, std::begin(ChunkDecompressed) + cur_pointer + targetlength, std::begin(tmp) + startAddr);
		cur_pointer += (int)targetlength;
	}
	else
	{
		int tmp_can_read_length = 0x40000 - cur_pointer;
		long tmp_remain_length = targetlength - tmp_can_read_length;
		std::copy(std::begin(ChunkDecompressed) + cur_pointer, std::begin(ChunkDecompressed) + cur_pointer + tmp_can_read_length, std::begin(tmp) + startAddr);
		cur_pointer = 0;
		ChunkDecompressed = NextChunkDecompressed;
		cur_index += 1;
		if (cur_index + 1 < DictCount) { NextChunkDecompressed = getDecompressedChunk(ChunkOffsetDict[cur_index + 1], MetaChunk[ChunkOffsetDict[cur_index + 1]], Reader, FlagBaseGame, cur_index + 1); }
		else
		{
			NextChunkDecompressed.resize(0);
		}
		getOnLength(tmp_remain_length, tmp, startAddr + tmp_can_read_length, FlagBaseGame);
	}
	return tmp;
}

int ChunkOTF::ExtractSelected(std::list<std::shared_ptr<FileNode>>& itemlist, std::string BaseLocation, bool FlagBaseGame)
{
	int failed = 0;
	for (const std::shared_ptr<FileNode>& node : itemlist)
	{
		if (!node->Childern.empty())
		{
			failed += ExtractSelected(node->Childern, BaseLocation, FlagBaseGame);
		}
		else if (node->IsSelected())
		{
			ChunkOTF* CurNodeChunk = this;
			CurNodeChunk->cur_index = node->ChunkIndex;
			CurNodeChunk->cur_pointer = node->ChunkPointer;
			long size = node->Size;
			if (CurNodeChunk->ChunkCache.find(CurNodeChunk->cur_index) != CurNodeChunk->ChunkCache.end())
			{
				CurNodeChunk->ChunkDecompressed = CurNodeChunk->ChunkCache[CurNodeChunk->cur_index];
			}
			else
			{
				if (CurNodeChunk->ChunkCache.size() > 20) CurNodeChunk->ChunkCache.clear();
				CurNodeChunk->ChunkDecompressed =
					CurNodeChunk->getDecompressedChunk(CurNodeChunk->ChunkOffsetDict[CurNodeChunk->cur_index],
						CurNodeChunk->MetaChunk[CurNodeChunk->ChunkOffsetDict[CurNodeChunk->cur_index]],
						CurNodeChunk->Reader,
						FlagBaseGame,
						CurNodeChunk->cur_index);
				CurNodeChunk->ChunkCache.emplace(CurNodeChunk->cur_index, CurNodeChunk->ChunkDecompressed);
			}
			if (CurNodeChunk->ChunkCache.find(CurNodeChunk->cur_index + 1) != CurNodeChunk->ChunkCache.end())
			{
				CurNodeChunk->NextChunkDecompressed = CurNodeChunk->ChunkCache[CurNodeChunk->cur_index + 1];
			}
			else
			{
				if (CurNodeChunk->ChunkCache.size() > 20) CurNodeChunk->ChunkCache.clear();
				if (CurNodeChunk->cur_index + 1 < CurNodeChunk->DictCount)
				{
					CurNodeChunk->NextChunkDecompressed =
						CurNodeChunk->getDecompressedChunk(CurNodeChunk->ChunkOffsetDict[CurNodeChunk->cur_index + 1],
							CurNodeChunk->MetaChunk[CurNodeChunk->ChunkOffsetDict[CurNodeChunk->cur_index + 1]],
							CurNodeChunk->Reader,
							FlagBaseGame,
							CurNodeChunk->cur_index + 1);
				}
				else { CurNodeChunk->NextChunkDecompressed.clear(); }
				CurNodeChunk->ChunkCache.emplace(CurNodeChunk->cur_index + 1, CurNodeChunk->NextChunkDecompressed);
			}
			if (!node->IsFile)
				fs::create_directories(BaseLocation + node->EntireName + "\\");
			else
				fs::create_directories(BaseLocation + node->EntireName);
			// what does this part mean???
			if (node->IsFile)
			{
				std::cout << "Extracting " << node->EntireName << " ...                          \r";
				std::ofstream writer(BaseLocation + node->EntireName);
				std::vector<uint8_t> temp(size);
				temp = CurNodeChunk->getOnLength(temp.size(), temp, 0, FlagBaseGame);
				writer.write(reinterpret_cast<char*> (temp.data()), temp.size());
			}
		}
	}
	return failed;
}
