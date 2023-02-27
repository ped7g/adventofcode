#include <Core/Core.h>

using namespace Upp;

static constexpr int16 INIT_VALUE = 10000;

class AoC2015Day03Task {

	static int32 xy2k(int16 x, int16 y) { return (int32(x) << 16) | y; }

	static int32 move(char d, int16 coords[2]) {
		switch (d) {
			case '<': --coords[0]; break;
			case '>': ++coords[0]; break;
			case 'v': --coords[1]; break;
			case '^': ++coords[1]; break;
		}
		return xy2k(coords[0], coords[1]);
	}

public:

	void init() { Cout() << "***"; }

	bool line(const String & line) {
		int16 coords[3][2] { { INIT_VALUE, INIT_VALUE }, { INIT_VALUE, INIT_VALUE }, { INIT_VALUE, INIT_VALUE } };
		int p2 = 1;								// santa is active for part two first move (robot idle)
		Index<int32> visited_p1, visited_p2;	// visited houses for part one/two (init both with starting house)
		visited_p1.Reserve(INIT_VALUE), visited_p2.Reserve(INIT_VALUE);
		visited_p1.Add(xy2k(coords[0][0], coords[0][1])), visited_p2.Add(xy2k(coords[0][0], coords[0][1]));
		for (char d : line) {					// process input directions
			visited_p1.FindAdd(move(d, coords[0]));
			visited_p2.FindAdd(move(d, coords[p2]));
			p2 ^= 1 ^ 2;						// alternate santa/robot being active for part two
		}
		Cout() << "part 1 visited houses: " << visited_p1.GetCount() << EOL;	// print result
		Cout() << "part 2 visited houses: " << visited_p2.GetCount() << EOL;
		return false;							// not finished yet, try next line
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
	for (const String & arg : CommandLine()) lines_loop(AoC2015Day03Task(), arg);
}
