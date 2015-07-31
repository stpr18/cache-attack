#pragma once
#include <fstream>
#include <cstdint>
#include <array>

namespace Log
{
	class Logger
	{
	private:
		std::fstream fout_;

		void write(uint32_t value);
		//void read(uint32_t &value);
		//uint32_t read();

	public:
		Logger();
		~Logger();

		void open(const char* path);
		//void read_mode();
		void close();

		Logger& operator<<(uint32_t value)
		{
			write(value);
			return *this;
		}

		//Logger& operator>>(uint32_t &value)
		//{
		//	read(value);
		//	return *this;
		//}

		template <size_t SIZE>
		Logger& operator<<(const std::array<uint32_t, SIZE> &value)
		{
			for (auto v : value)
				write(v);
			return *this;
		}

		//template <size_t SIZE>
		//Logger& operator>>(std::array<uint32_t, SIZE> &value)
		//{
		//	for (auto &v : value)
		//		read(v);
		//	return *this;
		//}
	};

	extern Logger out;

	//void read_log();
}
