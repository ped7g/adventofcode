#include <Core/Core.h>

using namespace Upp;

class Aoc2015Day10Task {

public:

	void init() { Cout() << "***"; }

	bool line(const String & line) {
		String in, out = line;
		int l40;
		for (int i = 50; i--;) {
			in = out, out.Clear();
			for (int j = 0; j < in.GetCount();) {
				const char a = in[j];
				int c = 1;
				while (j + c < in.GetCount() && a == in[j + c]) ++c;
				out.Cat(AsString(c));
				out.Cat(a);
				j += c;
			}
			if (i == 50 - 40) l40 = out.GetCount();
		}
		Cout() << Format("\"%s\": 40x %d 50x %d\n", line, l40, out.GetCount());
		return false;							// not finished yet, try next line
	}

	void finish() {}
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
	for (const String & arg : CommandLine()) lines_loop(Aoc2015Day10Task(), arg);
} // expected output: "1113122113": 40x 360154 50x 5103798
