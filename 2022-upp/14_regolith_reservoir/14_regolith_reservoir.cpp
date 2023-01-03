#include <Core/Core.h>

using namespace Upp;	// expected result: sample.txt: 24, 93, input.txt: 638, 31722

// version 2 - instead of simulating each sand unit individual fall, it does use DFS backtrack
// queue to mark/process all paths just once, making the whole runtime about 10x faster

class AoC2022Day14Task_v2 {

	struct Position : Moveable<Position> { short x, y; };

	// X_OFS is tuned for known input data to keep the map size as small as possible (with small margin)
	constexpr static short X_OFS = -300, POUR_X = 500 + X_OFS, POUR_Y = 0, X_SZ = 2 * POUR_X, Y_SZ = 200;
	Vector<String> m { Y_SZ };
	Position min { .x = X_SZ, .y = Y_SZ }, max { .x = 0, .y = 0 };

	void fill_and_remember(const Position & next, Position & previous) {
		m[next.y].Set(next.x, '#');	// mark next [x,y]
		if (-1 == previous.x) {		// no previous coordinates, no line, update previous to next
			previous = next;
			return;
		}
		short dx = sgn(next.x - previous.x), dy = sgn(next.y - previous.y);
		while (previous.x != next.x || previous.y != next.y) {
			m[previous.y].Set(previous.x, '#');
			previous.x += dx, previous.y += dy;
		}
	}

	void mark_and_add(Vector<Position> & queue, const Position & p) {
		if ('.' != m[p.y][p.x]) return;				// no space here
		m[p.y].Set(p.x, '*');						// mark falling sand unit
		queue.Add(p);
	}

public:

	void init() {
		for (auto & my : m) {
			my.Cat('.', X_SZ + 2);	// X_SZ x Y_SZ char map filled with dots (plus "\n\0" for debug)
			my.Set(X_SZ, 10);
			my.Set(X_SZ + 1, 0);
		}
		Cout() << "***";
	}

	bool line(const String & line) {
		const char* l = ~line;
		Position previous_pos { .x = -1, .y = -1 }, pos;
		while (*l) {
			if (!strncmp(" -> ", l, 4)) l += 4;		// skip " -> " ahead of next coordinates
			if (2 != sscanf(l, "%hd,%hd", &pos.x, &pos.y)) return true;	// two coordinats expected
			pos.x += X_OFS;
			min.x = std::min(pos.x, min.x), min.y = std::min(pos.y, min.y);
			max.x = std::max(pos.x, max.x), max.y = std::max(pos.y, max.y);
			fill_and_remember(pos, previous_pos);
			while (IsDigit(*l) || ',' == *l) ++l;	// advance over coordinates
		}
		return false;								// ready for next line
	}

	void finish() {
		// add bottom floor to map
		const short floor_y = max.y + 2;
		Position f1 { .x = 0, .y = floor_y }, f2 { .x = X_SZ - 1, .y = floor_y };
		fill_and_remember(f1, f2);
		// simulate falling sand until filled up to pouring point
		int sand_units = 0, part1 = 0;
		Vector<Position> queue;
		queue.Reserve(X_SZ * Y_SZ);
		mark_and_add(queue, { .x = POUR_X, .y = POUR_Y });	// pouring sand point as seed
		while (!queue.IsEmpty()) {					// until all sand units landed
			const Position t = queue.Top();			// keep copy of coordinates of top of queue
			if ('*' == m[t.y][t.x]) {				// falling sand, try to queue positions below
				m[t.y].Set(t.x, 'o');				// mark it as landed one (processed)
				// marking order is important: +1, -1, +0 to have centre one at top of queue
				mark_and_add(queue, { .x = short(t.x + 1), .y = short(t.y + 1) });
				mark_and_add(queue, { .x = short(t.x - 1), .y = short(t.y + 1) });
				mark_and_add(queue, { .x = short(t.x + 0), .y = short(t.y + 1) });	// top of queue
			} else {								// landed sand, count it
				ASSERT('o' == m[t.y][t.x]);
				if (!part1 && max.y <= t.y) part1 = sand_units;	// remember part1 result (when first sand unit falls below input maxy)
				++sand_units;
				queue.Pop();
			}
		}
		Cout() << "part1: " << part1 << EOL;
		Cout() << "part2: " << sand_units << EOL;
	}
};

// original naive approach, doing whole fall simulation of each sand unit

class AoC2022Day14Task {

	// X_OFS is tuned for known input data to keep the map size as small as possible (with small margin)
	constexpr static int X_OFS = -300, POUR_X = 500 + X_OFS, POUR_Y = 0, X_SZ = 2 * POUR_X, Y_SZ = 200;
	Vector<String> m { Y_SZ };
	int minx = 1000, maxx = 0, miny = 1000, maxy = 0;

	void fill(int x1, int y1, int x2 = -1, int y2 = -1) {
		m[y1].Set(x1, '#');			// mark target [x,y]
		if (-1 == x2) return;		// no previous coordinates, no line
		int dx = sgn(x1 - x2), dy = sgn(y1 - y2);
		while (x2 != x1 || y2 != y1) {
			m[y2].Set(x2, '#');
			x2 += dx, y2 += dy;
		}
	}

public:

	void init() {
		for (auto & my : m) {
			my.Cat('.', X_SZ + 2);	// X_SZ x Y_SZ char map filled with dots (plus "\n\0" for debug)
			my.Set(X_SZ, 10);
			my.Set(X_SZ + 1, 0);
		}
		Cout() << "***";
	}

	bool line(const String & line) {
		const char* l = ~line;
		int px = -1, py = -1, x, y;
		while (*l) {
			if (!strncmp(" -> ", l, 4)) l += 4;	// skip " -> " ahead of next coordinates
			if (2 != sscanf(l, "%d,%d", &x, &y)) return true;	// two coordinats expected
			x += X_OFS;
			minx = std::min(x, minx), maxx = std::max(x, maxx);
			miny = std::min(y, miny), maxy = std::max(y, maxy);
			fill(x, y, px, py);
			px = x, py = y;
			while (IsDigit(*l) || ',' == *l) ++l;				// advance over coordinates
		}
		return false;							// ready for next line
	}

	void finish() {
		fill(0, maxy + 2, X_SZ - 1, maxy + 2);	// bottom floor
		int sand_units = 0, x, y, part1 = 0;
		while ('.' == m[POUR_Y][POUR_X]) {
			x = POUR_X, y = POUR_Y;				// simulate fall from POUR [x,y]
			while ('o' != m[y][x]) {			// until it lands
				for (int dx : {0, -1, +1, 1000}) {
					if (1000 == dx) {			// can't move any more, land it
						if (!part1 && maxy <= y) part1 = sand_units;	// remember part1 result
						m[y].Set(x, 'o');
						++sand_units;
					} else if ('.' == m[y+1][x+dx]) {
						++y;
						x += dx;
						break;
					}
				}
			}
		}
		//for (int y = 0; y < maxy + 3; ++y) Cout() << m[y];
		Cout() << Format("inputs: x %d -> %d, y %d -> %d\n", minx - X_OFS, maxx - X_OFS, miny, maxy);
		Cout() << "part1: " << part1 << EOL;
		Cout() << "part2: " << sand_units << EOL;
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
	for (const String & arg : CommandLine()) lines_loop(AoC2022Day14Task_v2(), arg);
}
