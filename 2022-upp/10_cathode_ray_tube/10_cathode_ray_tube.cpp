#include <Core/Core.h>

using namespace Upp;

class CrtTubeEmulator {	// expected results: sample: 13140, *stripes*, input: 14040, ZGCJZJFL
	int sprite_x = 1, cycle = 1;					// CPU internals
	int crt_x = 1;									// CRT internals
	int signal_sum = 0, cycles_to_sample = 20;		// signal strength sampling

public:
	void init() { Cout() << "***"; }

	void tick(int addx = 0) {
		// emulate CRT for part 2 answer
		Cout() << ((sprite_x <= crt_x && crt_x < sprite_x+3) ? '#' : '.');
		if (41 == ++crt_x) Cout() << EOL, crt_x = 1;
		// tick CPU and sample signal strength for part 1 answer
		if (0 == --cycles_to_sample) signal_sum += (cycle * sprite_x), cycles_to_sample = 40;
		sprite_x += addx, ++cycle;
	}

	bool line(const String & line) {
		if (line.StartsWith("addx")) tick(), tick(atoi(line.Mid(5)));
		else if ("noop" == line) tick();
		return false;
	}

	void finish() {
		Cout() << EOL << "part 1 signal strength sum: " << signal_sum << EOL << EOL;
	}
};

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
	FileIn in(FileExists(filename) ? filename : GetDataFile(filename)); // 2nd try exe-dir
	if (!in) return;
	task.init();
	Cout() << " input filename: " << filename << EOL;
	while (!task.line(in.GetLine()) && !in.IsEof()) ;
	task.finish();
}

CONSOLE_APP_MAIN {
	for (const String & arg : CommandLine()) lines_loop(CrtTubeEmulator(), arg);
}
