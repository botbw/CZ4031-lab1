

#include <vector>
#include <tuple>

using namespace std;

class Disk
{
public:
    Disk(std::size_t size, std::size_t blockSize);
    ~Disk();
    int getUsedSpace();
    Node allocateNode(int size);
    Record allocateRecord(int size);

private:
    // total memory size, from 100 MB to 500 MB
    // 1MB = 1,048,576 Bytes, we store in bytes
    std::size_t size;
    // block size, fixed 200 B
    std::size_t blockSize = int(200);
    // pointer to the memory pool
    char *pool;
    // pointer to the current position in the memory pool
    char *current;
    // remaining space in the current block
    std::size_t remain_space;
    // number of blocks used
    int used_num_blocks = int(0);
};

// constructor
Disk::Disk(std::size_t size, std::size_t blockSize)
{
    this->size = size;
    this->blockSize = blockSize;
    this->pool = operator new(size);
    std::memset(this->pool, NULL, size);
    this->block = nullptr;
    this->remain_space = blockSize;
}

// method
int Disk::getUsedSpace()
{
    return this->used_num_blocks * this->blockSize - this->remain_space;
}
Node Disk::allocateNode(int size)
{
    // need to allocate a new block
    if (size > remain_space)
    {
        current = pool + (used_num_blocks * blockSize);
        remain_space = blockSize;
        used_num_blocks++;
        return new (current) Node;
    }
    // use the remain space in the current block
    else
    {
        remain_space -= size;
        Node *node = new (current) Node;
        current += size;
        return node;
    }
}

Record Disk::allocateRecord(int size)
{
    // need to allocate a new block
    if (size > remain_space)
    {
        current = pool + (used_num_blocks * blockSize);
        remain_space = blockSize;
        used_num_blocks++;
        return new (current) Record;
    }
    // use the remain space in the current block
    else
    {
        remain_space -= size;
        Record *record = new (current) Record;
        current += size;
        return record;
    }
}
