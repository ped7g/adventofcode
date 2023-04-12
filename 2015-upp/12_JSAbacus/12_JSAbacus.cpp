#include <Core/Core.h>

using namespace Upp;

class Part1 {

	int sum = 0;

public:

	void init() { Cout() << "part1"; }

	bool line(const String & line) {
		bool wasDigit = false;
		for (int i = line.GetCount(); i--;) {
			if (i && IsDigit(line[i - 1])) continue;
			if ('-' != line[i] && !(wasDigit = IsDigit(line[i]))) continue;
			if (wasDigit && i && '-' == line[i - 1]) continue;
			if ('-' == line[i] && !wasDigit) continue;
			sum += atoi(~line + i);
		}
		return false;							// not finished yet, try next line
	}

	void finish() { Cout() << "part1: " << sum << EOL; }
};

class Part2 {

	int sum = 0;

	static bool hasRed(const String & s, int i) {
		if (':' != s[i] || s.GetCount() <= i + 5) return false;
		if ('"' != s[i+5] || '"' != s[i+1]) return false;
		return ('d' == s[i+4] && 'e' == s[i+3] && 'r' == s[i+2]);
	}

	int sumObjArr(const String & line, int & i) {
		ASSERT(0 < i && ']' == line[i] || '}' == line[i]);
		bool isObj = ('}' == line[i--]), isRed = false;
		int sum = 0;
		for (;0 <= i && line[i] != (isObj ? '{' : '['); --i) {
			while (0 < i && '}' == line[i] || ']' == line[i]) sum += sumObjArr(line, i);
			if (line[i] == (isObj ? '{' : '[')) break;
			if (isObj) isRed |= hasRed(line, i);
			while (IsDigit(line[i]) && i && (IsDigit(line[i - 1]) || '-' == line[i - 1])) --i;
			if ('-' == line[i] && IsDigit(line[i+1]) || IsDigit(line[i])) sum += atoi(~line + i);
		}
		ASSERT(0 <= i && line[i] == (isObj ? '{' : '['));
		return --i, isRed ? 0 : sum;
	}

public:

	void init() { Cout() << "part2"; }

	bool line(const String & line) {
		int i = line.GetCount() - 1;
		if (i < 1 || ('}' != line[i] && ']' != line[i])) return true;
		sum += sumObjArr(line, i);
		return false;							// not finished yet, try next line
	}

	void finish() { Cout() << "part2: " << sum << EOL; }
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
	for (const String & arg : CommandLine()) {
		lines_loop(Part1(), arg);
		lines_loop(Part2(), arg);
	}
} // expected answer: part1: 156366, part2: 96852
