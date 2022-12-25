#include <Core/Core.h>

//expected results: sample: 4890 = "2=-1=0", input: 34182852926025 = "2-0-01==0-1=2212=100"

using namespace Upp;

static int8 translate[256];

static void init_translate() {
	memset(translate, +127, sizeof(translate));		// set everything unexpected to +127 as debug
	// use first five items as digit -> char translators
	translate[0] = '0', translate[1] = '1', translate[2] = '2', translate[3] = '=', translate[4] = '-';
	// translate ASCII to digit value
	// from task description: digits are 2, 1, 0, minus (written -), and double-minus (written =).
	translate[0+'='] = -2, translate[0+'-'] = -1, translate[0+'0'] =  0, translate[0+'1'] = +1, translate[0+'2'] = +2;
			//0+ because clang is warning-unhappy about chars being an array subscript. WTF?
};

class AoC2022Day25Task {
	int64 sum = 0;

public:

	void init() { Cout() << "***"; }

	bool line(const String & line) {
		for (int64 pow5 = 1, i = line.GetLength(); i--; pow5 *= 5) sum += translate[line[i]] * pow5;
		return false;
	}

	void finish() {
		String encode = "";
		for (int64 tmp = sum; tmp; ) {				// until whole sum is encoded
			int digit = tmp % 5;
			encode.Insert(0, translate[digit]);
			tmp = (tmp + digit) / 5;
			// "+ digit": digits -2, -1 (remainders 3, 4) after div 5 lend extra +1 to next tmp
			// extra added remainders 0, 1, 2 are reduced to zero after div 5 -> +0 to next tmp
		}
		Cout() << "part1: " << sum << " = \"" << encode << '"' << EOL;
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
	init_translate();
	for (const String & arg : CommandLine()) lines_loop(AoC2022Day25Task(), arg);
}
