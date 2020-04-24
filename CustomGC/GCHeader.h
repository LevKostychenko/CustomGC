#pragma once
enum { STRUCT_SZ = 0, REF_COUNT = 1 };
struct GCHeader {
    union {
        int gcData[2];
        GCHeader* post_gcAddress;
    };
};