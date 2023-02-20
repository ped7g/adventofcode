#include <Core/Core.h>	// expected output: paper 1606483, ribbon 3842356

using namespace Upp;

class AoC2015d02Task {

	int total_paper, total_ribbon;

public:

	void init() {
		Cout() << "***";
		total_paper = total_ribbon = 0;
	}

	bool line(const String & line) {
		int l, w, h;
		if (3 != sscanf(~line, "%dx%dx%d", &l, &w, &h)) return true;	// invalid input
		if (h < l) Swap(h, l);
		if (h < w) Swap(h, w);					// partial sort to l <= h && w <= h
		ASSERT(l <= h && w <= h);
		int paper = 2*l*w + 2*w*h + 2*h*l + l*w, ribbon = 2 * (l + w) + l * w * h;
		total_paper += paper, total_ribbon += ribbon;
		//Cout() << line << " paper = " << paper << " ribbon = " << ribbon << EOL; // DEBUG
		return false;
	}

	void finish() { Cout() << "### paper: " << total_paper << " ribbon: " << total_ribbon << EOL; }
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
	for (const String & arg : CommandLine()) lines_loop(AoC2015d02Task(), arg);
}
