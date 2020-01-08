// ResLimitsOnWin.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h>
#include <iostream>

LPCWSTR stringToLPCWSTR(std::string orig)
{
    size_t origsize = orig.length() + 1;
    const size_t newsize = 100;
    size_t convertedChars = 0;
    wchar_t *wcstring = (wchar_t *)malloc(sizeof(wchar_t)*(orig.length() - 1));
    mbstowcs_s(&convertedChars, wcstring, origsize, orig.c_str(), _TRUNCATE);

    return wcstring;
}


int setMemoryLimits(HANDLE job)
{
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION extendLimit;
    memset(&extendLimit, 0, sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION));
    extendLimit.BasicLimitInformation.LimitFlags = 0;

    unsigned long phyMem = 2048;
    unsigned long virt = 1024;

    // phyMem, only limit working set size for each process ! 
    // 注意：只支持对job内每一个进程的最大最小物理内存进行限制，不支持对job内所有进程的物理内存之和进行限制
    if (phyMem > 0) {
        extendLimit.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_WORKINGSET;
        extendLimit.BasicLimitInformation.MinimumWorkingSetSize = 1 * 1024 * 1024;
        extendLimit.BasicLimitInformation.MaximumWorkingSetSize = phyMem * 1024;
    }
    

    // virtMem, support virtual memory of all processes
    // 支持对job内所有的进程的虚拟内存之和进行限制
    if (virt > 0) {
        extendLimit.BasicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_JOB_MEMORY;
        extendLimit.JobMemoryLimit = virt * 1024;
    }


    int success = SetInformationJobObject(job, JobObjectExtendedLimitInformation, &extendLimit, sizeof(extendLimit));
    if (0 == success) {
        std::cout << GetLastError() << std::endl;
    }

    return 0;
}

int queryMemoryLimits(HANDLE job)
{
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION lpJobObjectInfo;
    if (::QueryInformationJobObject(job, JobObjectExtendedLimitInformation, &lpJobObjectInfo, sizeof(JOBOBJECT_EXTENDED_LIMIT_INFORMATION), NULL)) {
        if (lpJobObjectInfo.JobMemoryLimit != 0) {
            std::cout << " VirtualMemoryLimit(JobMemoryLimit) size = " << lpJobObjectInfo.JobMemoryLimit / (double)1024 / (double)1024 << " M" << std::endl;
        } else {
            std::cout << " VirtualMemoryLimit is not configured." << std::endl;
        }
    }

    JOBOBJECT_BASIC_LIMIT_INFORMATION lpJobObjectInfo1;
    if (::QueryInformationJobObject(job, JobObjectBasicLimitInformation, &lpJobObjectInfo1, sizeof(JOBOBJECT_BASIC_LIMIT_INFORMATION), NULL)) {
        if (lpJobObjectInfo1.MaximumWorkingSetSize != 0) {
            std::cout << " PhysicalMemoryLimit(MaximumWorkingSetSize) size = " << lpJobObjectInfo1.MaximumWorkingSetSize / (double)1024 / (double)1024 << " M" << std::endl;
        } else {
            std::cout << " PhysicalMemoryLimit is not configured." << std::endl;
        }
    }

    return 0;
}

int main()
{
    std::string jobName = "job1";

    HANDLE job1 = CreateJobObject(NULL, stringToLPCWSTR(jobName));

    setMemoryLimits(job1);

    queryMemoryLimits(job1);

    return 0;
}

