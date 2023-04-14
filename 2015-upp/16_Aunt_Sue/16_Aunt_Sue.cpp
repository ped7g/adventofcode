#include <Core/Core.h>

using namespace Upp;

class AoC2015Day16Task {

	static constexpr int COMPOUNDS_N = 10;
	static inline const String COMPOUNDS[COMPOUNDS_N] = {
		"children", "samoyeds", "akitas", "cars", "perfumes",
		"vizslas", "pomeranians", "goldfish", "cats", "trees"
	};
	Vector< Vector<int> > aunts;

	static int auntCompare(const Vector<int> & a, const Vector<int> & b) {
		ASSERT(COMPOUNDS_N == a.GetCount() && COMPOUNDS_N == b.GetCount());
		for (int i = COMPOUNDS_N; i--;)
			if (-1 == a[i] || -1 == b[i] || a[i] == b[i]) continue; else return a[i] - b[i];
		return 0;
	}

	static int auntCompareP2(const Vector<int> & a, const Vector<int> & b) {
		for (int i = COMPOUNDS_N - 4; i--;)
			if (-1 == a[i] || -1 == b[i] || a[i] == b[i]) continue; else return a[i] - b[i];
		for (int i : { COMPOUNDS_N - 4, COMPOUNDS_N - 3 })
			if (-1 == a[i] || -1 == b[i] || b[i] < a[i]) continue; else return 100 + b[i];
		for (int i : { COMPOUNDS_N - 2, COMPOUNDS_N - 1 })
			if (-1 == a[i] || -1 == b[i] || a[i] < b[i]) continue; else return 200 + b[i];
		return 0;
	}

public:

	void init() {
		Cout() << "***";
		line("Sue X: children: 3, cats: 7, samoyeds: 2, pomeranians: 3, akitas: 0, vizslas: 0, goldfish: 5, trees: 3, cars: 2, perfumes: 1");
	}

	bool line(const String & line) {
		const auto w = Split(~line, ' ');
		if (w.GetCount() < 4) return true;		// invalid input
		Vector<int> a(COMPOUNDS_N, -1);
		for (int wi = 2; wi < w.GetCount(); wi += 2) for (int ci = 0; ci < COMPOUNDS_N; ++ci) {
			if (w[wi].StartsWith(COMPOUNDS[ci])) a[ci] = ScanInt(w[wi+1]);
		}
		aunts.Add(std::move(a));
		return false;							// not finished yet, try next line
	}

	void finish() {
		for (int ai = aunts.GetCount(); --ai;) {
			if (!auntCompare(aunts[0], aunts[ai])) Cout() << "part1 match with aunt " << ai << EOL;
			if (!auntCompareP2(aunts[0], aunts[ai])) Cout() << "part2 match with aunt " << ai << EOL;
		}
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
	for (const String & arg : CommandLine()) lines_loop(AoC2015Day16Task(), arg);
} // expected answer: part 1: 103, part 2: 405
