#include <Core/Core.h>

using namespace Upp;

// version 2 refactoring to have some fun with C++20

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

enum {
	UNKNOWN		= 0,

	ROCK		= 1,
	PAPER		= 2,
	SCISSORS	= 3,

	LOSS		= 0,
	DRAW		= 3,
	WIN			= 6,
};

class Part1 {
	int total_score = 0;

	int letter_value(const char l) {
		switch (l) {
			case 'A':	return ROCK;
			case 'B':	return PAPER;
			case 'C':	return SCISSORS;
			case 'X':	return ROCK;
			case 'Y':	return PAPER;
			case 'Z':	return SCISSORS;
		}
		return UNKNOWN;
	}

	int round_result(const int elf, const int you) {
		if (elf == you) return DRAW;
		switch (elf) {
			case ROCK:		return PAPER	== you ? WIN : LOSS;
			case PAPER:		return SCISSORS	== you ? WIN : LOSS;
			case SCISSORS:	return ROCK		== you ? WIN : LOSS;
		}
		return UNKNOWN;
	}

public:

	void init() { Cout() << "part1"; }
	
	void finish() { Cout() << "part1 total score: " << total_score << EOL; }

	bool line(const String & line) {
		if (line.GetLength() < 3) return true;	// wrong input line, terminate
		const int elf = letter_value(line[0]);
		const int you = letter_value(line[2]);
		total_score += round_result(elf, you) + you;
		return false;							// not finished yet, try next line
	}
};

class Part2 {
	int total_score = 0;

	int letter_value(const char l) {
		switch (l) {
			case 'A':	return ROCK;
			case 'B':	return PAPER;
			case 'C':	return SCISSORS;
			case 'X':	return LOSS;
			case 'Y':	return DRAW;
			case 'Z':	return WIN;
		}
		return UNKNOWN;
	}

	int symbol_to_play(const int result, const int elf) {
		if (DRAW == result) return elf;
		switch (elf) {
			//										to win		to lose
			case ROCK:		return WIN == result ? PAPER	: SCISSORS;
			case PAPER:		return WIN == result ? SCISSORS	: ROCK;
			case SCISSORS:	return WIN == result ? ROCK		: PAPER;
		}
		return UNKNOWN;
	}

public:

	void init() { Cout() << "part2"; }
	
	void finish() { Cout() << "part2 total score: " << total_score << EOL; }

	bool line(const String & line) {
		if (line.GetLength() < 3) return true;	// wrong input line, terminate
		const int elf = letter_value(line[0]);
		const int result = letter_value(line[2]);
		total_score += result + symbol_to_play(result, elf);
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

/*
// version 1 used to pass the task

enum {
	UNKNOWN		= 0,
	ROCK		= 1,
	PAPER		= 2,
	SCISSORS	= 3,
	LOSS		= 0,
	DRAW		= 3,
	WIN			= 6,
};

static int p1_symbol_value(const char s) {
	switch (s) {
		case 'A':	return ROCK;
		case 'B':	return PAPER;
		case 'C':	return SCISSORS;
		case 'X':	return ROCK;
		case 'Y':	return PAPER;
		case 'Z':	return SCISSORS;
		default:	return UNKNOWN;
	}
}

static int p1_round_result(const int s1, const int s2) {
	if (s1 == s2) return DRAW;
	switch (s1) {
		case ROCK:		return PAPER	== s2 ? WIN : LOSS;
		case PAPER:		return SCISSORS	== s2 ? WIN : LOSS;
		case SCISSORS:	return ROCK		== s2 ? WIN : LOSS;
		default:		return UNKNOWN;
	}
}

static int p1_round_value(const char s_elf, const char s_you) {
	const int elf = p1_symbol_value(s_elf);
	const int you = p1_symbol_value(s_you);
	return p1_round_result(elf, you) + you;
}

static void part1(const String & filename) {
	FileIn in(GetDataFile(filename));
	if (!in) return;
	Cout() << "p1 input filename: " << filename << EOL;
	int total_score = 0;
	do {
		String line = in.GetLine();
		if (3 <= line.GetLength()) {
			const int score = p1_round_value(line[0], line[2]);
			total_score += score;
			//Cout() << "round: " << line << " = " << score << EOL;
		}
	} while(!in.IsEof());
	Cout() << "p1 total score: " << total_score << EOL;
}

static int p2_symbol_value(const char s) {
	switch (s) {
		case 'A':	return ROCK;
		case 'B':	return PAPER;
		case 'C':	return SCISSORS;
		case 'X':	return LOSS;
		case 'Y':	return DRAW;
		case 'Z':	return WIN;
		default:	return UNKNOWN;
	}
}

static int p2_symbol_to_play(const int result, const int elf) {
	if (DRAW == result) return elf;
	switch (elf) {
		//										to win		to lose
		case ROCK:		return WIN == result ? PAPER	: SCISSORS;
		case PAPER:		return WIN == result ? SCISSORS	: ROCK;
		case SCISSORS:	return WIN == result ? ROCK		: PAPER;
		default:		return UNKNOWN;
	}
}

static int p2_round_value(const char s_elf, const char s_you) {
	const int elf = p2_symbol_value(s_elf);
	const int result = p2_symbol_value(s_you);
	return result + p2_symbol_to_play(result, elf);
}

static void part2(const String & filename) {
	FileIn in(GetDataFile(filename));
	if (!in) return;
	Cout() << "p2 input filename: " << filename << EOL;
	int total_score = 0;
	do {
		String line = in.GetLine();
		if (3 <= line.GetLength()) {
			const int score = p2_round_value(line[0], line[2]);
			total_score += score;
			//Cout() << "round: " << line << " = " << score << EOL;
		}
	} while(!in.IsEof());
	Cout() << "p2 total score: " << total_score << EOL;
}

CONSOLE_APP_MAIN
{
	const Vector<String>& cmdline = CommandLine();
	for(int i = 0; i < cmdline.GetCount(); i++) {
		part1(cmdline[i]);
		part2(cmdline[i]);
	}
}
*/