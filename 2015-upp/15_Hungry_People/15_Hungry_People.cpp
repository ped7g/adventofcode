#include <Core/Core.h>

using namespace Upp;

struct Ingredient : public Moveable_<Ingredient> {
	String name;
	int cap, dur, flv, tex, cal;
	int units;

	uint64_t getP1Score() const {
		uint64_t score = max(cap, 0);
		score *= max(dur, 0);
		score *= max(flv, 0);
		score *= max(tex, 0);
		return score;
	}

	uint64_t getP2Score() const { return (500 == cal) ? getP1Score() : 0UL; }
};

class AoC2015Day15Task {
	Vector<Ingredient> ingredients;
	uint64_t max_score = 0, max_p2_score = 0;

	void tryIng(int unitsLeft, int i) {
		if (i) {
			for (ingredients[i].units = unitsLeft + 1; ingredients[i].units--;) {
				tryIng(unitsLeft - ingredients[i].units, i - 1);
			}
		} else {
			ingredients[i].units = unitsLeft;
			Ingredient score { .cap = 0, .dur = 0, .flv = 0, .tex = 0, .cal = 0, .units = 0 };
			for (const auto & ing : ingredients) {
				score.cap += ing.cap * ing.units;
				score.dur += ing.dur * ing.units;
				score.flv += ing.flv * ing.units;
				score.tex += ing.tex * ing.units;
				score.cal += ing.cal * ing.units;
				score.units += ing.units;
			}
			ASSERT(100 == score.units);
			max_score = max(max_score, score.getP1Score());
			max_p2_score = max(max_p2_score, score.getP2Score());
		}
	}

public:

	void init() { Cout() << "***"; }

	bool line(const String & line) {
		auto w = Split(~line, ' ');
		if (11 != w.GetCount()) return true;	// unexpected input
		ingredients.Add({
			.name = w[0],
			.cap = ScanInt(w[2]),
			.dur = ScanInt(w[4]),
			.flv = ScanInt(w[6]),
			.tex = ScanInt(w[8]),
			.cal = ScanInt(w[10])
		});
		return false;							// not finished yet, try next line
	}

	void finish() {
		tryIng(100, ingredients.GetCount() - 1);
		Cout() << "part1: " << max_score << EOL;
		Cout() << "part2: " << max_p2_score << EOL;
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
	for (const String & arg : CommandLine()) lines_loop(AoC2015Day15Task(), arg);
} //expected answer for input.txt: part1: 21367368, part2: 1766400
