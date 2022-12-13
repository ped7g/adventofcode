#include <Core/Core.h>

using namespace Upp;

class AocTask {	// expected answer sample: 13, 140, input: 6420, 22000

	int p1_sum = 0, pairI = 0;
	Vector<String> packets { "[[2]]", "[[6]]" };

	static const char* find_closing(const char* c, const char* cE) {
		while (c < cE && ']' != *c) if ('[' == *c++) c = find_closing(c, cE) + 1;
		return c;
	}

	static const char* skip_number(const char* c, const char* cE) {
		while (c < cE && IsDigit(*c)) ++c;
		return c;
	}

	static int compare(const char* l, const char* lE, const char* r, const char* rE) {
		int cmp;
		while (l < lE && r < rE) {
			if (',' == *l && ',' == *r) ++l, ++r;
			else if ('[' == *l && '[' == *r) {
				auto nlE = find_closing(++l, lE), nrE = find_closing(++r, rE);
				if (0 != (cmp = compare(l, nlE, r, nrE))) return cmp;
				l = nlE + 1, r = nrE + 1;
			} else if ('[' == *l) {		// r is integer -> needs extra list-boxing
				ASSERT(IsDigit(*r));
				auto nlE = find_closing(++l, lE), nrE = skip_number(r, rE);
				if (0 != (cmp = compare(l, nlE, r, nrE))) return cmp;
				l = nlE + 1, r = nrE;
			} else if ('[' == *r) {		// l is integer -> needs extra list-boxing
				ASSERT(IsDigit(*l));
				auto nlE = skip_number(l, lE), nrE = find_closing(++r, rE);
				if (0 != (cmp = compare(l, nlE, r, nrE))) return cmp;
				l = nlE, r = nrE + 1;
			} else {
				ASSERT(IsDigit(*l) && IsDigit(*r));
				if (0 != (cmp = atoi(l) - atoi(r))) return cmp;
				l = skip_number(l, lE), r = skip_number(r, rE);
			}
		}
		return (rE <= r) - (lE <= l);	// shorter is less
	}

	static bool less(const String & a, const String & b) {
		return compare(a.Begin(), a.End(), b.Begin(), b.End()) < 0;
	}

public:

	void init() { Cout() << "***"; }

	bool line(const String & line) {
		if (!line.IsEmpty()) packets.Add(line);
		else {
			++pairI;
			if (less(packets.End()[-2], packets.End()[-1])) p1_sum += pairI;
		}
		return false;							// not finished yet, try next line
	}

	void finish() {
		Cout() << "part1 sum of indices: " << p1_sum << EOL;
		Sort(packets, less);
		int p2_decoder_key = 1;
		for (int i = 0; i < packets.GetCount(); ++i) {
			if ("[[2]]" == packets[i] || "[[6]]" == packets[i]) p2_decoder_key *= (i+1);
		}
		Cout() << "part2 decoder key: " << p2_decoder_key << EOL;
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
