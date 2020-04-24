#pragma once
#include "GCHeader.h"

class TestDisposedObject
{
public:
	GCHeader header;
	static const int ref_count = 1;
	TestDisposedObject* other;
};

