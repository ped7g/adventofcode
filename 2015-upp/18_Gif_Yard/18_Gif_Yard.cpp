#include <Core/Core.h>

using namespace Upp;

static constexpr int N = 100;

template<int PART>
struct LifeGame {
	Vector< Vector<bool> > m;

	LifeGame() { m.Reserve(N); }

	bool line(const String & line) {
		if (line.IsEmpty()) return true;
		m.Add().Reserve(N);
		for (char c : line) m.back().push_back('#' == c);
		return false;
	}

	int lights_on(int x, int y) {
		int t = m[y][x] + (0 < y && m[y-1][x]) + (y + 1 < N && m[y+1][x]);
		if (0 < x) {
			t += m[y][x-1] + (0 < y && m[y-1][x-1]) + (y + 1 < N && m[y+1][x-1]);
		}
		if (x + 1 < N) {
			t += m[y][x+1] + (0 < y && m[y-1][x+1]) + (y + 1 < N && m[y+1][x+1]);
		}
		return t;
	}

	void step() {
		if constexpr (2 == PART) {
			m[0][0] = m[0][N-1] = m[N-1][0] = m[N-1][N-1] = true;
		}
		Vector< Vector<bool> > m2;
		m2.Reserve(N);
		for (int y = 0; y < N; ++y) {
			m2.Add().Reserve(N);
			for (int x = 0, l; x < N; ++x)
				l = lights_on(x, y), m2.back().Add(3 == l || (4 == l && m[y][x]));
		}
		if constexpr (2 == PART) {
			m2[0][0] = m2[0][N-1] = m2[N-1][0] = m2[N-1][N-1] = true;
		}
		m = std::move(m2);
	}

	int countOn() {
		int l = 0;
		for (const auto & row : m) l += Count(row, true);
		return l;
	}
};

struct Part1 {

	LifeGame<1> life;

	void init() { Cout() << "part1"; }

	bool line(const String & line) { return life.line(line); }

	void finish() {
		for (int step = 100; step--;) life.step();
		Cout() << "part1: " << life.countOn() << EOL;
	}
};

struct Part2 {

	LifeGame<2> life;

	void init() { Cout() << "part2"; }

	bool line(const String & line) { return life.line(line); }

	void finish() {
		for (int step = 100; step--;) life.step();
		Cout() << "part2: " << life.countOn() << EOL;
	}
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
} // expected answer: part1 821, part2 886
