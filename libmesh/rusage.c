#ifndef _WIN32
#error This is only intended for Windows.
#endif

#include <windows.h>
#include <psapi.h>

#include "rusage.h"

static void
usage_to_timeval(FILETIME *ft, struct timeval *tv)
{
	ULARGE_INTEGER time;
	time.LowPart = ft->dwLowDateTime;
	time.HighPart = ft->dwHighDateTime;

	tv->tv_sec = (long)(time.QuadPart / 10000000);
	tv->tv_usec = (time.QuadPart % 10000000) / 10;
}

int
getrusage(int who, struct rusage *usage)
{
	FILETIME creation_time, exit_time, kernel_time, user_time;
	PROCESS_MEMORY_COUNTERS pmc;

	memset(usage, 0, sizeof(struct rusage));

	if (who == RUSAGE_SELF) {
		if (!GetProcessTimes(GetCurrentProcess(), &creation_time, &exit_time,
			&kernel_time, &user_time)) {
			return -1;
		}

		if (!GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
			return -1;
		}

		usage_to_timeval(&kernel_time, &usage->ru_stime);
		usage_to_timeval(&user_time, &usage->ru_utime);
		usage->ru_majflt = pmc.PageFaultCount;
		usage->ru_maxrss =(long)(pmc.PeakWorkingSetSize / 1024);
		return 0;
	}
	else if (who == RUSAGE_THREAD) {
		if (!GetThreadTimes(GetCurrentThread(), &creation_time, &exit_time,
			&kernel_time, &user_time)) {
			return -1;
		}
		usage_to_timeval(&kernel_time, &usage->ru_stime);
		usage_to_timeval(&user_time, &usage->ru_utime);
		return 0;
	}
	else {
		return -1;
	}
}