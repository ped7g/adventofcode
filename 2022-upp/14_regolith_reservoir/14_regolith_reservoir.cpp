#include <Core/Core.h>

using namespace Upp;

class AoC2022Day14Task {	// expected result: sample.txt: 24, 93, input.txt: 638, 31722

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
		int sand_units = 0, x, y, part1 = maxy;
		while ('.' == m[POUR_Y][POUR_X]) {
			x = POUR_X, y = POUR_Y;				// simulate fall from POUR [x,y]
			while ('o' != m[y][x]) {			// until it lands
				for (int dx : {0, -1, +1, 1000}) {
					if (1000 == dx) {			// can't move any more, land it
						if (part1 <= y) part1 = sand_units;		// remember part1 result
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
	for (const String & arg : CommandLine()) lines_loop(AoC2022Day14Task(), arg);
}
