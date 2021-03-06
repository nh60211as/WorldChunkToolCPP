#ifndef FILENODE_H
#define FILENODE_H

#include <string>
#include <list>
#include <memory>

class FileNode
{
public:
    std::string Name; // file name
    std::list<std::shared_ptr<FileNode>> Childern;
    std::string Icon; // file path to icon image (dir or file)
    std::string EntireName; // full file path
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
    void setIsSelected(bool value);

    int getSelectedCount();

    void setChildrenSelected(bool selected);

    int64_t getSize();

    std::string getSizeStr(int64_t _size);


private:
    bool isSelected;

    void setNameWithSize(const std::string& name, int64_t _size);
};

#endif
