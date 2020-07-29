#ifndef FILENODE_H
#define FILENODE_H

#include <string>
#include <vector>
#include <list>

class FileNode
{
public:
	std::string Name;
	std::list<FileNode*> Childern;
	std::string Icon;
	std::string EntireName;
	int64_t Offset;
	int64_t Size;
	int ChunkIndex;
	bool IsFile;
	int ChunkPointer;
	std::string NameWithSize;
	std::string FromChunk;
	std::string FromChunkName;

	FileNode();
	FileNode(const std::string& name, bool isFile, const std::string& fromChunk);


	bool IsSelected();

	int getSelectedCount()
	{
		int count = 0;
		foreach(FileNode node in Childern)
		{
			count += node.getSelectedCount();
		}
		if (IsFile && IsSelected)
		{
			count++;
		}
		return count;
	}

	void setChilrenSelected(bool selected)
	{
		foreach(FileNode child in Childern)
		{
			child.IsSelected = selected;
		}
	}

	int64_t getSize();

	

	public string getSizeStr(long _size)
	{
		string sizestr = "";
		if (_size < 1024)
		{
			sizestr = $"{_size} B";
		}
		else if (_size >= 1024 && _size < 1048576)
		{
			sizestr = $"{_size / 1024f:F2} KB";
		}
		else if (_size < 1073741824 && _size >= 1048576)
		{
			sizestr = $"{(_size >> 10) / 1024f:F2} MB";
		}
		else
		{
			sizestr = $"{(_size >> 20) / 1024f:F2} GB";
		}
		return sizestr;
	}

	event PropertyChangedEventHandler PropertyChanged;



private:
	bool isSelected;

	void setNameWithSize(const std::string& name, int64_t _size);
};

#endif
