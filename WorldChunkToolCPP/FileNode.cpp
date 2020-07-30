#include "FileNode.h"

#include <filesystem>

#include "Utils.h"

namespace fs = std::filesystem;

FileNode::FileNode()
{

}

FileNode::FileNode(const std::string& name, bool isFile, const std::string& fromChunk)
{
	Name = name;
	NameWithSize = "";
	IsFile = isFile;
	if (isFile) Icon = fs::current_path().root_directory().string() + "\\file.png";
	else Icon = fs::current_path().root_directory().string() + "\\dir.png";
	Childern = std::list<std::shared_ptr<FileNode>>();
	setIsSelected(true);
	FromChunk = fromChunk;
	FromChunkName = Utils::removeExtension(fromChunk);
}

bool FileNode::IsSelected()
{
	return isSelected;
}

void FileNode::setIsSelected(bool value)
{
	isSelected = value;
	setChilrenSelected(value);
	//if (this.PropertyChanged != null)
	//{
	//	this.PropertyChanged.Invoke(this, new PropertyChangedEventArgs("IsSelected"));
	//}
}

int FileNode::getSelectedCount()
{
	int count = 0;
	for(const std::shared_ptr<FileNode>& node : Childern)
	{
		count += node->getSelectedCount();
	}
	if (IsFile && IsSelected())
	{
		count++;
	}
	return count;
}

void FileNode::setChilrenSelected(bool selected)
{
	for (const std::shared_ptr<FileNode>& child : Childern)
	{
		child->setIsSelected(selected);
	}
}

std::string FileNode::getSizeStr(int64_t _size)
{
	std::string sizestr = "";
	if (_size < 1024)
	{
		sizestr = std::to_string(_size) + " B";
	}
	else if (_size >= 1024 && _size < 1048576)
	{
		sizestr = std::to_string(_size/1024.0f) + " KB";
	}
	else if (_size < 1073741824 && _size >= 1048576)
	{
		sizestr = std::to_string((_size >> 10) / 1024.0f) + " MB";
	}
	else
	{
		sizestr = std::to_string((_size >> 20) / 1024.0f) + " GB";
	}
	return sizestr;
}

int64_t FileNode::getSize()
{
	if (IsFile) { setNameWithSize(Name, Size); return Size; }
	else
	{
		int64_t _size = 0;
		for (const std::shared_ptr<FileNode>& child : Childern)
		{
			_size += child->getSize();
		}
		Size = _size;
		setNameWithSize(Name, Size);
		return _size;
	}
}

void FileNode::setNameWithSize(const std::string& name, int64_t _size) // why was name passed?????
{
	NameWithSize = name + " (" + std::to_string(_size) + ")";
}
