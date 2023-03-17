#include <Core/Core.h>
#include <numeric>

using namespace Upp;

static constexpr int MAX_SZ_D1 = 1000, MAX_SZ = MAX_SZ_D1 * MAX_SZ_D1;

class Part1 {

protected:

	Buffer<uint8> map;

	virtual void patch_rect(int command, int* r) {
		const int & x1 = r[0], & y1 = r[1], & x2 = r[2], & y2 = r[3];
		ASSERT(x1 <= x2 && y1 <= y2 && 0 <= x1 && x2 <= 999 && 0 <= y1 && y2 <= 999);
		if (x2 < x1 || y2 < y1) return;
		if (1 == command) for (int y = y1; y <= y2; ++y) for (int x = x1; x <= x2; ++x) map[y * MAX_SZ_D1 + x] = 1;
		else if (0 == command) for (int y = y1; y <= y2; ++y) for (int x = x1; x <= x2; ++x) map[y * MAX_SZ_D1 + x] = 0;
		else if (-1 == command) for (int y = y1; y <= y2; ++y) for (int x = x1; x <= x2; ++x) map[y * MAX_SZ_D1 + x] = 1 - map[y * MAX_SZ_D1 + x];
	}

public:

	Part1() : map(MAX_SZ, 0) {}

	virtual void init() { Cout() << "part1"; }

	bool line(const String & line) {
		int rect[4] { 1000, 1000, -1, -1}, i = 0, lsz = line.GetCount();
		for (int* pos : { rect+0, rect+1, rect+2, rect+3 }) {	// read four coordinates
			while (i < lsz && !IsDigit(line[i])) ++i;
			if (lsz <= i) return true;				// terminate, missing coordinate
			*pos = atoi(~line + i);
			while (i < lsz && IsDigit(line[i])) ++i;
		}
		if (line.StartsWith("turn on"))			patch_rect(1, rect);
		else if (line.StartsWith("turn off"))	patch_rect(0, rect);
		else if (line.StartsWith("toggle"))		patch_rect(-1, rect);
		return false;							// not finished yet, try next line
	}

	virtual void finish() {
		Cout() << "part1: " << std::accumulate(map.begin(), map.begin() + MAX_SZ, 0) << EOL;
	}
};

class Part2 : public Part1 {

	void patch_rect(int command, int* r) override {
		const int & x1 = r[0], & y1 = r[1], & x2 = r[2], & y2 = r[3];
		ASSERT(x1 <= x2 && y1 <= y2 && 0 <= x1 && x2 <= 999 && 0 <= y1 && y2 <= 999);
		if (x2 < x1 || y2 < y1) return;
		if (1 == command) for (int y = y1; y <= y2; ++y) for (int x = x1; x <= x2; ++x) ++map[y * MAX_SZ_D1 + x];
		else if (0 == command) for (int y = y1; y <= y2; ++y) for (int x = x1; x <= x2; ++x) map[y * MAX_SZ_D1 + x] -= 0 < map[y * MAX_SZ_D1 + x];
		else if (-1 == command) for (int y = y1; y <= y2; ++y) for (int x = x1; x <= x2; ++x) map[y * MAX_SZ_D1 + x] += 2;
	}

public:

	void init() override { Cout() << "part2"; }

	void finish() override {
		Cout() << "part2: " << std::accumulate(map.begin(), map.begin() + MAX_SZ, 0) << EOL;
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
	for (const String & arg : CommandLine()) {
		lines_loop(Part1(), arg);
		lines_loop(Part2(), arg);
	}
}
