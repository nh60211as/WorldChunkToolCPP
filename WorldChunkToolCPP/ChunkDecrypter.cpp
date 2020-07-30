#include "ChunkDecrypter.h"

#include <fstream>

ChunkDecrypter::ChunkDecrypter(const std::string& chunkKeyPatternFilePath)
{
	// load chunkKeyPattern, chunkKeyPattern is a static variable and must be initalized 
	chunkKeyPattern = std::vector<uint8_t>(1000009); // set as fixed size instead of read the data size
	std::ifstream chunkKeyReader(chunkKeyPatternFilePath, std::ios::in | std::ios::binary);
	chunkKeyReader.read(reinterpret_cast<char*>(chunkKeyPattern.data()), chunkKeyPattern.size());
	chunkKeyReader.close();

	chunkKeys = std::vector<std::vector<uint8_t>>
	{
		// 0: ac76cb97ec7500133a81038e7a82c80a
		{ 0xac, 0x76, 0xcb, 0x97, 0xec, 0x75, 0x00, 0x13, 0x3a, 0x81, 0x03, 0x8e, 0x7a, 0x82, 0xc8, 0x0a },
		// 1: 6bb5c956e44d00bc305233cfbfaafa25
		{ 0x6b, 0xb5, 0xc9, 0x56, 0xe4, 0x4d, 0x00, 0xbc, 0x30, 0x52, 0x33, 0xcf, 0xbf, 0xaa, 0xfa, 0x25 },
		// 2: 8f021dccb0f2787206fbdee2390bbb5c
		{ 0x8f, 0x02, 0x1d, 0xcc, 0xb0, 0xf2, 0x78, 0x72, 0x06, 0xfb, 0xde, 0xe2, 0x39, 0x0b, 0xbb, 0x5c },
		// 3: da5c1e531d8359157875bd63bfaafa25
		{ 0xda, 0x5c, 0x1e, 0x53, 0x1d, 0x83, 0x59, 0x15, 0x78, 0x75, 0xbd, 0x63, 0xbf, 0xaa, 0xfa, 0x25 },
		// 4: 1f6d31c883dd716d7e8f598ce23f1929
		{ 0x1f, 0x6d, 0x31, 0xc8, 0x83, 0xdd, 0x71, 0x6d, 0x7e, 0x8f, 0x59, 0x8c, 0xe2, 0x3f, 0x19, 0x29 },
		// 5: 4bb0de04e4e0856980ccb2942f9ce9f9
		{ 0x4b, 0xb0, 0xde, 0x04, 0xe4, 0xe0, 0x85, 0x69, 0x80, 0xcc, 0xb2, 0x94, 0x2f, 0x9c, 0xe9, 0xf9 },
		// 6: 8bce54dc4c11139a7875bd63bfaafa25
		{ 0x8b, 0xce, 0x54, 0xdc, 0x4c, 0x11, 0x13, 0x9a, 0x78, 0x75, 0xbd, 0x63, 0xbf, 0xaa, 0xfa, 0x25 },
		// 7: ec13345966ce7312440089a2ceddcee9
		{ 0xec, 0x13, 0x34, 0x59, 0x66, 0xce, 0x73, 0x12, 0x44, 0x00, 0x89, 0xa2, 0xce, 0xdd, 0xce, 0xe9 },
		// 8: e4662c709c753a039a2c0f5ae23f1929
		{ 0xe4, 0x66, 0x2c, 0x70, 0x9c, 0x75, 0x3a, 0x03, 0x9a, 0x2c, 0x0f, 0x5a, 0xe2, 0x3f, 0x19, 0x29 },
		// 9: a492fc9033949c15a033ac223735cca7
		{ 0xa4, 0x92, 0xfc, 0x90, 0x33, 0x94, 0x9c, 0x15, 0xa0, 0x33, 0xac, 0x22, 0x37, 0x35, 0xcc, 0xa7 },
		// 10: 25099c1f911a26e5ce9172f07a82c80a
		{ 0x25, 0x09, 0x9c, 0x1f, 0x91, 0x1a, 0x26, 0xe5, 0xce, 0x91, 0x72, 0xf0, 0x7a, 0x82, 0xc8, 0x0a },
		// 11: d1d29d7446d4fdf1a033ac223735cca7
		{ 0xd1, 0xd2, 0x9d, 0x74, 0x46, 0xd4, 0xfd, 0xf1, 0xa0, 0x33, 0xac, 0x22, 0x37, 0x35, 0xcc, 0xa7 },
		// 12: 7eb268373b5d361ed6d313e2933c4dcb
		{ 0x7e, 0xb2, 0x68, 0x37, 0x3b, 0x5d, 0x36, 0x1e, 0xd6, 0xd3, 0x13, 0xe2, 0x93, 0x3c, 0x4d, 0xcb },
		// 13: a1c7d2ea661895ac7875bd63bfaafa25
		{ 0xa1, 0xc7, 0xd2, 0xea, 0x66, 0x18, 0x95, 0xac, 0x78, 0x75, 0xbd, 0x63, 0xbf, 0xaa, 0xfa, 0x25 },
		// 14: 82a43b2108797c6a440089a2ceddcee9
		{ 0x82, 0xa4, 0x3b, 0x21, 0x08, 0x79, 0x7c, 0x6a, 0x44, 0x00, 0x89, 0xa2, 0xce, 0xdd, 0xce, 0xe9 },
		// 15: 41d055b3dd6015167e8f598ce23f1929
		{ 0x41, 0xd0, 0x55, 0xb3, 0xdd, 0x60, 0x15, 0x16, 0x7e, 0x8f, 0x59, 0x8c, 0xe2, 0x3f, 0x19, 0x29 }
	};
}



void ChunkDecrypter::DecryptChunk(std::vector<uint8_t>& data, const int dictionaryCount)
{
	int keyPos = chunkKeyPattern[dictionaryCount + 8];
	const std::vector<uint8_t> & chunkKey = chunkKeys[keyPos]; // will it be optimized to take the reference

	for (int i = 0; i < data.size(); i++)
	{
		data[i] = (chunkKey[i % chunkKey.size()] ^ data[i]);
	}
}
