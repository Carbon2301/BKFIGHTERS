#include "stdafx.h"
#include "MemoryManager.h"
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>

const unsigned long CHECK_CODE = 0X12345678;


MemoryManager MemoryManager::ms_Instance;

MemoryManager::MemoryManager()
{
	m_noBuffers = 0;
}

MemoryManager::~MemoryManager()
{
}

void * MemoryManager::Alloc(unsigned int noBytes, char * fileName, unsigned int line)
{
	#ifdef _DEBUG
		SanityCheck();
	#endif

	m_dataBuffers[m_noBuffers].pAddress = malloc(noBytes + 4);
	m_dataBuffers[m_noBuffers].fileName = fileName;
	m_dataBuffers[m_noBuffers].line = line;
	m_dataBuffers[m_noBuffers].length = noBytes;

	char * temp = (char *) m_dataBuffers[m_noBuffers].pAddress;
	temp += noBytes;
	memcpy(temp, &CHECK_CODE, 4);

	return m_dataBuffers[m_noBuffers++].pAddress;
}

void MemoryManager::Free(void * pAddress)
{
	#ifdef _DEBUG
	//memory overrun checking
		SanityCheck();
	// deletion of Null pointer
		if(pAddress == NULL)
		{
			Error("Null pointer deletion\n");
			return;
		}
	#endif

	for(unsigned int i = 0; i < m_noBuffers; i++)
	{
		if (m_dataBuffers[i].pAddress == pAddress)
		{
			free(pAddress);
			if (i != m_noBuffers - 1)
			{
				memcpy(m_dataBuffers + i, m_dataBuffers + m_noBuffers - 1, sizeof(AllocBuffers));
			}
			m_noBuffers--;
			return;
		}
	}

	// bad pointer (already deleted) deletion

	#ifdef _DEBUG
		Error("Bad pointer deletion\n");
	#endif
}

void MemoryManager::Dump()
{
	unsigned long noTotalBytes = 0;

	esLogMessage("Starting memory dump\n");

	for(unsigned int i = 0; i < m_noBuffers; i++)
	{
		esLogMessage("%4d. 0x%08X: %d bytes(%s: %d)\n", i, (unsigned long)m_dataBuffers[i].pAddress, m_dataBuffers[i].length, 
			m_dataBuffers[i].fileName, m_dataBuffers[i].line);
		noTotalBytes += m_dataBuffers[i].length;
	}
	esLogMessage("---------------------------\n");
	esLogMessage("Total: %d buffers, %d bytes\n", m_noBuffers, noTotalBytes);
}

void MemoryManager::SanityCheck(bool bShowStats)
{
	if (bShowStats)
		esLogMessage("Sanity check start...\n");
	
	int count = 0;
	for(unsigned int i = 0; i < m_noBuffers; i++)
	{
		char * temp = (char *) m_dataBuffers[i].pAddress;
		temp += m_dataBuffers[i].length;
		if(memcmp(temp, &CHECK_CODE, 4) != 0)
		{
			esLogMessage("memory corruption at 0x%08X: %d bytes(%s: %d)\n", (unsigned long)m_dataBuffers[i].pAddress, m_dataBuffers[i].length, 
				m_dataBuffers[i].fileName, m_dataBuffers[i].line);
			count++;
		}
	}

	if (bShowStats || count > 0)
	{
		esLogMessage("---------------------------\n");
		esLogMessage("Total: %d corrupted buffers\n", count);
	}

	if (count > 0)
	{
		Error("Memory corruption detected");
	}	
}

void MemoryManager::Error(char * szMessage)
{
	esLogMessage(szMessage);
	switch (MessageBoxA(NULL,szMessage,"Memory Error", MB_ABORTRETRYIGNORE | MB_ICONERROR))
	{
	case IDABORT:
		exit(-1);
	case IDIGNORE:
		return;
	}
	throw;
}