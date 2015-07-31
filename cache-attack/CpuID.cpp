#include "CpuID.h"
#include "builtin.h"

using namespace util;

CpuID::CpuID()
{
	std::array<CpuIdInt, 4> cpui = {};
	CpuIdInt ids;

	call_cpuid(cpui, 0x00000000);
	ids = cpui[0];

	if (ids >= 0x00000001) {
		call_cpuid(cpui, 0x00000001);
		sse2 = (cpui[3] & 0x4000000) != 0;
		aesni = (cpui[2] & 0x00200000) != 0;
	}

	//if (ids >= 0x00000004) {
	//	//ECX = max
	//	for (int i = 0; i < 256; ++i) {
	//		call_cpuidex(cpui, 0x00000004, i);

	//		unsigned int cache_type = cpui[0] & 0x1F;
	//		if (cache_type == 0)
	//			break;
	//		if (cache_type != 1 && cache_type != 3)
	//			continue;
	//		
	//		unsigned int cache_level = (cpui[0] >> 5) & 0x3;
	//		if (cache_level > CACHE_MAX)
	//			continue;
	//	}
	//}

	call_cpuid(cpui, 0x80000000);
	ids = cpui[0];

	if (ids >= static_cast<CpuIdInt>(0x80000006)) {
		call_cpuid(cpui, 0x80000006);
		cache_line_size = cpui[2] & 0xff;
	}
}


CpuID::~CpuID()
{
}
