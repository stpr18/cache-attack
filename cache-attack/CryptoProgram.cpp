#include <algorithm>
#include <iterator>
#include <iostream>
#include <iomanip>
#include "CryptoProgram.h"
#include "Random.h"
#include "Log.h"
#include "util.h"

const unsigned int CryptoProgram::LOG = 1;

CryptoProgram::CryptoProgram()
{
}

CryptoProgram::CryptoProgram(std::initializer_list<uint32_t> list)
{
	//constexpr?
	key_t key = {};
	std::copy(list.begin(), list.end(), key.begin());

	key_expansion(key);
}

CryptoProgram::~CryptoProgram()
{
}

void CryptoProgram::encrypt(const text_t &in, text_t &out) const
{
	size_t i, j;
	text_t state;
	
	//if (LOG) {
	//	Log::out << in;
	//}

	for (i = 0; i < 4; ++i)
		out[i] = in[i] ^ rkey_[i];

	for (i = 1; i <= (10 - 1); ++i) {
		//if (LOG) {
		//	Log::out << out;
		//}

		for (j = 0; j < 4; ++j) {
			state[j] =
				table[0][util::int32toq0(out[(j + 0) % 4])] ^
				table[1][util::int32toq1(out[(j + 1) % 4])] ^
				table[2][util::int32toq2(out[(j + 2) % 4])] ^
				table[3][util::int32toq3(out[(j + 3) % 4])] ^
				rkey_[4 * i + j];

			//if (LOG) {
			//	Log::out << (
			//		(out[(j + 0) % 4] & 0xff000000) ^
			//		(out[(j + 1) % 4] & 0x00ff0000) ^
			//		(out[(j + 2) % 4] & 0x0000ff00) ^
			//		(out[(j + 3) % 4] & 0x000000ff));
			//}
		}
		out = state;
	}

	//if (LOG) {
	//	Log::out << out;
	//}

	//for (i = 0; i < 4; ++i)
	//	table[i].reset_cache_info(0);

	//flush_tables();

	for (j = 0; j < 4; ++j) {
		state[j] =
			(table[2][util::int32toq0(out[(j + 0) % 4])] & 0xff000000) ^
			(table[3][util::int32toq1(out[(j + 1) % 4])] & 0x00ff0000) ^
			(table[0][util::int32toq2(out[(j + 2) % 4])] & 0x0000ff00) ^
			(table[1][util::int32toq3(out[(j + 3) % 4])] & 0x000000ff) ^
			rkey_[4 * 10 + j];
		if (LOG >= 2) {
			Log::out << (
				((out[(j + 2) % 4] & 0x0000ff00) << 16) ^
				((out[(j + 3) % 4] & 0x000000ff) << 16) ^
				((out[(j + 0) % 4] & 0xff000000) >> 16) ^
				((out[(j + 1) % 4] & 0x00ff0000) >> 16));
		}
	}
	out = state;

	if (LOG >= 2) {
		Log::out << out;
	}
}

void CryptoProgram::random_key()
{
	key_t key;
	for (auto &k : key)
		k = Random::next_u32();

	key_expansion(key);
}

void CryptoProgram::key_expansion(const key_t &key)
{
	size_t i;

	for (i = 0; i < KEY_SIZE; ++i)
		rkey_[i] = key[i];

	uint32_t temp;
	for (; i < ROUND_KEY_SIZE; ++i) {
		temp = rkey_[i - 1];
		if (i % 4 == 0) {
			temp =
				(table[2][util::int32toq1(temp)] & 0xff000000) ^
				(table[3][util::int32toq2(temp)] & 0x00ff0000) ^
				(table[0][util::int32toq3(temp)] & 0x0000ff00) ^
				(table[1][util::int32toq0(temp)] & 0x000000ff) ^
				rcon[i / 4];
		}
		rkey_[i] = rkey_[i - 4] ^ temp;
	}

	if (LOG >= 1) {
		Log::out << LOG << ROUND_KEY_SIZE << table_t::CACHE_LINE_SIZE << 0;
		//Log::out << key;
		Log::out << rkey_;
	}
}

void CryptoProgram::flush_tables()
{
	for (auto &t : table)
		t.flush();
}
