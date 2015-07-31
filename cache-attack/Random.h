#pragma once
#include <random>
#include <cstdint>

class Random
{
public:
	using engine_t = std::mt19937;

	Random() = delete;
	~Random() = delete;

	static engine_t& get_engine();
	static uint8_t next_u8();
	static uint32_t next_u32();
};
