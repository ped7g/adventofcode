#include <Core/Core.h>

using namespace Upp;

class AocTask {	// expected answer sample: 13, 140, input: 6420, 22000

	int p1_sum = 0, pairI = 0, p2_divider_2 = 1, p2_divider_6 = 2;
	String left;

	// like-lexicographic type of code, quite tricky to write and debug, not recommended for
	// "production" in real projects, but if you need every little bit of performance and can
	// afford to cover this type of code with extensive unit tests and extra budget... :)
	static int lex_compare(const char* l, const char* lE, const char* r, const char* rE) {
		int li = 0, ri = 0, cmp;						// left/right integers boxed into fake list
		while (l < lE && r < rE) {
			if (',' == *l && ',' == *r) ++l, ++r;		// skip paired commas
			while ('[' == *l && '[' == *r) ++l, ++r;	// open paired lists
			if (IsDigit(*l) || IsDigit(*r)) {
				while ('[' == *l) { ++ri, ++l; }		// box single integer into fake list
				while ('[' == *r) { ++li, ++r; }
				if (']' == *l || ']' == *r) return (']' == *r) - (']' == *l);	// shorter list at one side
				ASSERT(IsDigit(*l) && IsDigit(*r));		// integer vs integer
				auto a = l, b = r;						// remember start of integers
				while (IsDigit(*l) && IsDigit(*r)) ++l, ++r;			// find end of one
				if (0 != (cmp = IsDigit(*l) - IsDigit(*r))) return cmp;	// longer is bigger
				while (a < l && *a == *b) ++a, ++b;		// same length, skip same digits
				if (a < l) return *a - *b;				// some digit is different => result
			}
			while (li && ']' == *r) --li, ++r;			// pair boxed-integer-end vs real-list-end
			while (ri && ']' == *l) --ri, ++l;
			if (li || ri) return ri - li;				// other list is longer (didn't fully unbox)
			while (']' == *l && ']' == *r) ++l, ++r;	// all these lists did match (both real ends)
			if (']' == *l || '[' == *r) return -1;		// left list is smaller (open/close don't match)
			if (']' == *r || '[' == *l) return +1;		// right list is smaller
			ASSERT((l == lE && r == rE) || (',' == *l && ',' == *r));	// expected commas or end of strings
		}
		ASSERT(l == lE && r == rE);
		return 0;										// whole left did match whole right string
	}

	static bool less(const String & a, const String & b) {
		return lex_compare(a.Begin(), a.End(), b.Begin(), b.End()) < 0;
	}

public:

	void init() { Cout() << "***"; }

	bool line(const String & line) {
		if (line.IsEmpty()) return false;
		// part 2 - tracking position of divider packets
		p2_divider_2 += less(line, "[[2]]"), p2_divider_6 += less(line, "[[6]]");
		// part 1 - tick-tock left/right string, update sum if left < right
		if (left.IsEmpty()) left = line;		// left
		else {									// right
			++pairI;
			if (less(left, line)) p1_sum += pairI;
			left.Clear();
		}
		return false;							// not finished yet, try next line
	}

	void finish() {
		Cout() << "part1 sum of indices: " << p1_sum << EOL;
		Cout() << "part2 decoder key: " << p2_divider_2 * p2_divider_6 << EOL;
	}
};

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

CONSOLE_APP_MAIN {
	for (const String & arg : CommandLine()) lines_loop(AocTask(), arg);
}
