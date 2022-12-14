#include <Core/Core.h>

using namespace Upp;

class Part1 {	// expected result: sample.txt: 24, 93, input.txt: 638, 31722

	constexpr static int X_OFS = -300, X_SZ = 2 * (500 + X_OFS), Y_SZ = 200;
	Vector<Vector<uint8>> m { Y_SZ };
	int minx = 1000, maxx = 0, miny = 1000, maxy = 0;

public:

	void init() {
		for (auto & my : m) {
			my.Reserve(X_SZ + 10);
			my.Insert(0, '.', X_SZ);	// X_SZ x Y_SZ uint8 map
			my.Add(10);
			my.Add(0);
		}
		Cout() << "part1";
	}

	bool line(const String & line) {
		const char* l = ~line;
		int px = -1, py = -1;
		do {
			int x, y;
			if (2 != sscanf(l, "%d,%d", &x, &y)) return true;
			x += X_OFS;
			minx = std::min(x, minx); maxx = std::max(x, maxx);
			miny = std::min(y, miny); maxy = std::max(y, maxy);
			m[y][x] = '#';
			if (0 <= px) {
				int dx = sgn(x - px), dy = sgn(y - py);
				do {
					m[py][px] = '#';
					px += dx;
					py += dy;
				} while (px != x || py != y);
			}
			while (IsDigit(*l) || ',' == *l) ++l;
			if (!*l) return false;
			if (strncmp(" -> ", l, 4)) return true;
			l += 4;
			px = x, py = y;
		} while(true);
		return false;							// not finished yet, try next line
	}

	void finish() {
		m[maxy + 2].Set(0, '#', X_SZ);
		maxy += 4;
		int sand_units = 0, x, y;
		do {
			x = 500 + X_OFS, y = 0;
			if ('o' == m[y][x]) break;
			while (y < maxy && ('o' != m[y][x])) {
				for (int dx : {0, -1, +1, 1000}) {
					if (1000 == dx) {
						m[y][x] = 'o';
						++sand_units;
						break;
					}
					if ('.' == m[y+1][x+dx]) {
						++y;
						x += dx;
						break;
					}
				}
			}
		} while (y < maxy);

		//for (int y = 0; y < maxy; ++y) { Cout() << (const char*)(&m[y][0]); }
		Cout() << Format("x %d -> %d, y %d -> %d\n", minx, maxx, miny, maxy);
		Cout() << "part1: " << sand_units << EOL;
	}
};

class Part2 {

public:

	void init() { Cout() << "part2"; }

	bool line(const String & line) {
		return false;							// not finished yet, try next line
	}

	void finish() { Cout() << "part2: " << 0 << EOL; }
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
	for (const String & arg : CommandLine()) {
		lines_loop(Part1(), arg);
		lines_loop(Part2(), arg);
	}
}
