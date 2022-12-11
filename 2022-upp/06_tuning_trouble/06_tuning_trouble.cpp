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

// version 2 after spending some time on the algorithm to get strict O(N)

// expected answers: part1: sample.txt: 7,5,6,10,11, input.txt: 1109
//                   part2: sample.txt: 19,23,23,29,26, input.txt: 3965

class PartK {

	const int K;

public:

	PartK(const int k) : K(k) {}

	void init() { Cout() << "part K = " << K; }

	bool line(const String & line) {
		int processed = 0, uniq_from = 0, last_proc[1+'z'-'a'] {};
		for (const char c : line) {
			if (c < 'a' || 'z' < c) throw Exc("invalid input");
			++processed;
			uniq_from = std::max(uniq_from, last_proc[c - 'a']);
			last_proc[c - 'a'] = processed;
			if (K <= processed - uniq_from) {
				Cout() << "processed " << processed << " in line: " << line.Left(10)
					<< "... [" << line.Mid(uniq_from, K) << "]" << EOL;
				return false;
			}
		}
		Cout() << "not found in line: " << line.Left(10) << "..." << EOL;
		return false;
	}

	void finish() {}
};

CONSOLE_APP_MAIN
{
	const Vector<String>& cmdline = CommandLine();
	for(int i = 0; i < cmdline.GetCount(); i++) {
		lines_loop(PartK(4), cmdline[i]);
		lines_loop(PartK(14), cmdline[i]);
	}
}

// version 1 used to submit the answers
/*

class Part1 {	// expected sample.txt: 7,5,6,10,11, input.txt: 1109

public:

	void init() { Cout() << "part1"; }

	bool line(const String & line) {
		int pos = 3-1;	// skip first three chars
		const int e = line.GetLength();
		while (++pos < e) {
			char a = line[pos-3], b = line[pos-2], c = line[pos-1], d = line[pos];
			if (d == a || d == b || d == c) continue;
			if (c == a || c == b) continue;
			if (b == a) continue;
			// four unique chars found
			Cout() << "pos " << pos + 1 << " in line: " << line.Left(5) << "..." << EOL;
			return false;
		}
		Cout() << "not found in line: " << line.Left(5) << "..." << EOL;
		return false;							// not finished yet, try next line
	}

	void finish() { Cout() << "part1 done" << EOL; }
};

class Part2 {	// expected sample.txt: 19,23,23,29,26, input.txt: 3965

public:

	void init() { Cout() << "part2"; }

	bool line(const String & line) {
		int pos = 13-1;	// skip first 13 chars
		const int e = line.GetLength();
		while (++pos < e) {
			uint32 used_chars = 0, m = 0;
			for (int i = pos; i >= pos - 13; --i) {
				ASSERT('a' <= line[i] && line[i] <= 'z');
				m = 1 << (line[i] - 'a');
				if (used_chars & m) break;
				used_chars |= m;
				m = 0;
			}
			if (0 == (used_chars & m)) {	// 14 unique chars found
				Cout() << "pos " << pos + 1 << " in line: " << line.Left(5) << "..." << EOL;
				return false;
			}
		}
		Cout() << "not found in line: " << line.Left(5) << "..." << EOL;
		return false;							// not finished yet, try next line
	}

	void finish() { Cout() << "part2 done" << EOL; }
};

CONSOLE_APP_MAIN
{
	const Vector<String>& cmdline = CommandLine();
	for(int i = 0; i < cmdline.GetCount(); i++) {
		lines_loop(Part1, cmdline[i]);
		lines_loop(Part2, cmdline[i]);
	}
}

*/
