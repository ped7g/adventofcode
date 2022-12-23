#include <Core/Core.h>

using namespace Upp;

typedef Tuple<short, short> Elf;

class AoC2022Day23Task {
	short ly = 0;
	Index<Elf> elves, p_check;
	Index<Tuple<Elf, Elf>> proposals;

public:

	void init() { Cout() << "***"; }

	bool line(const String & line) {
		for (short x = 0; x < line.GetLength(); ++x) if ('#' == line[x]) elves.Add({ x, ly });
		++ly;
		return false;							// not finished yet, try next line
	}

	const Vector<Elf> check { {+0, -1}, {+1, -1}, {+1, +0}, {+1, +1}, {+0, +1}, {-1, +1}, {-1, +0}, {-1, -1} };
	const int dir_check[4][3] = { {7, 0, 1}, {3, 4, 5}, {5, 6, 7}, {1, 2, 3} };	// N, S, W, E
	const short dir_move[4][2] = { {+0, -1}, {+0, +1}, {-1, +0}, {+1, +0} };	// N, S, W, E

	bool round(int first_dir) {
		proposals.Clear(), p_check.Clear();
		int total_moves = 0;
		for (auto & e : elves) {
			int around[8], count = 0, dir = first_dir;
			for (int i = 0; i < 8; ++i) {
				around[i] = (0 <= elves.Find({ short(e.a + check[i].a), short(e.b + check[i].b) }));
				count += around[i];
			}
			Elf proposal = e;						// if no elves around or no free way, stay where you are
			if (count) {
				do {
					if (0 == around[dir_check[dir][0]] + around[dir_check[dir][1]] + around[dir_check[dir][2]]) {
						proposal.a = e.a + dir_move[dir][0];	// propose move in direction "dir"
						proposal.b = e.b + dir_move[dir][1];
						++total_moves;
						break;
					}
					dir = (dir + 1) & 3;
				} while (dir != first_dir);
			}
			proposals.Add({e, proposal}), p_check.Add(proposal);
		}
		// second half, move elves
		elves.Clear();
		for (const auto & em : proposals) {
			int check1 = p_check.Find(em.b), check2 = p_check.FindNext(check1);
			ASSERT(0 <= check1);
			elves.Add((check2 < 0) ? em.b : em.a);
		}
		return (0 < total_moves);
	}

	void finish() {
		int r = 0;
		for (; r < 10; ++r) round(r & 3);
		// part 1 print result
		Elf emin { 0x7FFF, 0x7FFF }, emax { -0x8000, -0x8000 };
		for (auto & e : elves) {
			emin.a = std::min(emin.a, e.a), emin.b = std::min(emin.b, e.b);
			emax.a = std::max(emax.a, e.a), emax.b = std::max(emax.b, e.b);
		}
		int area = int(1 + emax.a - emin.a) * int(1 + emax.b - emin.b);
		Cout() << Format("part1: [%d,%d] -> [%d,%d] = %d tiles - %d elves = %d\n",
							emin.a, emin.b, emax.a, emax.b, area, elves.GetCount(), area - elves.GetCount());
		// continue with part 2
		for (; round(r & 3); ++r) ;
		Cout() << "part2 no move in round " << r + 1 << EOL;
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
	for (const String & arg : CommandLine()) lines_loop(AoC2022Day23Task(), arg);
}
