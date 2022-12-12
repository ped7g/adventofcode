#include <Core/Core.h>

using namespace Upp;

struct Position : Moveable<Position> {
	int x, y;
	Position(int x, int y) : x(x), y(y) {}
};

class AocTask {

	int M = 0, N = 0;
	Vector<String> map;

public:

	void init() { Cout() << "***"; }

	bool line(const String & line) {
		if (line.IsEmpty()) return true;
		if (0 == M) M = line.GetLength(); else ASSERT(M == line.GetLength());
		map.Add(line);
		return false;							// not finished yet, try next line
	}

	void finish() {
		if (map.IsEmpty()) return;
		N = map.GetCount();
		Position S { -1, -1 }, E { -1, -1 };
		int x, y, ci = 0;
		// find 'S' and 'E' positions, replace them with 'a' and 'z'
		for (y = 0; y < N; ++y) {
			x = map[y].Find('S');
			if (0 <= x) S.x = x, S.y = y, map[S.y].Set(S.x, 'a');
			x = map[y].Find('E');
			if (0 <= x) E.x = x, E.y = y, map[E.y].Set(E.x, 'z');
		}
		// debug output of start/end positions and [modified] map
		Cout() << Format("map %d`x%d S[%d, %d] E[%d, %d]\n", M, N, S.x, S.y, E.x, E.y);
		for (const auto & s : map) Cout() << s << EOL;
		// find shortest path to E from any reachable point of map
		static const Vector<Position> deltas { {-1, 0}, {+1, 0}, {0, -1}, {0, +1}};
		Vector<Vector<int>> steps(N, Vector<int>(M, -1));	// MxN steps to E from anywhere
		steps[E.y][E.x] = 0;								// steps to E from E is 0
		Vector<Position> climb_to;							// queue of positions to check
		climb_to.Reserve(M * N);
		climb_to.Add(E);									// start from E point
		while (ci < climb_to.GetCount()) {					// while there is position to check
			const auto & to = climb_to[ci++];
			char min_height = map[to.y][to.x] - 1;			// "from" can be -1 lower height
			for (const auto & delta : deltas) {				// check four possible "from"
				Position from { to.x + delta.x, to.y + delta.y };
				if (from.x < 0 || M <= from.x || from.y < 0 || N <= from.y) continue;
				if (-1 != steps[from.y][from.x] || map[from.y][from.x] < min_height) continue;
				// "from" position is within map coordinates and with acceptable height
				steps[from.y][from.x] = 1 + steps[to.y][to.x];	// it is +1 step away from "to"
				climb_to.Add(from);							// add it to queue to reach further
			}
		}
		Cout() << "Reachable positions " << climb_to.GetCount() << " out of " << M * N << EOL;
		// steps at S position now contain minimum amount of steps to reach E -> part 1 result
		int min_steps = steps[S.y][S.x];
		Cout() << "part1: " << min_steps << EOL;
		// part 2: find reachable 'a' height with smallest amount of steps
		for (y = 0; y < N; ++y) {
			const String & s = map[y];
			for (x = 0; x < M; ++x) {
				if ('a' != s[x] || -1 == steps[y][x]) continue;
				min_steps = std::min(min_steps, steps[y][x]);
			}
		}
		Cout() << "part2: " << min_steps << EOL;
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
	for (const String & arg : CommandLine()) lines_loop(AocTask(), arg);
}
