#pragma once
#include <cstdlib>
#include <cstdint>
#include <array>
#include <vector>
#ifdef _WIN32
#include <intrin.h>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <x86intrin.h>
#include <unistd.h>
#include <cpuid.h>
#endif

#ifndef CACHE_LINE_BYTES
#define CACHE_LINE_BYTES 64
#else
#error already defined CACHE_LINE_BYTES
#endif

namespace util {
	inline void pause()
	{
#ifdef _WIN32
		std::system("pause");
#endif
	}

	inline uint64_t rdtsc()
	{
		uint64_t ret;
		_mm_mfence();
		ret = __rdtsc();
		_mm_mfence();
		return ret;
	}

	inline unsigned char maccess(void *addr)
	{
		volatile unsigned char r = *reinterpret_cast<unsigned char*>(addr);
		return r;
	}

	inline void clflush(void *addr)
	{
		_mm_clflush(addr);
	}

	inline void switch_to_thread()
	{
#ifdef _WIN32
		SwitchToThread();
#elif defined(_POSIX_PRIORITY_SCHEDULING)
		sched_yield();
#endif
	}

#ifdef _WIN32
	using CpuIdInt = int;
#else
	using CpuIdInt = unsigned int;
#endif

	inline void call_cpuid(std::array<CpuIdInt, 4> &cpui, int id)
	{
#ifdef _WIN32
		__cpuid(cpui.data(), id);
#else
		__cpuid(id, cpui[0], cpui[1], cpui[2], cpui[3]);
#endif
	}

//	inline void call_cpuidex(std::array<CpuIdInt, 4> &cpui, int id, int sub)
//	{
//#ifdef _WIN32
//		__cpuidex(cpui.data(), id, sub);
//#else
//		__cpuid_count(id, sub, cpui[0], cpui[1], cpui[2], cpui[3]);
//#endif
//	}
}
