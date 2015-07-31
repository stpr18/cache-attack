#include <ios>
#include "Log.h"

using namespace Log;

Logger Log::out;

Logger::Logger()
{
}


Logger::~Logger()
{
	if (fout_.is_open())
		fout_.close();
}

void Logger::open(const char* path)
{
	if (fout_.is_open())
		fout_.close();

	fout_.open(path, std::ios::out | std::ios::binary);
	if (!fout_) {
		throw std::ios_base::failure("file cannot open");
	}
}

//void Logger::read_mode()
//{
//	if (fout_.is_open())
//		fout_.close();
//
//	fout_.open(LOG_PATH, std::ios::in | std::ios::binary);
//	if (!fout_) {
//		throw std::ios_base::failure("file cannot open");
//	}
//}

void Logger::close()
{
	if (fout_.is_open())
		fout_.close();
}

void Logger::write(uint32_t value)
{
	fout_.write(reinterpret_cast<char*>(&value), sizeof(value));
}

//void Logger::read(uint32_t &value)
//{
//	fout_.read(reinterpret_cast<char*>(&value), sizeof(value));
//}
//
//uint32_t Logger::read()
//{
//	uint32_t value;
//	read(value);
//	return value;
//}
