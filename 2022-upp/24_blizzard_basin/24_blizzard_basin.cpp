#include <Core/Core.h>

using namespace Upp;

constexpr int get_gcd(int a, int b) { return b ? get_gcd(b, a % b) : a; }

constexpr int get_lcm(int a, int b) { return a * b / get_gcd(a, b); }

constexpr int8 deltas[5][2] = { {-1, 0}, {+1, 0}, {0, -1}, {0, +1}, {0, 0} };	// < > ^ v .

struct Position : Moveable<Position> {
	uint8	x, y;
};

typedef Vector< Vector<uint16> > map_t;

void clear_map(map_t & m, int M, int N) {	// clear provided map to all zeroes (reusing memory)
	if (m.GetCount() < N) m.Reserve(N);
	for (int y = 0; y < N; ++y) {
		if (m.GetCount() <= y) m.Add().Insert(0, 0, M);
		else {
			ASSERT(M == m[y].GetCount());
			memset16(m[y].begin(), 0, M);
		}
	}
}

struct BlizzardsInput {
	Vector<Position> l, r, u, d;

	Vector<Position>* blizzards(char s) {
		switch (s) {
			case '<': return &l;
			case '>': return &r;
			case '^': return &u;
			case 'v': return &d;
		}
		return nullptr;
	}

	void add(char s, uint8 x, uint8 y) {
		auto b = blizzards(s);
		if (nullptr == b) return;			// unknown blizzard type or empty field, ignore
		b->Add({ .x = x, .y = y });			// known blizzard type, add it
	}

	void populate(map_t & m, int M, int N, int T) const {
		clear_map(m, M, N);					// clear provided map to all zeroes
		// populate with defined blizzards for time T
		int TM = T % M, TN = T % N;
		for (const auto & b : l) m[b.y][(b.x + M - TM) % M] = 0xFFFF;
		for (const auto & b : r) m[b.y][(b.x + TM) % M] = 0xFFFF;
		for (const auto & b : u) m[(b.y + N - TN) % N][b.x] = 0xFFFF;
		for (const auto & b : d) m[(b.y + TN) % N][b.x] = 0xFFFF;
		// ^ these can be sped up almost by 2x by avoiding `% M` and `% N` in favour of doing
		// `if (M >= x) x -= M;` (as the addition will always fit <0,2M) and <0,2N) range
	}
};

class AoC2022TaskDay24 {
	int entry = -1, exit = -1, M, N = -2, LCM;
	BlizzardsInput blizzards;

	int search(int T, const Position & from,  const Position & to) {
		map_t s, m;								// shortest path map, working map
		clear_map(s, M, N);						// set shortest path map to MxN zeroes
		BiVector<Position> queue;				// positions from T-1 to explore from
		int LCM_T = LCM + T;					// LCM adjusted by initial T
		// until shortest path to "to" is found or queue is empty after LCM+ rounds
		while (0 == s[to.y][to.x] && (++T <= LCM_T || !queue.IsEmpty())) {	// advance T and check positions
			blizzards.populate(m, M, N, T);		// clear m and setup blizzards for time T
			int q_cnt = queue.GetCount();		// process full queue from T-1
			while (q_cnt--) {
				auto p = queue.PopHead();		// position from T-1 to consider in T
				for (const auto & delta : deltas) {						// try all moves + wait
					int x = delta[0] + p.x, y = delta[1] + p.y;
					if (x < 0 || M <= x || y < 0 || N <= y) continue;	// move out of bounds
					if (0 != m[y][x]) continue;							// blizzard or reached before in this T
					m[y][x] = T;										// mark as reached
					if (0 == s[y][x]) s[y][x] = T;						// new shortest path
					else if (s[y][x] + LCM_T <= T) continue;			// been here, done that in previous LCM cycle
					queue.AddTail({.x = uint8(x), .y = uint8(y)});		// add to queue for next T
				}
			}
			if (0 == m[from.y][from.x]) queue.AddTail(from);			// seed possible entry
		}
		return s[to.y][to.x] ? s[to.y][to.x] + 1 : -1;
	}

public:
	void init() { Cout() << "***"; }

	bool line(const String & line) {
		if (-1 == ++N) M = line.GetLength() - 2, entry = line.Find("#.#");	// first line defines entry and M
		else if ((exit = line.Find("#.#")) < 0) {							// exit line defines exit
			for (int i = 0; i < M; ++i) blizzards.add(line[i+1], i, N);		// field line defines blizards
		}
		return (0 <= exit);						// parsing finished when exit is defined
	}

	void finish() {
		LCM = get_lcm(M, N);					// least common multiple of [M, N]
		Cout() << "input: " << Format("%d`x%d (LCM %d) entry %d, exit %d\n", M, N, LCM, entry, exit);
		Cout() << Format("blizzards l %d r %d u %d d %d\n", blizzards.l.GetCount(), blizzards.r.GetCount(), blizzards.u.GetCount(), blizzards.d.GetCount());
		const Position entryP { .x = uint8(entry), .y = 0 }, exitP { .x = uint8(exit), .y = uint8(N-1) };
		// part one - search quickest path from entry to exit
		int part1_time = search(0, entryP, exitP);
		Cout() << "part1: shortest path is " << part1_time << " minutes\n";
		// part two - re-run the search from exit to entry at correct time, then one more
		int part2_time_back = search(part1_time, exitP, entryP);
		int part2_time_forth = search(max(part1_time, part2_time_back), entryP, exitP);
		Cout() << Format("part2: shortest path is %d + %d + %d = %d minutes\n", part1_time,
			part2_time_back - part1_time, part2_time_forth - part2_time_back, part2_time_forth);
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
	for (const String & arg : CommandLine()) lines_loop(AoC2022TaskDay24(), arg);
}
