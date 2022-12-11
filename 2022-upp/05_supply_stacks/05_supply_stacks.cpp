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

class Stacks {

private:
	Vector<String> lines_;

public:
	Vector<String> stacks;

	bool line(const String & line) {
		if (!stacks.IsEmpty()) return true;		// finished already, if stacks exists
		// collect input lines until the empty line
		if (!line.IsEmpty()) {
			lines_.Add(line);
			return false;
		}
		// parse stacks input, last stored should contain numbers of stacks
		int n = lines_.Pop().GetLength()/4 + 1;	// ignore content, calculate N from length
		Cout() << "stacks n = " << n << EOL;	ASSERT(1 <= n && n <= 9);
		stacks.AddN(n);							// init N empty strings (stacks)
		// add crates to stacks from bottom to up
		while (lines_.GetCount()) {
			auto crates = lines_.Pop();			ASSERT(4*n - 1 == crates.GetLength());
			for (int i = 0; i < n; ++i) {
				char crate = crates[1 + 4*i];
				if (crate != ' ') stacks[i].Cat(crate);
			}
		}
		return false;							// false to skip empty line in main loop
	}
};

class Part1 {	// expected results: sample = CMZ, input = FCVRLMVQP

	Stacks s_;

public:

	void init() { Cout() << "part1"; }

	bool line(const String & line) {
		// read stacks from input until the commands are given
		if (!s_.line(line)) return false;
		// follow the commands
		int count, fromI, toI;
		if (3 != sscanf(line, "move %d from %d to %d", &count, &fromI, &toI)) return true;	// bad input
		// process command
		auto & from = s_.stacks[fromI - 1], & to = s_.stacks[toI - 1];
		int i = from.GetLength(), stop = i - count;	ASSERT(count <= i);
		while (stop < i) to.Cat(from[--i]);		// append one by one
		from.TrimLast(count);					// remove appended
		return false;							// not finished yet, try next line
	}

	void finish() {
		Cout() << "part1: [";					// print top crate in each stack
		for (const auto & s : s_.stacks) Cout() << *s.Last();
		Cout() << "]" << EOL;
	}
};

class Part2 {	// expected results: sample = MCD, input = RWLWGJGFD

	Stacks s_;

public:

	void init() { Cout() << "part2"; }

	bool line(const String & line) {
		// read stacks from input until the commands are given
		if (!s_.line(line)) return false;
		// follow the commands
		int count, fromI, toI;
		if (3 != sscanf(line, "move %d from %d to %d", &count, &fromI, &toI)) return true;	// bad input
		// process command
		auto & from = s_.stacks[fromI - 1], & to = s_.stacks[toI - 1];
		to.Cat(from.Right(count));				// append count-crates
		from.TrimLast(count);					// remove appended
		return false;							// not finished yet, try next line
	}

	void finish() {
		Cout() << "part2: [";					// print top crate in each stack
		for (const auto & s : s_.stacks) Cout() << *s.Last();
		Cout() << "]" << EOL;
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
