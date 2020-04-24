#pragma once
struct MemoryBlock
{
    size_t Size;
    bool IsFree;
    MemoryBlock* NextMemoryBlock;
};

