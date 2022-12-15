#include <Core/Core.h>

using namespace Upp;

struct Position : Moveable<Position> {
	int x, y;

	int manhattan_d(const Position & b) const {
		return abs(x - b.x) + abs(y - b.y);
	}
};

struct Span : Moveable<Span> {
	int from, to;

	Span(int f, int t) : from(f), to(t) {}

	static bool less(const Span & a, const Span & b) {
		if (a.from == b.from) return b.to < a.to;			// longer span for same "from"
		return a.from < b.from;								// order spans by "from"
	}
};

class Part1 {

	int p1_y = 0;				// *the* part 1 answer line Y (read from patched input file)
	Vector<Span> spans;			// [close, open) spans of coverage at line Y
	Index<int> beacons;			// all unique beacons (X coordinates) at line Y

public:

	void init() { Cout() << "part1"; }

	bool line(const String & line) {
		Position s, b;
		if (4 != sscanf(~line, "Sensor at x=%d, y=%d: closest beacon is at x=%d, y=%d",
						&s.x, &s.y, &b.x, &b.y)) {
			return (1 != sscanf(~line, "%d", &p1_y));		// custom input: first line = Y
		}
		if (p1_y == b.y) beacons.FindAdd(b.x);				// track all unique beacons at Y
		int line_power = b.manhattan_d(s) - abs(p1_y-s.y);	// sensor X-range at Y
		if (0 <= line_power) spans.Add({s.x - line_power, s.x + line_power + 1});
		return false;										// not finished yet, try next line
	}

	void finish() {
		Sort(spans, Span::less);
		int covered_pos = -beacons.GetCount(), x = spans.IsEmpty() ? 0 : spans[0].from;
		for (auto & s : spans) {
			if (x < s.from) x = s.from;						// skip uncovered area
			if (x < s.to) covered_pos += s.to - x, x = s.to;
		}
		Cout() << "part1: " << covered_pos << EOL;
	}
};

class Part2 {

	int p1_y = 0, p2_sz = 0;	// part 2 size of area of interest
	Vector<Tuple<Position, Position>> input;
	Vector<Span> spans;
	Index<int> beacons;

public:

	void init() { Cout() << "part2"; }

	bool line(const String & line) {
		Position s, b;
		if (4 != sscanf(~line, "Sensor at x=%d, y=%d: closest beacon is at x=%d, y=%d",
						&s.x, &s.y, &b.x, &b.y)) {
			// custom input: first line: part1_y part2_area_size
			return (2 != sscanf(~line, "%d %d", &p1_y, &p2_sz));
		}
		input.Add({s, b});		// just record all sensor + beacon positions
		return false;
	}

	void finish() {
		for (int ly = 0; ly <= p2_sz; ++ly) {
			spans.Clear(), beacons.Clear();
			for (const auto & i : input) {
				if (ly == i.b.y) beacons.FindAdd(i.b.x);	// track all unique beacons at ly
				int line_power = i.b.manhattan_d(i.a) - abs(ly-i.a.y);	// sensor X-range at ly
				if (0 <= line_power) spans.Add({i.a.x - line_power, i.a.x + line_power + 1});
			}
			ASSERT(!spans.IsEmpty());						// this would cause 4mil of output
			Sort(spans, Span::less);
			int x = 0;
			for (const auto & s : spans) {					// look for uncovered position
				if (p2_sz < x) break;						// whole range 0..p2_sz checked
				while (x < s.from) {			// uncovered position detected, print it out
					Cout() << Format("part2 beacon can be at [%d,%d] = %lld", x, ly, 4000000LL * x + ly) << EOL;
					++x;
				}
				if (x < s.to) x = s.to;
			}
		}
		Cout() << "part2 finished (full scan)" << EOL;
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
