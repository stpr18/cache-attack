#include <iostream>
#include <cstdlib>
#include <array>
#include <iomanip>
#include <openssl/aes.h>

int main(int argc, char *argv[])
{
	std::array<unsigned char, 16> secret_key = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};

	std::array<unsigned char, 16> plain_text = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	};
	std::array<unsigned char, 16> cipher_text;

	AES_KEY key;
	AES_set_encrypt_key(secret_key.data(), 128, &key);

	AES_encrypt(plain_text.data(), cipher_text.data(), &key);

	std::cout << std::hex;
	for (auto x : cipher_text)
		std::cout << std::setfill('0') << std::setw(2) << (int) x << " ";
	std::cout << std::endl;

	std::system("pause");

	return EXIT_SUCCESS;
}
