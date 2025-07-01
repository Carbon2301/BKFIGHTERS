#include "stdafx.h"
#include "MemoryManager.h"

//new section
void * operator new(size_t noBytes)
{
	return MemoryManager::GetInstance()->Alloc(noBytes, __FILE__, __LINE__);
}


void * operator new[](size_t noBytes)
{
	return MemoryManager::GetInstance()->Alloc(noBytes, __FILE__, __LINE__);
}


void operator delete (void * p)
{
	return MemoryManager::GetInstance()->Free(p);
}

void operator delete[] (void * p)
{
	return MemoryManager::GetInstance()->Free(p);
}

void * operator new(size_t noBytes, char * fileName, unsigned long line)
{
	return MemoryManager::GetInstance()->Alloc(noBytes, fileName, line);
}


void * operator new[](size_t noBytes, char * fileName, unsigned long line)
{
	return MemoryManager::GetInstance()->Alloc(noBytes, fileName, line);
}


void operator delete (void * p, char * fileName, unsigned long line)
{
	return MemoryManager::GetInstance()->Free(p);
}


void operator delete[] (void * p, char * fileName, unsigned long line)
{
	return MemoryManager::GetInstance()->Free(p);
}

//malloc section
void * CustomAlloc(size_t noBytes)
{
	return MemoryManager::GetInstance()->Alloc(noBytes, __FILE__, __LINE__);
}

void * CustomAllocDbg(size_t noBytes, char * fileName, unsigned long line)
{
	return MemoryManager::GetInstance()->Alloc(noBytes, fileName, line);
}

void CustomFree(void * p)
{
	return MemoryManager::GetInstance()->Free(p);
}

//memory dump section
void MemoryDump()
{
	MemoryManager::GetInstance()->Dump();
}

//sanity check
void SanityCheck()
{
	MemoryManager::GetInstance()->SanityCheck(true);
}