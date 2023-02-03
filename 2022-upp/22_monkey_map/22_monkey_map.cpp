#include <Core/Core.h>

using namespace Upp;	// expected answer: sample [6032, 5031], input [27492, 78291]

enum { D_R = 0, D_D = 1, D_L = 2, D_U = 3, SAME = 0, ROTL = 1, ROTU = 2, ROTR = 3, NONE = -1 };

static constexpr int move_delta[4][2] = { { +1,  0 }, {  0, +1 }, { -1,  0 }, {  0, -1 } };

struct CoordinateSystem {		// side number at edge, required coordinate transformation
	int s = NONE, t = NONE;
};

struct Position {				// local (within side) coordinates [x,y], direction, side number
	int x = 0, y = 0, d = D_R, s = 0;
};

struct Side {
	Position origin;			// top-left of area in board for this side (only [x,y] is used)
	CoordinateSystem next[4];	// sides at right, down, left, up + required coordinates transformation
};

class AoC2022Day22Task {
	Vector<String> board;
	String path;
	Position p;
	int M = 0, N = 0;
	Side sides[6];

public:

	void init() { Cout() << "***"; }

	bool line(const String & line) {
		if (line.IsEmpty()) return false;
		if (IsDigit(line[0])) path = line;					// this is path line
		else board.Add(line), M = max(M, line.GetLength());
		return !path.IsEmpty();								// path line terminates input
	}

	template<int part> Position wrap(Position p) {			// wrap coordinates to next side
		ASSERT((D_R == p.d && p.x == M) || (D_D == p.d && p.y == N) || (D_L == p.d && p.x == -1) || (D_U == p.d && p.y == -1));
		p.x = (p.x + M) % M, p.y = (p.y + N) % N;			// wrap coordinates to MxN first
		if constexpr (1 == part) {
			return p;	// part 1 only wraps around to big MxN board
		} else {		// part 2 requires also coordinates transformation from side to side
			int t = sides[p.s].next[p.d].t, nd = (p.d + t) & 3, ns = sides[p.s].next[p.d].s;
			switch (t) {									// transform wrapped coordinates
				case ROTL:	return { M-1-p.y,	p.x,		nd,	ns };
				case ROTR:	return { p.y,		N-1-p.x,	nd,	ns };
				case ROTU:	return { M-1-p.x,	N-1-p.y,	nd,	ns };
				default:	return { p.x,		p.y,		nd,	ns };	// SAME transform
			}
		}
	}

	template<int part> bool fwd() {
		Position np = p;
		char n_tile;
		do {
			np.x += move_delta[np.d][0], np.y += move_delta[np.d][1];
			if (np.y < 0 || N <= np.y || np.x < 0 || M <= np.x) np = wrap<part>(np);
			n_tile = board[sides[np.s].origin.y + np.y][sides[np.s].origin.x + np.x];
		} while (1 == part && ' ' == n_tile);				// part 1 -> jump over whitespace
		if ('.' == n_tile) p = np; else ASSERT('#' == n_tile);
		return ('.' == n_tile);
	}

	template<int part> void follow_path() {
		const char* lb = path.Begin(), * le = path.End();
		while (lb < le) {
			for (int r = atoi(lb); r-- && fwd<part>(); ) ;	// move until wall
			while (++lb < le && IsDigit(*lb)) ;				// skip input to turn-letter
			p.d = (p.d - ('L' == *lb) + ('R' == *lb)) & 3;	// turn
			++lb;
		}
	}

	void part1() {
		sides[0] = { {0, 0} }, N = board.GetCount(), p = {};		// side 0 covers whole global board
		while (' ' == board[p.y][p.x]) ++p.x;						// find top-left tile of board
		follow_path<1>();
		Cout() << "part1: " << (1000 * (p.y+1) + 4 * (p.x+1) + p.d) << EOL;
	}

