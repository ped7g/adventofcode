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
		else board.Add(line), M = std::max(M, line.GetLength());
		return IsDigit(line[0]);							// path line terminates input
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
		int li = 0, le = path.GetLength();
		while (li < le) {
			for (int r = atoi(~path + li); r-- && fwd<part>(); ) ;	// move until wall
			while (++li < le && IsDigit(path[li])) ;				// skip input to turn-letter
			p.d = (p.d - ('L' == path[li]) + ('R' == path[li])) & 3;// turn
			++li;
		}
	}

	void part1() {
		sides[0] = { {0, 0} }, N = board.GetCount(), p = {};		// side 0 covers whole global board
		while (' ' == board[p.y][p.x]) ++p.x;						// find top-left tile of board
		follow_path<1>();
		Cout() << "part1: " << (1000 * (p.y+1) + 4 * (p.x+1) + p.d) << EOL;
	}

	void part2() {
		// guess size of cube side plane based on collected board data
		M = N = board.GetCount() <= 4 * 5 ? 4 : 50;	// layout could be at most 5 squares tall
		// map the cube layout on the board data
		/* THIS IS INCORRECT ALGORITHM, it's more tricky than fixed dx,dy -> fixed transformation
		x = 0, y = 0, s = 0;
		while (' ' == board[y][x]) x += N;			// skip empty sides until first side in board
		while (s < 6) {
			ASSERT(y < board.GetCount() && x < board[y].GetLength());
			sides[s].b_x1 = x, sides[s].b_x2 = x + N - 1, sides[s].b_y1 = y, sides[s].b_y2 = y + N - 1;
			for (int ps = 0; ps < s; ++ps) {			// adjust previous sides with info about new one
				static const int transforms[4][7][4] = {
					{	// y-3
						{ D_R , ROTL, D_U , ROTR },		// x-3
						{ D_U , ROTU, D_U , ROTU },		// x-2
						{ D_L , ROTR, D_U , ROTL },		// x-1
						{ D_D , SAME, D_U , SAME },		// x+0
						{ D_R , ROTL, D_U , ROTR },		// x+1
						{ D_U , ROTU, D_U , ROTU },		// x+2
						{ D_L , ROTR, D_U , ROTL }		// x+3
					},
					{	// y-2
						{ D_R , ROTU, D_R , ROTU },		// x-3
						{ NONE, NONE, NONE, NONE },		// x-2
						{ D_L , ROTU, D_L , ROTU },		// x-1
						{ NONE, NONE, NONE, NONE },		// x+0
						{ D_R , ROTU, D_R , ROTU },		// x+1
						{ NONE, NONE, NONE, NONE },		// x+2
						{ D_L , ROTU, D_L , ROTU }		// x+3
					},
					{	// y-1
						{ D_R , ROTL, D_D , ROTR },		// x-3
						{ D_D , ROTU, D_D , ROTU },		// x-2
						{ D_L , ROTR, D_D , ROTL },		// x-1
						{ D_U , SAME, D_D , SAME },		// x+0
						{ D_R , ROTL, D_D , ROTR },		// x+1
						{ D_D , ROTU, D_D , ROTU },		// x+2
						{ D_L , ROTR, D_D , ROTL }		// x+3
					},
					{	// y-0, only left side is relevant, right side will be processed next)
						{ D_R , SAME, D_L , SAME },		// x-3
						{ NONE, NONE, NONE, NONE },		// x-2
						{ D_L , SAME, D_R , SAME },		// x-1
						{ NONE, NONE, NONE, NONE },		// x+0
						{ NONE, NONE, NONE, NONE },		// x+1
						{ NONE, NONE, NONE, NONE },		// x+2
						{ NONE, NONE, NONE, NONE }		// x+3
					},
				};
				for (int dy = -3; dy <= 0; ++dy) for (int dx = -3; dx <= +3; ++dx) {
					auto & t = transforms[3+dy][3+dx];
					if (NONE == t[0]) continue;	// opposite/same side
					if (sides[ps].b_y1 == y + N * dy && sides[ps].b_x1 == x + N * dx) {
						ASSERT(NONE == sides[s].next[t[0]].s && NONE == sides[ps].next[t[2]].s);
						sides[s].next[t[0]] = { ps, t[1] };
						sides[ps].next[t[2]] = { s, t[3] };
					}
				}
			}
			++s;
			do x += N; while (x < board[y].GetLength() && ' ' == board[y][x]);
			if (board[y].GetLength() <= x) x = 0, y += N;
		}
		*/
		// hard-coded layout for sample input and my input file
		if (4 == N) {	// sample input
			// ..0.
			// 123.
			// ..45
			sides[0] = { {0 + 2*N, 0 + 0*N}, { {5,ROTU}, {3,SAME}, {2,ROTR}, {1,ROTU} } };
			sides[1] = { {0 + 0*N, 0 + 1*N}, { {2,SAME}, {4,ROTU}, {5,ROTL}, {0,ROTU} } };
			sides[2] = { {0 + 1*N, 0 + 1*N}, { {3,SAME}, {4,ROTR}, {1,SAME}, {0,ROTL} } };
			sides[3] = { {0 + 2*N, 0 + 1*N}, { {5,ROTL}, {4,SAME}, {2,SAME}, {0,SAME} } };
			sides[4] = { {0 + 2*N, 0 + 2*N}, { {5,SAME}, {1,ROTU}, {2,ROTL}, {3,SAME} } };
			sides[5] = { {0 + 3*N, 0 + 2*N}, { {0,ROTU}, {1,ROTR}, {4,SAME}, {3,ROTR} } };
		} else {
			// .01.
			// .2..
			// 34..
			// 5...
			sides[0] = { {0 + 1*N, 0 + 0*N}, { {1,SAME}, {2,SAME}, {3,ROTU}, {5,ROTL} } };
			sides[1] = { {0 + 2*N, 0 + 0*N}, { {4,ROTU}, {2,ROTL}, {0,SAME}, {5,SAME} } };
			sides[2] = { {0 + 1*N, 0 + 1*N}, { {1,ROTR}, {4,SAME}, {3,ROTR}, {0,SAME} } };
			sides[3] = { {0 + 0*N, 0 + 2*N}, { {4,SAME}, {5,SAME}, {0,ROTU}, {2,ROTL} } };
			sides[4] = { {0 + 1*N, 0 + 2*N}, { {1,ROTU}, {5,ROTL}, {3,SAME}, {2,SAME} } };
			sides[5] = { {0 + 0*N, 0 + 3*N}, { {4,ROTR}, {1,SAME}, {0,ROTR}, {3,SAME} } };
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
