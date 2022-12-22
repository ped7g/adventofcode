#include <Core/Core.h>

using namespace Upp;	// expected answer: sample [6032, 5031], input [27492, 78291]

enum { D_R = 0, D_D = 1, D_L = 2, D_U = 3, SAME = 0x00, ROTL = 0x01, ROTR = 0x02, ROTU = 0x04, NONE = 0xFF };

static constexpr int m[4][2] = { { +1,  0 }, {  0, +1 }, { -1,  0 }, {  0, -1 } };

struct CoordinateSystem {
	int s = NONE;				// side number
	int t = NONE;				// required transformation while wrapping around
};

struct Side {
	int b_x1, b_x2, b_y1, b_y2;	// valid range of char data in board array for this side
	CoordinateSystem next[4];	// sides at right, down, left, up (like directions are mapped)
};

class AoC2022Day22Task {
	Vector<String> board;
	String path;
	int x = 0, y = 0, d = 0, max_l = 0, N, Nm, s = 0;
	Side sides[6];

public:

	void init() { Cout() << "***"; }

	bool p1_fwd() {
		int nx = x, ny = y;
		do {
			nx += m[d][0], ny += m[d][1];
			if (ny < 0) ny = board.GetCount() - 1;
			if (board.GetCount() <= ny) ny = 0;
			if (nx < 0) nx = board[ny].GetLength() - 1;
			if (board[ny].GetLength() <= nx) nx = 0;
		} while (' ' == board[ny][nx]);
		if ('.' == board[ny][nx]) x = nx, y = ny;
		else ASSERT('#' == board[ny][nx]);
		return ('.' == board[ny][nx]);
	}

	void p2_wrap(int & s, int & x, int & y, int & d) {
		ASSERT((D_R == d && x == N) || (D_D == d && y == N) || (D_L == d && x == -1) || (D_U == d && y == -1));
		int tx = (x + N) % N, ty = (y + N) % N, td = d;	// wrapped coordinates for SAME transformation
		switch (sides[s].next[td].t) {
			case ROTL:	y = tx,		x = Nm-ty,	d = (td+1)&3;	break;
			case ROTR:	y = Nm-tx,	x = ty,		d = (td-1)&3;	break;
			case ROTU:	y = Nm-ty,	x = Nm-tx,	d = (td+2)&3;	break;
			case SAME:	y = ty,		x = tx;		d = td;			break;
			default:							ASSERT(false);
		}
		s = sides[s].next[td].s;					// new side
	}

	bool p2_fwd() {
		int nx = x, ny = y, ns = s, nd = d;
		nx += m[nd][0], ny += m[nd][1];
		if (ny < 0 || N <= ny || nx < 0 || N <= nx) p2_wrap(ns, nx, ny, nd);
		char n_tile = board[sides[ns].b_y1 + ny][sides[ns].b_x1 + nx];
		if ('.' == n_tile) s = ns, x = nx, y = ny, d = nd;
		else ASSERT('#' == n_tile);
		return ('.' == n_tile);
	}

	bool line(const String & line) {
		if (line.IsEmpty()) return false;
		if (!IsDigit(line[0])) {
			board.Add(line);
			max_l = std::max(max_l, line.GetLength());
		} else path = line;
		return IsDigit(line[0]);									// path line terminates input
	}

	void part1() {
		// extend board on each line with space
		for (auto & line : board) line.Cat(' ', max_l - line.GetLength());
		int li = 0, le = path.GetLength();
		while (' ' == board[y][x]) ++x;								// find top-left tile of board
		while (li < le) {
			for (int r = atoi(~path + li); r-- && p1_fwd(); ) ;		// move until wall
			while (++li < le && IsDigit(path[li])) ;				// skip input to turn-letter
			d = (d - ('L' == path[li]) + ('R' == path[li])) & 3;	// turn
			++li;
		}
		int password = 1000 * (y+1) + 4 * (x+1) + d;
		Cout() << "part1: " << password << EOL;
	}

	void part2() {
		// guess size of cube side plane based on collected board data
		N = board.GetCount() <= 4 * 5 ? 4 : 50;		// layout could be at most 5 squares tall
		Nm = N - 1;
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
			sides[0] = { 0 + 2*N, -1 + 3*N, 0 + 0*N, -1 + 1*N, { {5,ROTU}, {3,SAME}, {2,ROTR}, {1,ROTU} } };
			sides[1] = { 0 + 0*N, -1 + 1*N, 0 + 1*N, -1 + 2*N, { {2,SAME}, {4,ROTU}, {5,ROTL}, {0,ROTU} } };
			sides[2] = { 0 + 1*N, -1 + 2*N, 0 + 1*N, -1 + 2*N, { {3,SAME}, {4,ROTR}, {1,SAME}, {0,ROTL} } };
			sides[3] = { 0 + 2*N, -1 + 3*N, 0 + 1*N, -1 + 2*N, { {5,ROTL}, {4,SAME}, {2,SAME}, {0,SAME} } };
			sides[4] = { 0 + 2*N, -1 + 3*N, 0 + 2*N, -1 + 3*N, { {5,SAME}, {1,ROTU}, {2,ROTL}, {3,SAME} } };
			sides[5] = { 0 + 3*N, -1 + 4*N, 0 + 2*N, -1 + 3*N, { {0,ROTU}, {1,ROTR}, {4,SAME}, {3,ROTR} } };
		} else {
			// .01.
			// .2..
			// 34..
			// 5...
			sides[0] = { 0 + 1*N, -1 + 2*N, 0 + 0*N, -1 + 1*N, { {1,SAME}, {2,SAME}, {3,ROTU}, {5,ROTL} } };
			sides[1] = { 0 + 2*N, -1 + 3*N, 0 + 0*N, -1 + 1*N, { {4,ROTU}, {2,ROTL}, {0,SAME}, {5,SAME} } };
			sides[2] = { 0 + 1*N, -1 + 2*N, 0 + 1*N, -1 + 2*N, { {1,ROTR}, {4,SAME}, {3,ROTR}, {0,SAME} } };
			sides[3] = { 0 + 0*N, -1 + 1*N, 0 + 2*N, -1 + 3*N, { {4,SAME}, {5,SAME}, {0,ROTU}, {2,ROTL} } };
			sides[4] = { 0 + 1*N, -1 + 2*N, 0 + 2*N, -1 + 3*N, { {1,ROTU}, {5,ROTL}, {3,SAME}, {2,SAME} } };
			sides[5] = { 0 + 0*N, -1 + 1*N, 0 + 3*N, -1 + 4*N, { {4,ROTR}, {1,SAME}, {0,ROTR}, {3,SAME} } };
		}
		// follow the path
		x = 0, y = 0, d = 0, s = 0;		// start position is at side 0, x = 0, y = 0, d = 0
		int li = 0, le = path.GetLength();
		while (li < le) {
			for (int r = atoi(~path + li); r-- && p2_fwd(); ) ;		// move until wall
			while (++li < le && IsDigit(path[li])) ;				// skip input to turn-letter
			d = (d - ('L' == path[li]) + ('R' == path[li])) & 3;	// turn
			++li;
		}
		// transform local side x,y,d into global board position
		x += sides[s].b_x1;
		y += sides[s].b_y1;
		int password = 1000 * (y+1) + 4 * (x+1) + d;
		Cout() << "part2: " << password << EOL;
	}

	void finish() {
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
