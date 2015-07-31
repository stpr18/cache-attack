#pragma once
#include <array>
#include <bitset>
#include <algorithm>
#include <cstdint>
#include <limits>
#include "builtin.h"

namespace memory {
	template <class T, size_t SIZE>
	class MemoryBase
	{
	public:
		//static const size_t CACHE_LINE_BYTES = 64;
		static const size_t CACHE_LINE_SIZE = CACHE_LINE_BYTES / sizeof(T);
		static const size_t CACHE_SIZE = SIZE / CACHE_LINE_SIZE;

		static_assert(CACHE_LINE_BYTES % sizeof(T) == 0, "wrong sizeof(T)");
		static_assert(SIZE % CACHE_LINE_SIZE == 0, "wrong SIZE");

	protected:
		//cache line size aligned
#ifdef _MSC_VER
		static const uint64_t CACHE_NONE = UINT64_MAX;
		__declspec(align(CACHE_LINE_BYTES)) std::array<T, SIZE> data_;
#else
		static const uint64_t CACHE_NONE = std::numeric_limits<uint64_t>::max();
		alignas(CACHE_LINE_BYTES) std::array<T, SIZE> data_;
#endif

	public:
		MemoryBase()
		{
		}

		MemoryBase(std::initializer_list<T> list)
		{
			std::copy(list.begin(), list.end(), data_.begin());
		}

		virtual ~MemoryBase()
		{
		}

		virtual T& operator[](size_t i)
		{
			return data_[i];
		}

		void fill(const T& value)
		{
			data_.fill(value);
		}

		T* data()
		{
			return data_.data();
		}

		virtual bool is_cached(size_t i) = 0;
		virtual void flush() = 0;
		virtual void flush(size_t i) = 0;
		virtual size_t count_cache() = 0;

		virtual void reset_cache_info(size_t) {}
		virtual bool is_cached_correct(size_t) { return false; }
		virtual uint64_t saved_access_time(size_t) { return CACHE_NONE; }

		static size_t cache_index(size_t i)
		{
			return i / CACHE_LINE_SIZE;
		}
	};

	template <class T, size_t SIZE>
	class MemoryImpl : public MemoryBase<T, SIZE>
	{
	private:
		using super_t = MemoryBase < T, SIZE > ;

		//using cache_t = std::array < unsigned int, super_t::CACHE_SIZE >;
		using cache_t = std::bitset < super_t::CACHE_SIZE >;
		cache_t is_cached_;
//#ifdef _MSC_VER
//		static const unsigned int NOT_CACHED = UINT_MAX;
//#else
//		static const unsigned int NOT_CACHED = std::numeric_limits<unsigned int>::max();
//#endif
//		static const unsigned int WAY = 16;
//		
//		cache_t is_cached_; //not initialized in constructor
//		unsigned int current_time_; //not initialized in constructor

	public:
		MemoryImpl() : super_t()
		{
		}

		MemoryImpl(std::initializer_list<T> list) : super_t(list)
		{
		}

		~MemoryImpl()
		{
		}
		
		T& operator[](size_t i) override
		{
			//unsigned int &i_is_cached = is_cached_[i / super_t::CACHE_LINE_SIZE];
			//if (i_is_cached == NOT_CACHED && count_cache() >= WAY) {
			//	*std::min_element(is_cached_.begin(), is_cached_.end()) = NOT_CACHED;
			//}
			//i_is_cached = current_time_++;
			is_cached_[i / super_t::CACHE_LINE_SIZE] = true;
			return super_t::operator[](i);
		}

		bool is_cached(size_t i) override
		{
			//return is_cached_[i] != NOT_CACHED;
			return is_cached_[i];
		}

		void flush() override
		{
			//current_time_ = 0;

			//is_cached_.fill(NOT_CACHED);
			is_cached_.reset();
		}

		void flush(size_t i) override
		{
			//current_time_ = 0;

			//is_cached_[i] = NOT_CACHED;
			is_cached_[i] = false;
		}

		size_t count_cache() override
		{
			//return std::count_if(is_cached_.begin(), is_cached_.end(), [](unsigned int i)->bool{return (i != NOT_CACHED); });
			return is_cached_.count();
		}
	};

	template <class T, size_t SIZE>
	class MemoryBuiltin : public MemoryBase < T, SIZE >
	{
	protected:
		static const uint64_t CACHE_MISS_THRESHOLD = 240;

		using super_t = MemoryBase < T, SIZE > ;
	public:
		MemoryBuiltin() : super_t()
		{
		}

		MemoryBuiltin(std::initializer_list<T> list) : super_t(list)
		{
		}

		virtual ~MemoryBuiltin()
		{
		}

		uint64_t reload_time(size_t i)
		{
			T *addr = super_t::data_.data() + i;
			uint64_t time = util::rdtsc();
			util::maccess(addr);
			return util::rdtsc() - time;
		}

		static bool is_cached_time(uint64_t time) {
			return time <= CACHE_MISS_THRESHOLD;
		}

		bool is_cached(size_t i) override
		{
			//util::switch_to_thread();
			uint64_t t = reload_time(i * super_t::CACHE_LINE_SIZE);
			return is_cached_time(t);
		}

		void flush() override
		{
			for (size_t i = 0; i < super_t::CACHE_SIZE; ++i)
				flush(i);
		}

		void flush(size_t i) override
		{
			T *addr = super_t::data_.data() + i * super_t::CACHE_LINE_SIZE;
			util::clflush(addr);
		}

		size_t count_cache() override
		{
			size_t count = 0;
			for (size_t i = 0; i < super_t::CACHE_SIZE; ++i)
				if (is_cached(i))
					++count;
			return count;
		}
	};

	template <class T, size_t SIZE>
	class MemoryBuiltinCheck : public MemoryBuiltin < T, SIZE >
	{
	private:
		using super_t = MemoryBuiltin < T, SIZE >;

		struct CacheInfo {
			uint64_t time;
			bool enabled;
		};
		using cache_t = std::array < CacheInfo, super_t::CACHE_SIZE >;
#ifdef _MSC_VER
		__declspec(align(CACHE_LINE_BYTES)) cache_t cache_info_;
#else
		alignas(CACHE_LINE_BYTES) cache_t cache_info_;
#endif
	public:
		MemoryBuiltinCheck() : super_t()
		{
		}

		MemoryBuiltinCheck(std::initializer_list<T> list) : super_t(list)
		{
		}

		~MemoryBuiltinCheck()
		{
		}

		bool is_cached(size_t i) override
		{
			uint64_t t = super_t::reload_time(i * super_t::CACHE_LINE_SIZE);
			cache_info_[i].time = t;
			return super_t::is_cached_time(t);
		}

		T& operator[](size_t i) override
		{
			cache_info_[i / super_t::CACHE_LINE_SIZE].enabled = true;
			return super_t::operator[](i);
		}

		void reset_cache_info(size_t i) override
		{
			cache_info_[i] = { super_t::CACHE_NONE, false };
		}

		bool is_cached_correct(size_t i) override
		{
			return cache_info_[i].enabled;
		}

		uint64_t saved_access_time(size_t i) override
		{
			return cache_info_[i].time;
		}
	};
}

template <class T, size_t SIZE>
using Memory = memory::MemoryBuiltin<T, SIZE>;
//using Memory = memory::MemoryImpl<T, SIZE>;
