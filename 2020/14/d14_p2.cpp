// I had nasty non-obvious bug in sjasmplus script initially, so I had to
// produce C++ solution first, allowing me to produce correct results for
// any input, and then bisect the original input until I localized the bug.
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>

int main()
{
	std::ifstream input("input.txt");
	std::string temp, mask;
	uint64_t memAdr;
	uint32_t memValue;
	int lines = 0;
	std::unordered_map<uint64_t, uint32_t> memory;
	while (!input.eof()) {
		if ('m' != input.get()) break;
		++lines;
		switch (input.get()) {
		case 'a':	// mask
			std::getline(input, temp, '=');
			if (' ' == input.peek()) input.get();
			std::getline(input, mask);
//			std::cout << "mask [" << mask << "]\n";
			if (36 != mask.size()) {
				std::cout << "Invalid mask!\n";
				exit(1);
			}
			break;
		case 'e':	// mem
		{
			std::getline(input, temp, '[');
			input >> memAdr;
			std::getline(input, temp, '=');
			input >> memValue;
			std::getline(input, temp);
			// use mask to generate masked addresses
			uint64_t bitMask = (1UL<<35), floatBits = 0UL, lastFloatBit = 0UL, maskedAdr = memAdr;
			for (const auto mChar : mask) {
				if ('1' == mChar) maskedAdr |= bitMask;
				if ('X' == mChar) floatBits |= bitMask, lastFloatBit = bitMask;
				bitMask >>= 1;
			}
			maskedAdr &= ~floatBits;	// base address (lowest)
			do {
//				std::cout << ((memory.cend() != memory.find(maskedAdr)) ? "* " : "  ");
//				std::cout << "m 0x" << std::hex << maskedAdr << " (" << std::dec << memAdr << ") = " << memValue << "\n";
				memory[maskedAdr] = memValue;
				// generate next combination in float bits
				bitMask = lastFloatBit;
				maskedAdr ^= bitMask;
				while (bitMask && !(maskedAdr & bitMask)) {
					do { bitMask <<= 1; } while (bitMask && !(bitMask & floatBits));
					maskedAdr ^= bitMask;
				}
			} while (bitMask);			// until combinations exhausted completely
			break;
		}
		default:	// invalid input file
			input.close();
		}
	}
	std::cout << "Lines parsed: " << lines << " | Size of map: " << memory.size() << "\n";
	uint64_t sum = 0UL;
	for (const auto& value : memory) sum += value.second;
	std::cout << "Sum: " << sum << " == 0x" << std::hex << sum << "\n";
}
