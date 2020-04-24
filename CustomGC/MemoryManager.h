#pragma once
#include "MemoryBlock.h"
#include "GCHeader.h"
#include "StackRef.h"
#include<stdio.h>
#include<stddef.h>

template <class T>
class MemoryManager
{
public:
    MemoryManager();  
    void Split(MemoryBlock* fitting_slot, size_t size);
    void DefragmentationMerge();
    void* MallocL(size_t noOfBytes);
    void FreeL(void* ptr);
    void GCMove(GCHeader** gcobject);
    void GCCollect();

private:
    GCHeader* GCRawAlloc(size_t size, int ref_count);
    bool IsPointer(GCHeader gcobject);
    size_t size_of_memory = 20000;
    char memory[20000];
    MemoryBlock memory_block;
    MemoryBlock* current_block;
    MemoryBlock* free_list;
    int memory_blocks_count;
    int current_offset;
    void Initialize();   
};

template <class T>
MemoryManager<T>::MemoryManager()
{
    this->free_list = static_cast<MemoryBlock*>((void*)(this->memory));
    this->Initialize();
}

template <class T>
void MemoryManager<T>::Initialize() {
    this->free_list->Size = this->size_of_memory - sizeof(MemoryBlock);
    this->free_list->IsFree = true;
    this->current_block = new MemoryBlock();
    free_list->NextMemoryBlock = NULL;
}

template <class T>
void MemoryManager<T>::Split(MemoryBlock* fitting_slot, size_t size) {
    MemoryBlock* new_memory_block = static_cast<MemoryBlock*>((void*)(fitting_slot + size + sizeof(MemoryBlock)));
    new_memory_block->Size = (fitting_slot->Size) - size - sizeof(MemoryBlock);
    new_memory_block->IsFree = 1;
    new_memory_block->NextMemoryBlock = fitting_slot->NextMemoryBlock;
    fitting_slot->Size = size;
    fitting_slot->IsFree = 0;
    fitting_slot->NextMemoryBlock = new_memory_block;
    current_block = fitting_slot;
}

template <class T>
void* MemoryManager<T>::MallocL(size_t noOfBytes) {
    MemoryBlock* curr, * prev;

    void* result;
    if (!(this->free_list->Size))
    {
        this->Initialize();
        printf("Memory initialized\n");
    }

    curr = this->free_list;
    while ((((curr->Size) < noOfBytes) || ((curr->IsFree) == false)) && (curr->NextMemoryBlock != NULL))
    {
        prev = curr;
        curr = curr->NextMemoryBlock;
        printf("One block checked\n");
    }

    if ((curr->Size) == noOfBytes) {
        curr->IsFree = false;
        result = (void*)(++curr);
        current_block = curr;
        printf("Exact fitting block allocated\n");
        return result;
    }
    else if ((curr->Size) > (noOfBytes + sizeof(MemoryBlock)))
    {
        this->Split(curr, noOfBytes);
        result = (void*)(++curr);
        current_block = curr;
        printf("Fitting block allocated with a split\n");
        return result;
    }
    else
    {
        result = NULL;
        printf("No sufficient memory to allocate\n");
        return result;
    }
}

template <class T>
void MemoryManager<T>::DefragmentationMerge()
{
    MemoryBlock* curr, * prev;
    curr = this->free_list;
    while (curr != NULL || (curr->NextMemoryBlock) != NULL)
    {
        if ((curr->IsFree) && (curr->NextMemoryBlock->IsFree))
        {
            curr->Size += (curr->NextMemoryBlock->Size) + sizeof(MemoryBlock);
            curr->NextMemoryBlock = curr->NextMemoryBlock->NextMemoryBlock;
        }

        prev = curr;
        curr = curr->NextMemoryBlock;

        if (curr == NULL)
        {
            return;
        }
    }
}

template <class T>
void MemoryManager<T>::FreeL(void* ptr)
{
    if (((void*)this->memory <= ptr) && (ptr <= (void*)(this->memory + this->size_of_memory)))
    {
        MemoryBlock* curr = static_cast<MemoryBlock*>(ptr);
        --curr;
        curr->IsFree = 1;
        this->DefragmentationMerge();
    }
    else
    {
        printf("Please provide a valid pointer allocated by MyMalloc\n");
    }
}

template<class T>
bool MemoryManager<T>::IsPointer(GCHeader gcobject)
{
    return (gcobject.gcData[REF_COUNT] & 1) == 0;
}

template<class T>
void MemoryManager<T>::GCMove(GCHeader** gcobject)
{
    if (*gcobject == NULL)
    {
        return;
    }
        
    if (IsPointer(**gcobject)) //—сылка на уже перемещенный объект. ѕеренаправл€ю куда надо
    {
        (*gcobject) = (*gcobject)->post_gcAddress;
        return;
    }

    GCHeader* new_obj = GCRawAlloc((*gcobject)->gcData[STRUCT_SZ], (*gcobject)->gcData[REF_COUNT]);
    memcpy(new_obj, (*gcobject), sizeof(char) * (*gcobject)->gcData[STRUCT_SZ]);

    GCHeader** iterator = reinterpret_cast<GCHeader**>(new_obj) + 1;

    (*gcobject)->post_gcAddress = new_obj;
    (*gcobject) = new_obj;
    int refCount = new_obj->gcData[REF_COUNT] >> 1;
    for (int i = 0; i < refCount; ++i, ++iterator)
    {
        GCMove(iterator);
    }
}

template<class T>
GCHeader* MemoryManager<T>::GCRawAlloc(size_t size, int ref_count) 
{
    if (size > this->size_of_memory)
    {
        return NULL;
    }

    if (this->current_offset+ size > this->size_of_memory)
    { 
        ++this->memory_blocks_count;
        this->current_block->NextMemoryBlock = new MemoryBlock();
        this->current_block = current_block->NextMemoryBlock;
        this->current_block->NextMemoryBlock = NULL;
        this->current_offset = 0;
    }

    GCHeader* new_obj = reinterpret_cast<GCHeader*>(&(memory[current_offset]));
    new_obj->gcData[STRUCT_SZ] = size;
    new_obj->gcData[REF_COUNT] = (ref_count << 1) | 1;
    current_offset += size;

    return new_obj;
}

template<class T>
void MemoryManager<T>::GCCollect()
{
    MemoryBlock* new_first_memblock = current_block;
    current_block->NextMemoryBlock = NULL;
    current_offset = 0;
    memory_blocks_count = 1;
    for (auto i = stck_references.begin(); i != stck_references.end(); ++i)
    {
        GCMove(*i);
    }

    //—борка завершена, достижимые данные перемещены в другую область пам€ти, старые можно безболезненно удалить
    MemoryBlock* iter = new_first_memblock;
    new_first_memblock = new_first_memblock;
    while (iter != NULL) 
    {
        MemoryBlock* t = iter->NextMemoryBlock;
        delete[] iter;
        iter = t;
    }
}

