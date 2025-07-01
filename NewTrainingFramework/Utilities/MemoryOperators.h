#pragma once

//new section
void * operator new(size_t noBytes);
void * operator new[](size_t noBytes);
void operator delete (void * p);
void operator delete[] (void * p);

void * operator new(size_t noBytes, char * fileName, unsigned long line);
void * operator new[](size_t noBytes, char * fileName, unsigned long line);
void operator delete (void * p, char * fileName, unsigned long line);
void operator delete[] (void * p, char * fileName, unsigned long line);

#ifdef _DEBUG
#define DEBUG_NEW new(__FILE__, __LINE__)
#else
#define DEBUG_NEW new
#endif
#define new DEBUG_NEW

//malloc section
void * CustomAlloc(size_t noBytes);
void * CustomAllocDbg(size_t noBytes, char * fileName, unsigned long line);
void CustomFree(void * p);

#ifdef _DEBUG
#define CustomAlloc(noBytes) CustomAllocDbg(noBytes, __FILE__, __LINE__)
#endif

//memory dump section
void MemoryDump();

//sanity check
void SanityCheck();