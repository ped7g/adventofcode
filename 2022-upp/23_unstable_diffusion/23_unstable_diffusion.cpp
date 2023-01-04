#include <Core/Core.h>

using namespace Upp;

typedef Tuple<short, short> Position;
typedef Tuple<Position, Position> Move;

class AoC2022Day23Task {
	// eight positions to check around elf in clockwise direction (N, NE, E, ..., W, NW)
	static const Position space_around[8];
	// three positions (from space_around) to check for particular N, S, W, E direction
	static constexpr int dir_around[4][3] = { {7, 0, 1}, {3, 4, 5}, {5, 6, 7}, {1, 2, 3} };
	// vector in N, S, W, E direction
	static const Position move[4];

	short line_y = 0;
	Index<Position> elves, proposals;
	Index<Move> moves;

public:

	void init() { Cout() << "***"; }

	bool line(const String & line) {
		for (short x = 0; x < line.GetLength(); ++x) if ('#' == line[x]) elves.Add({ x, line_y });
		++line_y;
		return false;							// not finished yet, try next line
	}

	int find_move_direction(int a[8], int first_dir) {
		int d = first_dir;
		do {									// check if direction "d" is free to move in
			if (0 == a[dir_around[d][0]] + a[dir_around[d][1]] + a[dir_around[d][2]]) return d;
			d = (d + 1) & 3;
		} while (d != first_dir);
		return -1;
	}

	bool round(int first_dir) {
		Index<Position> stands;
		stands.Reserve(elves.GetCount()), moves.Trim(0), proposals.Trim(0);
		int a[8], c, d;
		// first half of round, each elf considers where they would like to move
		for (auto & e : elves) {
			c = 0;								// count elves around
			for (int i = 0; i < 8; ++i) {		// cache the result for each of the eight tiles
				a[i] = (0 <= elves.Find({short(e.a + space_around[i].a), short(e.b + space_around[i].b)}) );
				c += a[i];
			}
			if (c && 0 <= (d = find_move_direction(a, first_dir))) {	// has elves around and can move
				Position proposal {short(e.a + move[d].a), short(e.b + move[d].b)};
				moves.Add({e, proposal}), proposals.Add(proposal);
			} else stands.Add(e);				// else stand still
		}
		// second half, move elves if the proposed move doesn't clash
		c = 0;									// total moved counter
		for (const auto & em : moves) {
			if (0 <= proposals.FindNext(proposals.Find(em.b))) stands.Add(em.a);	// clash, stands still
			else stands.Add(em.b), ++c;			// no clash, move and count moves
		}
		return elves = std::move(stands), c;
	}

	void finish() {
		int r = 0;
		for (; r < 10; ++r) round(r & 3);		// do 10 rounds to figure out part one answer
		// part 1 print result
		Position emin { 0x7FFF, 0x7FFF }, emax { -0x8000, -0x8000 };
		for (auto & e : elves) {
			emin.a = std::min(emin.a, e.a), emin.b = std::min(emin.b, e.b);
			emax.a = std::max(emax.a, e.a), emax.b = std::max(emax.b, e.b);
		}
		const int area = int(1 + emax.a - emin.a) * int(1 + emax.b - emin.b), cnt = elves.GetCount();
		Cout() << Format("part1: [%d,%d] -> [%d,%d] = %d tiles - %d elves = %d empty ground tiles\n",
							emin.a, emin.b, emax.a, emax.b, area, cnt, area - cnt);
		// continue with part 2
		while (round(r++ & 3)) ;				// do rounds until something moves
		Cout() << "part2: no move in round " << r << EOL;
	}
};

const Position AoC2022Day23Task::space_around[8] {	// N, NE, E, ..., W, NW
	{+0, -1}, {+1, -1}, {+1, +0}, {+1, +1}, {+0, +1}, {-1, +1}, {-1, +0}, {-1, -1}
};

const Position AoC2022Day23Task::move[4] {
	{+0, -1}, {+0, +1}, {-1, +0}, {+1, +0}		// N, S, W, E
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
	for (const String & arg : CommandLine()) lines_loop(AoC2022Day23Task(), arg);
}
