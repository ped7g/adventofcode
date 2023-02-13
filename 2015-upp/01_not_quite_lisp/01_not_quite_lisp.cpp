#include <Core/Core.h>

using namespace Upp;

class AoC2015Day01Task {	// expected result for input.txt 138, 1771

public:

	void init() { Cout() << "***"; }

	bool line(const String & line) {
		int floor = 0, basement = 0;
		for (int i = 0; i < line.GetCount(); ++i) {
			floor += ('(' == line[i]) - (')' == line[i]);
			if (-1 == floor && !basement) basement = 1 + i;
		}
		Cout() << "floor " << floor << ", basement position " << basement << EOL;
		return false;	// try next line
	}

	void finish() {}
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
	for (const String & arg : CommandLine()) lines_loop(AoC2015Day01Task(), arg);
}
