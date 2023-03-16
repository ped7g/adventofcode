#include <Core/Core.h>

using namespace Upp;

class AoC2015Day04Task {

public:

	void init() { Cout() << "***"; }

	bool line(const String & line) {
		bool p1found = false;
		for (int64 i = 1; i; ++i) {
			String md5s = MD5String(line + IntStr64(i));
			if (!p1found && md5s.StartsWith("00000", 5)) {
				Cout() << line << " with " << i << " has 5 zeroes: " << md5s << EOL;
				p1found = true;
			}
			if (md5s.StartsWith("000000", 6)) {
				Cout() << line << " with " << i << " has 6 zeroes: " << md5s << EOL;
				break;
			}
		}
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
	for (const String & arg : CommandLine()) lines_loop(AoC2015Day04Task(), arg);
}
