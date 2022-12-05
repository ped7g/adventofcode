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
	FileIn in(GetDataFile(filename));
	if (!in) return;
	task.init();
	Cout() << " input filename: " << filename << EOL;
	while (!task.line(in.GetLine()) && !in.IsEof()) ;
	task.finish();
}

class Part1 {

	long fully_contained { 0 };

public:

	void init() { Cout() << "part1"; }

	void finish() { Cout() << "part1: " << fully_contained << EOL; }

	bool line(const String & line) {
		long r1b,r1e,r2b,r2e;
		if (4 != sscanf(~line, "%ld-%ld,%ld-%ld", &r1b, &r1e, &r2b, &r2e)) return true; // invalid format, terminate
		if ((r2b >= r1b && r2e <= r1e) || (r1b >= r2b && r1e <= r2e)) ++fully_contained;
		return false;							// not finished yet, try next line
	}
};

class Part2 {

	long overlap { 0 };

public:

	void init() { Cout() << "part2"; }

	void finish() { Cout() << "part2: " << overlap << EOL; }

	bool line(const String & line) {
		long r1b,r1e,r2b,r2e;
		if (4 != sscanf(~line, "%ld-%ld,%ld-%ld", &r1b, &r1e, &r2b, &r2e)) return true; // invalid format, terminate
		// ..--.. ..--..
		// --.... ....--
		if (r2e < r1b || r1e < r2b) return false;	// no overlap
		// ..--.. ..--.. ..---.. ..--.. ..--.. ..--.. ..--.. ..--..
		// .--... ...--. ...-... ..-... ...-.. .----. .---.. ..---.
		ASSERT(r2b <= r1e && r1b <= r2e);
		++overlap;
		return false;							// not finished yet, try next line
	}
};

CONSOLE_APP_MAIN
{
	const Vector<String>& cmdline = CommandLine();
	for(int i = 0; i < cmdline.GetCount(); i++) {
		lines_loop(Part1(), cmdline[i]);
		lines_loop(Part2(), cmdline[i]);
	}
}
