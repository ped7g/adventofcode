#include <Core/Core.h>

using namespace Upp;

class AoC2015Day17Task {

	Vector<int> containers;

public:

	void init() { Cout() << "***"; }

	bool line(const String & line) {
		if (line.IsEmpty()) return true;
		containers.Add(ScanInt(line));
		return false;							// not finished yet, try next line
	}

	void finish() {
		Sort(containers);
		int eggnog = (55 == Sum(containers)) ? 25 : 150;	// sample.txt is 25L, input 150L
		// part 1 - all possible combinations
		SortedVectorMap<int, int> combinations;
		combinations.Add(0, 1);
		for (int c : containers) for (int i = combinations.GetCount(); i--;) {	// DP adding new combinations
			combinations.GetAdd(combinations.GetKey(i) + c, 0) += combinations[i];
		}
		Cout() << "part1: " << eggnog << "L has combinations: " << combinations.Get(eggnog, -1) << EOL;
		// part 2 - combinations with minimum amount of containers only
		SortedVectorMap<int, Tuple<int, int> > mincomb;
		mincomb.Add(0, {1, 0});
		for (int c : containers) for (int i = mincomb.GetCount(); i--;) {	// DP adding new combinations
			auto from = mincomb[i];	// [combinations, containers]
			auto & to = mincomb.GetAdd(mincomb.GetKey(i) + c, {0, from.b + 1});
			if (to.b < from.b + 1) continue;
			if (from.b + 1 < to.b) to.a = from.a, to.b = from.b + 1;
			else to.a += from.a;
		}
		Cout() << "part2: " << eggnog << "L has [combinations, containers]: "
				<< mincomb.Get(eggnog).a << ", " << mincomb.Get(eggnog).b << EOL;
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
	for (const String & arg : CommandLine()) lines_loop(AoC2015Day17Task(), arg);
} // expected answer: 25L: 4, [3, 2], 150L: 1304, [18, 4]
