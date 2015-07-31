#include <iostream>
#include <algorithm>
#include <iomanip>
#include <numeric>
#include <memory>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#include "test.h"
#include "Memory.h"
#include "Random.h"
#include "CryptoProgram.h"
#include "CpuID.h"

using namespace test;

bool test::check_cpu()
{
	const util::CpuID cpu_id = util::CpuID::getInstance();

	if (!cpu_id.sse2) {
		std::cout << "SSE2 unsupported" << std::endl;
		return false;
	}

	if (!cpu_id.aesni) {
		std::cout << "AES-NI unsupported" << std::endl;
		return false;
	}

	if (cpu_id.cache_line_size != CACHE_LINE_BYTES) {
		std::cout << "Cache line size is wrong (" << cpu_id.cache_line_size << ")" << std::endl;
		return false;
	}
	
	return true;
}

void test::prob_check(unsigned int text_max)
{
	std::cout << "--- Prob check ---" << std::endl;

	using array_t = Memory < uint32_t, 256 > ;
	//std::unique_ptr<array_t> memory(new array_t);
	static array_t memory; // for align
	size_t cached[16] = { 0 };

	for (unsigned int i = 0; i < text_max; ++i) {
		memory.flush();

		for (unsigned int j = 0; j < 40; ++j)
			memory[Random::next_u8()];

		++cached[memory.count_cache() - 1];
	}

	for (unsigned int i = 0; i < array_t::CACHE_SIZE; ++i)
		std::cout << (i + 1) << ":" << (static_cast<double>(cached[i]) / static_cast<double>(text_max)) << std::endl;
}

bool test::test_vector()
{
	std::cout << "--- Test vector ---" << std::endl;

	CryptoProgram crypto = { 0x2b7e1516, 0x28aed2a6, 0xabf71588, 0x09cf4f3c };
	CryptoProgram::text_t plain = { 0x3243f6a8, 0x885a308d, 0x313198a2, 0xe0370734 };
	CryptoProgram::text_t cipher;
	crypto.encrypt(plain, cipher);
	if (cipher[0] == 0x3925841d && cipher[1] == 0x02dc09fb && cipher[2] == 0xdc118597 && cipher[3] == 0x196a0b32) {
		std::cout << "Succeed" << std::endl;
		return true;
	}
	
	std::cout << "Failed" << std::endl;

	//CryptoProgram::key_t crypto = { 0x2b7e1516, 0x28aed2a6, 0xabf71588, 0x09cf4f3c };
	//CryptoProgram::text_t plain = { 0x3243f6a8, 0x885a308d, 0x313198a2, 0xe0370734 };
	//CryptoProgram::text_t cipher;
	//CryptoProgram::ni_encrypt(crypto, plain, cipher);
	//std::cout << std::hex << cipher[0] << std::endl;
	//std::cout << std::hex << cipher[1] << std::endl;
	//std::cout << std::hex << cipher[2] << std::endl;
	//std::cout << std::hex << cipher[3] << std::endl;
	return false;
}

void test::setup_fr_attack(unsigned int text_max)
{
	static const size_t TEST_MAX = 1024;
	using array_t = memory::MemoryBuiltin < uint32_t, 4 * TEST_MAX >;
	//std::unique_ptr<array_t> array(new array_t);
	static array_t array; // for align

	using namespace boost::accumulators;
	accumulator_set<uint64_t, stats<tag::count, tag::max, tag::min, tag::mean> > hit_result_delta, miss_result_delta;
	accumulator_set<uint64_t, stats<tag::count, tag::max, tag::min, tag::mean> > wrong_miss_result_delta, wrong_hit_result_delta;

	array.fill(0xffffffff);
	std::cout << "--- Setup Flash+Reload attack ---" << std::endl;

	text_max *= 20;
	for (unsigned int i = 0; i < text_max; ++i) {
		size_t index = 0 * TEST_MAX;
		util::switch_to_thread();
		//array[index];
		util::maccess(array.data() + index);
		uint64_t t = array.reload_time(index);
		hit_result_delta(t);
		if (!array_t::is_cached_time(t))
			wrong_miss_result_delta(t);
	}

	for (unsigned int i = 0; i < text_max; ++i) {
		size_t index = 1 * TEST_MAX;
		util::switch_to_thread();
		//array.flush();
		util::clflush(array.data() + index);
		uint64_t t = array.reload_time(index);
		miss_result_delta(t);
		if (array_t::is_cached_time(t))
			wrong_hit_result_delta(t);
	}

	std::cout << "--- Hit:" << extract::count(hit_result_delta) << std::endl;
	std::cout << "Max:" << extract::max(hit_result_delta) << std::endl;
	std::cout << "Min:" << extract::min(hit_result_delta) << std::endl;
	std::cout << "Ave:" << extract::mean(hit_result_delta) << std::endl;

	std::cout << "--- Miss:" << extract::count(miss_result_delta) << std::endl;
	std::cout << "Max:" << extract::max(miss_result_delta) << std::endl;
	std::cout << "Min:" << extract::min(miss_result_delta) << std::endl;
	std::cout << "Ave:" << extract::mean(miss_result_delta) << std::endl;


	std::cout << "--- Hit-false:" << extract::count(wrong_miss_result_delta) << std::endl;
	std::cout << "Max:" << extract::max(wrong_miss_result_delta) << std::endl;
	std::cout << "Min:" << extract::min(wrong_miss_result_delta) << std::endl;
	std::cout << "Ave:" << extract::mean(wrong_miss_result_delta) << std::endl;

	std::cout << "--- Miss-false:" << extract::count(wrong_hit_result_delta) << std::endl;
	std::cout << "Max:" << extract::max(wrong_hit_result_delta) << std::endl;
	std::cout << "Min:" << extract::min(wrong_hit_result_delta) << std::endl;
	std::cout << "Ave:" << extract::mean(wrong_hit_result_delta) << std::endl;

	//unsigned int incorrect_count[2] = { 0, 0 };
	//for (unsigned int i = 0; i < text_max; ++i) {
	//	size_t index = Random::next_u8();

	//	array.check_start();
	//	array.flush();

	//	util::switch_to_thread();
	//	util::maccess(array.data() + index);
	//	
	//	bool is_cached = array.is_cached(array_t::cache_index(index));

	//	array.access_cache(index);
	//	bool is_cached_correct = array.is_cached_correct(array_t::cache_index(index));

	//	if (is_cached && !is_cached_correct)
	//		++incorrect_count[0];
	//	else if (!is_cached && is_cached_correct)
	//		++incorrect_count[1];
	//}
	//std::cout << "Time-miss : " << incorrect_count[0] << " + " << incorrect_count[1] << std::endl;
}
