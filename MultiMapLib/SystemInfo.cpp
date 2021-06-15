#include "MultiMap.h"
#include "SystemInfo.h"
#include "MFUtils.h"

DISABLE_WARNINGS
#if defined (__windows__)
#include <windows.h>
#include <malloc.h>
#include <iostream>

typedef BOOL (WINAPI *LPFN_GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

#else if defined ( __linux__ )
#include <unistd.h>
#endif
ENABLE_WARNINGS

MULTIMAP_API SystemInfo::~SystemInfo(void)
{
	if ( availableNodeMemory != NULL ) 
	{
		free(availableNodeMemory);
		availableNodeMemory = NULL;
	}
}

MULTIMAP_API SystemInfo::SystemInfo(void) {
	availableNodeMemory = NULL;
	Refresh();
}

/**
 * We need a assignment because we need one for Parameters 
**/
MULTIMAP_API SystemInfo& SystemInfo::operator=(const SystemInfo& other) {
	totalPhysicalMemory = other.totalPhysicalMemory;
	availablePhysicalMemory = other.availablePhysicalMemory;
	totalVirtualMemory = other.totalVirtualMemory;
	availableVirtualMemory = other.availableVirtualMemory;
	logicalProcessorCount = other.logicalProcessorCount;
	numaNodeCount = other.numaNodeCount;
	availableNodeMemory = NULL;
	if ( numaNodeCount > 0 ) {
		availableNodeMemory = (unsigned long long *)calloc(numaNodeCount,sizeof(unsigned long long));
		memcpy(availableNodeMemory,other.availableNodeMemory,numaNodeCount*sizeof(unsigned long long));
	}
	processorCoreCount = other.processorCoreCount;
	processorL1CacheCount = other.processorL1CacheCount;
	processorL2CacheCount = other.processorL2CacheCount;
	processorL3CacheCount = other.processorL3CacheCount;
	processorPackageCount = other.processorPackageCount;

	useMaxThreads = other.useMaxThreads;
	useMaxCores = other.useMaxCores;
	useHyperThread = other.useHyperThread;
	interleaveThreads = other.interleaveThreads;

	return *this;
}

#define HYPERTHREADED (logicalProcessorCount == processorCoreCount?false:true)
#define THREADS_PER_CORE (logicalProcessorCount/processorCoreCount)

MULTIMAP_API unsigned long SystemInfo::GetMaxThreads(void) {
	if ( useMaxThreads <= 0 ) {
		return logicalProcessorCount;
	} else {
	    return min(logicalProcessorCount,useMaxThreads);
	}
}

MULTIMAP_API unsigned long SystemInfo::GetMaxCores(void) {
	return processorCoreCount;
}

MULTIMAP_API bool SystemInfo::IsHyperthreaded(void) {
	return HYPERTHREADED;
}

#if defined (__windows__)
// Helper function to count set bits in the processor mask.
MULTIMAP_API DWORD CountSetBits(ULONG_PTR bitMask) {
	DWORD LSHIFT = sizeof(ULONG_PTR)*8 - 1;
	DWORD bitSetCount = 0;
	ULONG_PTR bitTest = (ULONG_PTR)1 << LSHIFT;    
	DWORD i;

	for (i = 0; i <= LSHIFT; ++i) {
		bitSetCount += ((bitMask & bitTest)?1:0);
		bitTest/=2;
	}

	return bitSetCount;
}
#endif
MULTIMAP_API void SystemInfo::Refresh(void) {
	totalPhysicalMemory = 0;
	availablePhysicalMemory = 0;
	totalVirtualMemory = 0;
	availableVirtualMemory = 0;
	logicalProcessorCount = 0;
	numaNodeCount = 0;
	processorCoreCount = 0;
	processorL1CacheCount = 0;
	processorL2CacheCount = 0;
	processorL3CacheCount = 0;
	processorPackageCount = 0;

	if ( availableNodeMemory != NULL ) 
	{
		free(availableNodeMemory);
		availableNodeMemory = NULL;
	}

	useMaxThreads = 0;
	useMaxCores = 0;
	useHyperThread = true;
	interleaveThreads = false;

#if defined (__windows__) 
	LPFN_GLPI glpi;
	BOOL done = FALSE;
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = NULL;
	DWORD returnLength = 0;

	DWORD byteOffset = 0;
	PCACHE_DESCRIPTOR Cache;

	glpi = (LPFN_GLPI) GetProcAddress(GetModuleHandle(TEXT("kernel32")),"GetLogicalProcessorInformation");

	if (glpi != NULL) {
		while (!done) {
			DWORD rc = glpi(buffer, &returnLength);

			if (FALSE == rc) {
				if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
					if (buffer) {
						free(buffer);
						buffer = NULL;
					}
					buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(returnLength);

					if (NULL == buffer) {
						return;
					}
				} else {
					if ( NULL != buffer ) {
						free(buffer);
						buffer = NULL;
					}
					return;
				}
			} else {
				done = TRUE;
			}
		}

		ptr = buffer;

		while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength) 	{
			switch (ptr->Relationship) 	{

			case RelationNumaNode:
				// Non-NUMA systems report a single record of this type.
				numaNodeCount++;
				break;

			case RelationProcessorCore:
				processorCoreCount++;

				// A hyperthreaded core supplies more than one logical processor.
				logicalProcessorCount += CountSetBits(ptr->ProcessorMask);
				break;

			case RelationCache:
				// Cache data is in ptr->Cache, one CACHE_DESCRIPTOR structure for each cache. 
				Cache = &ptr->Cache;
				if (Cache->Level == 1) {
					processorL1CacheCount++;
				} else if (Cache->Level == 2) {
					processorL2CacheCount++;
				} else if (Cache->Level == 3) {
					processorL3CacheCount++;
				}
				break;

			case RelationProcessorPackage:
				// Logical processors share a physical package.
				processorPackageCount++;
				break;

			default:
				break;
			}
			byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
			ptr++;
		}

		if ( NULL != buffer ) {
			free(buffer);
		}
	}
