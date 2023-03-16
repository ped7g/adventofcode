#include <Core/Core.h>

using namespace Upp;

class Part1 {

	int nice_strings = 0;

public:

	void init() { Cout() << "- part1"; }

	bool line(const String & line) {
		int vowels = 0, twice = 0, disallowed = 0;
		for (int i = 0; i < line.GetCount(); ++i) {
			const char c = line[i], nc = i + 1 < line.GetCount() ? line[i+1] : 0;
			if ('a' == c || 'e' == c || 'i' == c || 'o' == c || 'u' == c) ++vowels;
			if (c == nc) ++twice;
			if (c + 1 == nc && ('a' == c || 'c' == c || 'p' == c || 'x' == c)) ++disallowed;
		}
		if (3 <= vowels && 1 <= twice && 0 == disallowed) ++nice_strings;
		return false;							// not finished yet, try next line
	}

	void finish() { Cout() << "* part1: " << nice_strings << EOL; }
};

class Part2 {

	int nice_strings = 0;

public:

	void init() { Cout() << "- part2"; }

	bool line(const String & line) {
		int at_plus_2 = 0, lsz = line.GetCount();
		for (int i = 2; i < lsz; ++i) if (line[i-2] == line[i]) ++at_plus_2;
		if (at_plus_2 < 1) return false;
		for (int i = 1; i < lsz; ++i) for (int j = i + 2; j < lsz; ++j) {
			if (line[i] == line[j] && line[i-1] == line[j-1]) {
				++nice_strings;
				return false;
			}
		}
		return false;							// not finished yet, try next line
	}

	void finish() { Cout() << "* part2: " << nice_strings << EOL; }
};

// concept of part1/part2 task having init/line(line)/finish interface
template <typename T> concept TaskProcessingLines = requires(T task, const String & line) {
	task.init();
	{ task.line(line) } -> std::same_as<bool>;	// returns true when finished early
	task.finish();
};

// main loop reading lines from input file feeding them to task
template <typename T> requires TaskProcessingLines<T>
void lines_loop(T task, const String & filename) {
	FileIn in(FileExists(filename) ? filename : GetDataFile(filename)); // 2nd try exe-dir
	if (!in) return;
	task.init();
	Cout() << " input filename: " << filename << EOL;
	while (!task.line(in.GetLine()) && !in.IsEof()) ;
	task.finish();
}

CONSOLE_APP_MAIN {
	for (const String & arg : CommandLine()) {
		lines_loop(Part1(), arg);
		lines_loop(Part2(), arg);
	}
}
