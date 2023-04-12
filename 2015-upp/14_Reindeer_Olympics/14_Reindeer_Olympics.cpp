#include <Core/Core.h>

using namespace Upp;

class AoC2015Day14Task {

	int max1000 = 0, max2503 = 0;
	Vector< Tuple<int, int, int, int> > reindeers;

	static int getDistance(int time, int speed, int flytime, int rest) {
		int d = time / (flytime + rest) * speed * flytime;
		d += speed * min(time % (flytime + rest), flytime);
		return d;
	}

public:

	void init() { Cout() << "part1"; }

	bool line(const String & line) {
		auto w = Split(~line, ' ');
		if (15 != w.GetCount()) return true;	// unexpected input
		int speed = ScanInt(~w[3]), flytime = ScanInt(~w[6]), rest = ScanInt(~w[13]);
		reindeers.Add({speed, flytime, rest, 0});
		int l1 = getDistance(1000, speed, flytime, rest);
		int l2 = getDistance(2503, speed, flytime, rest);
		max1000 = max(max1000, l1), max2503 = max(max2503, l2);
		return false;							// not finished yet, try next line
	}

	void finish() {
		Cout() << "part1: 1k = " << max1000 << "; 2503 = " << max2503 << EOL;
		// part 2 - go per single second - being lazy, reuse getDistance function... :D
		max1000 = max2503 = 0;
		int n = reindeers.GetCount(), d[n], d_max = 0;
		for (int t = 1; t <= 2503; ++t) {
			for (int r = n; r--;) {	// calculate distance in time t
				d[r] = getDistance(t, reindeers[r].a, reindeers[r].b, reindeers[r].c);
				d_max = max(d_max, d[r]);
			}
			for (int r = n; r--;) {	// add score for reindeers in lead
				reindeers[r].d += (d[r] == d_max);
			}
			if (1000 == t) for (auto & r : reindeers) max1000 = max(max1000, r.d);
			if (2503 == t) for (auto & r : reindeers) max2503 = max(max2503, r.d);
		}
		Cout() << "part2: 1k = " << max1000 << "; 2503 = " << max2503 << EOL;
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
	for (const String & arg : CommandLine()) lines_loop(AoC2015Day14Task(), arg);
} // expected answer: input part1: 1k = 1092; 2503 = 2660 / part2: 1k = 472; 2503 = 1256
