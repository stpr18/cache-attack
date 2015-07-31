#include "Random.h"

Random::engine_t& Random::get_engine()
{
	static std::random_device rd;
	static engine_t engine(rd());
	return engine;
}

uint8_t Random::next_u8()
{
	static std::uniform_int_distribution<> dist(0, std::numeric_limits<uint8_t>::max());
	return dist(get_engine());
}

uint32_t Random::next_u32()
{
	static std::uniform_int_distribution<uint32_t> dist;
	return dist(get_engine());
}
