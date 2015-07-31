#pragma once
#include <array>
#include "CryptoProgram.h"

class CryptoProgram;

class AttackProgram
{
private:
	static const unsigned int LOG;
	static const unsigned int DEFAULT_TEXT_MAX;
	static const size_t COUNTER_PART_BYTE = 1;
	static const size_t COUNTER_PART = 16 / COUNTER_PART_BYTE;
	static const size_t COUNTER_PART_MAX = 1 << (8 * COUNTER_PART_BYTE);
	static const uint32_t COUNTER_THRESHOLD;

	const unsigned int text_max_;
	uint32_t reload_counter_[COUNTER_PART][COUNTER_PART_MAX][4];
	std::array<uint32_t, CryptoProgram::KEY_SIZE> recovered_last_round_key_;
	std::array<uint32_t, CryptoProgram::KEY_SIZE> recovered_secret_key_;

public:
	AttackProgram();
	AttackProgram(unsigned int text_max);
	~AttackProgram();

	void attack(const CryptoProgram &cryptoProg);
	void last_round_key_recover();
	void secret_key_recover();
};
