#include <Core/Core.h>

using namespace Upp;

// expected result:
//	sample.txt:		3068, 1514285714288, wraps [  +35,   +53]
//	sample2.txt:	4473, 2160701754672, wraps [+1425, +3079]
//	input.txt:		3227, 1597714285698, wraps [+1750, +2796]

struct Position { int x, y; };

constexpr int PIECES_N = 5, PIECES_M = 5, CHAMBER_SIZE = 1024;
// chamber size 32 is not enough for input data and 10k pieces (some piece must fall deep down)
// 1024 should be really plenty unless some jets are very very very carefully handcrafted

static const Position pieces[PIECES_N][PIECES_M] = {
//####
{ {0,0}, {1,0}, {2,0}, {3,0}, {-1,0} },

//.#.
//###
//.#.
{ {1,0}, {0,1}, {1,1}, {2,1}, {1,2} },

//..#
//..#
//###
{ {0,0}, {1,0}, {2,0}, {2,1}, {2,2} },

//#
//#
//#
//#
{ {0,0}, {0,1}, {0,2}, {0,3}, {-1,0} },

//##
//##
{ {0,0}, {1,0}, {0,1}, {1,1}, {-1,0} },
};

class Part1 {

	String jets;
	Vector<Vector<char>> chamber;

public:

	void init() { Cout() << "part1"; }

	bool line(const String & line) {
		ASSERT(jets.IsEmpty() && !line.IsEmpty());
		jets = line;
		return true;	// finished input
	}

	bool piece_fits(int p, int x, int y) const {
		if (y < 0 || x < 0) return false;
		for (auto & pos : pieces[p]) {
			if (-1 == pos.x) continue;
			if (7 <= x + pos.x) return false;
			if (chamber[(y + pos.y) % CHAMBER_SIZE][x + pos.x] != '.') return false;
		}
		return true;
	}

	void piece_fill(int p, int x, int y, int & last_y) {
		for (auto & pos : pieces[p]) {
			if (-1 == pos.x) continue;
			ASSERT(0 <= x + pos.x && x + pos.x < 7);
			chamber[(y + pos.y) % CHAMBER_SIZE][x + pos.x] = '#';
			last_y = std::max(last_y, y + pos.y);
		}
	}

	void clear_chamber(int y) {
		y %= CHAMBER_SIZE;
		for (int x = 0; x < 7; ++x) chamber[y][x] = '.';
	}

	void finish() {
		int piece = 0, jet = 0, last_y = -1, chamber_reserve = CHAMBER_SIZE;
		// reserve space in chamber
		while (chamber_reserve--) {
			chamber.Add().Insert(0, '.', 7);
			chamber.Top().Add(0);
		}
		int64 target_i[] = { 2022, 10000, 1000000, 1000000000000 };
		int target_i_i = 0, setup_wrapping = 1;
		Vector<Tuple<int, int64, int64>> wrapping;
		int64 y_ofs = 0, wrap_i = -1, wrap_t = -1, events_to_print = jets.GetLength();
		for (int64 i = 0; i < 1000000000000; ++i) {	// drop N pieces
			// print height of tower when piece and jet wraps around at the same time
			if (setup_wrapping && 0 == piece && 2 * jets.GetLength() <= i) {
				for (auto & w : wrapping) {
					if (w.a == jet) {
						int64 nwi = i - w.b, nwt = y_ofs + last_y + 1 - w.c;
						if (-1 == wrap_i) wrap_i = nwi, wrap_t = nwt;
						else if (nwi != wrap_i || nwt != wrap_t) {
							Cout() << Format("Different wrap at i %lld jet %d: wi %lld vs %lld, wt %lld vs %lld\n",
								i, jet, wrap_i, nwi, wrap_t, nwt);
							Exit();
						}
						w.b = i, w.c = y_ofs + last_y + 1;
						break;
					}
				}
				if (-1 == wrap_i) {
					wrapping.Add({jet, i, y_ofs + last_y + 1});
					//Cout() << Format("(seeding wrapping) piece, 0, jet, %5>d, i, %14>lld, tower, %14>lld\n", jet, i, y_ofs + last_y + 1);
				}
				if (0 == --events_to_print) {
					Cout() << Format("wrap_i %lld, wrap_t %lld\n", wrap_i, wrap_t);
					setup_wrapping = 0;
				}
			}
			if (!setup_wrapping && wrap_i < (target_i[target_i_i] - i)) {
				int64 advance_by = (target_i[target_i_i] - i) / wrap_i;
				i += advance_by * wrap_i;
				y_ofs += advance_by * wrap_t;
			}
			// print height of tower for one of target_i
			if (i == target_i[target_i_i]) {
				Cout() << "i " << target_i[target_i_i] << ": " << y_ofs + last_y + 1 << EOL;
				if (4 == ++target_i_i) Exit();
			}
			// clear chamber for next piece
			for (int y = last_y + 1; y <= last_y + 4 + 3; ++y) clear_chamber(y);
			// move piece until it rests
			int x = 2, y = last_y + 4;	ASSERT(piece_fits(piece, x, y));
			while (0 <= y) {
				// sideway jet
				int dx = jets[jet] - '=';			// -1 for '<', +1 for '>'
				if (++jet == jets.GetLength()) jet = 0;
				if (piece_fits(piece, x + dx, y)) x += dx;
				// downward
				if (!piece_fits(piece, x, y - 1)) break;
				--y;
			}
			// rest the piece and figure out new last_y
			piece_fill(piece, x, y, last_y);
			// wrap-around last_y to fit int, stay in [CHAMBER_SIZE..2xCHAMBER_SIZE) range
			if (2 * CHAMBER_SIZE <= last_y) last_y -= CHAMBER_SIZE, y_ofs += CHAMBER_SIZE;
			// next piece..
			if (PIECES_N == ++piece) piece = 0;
		}
//		// debug print of chamber
//		for (int y = chamber.GetCount(); y--; ) Cout() << "|" << chamber[y].begin() << "|" << EOL;
//		Cout() << "+-------+" << EOL;
		// print height of tower
		Cout() << "part2: " << y_ofs + last_y + 1 << EOL;
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

CONSOLE_APP_MAIN
{
	for (const String & arg : CommandLine()) lines_loop(Part1(), arg);
}
