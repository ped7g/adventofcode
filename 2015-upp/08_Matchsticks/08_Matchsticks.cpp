#include <Core/Core.h>

using namespace Upp;

class Aoc2015Day08Task {

	long p1 = 0, p2 = 0;

public:

	void init() { Cout() << "***"; }

	bool line(const String & line) {
		for (int i = 0; i < line.GetCount(); ++i) {		// part 1 counting
			if ('\\' == line[i] && ('\\' == line[i+1] || '"' == line[i+1])) ++p1, ++i;
			else if ('\\' == line[i] && 'x' == line[i+1]) p1 += 3, i += 3;
			else if ('"' == line[i]) ++p1;
		}
		// part 2 counting (can't share `for` loop in P1 vs P2 easily, because P1 skipping...)
		p2 += 2;
		for (int i = 0; i < line.GetCount(); ++i) p2 += ('\\' == line[i] || '"' == line[i]);
		return false;
	}

	void finish() { Cout() << "part1: " << p1 << " part2: " << p2 << EOL; }
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
	for (const String & arg : CommandLine()) lines_loop(Aoc2015Day08Task(), arg);
} // expected output: sample.txt: 12, 19, input.txt: 1333, 2046
