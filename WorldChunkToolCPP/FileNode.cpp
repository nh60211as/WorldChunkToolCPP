#include "FileNode.h"

#include "Utils.h"

FileNode::FileNode()
{

}

FileNode::FileNode(const std::string& name, bool isFile, const std::string& fromChunk)
{
	Name = name;
	NameWithSize = "";
	IsFile = isFile;
	if (isFile) Icon = AppDomain.CurrentDomain.BaseDirectory + "\\file.png";
	else Icon = AppDomain.CurrentDomain.BaseDirectory + "\\dir.png";
	Childern = std::list<FileNode>();
	IsSelected = true;
	FromChunk = fromChunk;
	FromChunkName = Utils::removeExtension(fromChunk);
}

bool FileNode::IsSelected()
{
	//get { return isSelected; }
	//set
	//{
	//	isSelected = value;
	//	setChilrenSelected(value);
	//	if (this.PropertyChanged != null)
	//	{
	//		this.PropertyChanged.Invoke(this, new PropertyChangedEventArgs("IsSelected"));
	//	}
	//}
}

int64_t FileNode::getSize()
{
	if (IsFile) { setNameWithSize(Name, Size); return Size; }
	else
	{
		int64_t _size = 0;
		for (FileNode& child : Childern)
		{
			_size += child.getSize();
		}
		Size = _size;
		setNameWithSize(Name, Size);
		return _size;
	}
}

void FileNode::setNameWithSize(const std::string & name, int64_t _size) // why was name passed?????
{
	NameWithSize = name + " (" + std::to_string(_size) + ")";
}
