#include <Core/Core.h>

using namespace Upp;

class AoC2015Day13Task {

	Vector< Vector<int> > weights;
	Vector< SortedVectorMap<int, int> > paths;

	int max_hamilton_circuit() {
		int max_circuit_w = std::numeric_limits<int>::min();
		const int n = paths.GetCount();
		std::vector<bool> seated;
		for (int p1 = 0; p1 < n; ++p1) {		// construct hamilton circuit for every person
			seated.assign(n, false);
			int circuit_w = 0, from = p1;
			for (int to_visit = n; --to_visit;) {
				seated[from] = true;
				for (int p_i = paths[from].GetCount(); p_i--;) {
					if (seated[paths[from][p_i]]) continue;
					circuit_w += paths[from].GetKey(p_i);
					from = paths[from][p_i];
					break;
				}
			}
			// close circuit back to p1
			for (int p_i = paths[from].GetCount(); p_i--;) {
				if (paths[from][p_i] == p1) {
					circuit_w += paths[from].GetKey(p_i);
					break;
				}
			}
			max_circuit_w = max(max_circuit_w, circuit_w);
		}
		return max_circuit_w;
	}

public:

	void init() { Cout() << "***"; weights.Clear(); }

	bool line(const String & line) {
		auto words = Split(~line, ' ');
		if (11 != words.GetCount() || (words[2].Compare("gain") && words[2].Compare("lose"))) return true;
		int p1 = words[0][0] - 'A', p2 = words[10][0] - 'A', w = ScanInt(words[3]);
		if (p1 == 'M'-'A') p1 = 'H'-'A';		// Mallory becomes Henry ;) (0..7)
		if (p2 == 'M'-'A') p2 = 'H'-'A';
		if (words[2].Compare("gain")) w = -w;
		weights.At(p1).At(p2, 0) += w;
		weights.At(p2).At(p1, 0) += w;
		return false;							// not finished yet, try next line
	}

	void finish() {
		const int n = weights.GetCount();
		paths.Clear();
		// merge opposite direction edges into single edge
		for (int p1 = 0; p1 < n - 1; ++p1) for (int p2 = p1 + 1; p2 < n; ++p2) {
			paths.At(p1).Add(weights[p1][p2], p2);
			paths.At(p2).Add(weights[p1][p2], p1);
		}
		int max_happiness = max_hamilton_circuit();
		// add yourself to the graph and find new max happiness
		for (int p = 0; p < n; ++p) paths.At(p).Add(0, n), paths.At(n).Add(0, p);
		int max_p2 = max_hamilton_circuit();
		Cout() << "part1: " << max_happiness << EOL;
		Cout() << "part2: " << max_p2 << EOL;
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
	for (const String & arg : CommandLine()) lines_loop(AoC2015Day13Task(), arg);
} // expected answer: sample: 330 / 286, input: 733 / 725
