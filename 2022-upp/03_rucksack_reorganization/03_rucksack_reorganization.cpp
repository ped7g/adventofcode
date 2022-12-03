#include <Core/Core.h>

using namespace Upp;

// concept of part1/part2 task having init/line(line)/finish interface
template <typename T>
concept TaskProcessingLines = requires(T task, const String & line) {
	task.init();
	{ task.line(line) } -> std::same_as<bool>;	// return true finished early
	task.finish();
};

// main loop reading lines from input file feeding them to task
template <typename T>
requires TaskProcessingLines<T>
void lines_loop(T task, const String & filename) {
	FileIn in(GetDataFile(filename));
	if (!in) return;
	task.init();
	Cout() << " input filename: " << filename << EOL;
	while (!task.line(in.GetLine()) && !in.IsEof()) ;
	task.finish();
}

class Part1 {

	uint64 priority_sum { 0UL };

public:

	static uint64 symbol_types_in(const char *b, const char *e) {
		uint64 types = 0UL;
		while (b < e) {
			if ('A' <= *b && *b <= 'Z') types |= 1UL << ('Z' - *b);
			if ('a' <= *b && *b <= 'z') types |= 1UL << ('z' - *b + 26);
			++b;
		}
		return types;
	}
	
	static uint64 top_priority(const uint64 types) {
		uint64 bit_pos = 0, bit_mask = 1;
		while (0 == (types & bit_mask) && bit_pos < 52UL) {
			++bit_pos;
			bit_mask <<= 1;
		}
		return 52UL - bit_pos;
	}

	void init() { Cout() << "part1"; }

	void finish() { Cout() << "part1: " << priority_sum << EOL; }

	bool line(const String & line) {
		const int l = line.GetLength();
		if (0 == l || (l&1)) {
			Cout() << "bad:" << line << EOL;
			return true;		// wrong input, terminate task
		}
		auto middle = line.Begin() + l/2;
		uint64 types = symbol_types_in(line.Begin(), middle);
		types &= symbol_types_in(middle, line.End());		// types which are in both compartments
		if (0 == types) { Cout() << "no item in: " << line << EOL; return false; }	// should not happen
		priority_sum += top_priority(types);
		return false;			// not finished yet, try next line
	}
};

class Part2 {

	uint64 priority_sum { 0UL };
	uint64 badge_type { ~0UL };
	int rucksacks { 0 };

public:

	void init() { Cout() << "part2"; }

	void finish() { Cout() << "part2: " << priority_sum << EOL; }

	bool line(const String & line) {
		if (line.IsEmpty()) return true;		// wrong input, terminate task
		badge_type &= Part1::symbol_types_in(line.Begin(), line.End());
		++rucksacks;
		if (3 == rucksacks) {
			if (0 == badge_type) { Cout() << "no badge found, last rucksack: " << line << EOL; return false; }	// should not happen
			priority_sum += Part1::top_priority(badge_type);
			// reinit for next group
			badge_type = ~0UL;
			rucksacks = 0;
		}
		return false;							// not finished yet, try next line
	}
};

CONSOLE_APP_MAIN
{
	const Vector<String>& cmdline = CommandLine();
	for(int i = 0; i < cmdline.GetCount(); i++) {
		lines_loop(Part1(), cmdline[i]);
		lines_loop(Part2(), cmdline[i]);
	}
}
