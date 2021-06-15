#pragma once
/**
  * Copyright 2014 Mizar, LLC
  * All Rights Reserved.
  *
  * This file is part of Mizar's MultiMap software library.
  * MultiMap is licensed under the terms of the GNU Lesser General Public License
  * as published by the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version a copy of which is available at http://www.gnu.org/licenses/
  *
  * MultiMap is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU Lesser General Public License for more details.
  *
  * You may NOT remove this copyright notice; it must be retained in any modified 
  * version of the software.
  **/
#include "MultiMap.h"
#include <stdio.h>
#include <string>

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

