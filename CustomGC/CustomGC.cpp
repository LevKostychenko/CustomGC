#include <iostream>
#include "TestDisposedObject.h"
#include "MemoryManager.h"

int main()
{
    MemoryManager<int> memoryManager;
    int* int_ptr = (int*)memoryManager.MallocL(100 * sizeof(int));

    int_ptr[0] = 1;
    int_ptr[1] = 2;
    int_ptr[2] = 5;
    int_ptr[3] = 9;
    int_ptr[4] = 2445;
    int_ptr[5] = -3456;
    int_ptr[6] = 0;

    for (int i = 0; i < 7; i++)
    {
        printf("%d\n", int_ptr[i]);
    }

    memoryManager.FreeL(int_ptr);
    MemoryManager<TestDisposedObject> new_mem_manager;

    StackRef<TestDisposedObject> test_obj;
    test_obj.ref = new TestDisposedObject;
    test_obj.ref->other = new TestDisposedObject;
    test_obj.ref->header.gcData[STRUCT_SZ] = sizeof(test_obj.ref);
    new_mem_manager.GCCollect();

    return 0;
}
