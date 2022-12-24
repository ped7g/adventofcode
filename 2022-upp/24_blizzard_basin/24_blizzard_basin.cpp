#include <Core/Core.h>

using namespace Upp;

struct Position : Moveable<Position> {
	uint16 t;
	uint8 x, y;
};

int get_gcd(int a, int b) {
	return b ? get_gcd(b, a % b) : a;
}

int get_lcm(int a, int b) {
	return a * b / get_gcd(a, b);
}

const char delta_symbol[4] = { '<', '>', '^', 'v' };
const int deltas[5][2] = { {-1, 0}, {+1, 0}, {0, -1}, {0, +1}, {0, 0} };	// < > ^ v .

class Part1 {
	int entry, exit, M, N = -2, LCM;
	Vector<String> input;
	Vector<Vector<Vector<uint16>>> m;			// LCM x field with all possible blizzard movement
	// uint16 -> 0 unknown, 1..0xFFFE -> steps to exit, 0xFFFF -> blizzard(s)
	BiVector<Position> positions;

public:
	void init() { Cout() << "***"; }

	bool line(const String & line) {
		if (-2 == N++) {
			M = line.GetLength() - 2;
			entry = line.Find("#.#");
		} else if ((exit = line.Find("#.#")) < 0) {	// not a last line
			input.Add(line.Mid(1, M));
		}
		return false;							// not finished yet, try next line
	}

	void clear_m_after_search() {
		for (auto & mt : m) for (auto & mty : mt) for (auto & mtyx : mty) {	// clear internal data
			if (0xFFFF == mtyx) continue;		// keep blizzards
			mtyx = 0;
		}
	}

	int search_path(Position from, const Position & to) {
		// seed path search with exit/entry path and fill the FIFO queue for the search
		positions.Clear();	positions.Reserve(M * N * N);
		for (uint16 t = 0; t < LCM; ++t) {
			if (m[t][to.y][to.x]) continue;		// blocked by blizzard in this T
			m[t][to.y][to.x] = 1;				// cost to "to" is 1 from here in every T
			positions.AddTail({.t = t, .x = to.x, .y = to.y});
		}
		// find valid entry point
		// ! doesn't account for using entry/exit to sidestep blizzards !
		int entry_wait = 1;
		while (0xFFFF == m[from.t][from.y][from.x]) {
			++entry_wait;
			if (LCM == ++from.t) from.t = 0;
		}
		// search for path to entry
		ASSERT(0 == m[from.t][from.y][from.x]);	// from at correct time must be free to move in
		while (positions.GetCount() && 0 == m[from.t][from.y][from.x] ) {
			auto p = positions.PopHead();
			uint16 cost = m[p.t][p.y][p.x] + 1;	// +1 minute to total cost
			if (0 == p.t--) p.t = LCM - 1;
			for (const auto & delta : deltas) {
				if (delta[0] + p.x < 0 || M <= delta[0] + p.x) continue;
				if (delta[1] + p.y < 0 || N <= delta[1] + p.y) continue;
				if (0xFFFF == m[p.t][delta[1] + p.y][delta[0] + p.x]) continue;	// blizzard there
				ASSERT(m[p.t][delta[1] + p.y][delta[0] + p.x] <= cost);
				if (0 != m[p.t][delta[1] + p.y][delta[0] + p.x]) continue;	// already reachable
				m[p.t][delta[1] + p.y][delta[0] + p.x] = cost;				// new reach
				positions.AddTail({.t = p.t, .x = uint8(delta[0] + p.x), .y = uint8(delta[1] + p.y)});
			}
		}
		if (0 == m[from.t][from.y][from.x]) {	// unreachable, try wait at entry even more
			clear_m_after_search();
			if (LCM == ++from.t) from.t = 0;
			return entry_wait + search_path(from, to);	// entry_wait +1 comes from recursion
		}
		ASSERT(m[from.t][from.y][from.x]);
		return entry_wait + m[from.t][from.y][from.x];
	}

	void finish() {
		LCM = get_lcm(M, N);					// least common multiple of [M, N]
		Cout() << "input: " << Format("%d`x%d (LCM %d) entry %d, exit %d\n", M, N, LCM, entry, exit);
		// set up empty fields for all possible t = [0,LCM)
		m.Reserve(LCM);
		for (uint16 t = 0; t < LCM; ++t) {
			m.Add().Reserve(N);
			auto & mt = m.Top();
			for (int y = 0; y < N; ++y) mt.Add().Insert(0, 0, M);	// empty MxN map in mt
		}
		// populate fields with blizzards from input
		for (int y = 0; y < N; ++y) {
			for (int x = 0; x < M; ++x) {
				int dt = 0;
				for (dt = 0; dt < 4; ++dt) if (delta_symbol[dt] == input[y][x]) break;
				if (4 == dt) continue;
				for (uint16 t = 0; t < LCM; ++t) {
					int bx = (x + t * deltas[dt][0]) % M;	if (bx < 0) bx += M;
					int by = (y + t * deltas[dt][1]) % N;	if (by < 0) by += N;
					m[t][by][bx] = 0xFFFF;
				}
			}
		}
		// part one - run the search from entry to exit at time 0
		int part1_time = search_path({.t = 1, .x = uint8(entry), .y = 0}, {.x = uint8(exit), .y = uint8(N-1)});
		Cout() << "part1: shortest path is " << part1_time << " minutes\n";
		// part two - re-run the search from exit to entry at correct time, then one more
		clear_m_after_search();
		int part2_time_back = search_path({.t = uint16((part1_time + 1) % LCM), .x = uint8(exit), .y = uint8(N-1)}, {.x = uint8(entry), .y = 0});
		clear_m_after_search();
		int part2_time_forth = search_path({.t = uint16((part1_time + part2_time_back + 1) % LCM), .x = uint8(entry), .y = 0}, {.x = uint8(exit), .y = uint8(N-1)});
		Cout() << Format("part2: shortest path is %d + %d + %d = %d minutes\n", part1_time,
			part2_time_back, part2_time_forth, part1_time + part2_time_back + part2_time_forth);
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
	for (const String & arg : CommandLine()) lines_loop(Part1(), arg);
}