	void map_cube(int & side_n, const Position & p) {
		ASSERT(' ' != board[p.y][p.x]);
		auto & side = sides[side_n++];
		side.origin = p;											// record new origin of side
		// check neighbour sides
		for (int d = 0; d < 4; ++d) {								// right, down, left, up
			if (side.next[d].t != NONE) continue;					// already defined side
			// derive position of neighbour side from current position
			Position np { .x = p.x + move_delta[d][0] * M, .y = p.y + move_delta[d][1] * N, .s = side_n };
			// check if new position is valid and contains non-empty side data
			if (np.y < 0 || board.GetCount() <= np.y) continue;
			if (np.x < 0 || board[np.y].GetLength() <= np.x) continue;
			if (' ' == board[np.y][np.x]) continue;
			// recursively map it as neighbour, link first, then recursion
			side.next[d] = { .s = np.s, .t = SAME };				// goes out there
			sides[np.s].next[d^2] = { .s = p.s, .t = SAME };		// comes from here
			map_cube(side_n, np);
		}
	}

	void part2() {
		// calculate size of cube size from board data
		int cube_area = 0;
		// sides must be consecutive (only 11 distinct nets for cube are possible) => Trim = line area
		for (const auto & line : board) cube_area += TrimBoth(line).GetLength();
		int cube_side = sqrt(cube_area / 6);
		M = N = cube_side;
		// map the input board layout onto cube - first six sides with SAME transformation
		cube_side = 0;												// sides mapped (counter)
		Position side0p {};
		while (' ' == board[side0p.y][side0p.x]) ++side0p.x;		// find top-left tile of side 0
		map_cube(cube_side, side0p);								// recursively explore + map neighbours
		ASSERT(6 == cube_side);										// also exactly five edges are defined
		// map flat "unwrapped" input net onto cube by "folding" sides to define all edges
		int edges_to_link = 12 - 5;									// 5 already defined, 12 is total
		while (edges_to_link) {										// keep folding until done
			if (6 <= ++cube_side) cube_side = 0;					// try to fold around next side
			auto & side = sides[cube_side];
			// look for corner with both sides defined and missing link between those two
			for (int corner = 0; corner < 4; ++corner) {
				int d1 = corner, d2 = (corner + 1) & 3;				// R+D, D+L, L+U, U+R
				if (NONE == side.next[d1].t || NONE == side.next[d2].t) continue;
				// first side turns right (+1) (+t transform), second turns left (-1)
				int s1 = side.next[d1].s, s2 = side.next[d2].s;
				int s1d = (d1 + side.next[d1].t + 1) & 3, s2d = (d2 + side.next[d2].t - 1) & 3;
				// so R+D sides could define shared edge right_side.D <-> down_side.R
				if (NONE != sides[s1].next[s1d].t) {				// shared edge defined already
					ASSERT(s1 == sides[s2].next[s2d].s && s2 == sides[s1].next[s1d].s && NONE != sides[s2].next[s2d].t);
					continue;
				}
				// "fold" each side to make them touch (to define the missing link)
				// s1d and s2d are opposite directions to enter the link, so transform should
				// convert s1d to opposite of s2d and s2d to opposite of s1d, ie. +2
				sides[s1].next[s1d] = { .s = s2, .t = (s2d + 2 - s1d) & 3 };
				sides[s2].next[s2d] = { .s = s1, .t = (s1d + 2 - s2d) & 3 };
				--edges_to_link;
			}
		}
		// reset starting position and follow the path
		p = {}, follow_path<2>();
		// transform local side [x,y] into global board position
		p.x += sides[p.s].origin.x, p.y += sides[p.s].origin.y;
		Cout() << "part2: " << (1000 * (p.y+1) + 4 * (p.x+1) + p.d) << EOL;
	}

	void finish() {
		for (auto & line : board) line.Cat(' ', M - line.GetLength()); // make board MxN
		part1();
		part2();
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
	for (const String & arg : CommandLine()) lines_loop(AoC2022Day22Task(), arg);
}
