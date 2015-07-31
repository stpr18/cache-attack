#pragma once
#include <cstdint>
#include <array>
#include <initializer_list>
#include "Memory.h"

class CryptoProgram
{
private:
	static const unsigned int LOG;
	static const size_t TABLE_SIZE = 256;
	static const int ROUND = 10;
	//static const int ACCESS_TIMES = 40;
	
public:
	static const size_t KEY_SIZE = 4;
	static const size_t ROUND_KEY_SIZE = 44;

	using table_t = Memory < uint32_t, TABLE_SIZE >;
	using key_t = std::array < uint32_t, KEY_SIZE >;
	using rkey_t = std::array < uint32_t, ROUND_KEY_SIZE >;
	using text_t = std::array < uint32_t, 4 >;

private:
	rkey_t rkey_;
public:
	static const uint32_t rcon[];
	static const uint8_t sbox[];
	static table_t table[4];

	CryptoProgram();
	CryptoProgram(std::initializer_list<uint32_t> list);
	~CryptoProgram();

	void random_key();
	void key_expansion(const key_t &key);
	void encrypt(const text_t &in, text_t &out) const;

	static void flush_tables();
};
