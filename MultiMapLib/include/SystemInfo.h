#pragma once
#include "MultiMap.h"

DISABLE_WARNINGS
#include <stdio.h>
ENABLE_WARNINGS

class SystemInfo
{
public:
	MULTIMAP_API ~SystemInfo(void);
	MULTIMAP_API SystemInfo(void);
    MULTIMAP_API SystemInfo& operator=(const SystemInfo& other);
	
	MULTIMAP_API void Refresh(void);
	MULTIMAP_API unsigned long GetMaxThreads(void);
	MULTIMAP_API unsigned long GetMaxCores(void);
	MULTIMAP_API bool IsHyperthreaded(void);
	MULTIMAP_API void RefreshAvailableMemoryStatus();
	MULTIMAP_API std::string ToString(void);

	static MULTIMAP_API void MonitorIncreasingAvailableMemory(int max = 18, unsigned int seconds = 10, FILE* pFile = stdout);
    static MULTIMAP_API unsigned long long WaitForIncreasingAvailableMemory(SystemInfo & systemInfo, unsigned long long target, int max = 180, unsigned int seconds = 1);

	unsigned long long totalPhysicalMemory; 
	unsigned long long availablePhysicalMemory;
	unsigned long long totalVirtualMemory; // includes page file memory
	unsigned long long availableVirtualMemory; // includes page file memory
	unsigned long long * availableNodeMemory;
    unsigned long numaNodeCount; // See note on NUMA Concepts below
    unsigned long processorCoreCount; // cores
	unsigned long logicalProcessorCount; // threads
	unsigned long processorL1CacheCount;
    unsigned long processorL2CacheCount;
    unsigned long processorL3CacheCount;
    unsigned long processorPackageCount;
	unsigned long useMaxThreads; // <= logicalProcessorCount
	unsigned long useMaxCores; // <= processorCoreCount
	bool useHyperThread; // if true max threads = logicalProcessorCount
	bool interleaveThreads; // if true distribute threads across all cores before sharing a core

private:

};

