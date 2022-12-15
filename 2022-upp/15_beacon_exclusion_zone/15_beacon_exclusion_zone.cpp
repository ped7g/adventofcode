#include <Core/Core.h>

using namespace Upp;

class Part1 {

	int p1_y = 0;
	Vector<Tuple<int, int>> p1_spans;
	Index<int> p1_beacons;

public:

	void init() { Cout() << "part1"; }

	bool line(const String & line) {
		int Sx, Sy, Bx, By;
		if (4 != sscanf(~line, "Sensor at x=%d, y=%d: closest beacon is at x=%d, y=%d",
						&Sx, &Sy, &Bx, &By)) {
			return (1 != sscanf(~line, "%d", &p1_y));
		}
		if (p1_y == By) p1_beacons.FindAdd(Bx);
		int dx = abs(Bx-Sx) + abs(By-Sy), dy = abs(p1_y-Sy);
		//Sensor at x=8, y=7: closest beacon is at x=2, y=10
		// dx=6+3 = 9, dy=3 (line 10), line 16: dx=9, dy=9
		// span -> Sx-(dx-dy), Sx+(dx-dy)
		//		   8-(10-4) = 2, 8+(10-4) = 14,   8-(10-10) = 8, 8+(10-10) = 8
		int p1_line_power = dx - dy;
		if (p1_line_power < 0) return false;
		p1_spans.Add({Sx - p1_line_power, Sx + p1_line_power});
		return false;							// not finished yet, try next line
	}

	void finish() {
		Sort(p1_spans, [] (const Tuple<int, int> & a, const Tuple<int, int> & b) -> bool { return a.a < b.a; });
		int p1_pos = -p1_beacons.GetCount(), x = p1_spans.IsEmpty() ? 0 : p1_spans[0].a;
		for (auto & s : p1_spans) {
			if (x < s.a) x = s.a;
			if (x <= s.b) p1_pos += 1 + (s.b - x), x = s.b + 1;
		}
		Cout() << "part1: " << p1_pos << EOL;
	}
};

class Part2 {

	int p1_y = 0, p2_sz = 0;
	Vector<Tuple<int, int, int, int>> input;
	Vector<Tuple<int, int>> spans;
	Index<int> beacons;

public:

	void init() { Cout() << "part2"; }

	bool line(const String & line) {
		int Sx, Sy, Bx, By;
		if (4 != sscanf(~line, "Sensor at x=%d, y=%d: closest beacon is at x=%d, y=%d",
						&Sx, &Sy, &Bx, &By)) {
			return (2 != sscanf(~line, "%d %d", &p1_y, &p2_sz));
		}
		input.Add({Sx, Sy, Bx, By});
		return false;							// not finished yet, try next line
	}

	void finish() {
		for (int ly = 0; ly <= p2_sz; ++ly) {
			spans.Clear(), beacons.Clear();
			for (const auto & i : input) {
				if (ly == i.d) beacons.FindAdd(i.c);
				int dx = abs(i.c-i.a) + abs(i.d-i.b), dy = abs(ly-i.b);
				int line_power = dx - dy;
				if (0 <= line_power) spans.Add({i.a - line_power, i.a + line_power});
			}
			Sort(spans, [] (const Tuple<int, int> & a, const Tuple<int, int> & b) -> bool { return a.a < b.a; });
			int x = spans.IsEmpty() ? 0 : std::max(0, spans[0].a);
			for (const auto & s : spans) {
				if (p2_sz < x) break;
				while (x < s.a) {
					Cout() << Format("part2 beacons free at [%d,%d] = ", x, ly) << 4000000ULL * x + ly << EOL;
					++x;
				}
				if (x <= s.b) x = s.b + 1;
			}
		}
		Cout() << "part2 finished" << EOL;
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
	for (const String & arg : CommandLine()) {
		lines_loop(Part1(), arg);
		lines_loop(Part2(), arg);
	}
}
