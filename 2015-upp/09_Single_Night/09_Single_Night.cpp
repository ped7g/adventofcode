#include <Core/Core.h>

using namespace Upp;

class AoC2015Day09Task {
	Index<String> city2idx;
	Vector< SortedVectorMap<int, int> > paths;
	int total_w = 0, min_hamilton_path_w = std::numeric_limits<int>::max(), max_hamilton_path_w = 0;

public:

	void init() { Cout() << "***"; }

	bool line(const String & line) {
		auto in = Split(~line, ' ');
		if (5 != in.GetCount()) return true;	// unexpected input, expected: city1 to city2 = distance
		int c1 = city2idx.FindAdd(in[0]), c2 = city2idx.FindAdd(in[2]), d = ScanInt(in[4]);
		paths.At(c1).Add(d, c2);
		paths.At(c2).Add(d, c1);
		total_w += d;
		return false;							// not finished yet, try next line
	}

	void finish() {
		const int n = city2idx.GetCount();
		std::vector<bool> visited;
		for (int city = 0; city < n; ++city) {	// construct hamilton path for every city
			// part 1 minimal path
			visited.assign(n, false);
			int path_w = 0, from = city;
			for (int to_visit = n; --to_visit;) {
				visited[from] = true;
				int path_n = paths[from].GetCount();
				for (int p_i = 0; p_i < path_n; ++p_i) {
					if (visited[paths[from][p_i]]) continue;
					path_w += paths[from].GetKey(p_i);
					from = paths[from][p_i];
					break;
				}
			}
			min_hamilton_path_w = min(min_hamilton_path_w, path_w);
			// part 2 maximal path
			visited.assign(n, false);
			path_w = 0, from = city;
			for (int to_visit = n; --to_visit;) {
				visited[from] = true;
				for (int p_i = paths[from].GetCount(); p_i--;) {
					if (visited[paths[from][p_i]]) continue;
					path_w += paths[from].GetKey(p_i);
					from = paths[from][p_i];
					break;
				}
			}
			max_hamilton_path_w = max(max_hamilton_path_w, path_w);
		}
		Cout() << "part1: " << min_hamilton_path_w << " (total_w " << total_w << ")\n";
		Cout() << "part2: " << max_hamilton_path_w << " (total_w " << total_w << ")\n";
	}
};	// expected answers: sample 605, 982, input 117, 909

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
	for (const String & arg : CommandLine()) lines_loop(AoC2015Day09Task(), arg);
}
