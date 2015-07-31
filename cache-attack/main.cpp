#include <iostream>
#include <cstdlib>
#include <exception>
#include <string>
#include <sstream>
#include "AttackProgram.h"
#include "Log.h"
#include "test.h"
#include "builtin.h"

class LogHelper
{
public:
	LogHelper(const char* log_path)
	{
		Log::out.open(log_path);
	}

	~LogHelper()
	{
		Log::out.close();
	}
};

void run(const char *log_path, unsigned int text_max)
{
	LogHelper log_helper(log_path);

	AttackProgram attacker(text_max);
	CryptoProgram crypto;

	crypto.random_key();

	attacker.attack(crypto);
	attacker.last_round_key_recover();
	attacker.secret_key_recover();
}

bool analyze_argv(const char* option, unsigned int &text_max, const char *&log_path)
{
	static const unsigned int DEFAULT_TEST_MAX = 0xffff;

	switch (option[0]) {
	case 't':
		if (!test::check_cpu()) {
			std::cout << "Unsupported CPU" << std::endl;
		}

		switch (option[1]) {
		case 'p':
			text_max = std::strtoul(option + 2, nullptr, 0);
			if (text_max == 0)
				text_max = DEFAULT_TEST_MAX;
			test::prob_check(text_max);
			break;
		case 'v':
			test::test_vector();
			break;
		case 'f':
			test::setup_fr_attack(text_max);
			break;
		default:
			test::prob_check(DEFAULT_TEST_MAX);
			test::test_vector();
			test::setup_fr_attack(DEFAULT_TEST_MAX);
			break;
		}

		break;
	case 'c':
		text_max = std::strtoul(option + 1, nullptr, 0);
		break;
	case 'o':
		if (option[1] != '\0')
			log_path = option + 1;
		break;
	case 'r':
		run(log_path, text_max);
		break;
	case 'e':
	case 'q':
		util::pause();
		return false;
	default:
		throw std::runtime_error("unexpected argument");
	}

	return true;
}

int main(int argc, char *argv[])
{
	try {
		unsigned int text_max = 0;
		const char *log_path = "log.txt";

		for (int i = 1; i < argc; ++i) {
			const char* option = argv[i];
			switch (option[0]) {
			case 's':
			{
				std::string sarg;
				std::string arg_val;
				while (std::cout << "Enter command : " << std::flush, std::getline(std::cin, sarg)) {
					if (sarg.empty())
						break;
					std::istringstream iss(sarg);
					while (iss >> arg_val)
						if (!analyze_argv(sarg.c_str(), text_max, log_path))
							return EXIT_SUCCESS;
				}
				break;
			}
			default:
				if (!analyze_argv(option, text_max, log_path))
					return EXIT_SUCCESS;
				break;
			}
		}

		run(log_path, text_max);
	}
	catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		util::pause();
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
