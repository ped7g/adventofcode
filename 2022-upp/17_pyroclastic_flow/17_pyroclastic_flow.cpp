#include <Core/Core.h>

using namespace Upp;

// expected result:
//	sample.txt:		3068, 1514285714288, wraps [  +35,   +53]
//	sample2.txt:	4473, 2160701754672, wraps [+1425, +3079]
//	input.txt:		3227, 1597714285698, wraps [+1750, +2796]

struct Position { int x, y; };

static constexpr int PIECES_N = 5, PIECES_M = 5, CHAMBER_SIZE = 1024;
// chamber size 32 is not enough for input data and 10k pieces (some piece must fall deep down)
// 1024 should be really plenty unless some jets are very very very carefully handcrafted

static constexpr int64 I_TARGETS[] = { 2022L, 10000L, 1000000L, 1000000000000L };
static constexpr const char* TARGET_NAMES[] = { "part1 [", "i     [", "part2 [" };

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
	Vector<uint8> chamber;

public:

	void init() { Cout() << "***"; }

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
			if (chamber[(y + pos.y) % CHAMBER_SIZE] & (1 << (x + pos.x))) return false;
		}
		return true;
	}

	void piece_fill(int p, int x, int y, int & last_y) {
		for (auto & pos : pieces[p]) {
			if (-1 == pos.x) continue;
			ASSERT(0 <= x + pos.x && x + pos.x < 7);
			chamber[(y + pos.y) % CHAMBER_SIZE] |= (1 << (x + pos.x));
			last_y = std::max(last_y, y + pos.y);
		}
	}

	uint64 chamber_top_8_lines(int y) {
		int64 r = -1;
		while (r < 0) r = (r << 8) | (chamber[y-- % CHAMBER_SIZE]);
		return r;
	}

	void finish() {
		int piece = 0, jet = 0, last_y = -1;
		chamber.Insert(0, 0, CHAMBER_SIZE);			// reserve space in chamber
		Vector<Tuple<int, int64, int64, uint64>> wrapping;
		int64 i = 0, y_ofs = 0, wrap_i = -1, wrap_t = -1;
		for (int64 target_i : I_TARGETS) {
			for (; i < target_i; ++i) {				// drop in total target_i pieces
				// track height of tower when piece and jet wraps around at the same time
				if (-1 == wrap_i && 0 == piece && 2 * jets.GetLength() <= i) {
					// look for possible wrapping period of whole state (extract [wrap_i, wrap_t])
					for (auto & w : wrapping) {
						if (w.a == jet) {			// same piece, same jet, verify chamber top
							if (chamber_top_8_lines(last_y) != w.d) {
								Cout() << Format("Top 8 chamber lines don't match at i %lld jet %d\n", i, jet);
								Exit();
							}
							// calculate wrap period for pieces and tower height
							wrap_i = i - w.b, wrap_t = y_ofs + last_y + 1 - w.c;
							Cout() << Format(" (wrap found: %+7lld,%+16lld)\n", wrap_i, wrap_t);
							break;
						}
					}
					if (-1 == wrap_i) {
						wrapping.Add({ jet, i, y_ofs + last_y + 1, chamber_top_8_lines(last_y) });
						//Cout() << Format("(seeding wrapping) piece, 0, jet, %5>d, i, %14>lld, tower, %14>lld, top 8 lines %016llX\n", jet, i, y_ofs + last_y + 1, int64(wrapping.Top().d));
					}
				}
				// if wrapping period is known, fast-forward [i, y_ofs] toward (target_i-1)
				if (0 < wrap_i && wrap_i < (target_i - 1 - i)) {
					int64 advance_by = (target_i - 1 - i) / wrap_i;
					i += advance_by * wrap_i;
					y_ofs += advance_by * wrap_t;
				}
				// clear chamber for next piece
				for (int y = last_y + 1; y <= last_y + 4 + 3; ++y) chamber[y % CHAMBER_SIZE] = 0;
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
			// print height of tower for each one of target_i
			const char* target_name = TARGET_NAMES[(I_TARGETS[0] < i) + (I_TARGETS[2] < i)];
			Cout() << Format("%s%14lld]: %14lld\n", target_name, i, y_ofs + last_y + 1);
		}
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
	for (const String & arg : CommandLine()) lines_loop(Part1(), arg);
}
