#pragma once
#include <vector>

namespace util {
	class CpuID
	{
	public:
		CpuID();
		~CpuID();

		unsigned int cache_line_size = 0;
		bool sse2 = false;
		bool aesni = false;

		//struct CacheInfo {
		//	int level;
		//	int type;
		//	int way;
		//	int line_size;
		//};

		//std::vector<CacheInfo> cache;

		static CpuID getInstance()
		{
			return CpuID();
		}
	};
}
