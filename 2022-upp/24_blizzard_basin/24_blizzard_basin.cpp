#include <Core/Core.h>

using namespace Upp;

constexpr int get_gcd(int a, int b) { return b ? get_gcd(b, a % b) : a; }
constexpr int get_lcm(int a, int b) { return a * b / get_gcd(a, b); }
constexpr char delta_symbol[5] = { '<', '>', '^', 'v', 0 };
constexpr int16 deltas[5][2] = { {-1, 0}, {+1, 0}, {0, -1}, {0, +1}, {0, 0} };	// < > ^ v .

struct Position : Moveable<Position> {
	uint16	t;							// warped [0, LCM) time of the position
	uint8	x, y;
};

struct PathSearchData {
	const int M, N, LCM;
	const Position to;
	// LCM x M x N field with all possible blizzard movement and path-costs to "to" tile
	Vector<Vector<Vector<uint16>>> m;	// 0 unreachable, 1..0xFFFE -> minutes to exit, 0xFFFF -> blizzard(s)
	bool do_init_positions = true;		// needs pre-seeding positions before first search
	BiVector<Position> positions;		// FIFO queue of positions to process during path search

	PathSearchData(const PathSearchData &) = delete;

	PathSearchData(int M, int N, int LCM, const Position & to, const Vector<String> & blizzards)
		: M(M), N(N), LCM(LCM), to(to)
	{
		// empty LCM x M x N arrays of uint16
		m.Reserve(LCM);
		for (uint16 t = 0; t < LCM; ++t) {
			auto & mt = m.Add();
			mt.Reserve(N);
			for (int y = 0; y < N; ++y) mt.Add().Insert(0, 0, M);
		}
		// populate fields with blizzards from input
		for (int y = 0; y < N; ++y) for (int x = 0; x < M; ++x) {
			for (int di = 0; di < 4; ++di) {
				if (delta_symbol[di] == blizzards[y][x]) {	// blizzard found, set it to all m[t] fields
					int bx = x, by = y, dx = deltas[di][0], dy = deltas[di][1];
					for (uint16 t = 0; t < LCM; ++t) {
						m[t][by][bx] = 0xFFFF;
						bx += dx; if (bx < 0) bx += M; else if (M <= bx) bx -= M;
						by += dy; if (by < 0) by += N; else if (N <= by) by -= N;
					}
					break;
				}
			}
		}
		//DEBUG print out all maps
		static bool DBG_print = true;
		if (DBG_print) {
			DBG_print = false;
			for (uint16 t = 0; t < LCM; ++t) {
				Cout() << EOL << "*** T = " << t << EOL;
				for (int y = 0; y < N; ++y) {
					for (int x = 0; x < M; ++x) {
						Cout() << (m[t][y][x] ? '@' : '_');
					}
					Cout() << EOL;
				}
			}
		}
	}

	// provided instance has to have "m" fields already initialised, but in pristine state (no costs yet)
	PathSearchData(const PathSearchData & b, const Position & to) : M(b.M), N(b.N), LCM(b.LCM), to(to) {
		m = clone(b.m);
		ASSERT(LCM == m.GetCount() && N == m[0].GetCount() && M == m[0][0].GetCount());
	}

	void init_positions() {
		// seed path search with "to": cost = 1 from tile next to "to" and fill the FIFO search-queue
		positions.Reserve(M * N * 2);
		for (uint16 t = 0; t < LCM; ++t) {
			if (m[t][to.y][to.x]) continue;		// blocked by blizzard in this T
			m[t][to.y][to.x] = 1;				// cost to "to" is 1 from here in every T
			positions.AddTail({.t = t, .x = to.x, .y = to.y});
		}
		do_init_positions = false;
	}

	int search_path_from(Position from) {
		// if needed, init starting search positions (based on "to" position)
		if (do_init_positions) init_positions();
		// look up answer, if cost is 0, try to run search, if still zero, wait longer at entry
		int entry_wait = 1;
		while (0xFFFF == m[from.t][from.y][from.x] || 0 == m[from.t][from.y][from.x]) {
			// if no cost assigned yet, exhaust first search-queue (until cost assigned or empty)
			while (0 == m[from.t][from.y][from.x] && !positions.IsEmpty()) {
				auto p = positions.PopHead();
				uint16 cost = m[p.t][p.y][p.x] + 1;						// +1 minute to total cost
				if (0 == p.t--) p.t = LCM - 1;							// check tiles in t-1 time
				for (const auto & delta : deltas) {						// try all moves + wait
					int x = delta[0] + p.x, y = delta[1] + p.y;
					if (x < 0 || M <= x || y < 0 || N <= y) continue;	// move out of bounds
					if (0xFFFF == m[p.t][y][x]) continue;				// blizzard there
					ASSERT(m[p.t][y][x] <= cost);
					if (0 != m[p.t][y][x]) continue;					// reached before
					m[p.t][y][x] = cost;								// new lowest-cost reach
					positions.AddTail({.t = p.t, .x = uint8(x), .y = uint8(y)});
				}
			}
			// if still no cost assigned (unreachable or blizzard), try "later" entry
			if (0 == m[from.t][from.y][from.x] || 0xFFFF == m[from.t][from.y][from.x]) {
				if (LCM == entry_wait++) return -1;						// no path at all
				if (LCM == ++from.t) from.t = 0;
			}
		}
		// return the cost of the path found
		ASSERT(0 < m[from.t][from.y][from.x] && m[from.t][from.y][from.x] < 0xFFFF);
		return entry_wait + m[from.t][from.y][from.x];
	}
};

class Part1 {
	int entry = -1, exit = -1, M, N = -2, LCM;
	Vector<String> input;

public:
	void init() { Cout() << "***"; }

	bool line(const String & line) {
		if (line.StartsWith(";",1)) return false;	//DEBUG comments in input.txt
		if (-2 == N++) M = line.GetLength() - 2, entry = line.Find("#.#");	// first line
		else if ((exit = line.Find("#.#")) < 0) input.Add(line.Mid(1, M));	// exit line or field line
		return (0 <= exit);						// finished when exit is defined
	}

	void finish() {
		LCM = get_lcm(M, N);					// least common multiple of [M, N]
		Cout() << "input: " << Format("%d`x%d (LCM %d) entry %d, exit %d\n", M, N, LCM, entry, exit);
		// prepare path-search data for both part one and part two
		Position p_entry {.x = uint8(entry), .y = 0}, p_exit {.x = uint8(exit), .y = uint8(N-1)};
		PathSearchData to_exit(M, N, LCM, p_exit, input);
		PathSearchData to_entry(to_exit, p_entry);
		// part one - run the search from entry to exit at time 0
		p_entry.t = 1;							// well, actually time 1 to move over entry tile
		int part1_time = to_exit.search_path_from(p_entry);
		Cout() << "part1: shortest path is " << part1_time << " minutes\n";
		// part two - re-run the search from exit to entry at correct time, then one more
		p_exit.t = (part1_time + 1) % LCM;
		int part2_time_back = to_entry.search_path_from(p_exit);
		p_entry.t = (part1_time + part2_time_back + 1) % LCM;
		int part2_time_forth = to_exit.search_path_from(p_entry);
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