#else if defined (__linux__ ) 
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
	totalPhysicalMemory = pages * pages_size;

	availablePhysicalMemory = 0;
	totalVirtualMemory = 0;
	availableVirtualMemory = 0;
	logicalProcessorCount = 0;
	numaNodeCount = 0;
	processorCoreCount = 0;
	processorL1CacheCount = 0;
	processorL2CacheCount = 0;
	processorL3CacheCount = 0;
	processorPackageCount = 0;
#endif
	RefreshAvailableMemoryStatus();
}

MULTIMAP_API void SystemInfo::RefreshAvailableMemoryStatus() {
	MEMORYSTATUS memoryStatus;
	GlobalMemoryStatus(&memoryStatus);
	totalPhysicalMemory = memoryStatus.dwTotalPhys;
	availablePhysicalMemory = memoryStatus.dwAvailPhys;
	totalVirtualMemory = memoryStatus.dwTotalVirtual;
	availableVirtualMemory = memoryStatus.dwAvailVirtual;

	if ( availableNodeMemory == NULL ) {
		availableNodeMemory = (unsigned long long *)calloc(numaNodeCount,sizeof(unsigned long long));
	}

	for ( UCHAR node=0; node<numaNodeCount; node++ ) {
		GetNumaAvailableMemoryNode(node,&availableNodeMemory[node]);
	}
}

MULTIMAP_API std::string SystemInfo::ToString() {
	char buffer[1024];
	memset(buffer,0,1024);
	double totalPhysical = static_cast<double>(totalPhysicalMemory) / MMM_GIGABYTE;
	double availablePhysical = static_cast<double>(availablePhysicalMemory) / MMM_GIGABYTE;
	double totalVirtual = static_cast<double>(totalVirtualMemory) / MMM_GIGABYTE;
	double availableVirtual = static_cast<double>(availableVirtualMemory) / MMM_GIGABYTE;

	sprintf(buffer,"Physical Memory %.2f of %.2f GB  VM %.2f of %.2f GB Cores %d Threads %d",
		availablePhysical,totalPhysical,availableVirtual,totalVirtual,processorCoreCount,logicalProcessorCount);

	return std::string(buffer);
}

STATIC MULTIMAP_API void SystemInfo::MonitorIncreasingAvailableMemory(int max, unsigned int seconds, FILE* pFile) {
	SystemInfo systemInfo = SystemInfo();
	unsigned long long previousAvailablePhysicalMemory;
	int iterations = 0;
	MFUtils mfUtils;
	do {
		previousAvailablePhysicalMemory = systemInfo.availablePhysicalMemory;
		mfUtils.Sleep(seconds*1000);
		systemInfo.Refresh();
		double amem = static_cast<double>(systemInfo.availablePhysicalMemory) / MMM_GIGABYTE;
	    fprintf(pFile,"Increasing available memory %.2f GB\n",amem);
		iterations++;
	} while ( systemInfo.availablePhysicalMemory > previousAvailablePhysicalMemory && iterations <= max );
}

STATIC MULTIMAP_API unsigned long long  SystemInfo::WaitForIncreasingAvailableMemory(SystemInfo & systemInfo, unsigned long long target, int max, unsigned int seconds) {
	systemInfo.RefreshAvailableMemoryStatus();

	if ( systemInfo.availablePhysicalMemory < target ) {
	    unsigned long long previousAvailablePhysicalMemory;
	    int iterations = 0;
		int noDecrease = 0;
		double tmem = static_cast<double>(target) / MMM_GIGABYTE;
		int milliseconds = seconds*1000;
		MFUtils mfUtils;
		do {
			previousAvailablePhysicalMemory = systemInfo.availablePhysicalMemory;
			double amem = static_cast<double>(previousAvailablePhysicalMemory) / MMM_GIGABYTE;
			if ( iterations%10 == 0 ) {
			    //logger->Log(INFO,"Waiting for available physical memory (%.2f) to reach %.2f GB",amem,tmem);
			}
			mfUtils.Sleep(milliseconds);
			systemInfo.RefreshAvailableMemoryStatus();
			if ( systemInfo.availablePhysicalMemory <= previousAvailablePhysicalMemory ) {
				noDecrease++;
			} else {
				noDecrease = 0;
			}
			iterations++;
		} while ( noDecrease < 10 &&                               // available memory increasing
				  systemInfo.availablePhysicalMemory < target &&  // target not yet met
				  iterations <= max );                            // max iterations not yet met
    }
	return systemInfo.availablePhysicalMemory;
}