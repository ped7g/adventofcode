#include <Core/Core.h>

using namespace Upp;

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

class Part1and2 {	// expected result: sample [21, 8], input [1845, 230112]

	Vector<String> map;

	class PositionPerHeight {	// helper class to track nearest "this tall or higher" tree

		int positions[10];

	public:

		const int operator[](int height) const { return positions[height]; }

		void set(int height, int position) {	// all smaller trees are hidden by this height
			while (height >= 0) positions[height--] = position;
		}

		void set(int position) {				// all tree heights are set
			set(9, position);
		}

		PositionPerHeight(int position) { set(position); }
	};

public:

	void init() { Cout() << "trees in"; }

	bool line(const String & line) {
		ASSERT(map.IsEmpty() || line.GetLength() == map.Top().GetLength());
		map.Add(line);
		return false;							// not finished yet, try next line
	}

	void finish() {
		// `map` contains full input, now process it
		const int M = map.Top().GetLength(), N = map.GetCount(), MxN = M * N;
		Cout() << Format("processing %d x %d map\n", M, N);
		Vector<int> counter(MxN, 0);			// MxN visibility/counter map
		// resolve visibility from left + top + right
		Vector<char> verticalH(M, ' ');			// height-horizon for vertical checks
		int* row_counter = counter.begin();
		for (const auto & line : map) {
			// resolve visibility from left + top
			char leftH = ' ';					// maximum height from left
			for (int x = 0; x < M; ++x) {
				if (leftH < line[x]) leftH = line[x], row_counter[x] = 1;
				if (verticalH[x] < line[x]) verticalH[x] = line[x], row_counter[x] = 1;
			}
			// resolve visibility from right
			char rightH = ' ';					// maximum height from right
			for (int x = M; x--; ) if (rightH < line[x]) rightH = line[x], row_counter[x] = 1;
			row_counter += M;
		}
		ASSERT(counter.end() == row_counter);
		// resolve visibility from bottom
		verticalH.Set(0, ' ', M);				// reset height-horizont for check from bottom
		for (int y = N; y--; ) {
			const auto & line = map[y];
			row_counter -= M;
			for (int x = 0; x < M; ++x) {
				if (verticalH[x] < line[x]) verticalH[x] = line[x], row_counter[x] = 1;
			}
		}
		// count visible trees and output it as result
		Cout() << "part1: " << Sum(counter) << EOL;

		// part 2 calculate scenic scores
		counter.Set(0, 0, MxN);					// clear visibility from part1
		// resolve scores from left x top x right
		Array<PositionPerHeight> vert_pos(M, PositionPerHeight(0));	// array of M positions
		ASSERT(counter.begin() == row_counter);
		for (int y = 0; y < N; ++y, row_counter += M) {
			const auto & line = map[y];
			// resolve score from left (initial value) x top
			PositionPerHeight horiz_pos = 0;	// left-view positions of trees (per height)
			for (int x = 0; x < M; ++x) {
				const int tree_height = line[x] - '0';
				row_counter[x] = x - horiz_pos[tree_height];
				horiz_pos.set(tree_height, x);
				row_counter[x] *= y - vert_pos[x][tree_height];
				vert_pos[x].set(tree_height, y);
			}
			// resolve score from right
			horiz_pos.set(M - 1);				// right-view positions of trees (per height)
			for (int x = M; x--; ) {
				const int tree_height = line[x] - '0';
				row_counter[x] *= horiz_pos[tree_height] - x;
				horiz_pos.set(tree_height, x);
			}
		}
		ASSERT(counter.end() == row_counter);
		// resolve score from bottom and keep updating max_score
		for (auto & column : vert_pos) column.set(N - 1);	// reset M positions from bottom
		for (int y = N; y--; ) {
			const auto & line = map[y];
			row_counter -= M;
			for (int x = M; x--; ) {
				const int tree_height = line[x] - '0';
				row_counter[x] *= vert_pos[x][tree_height] - y;
				vert_pos[x].set(tree_height, y);
			}
		}
		ASSERT(counter.begin() == row_counter);
		// output as result maximum scenic score of tree
		Cout() << "part2: " << Max(counter) << EOL;
	}
};

CONSOLE_APP_MAIN
{
	const Vector<String>& cmdline = CommandLine();
	for(int i = 0; i < cmdline.GetCount(); i++) {
		lines_loop(Part1and2(), cmdline[i]);
	}
}
