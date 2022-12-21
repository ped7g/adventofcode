#include <Core/Core.h>

using namespace Upp;
// expected answer:	sample.txt:	root = 152, humn = 301
//					input.txt:	root = 38731621732448, humn = 3848301405790

static constexpr int32 name2int(const char *s) {
	return s[0] | (s[1]<<8) | (s[2]<<16) | (s[3]<<24);
}

static constexpr int32 ROOT_K = name2int("root");
static constexpr int32 HUMN_K = name2int("humn");

enum E_CALC { ARG0_CALC = 0, ARG1_CALC = 1, ARG2_CALC = 2 };	// index into op_map

static constexpr char op_map[][7] {
	{ "*+0-0/" },	// r = a op b	// op is op, no transformation
	{ "/-0+0*" },	// a = r op b	// transform op to get a
		//  r = a * b  =>  a = r / b
		//  r = a + b  =>  a = r - b
		//  r = a - b  =>  a = r + b
		//  r = a / b  =>  a = r * b
	{ "/-0,0." }	// b = r op a	// transform op to get b
		//  r = a * b  =>  b = r / a
		//  r = a + b  =>  b = r - a
		//  r = a - b  =>  b = a - r
		//  r = a / b  =>  b = a / r
};

static char transform_op(E_CALC c, char o) {
	ASSERT('*' <= o && o <= '/' && 0 <= c && c <= 2);
	return op_map[c][o - '*'];
}

static void eval(VectorMap<int32, int64> & values, char op, int32 k0, int i1, int i2) {
	int64 value;
	switch (op) {
		case '*': value = values[i1] * values[i2]; break;	// a * b
		case '+': value = values[i1] + values[i2]; break;	// a + b
		case ',': value = values[i2] - values[i1]; break;	// b - a (reversed -)
		case '-': value = values[i1] - values[i2]; break;	// a - b
		case '.': value = values[i2] / values[i1]; break;	// b / a (reversed /)
		case '/': value = values[i1] / values[i2]; break;	// a / b
		case '0': default: Exit(1);							// invalid op
	}
	values.Add(k0, value);
}

class AoC2022Day21Task {
	VectorMap<int32, int64> p1_values, p2_values;
	BiVector<String> p1_ops, p2_ops;

public:

	void init() { Cout() << "***"; }

	bool line(const String & line) {
		if (line.GetLength() < 7) return true;
		int32 monkey = name2int(~line);
		if (IsDigit(line[6])) {
			ASSERT(p1_values.Find(monkey) < 0);
			int64 value = atoi(~line + 6);
			p1_values.Add(monkey, value);
			if (HUMN_K != monkey) p2_values.Add(monkey, value);
		} else {
			p1_ops.AddTail(~line), p2_ops.AddTail(~line);
		}
		return false;							// not finished yet, try next line
	}

	void finish() {
		// part 1 calculates "root"
		while (!p1_ops.IsEmpty()) {					// process operation queue
			auto op = p1_ops.PopHead();
			int m1i = p1_values.Find(name2int(~op + 6));	// look for arg1
			int m2i = (0 <= m1i) ? p1_values.Find(name2int(~op + 13)) : -1;	// if arg1, look..
			if (m2i < 0) p1_ops.AddTail(op);		// missing arg, operation back into queue
			else eval(p1_values, op[11], name2int(~op), m1i, m2i);	// args defined -> evaluate
		}
		// part 2 calculates "humn"
		while (!p2_ops.IsEmpty()) {					// process operation queue
			auto op = p2_ops.PopHead();
			int32 m0 = name2int(~op), m1 = name2int(~op + 6), m2 = name2int(~op + 13);
			int m0i = p2_values.Find(m0), m1i = p2_values.Find(m1), m2i = p2_values.Find(m2);
			const int known = (0 <= m0i) + (0 <= m1i) + (0 <= m2i);
			if (known <= 1) {						// not enough known values
				if (1 == known && ROOT_K == m0) {	// special case "root" -> define other arg
					if (m2i < 0) p2_values.Add(m2, p2_values[m1i]);		// m2 = m1
					else p2_values.Add(m1, p2_values[m2i]);				// m1 = m2
				} else p2_ops.AddTail(op);			// put operation back into queue
			} else if (m0i < 0) {					// both args are known
				eval(p2_values, transform_op(ARG0_CALC, op[11]), m0, m1i, m2i);
			} else if (m1i < 0) {					// result and second arg is known
				eval(p2_values, transform_op(ARG1_CALC, op[11]), m1, m0i, m2i);
			} else {								// result and first arg is known
				eval(p2_values, transform_op(ARG2_CALC, op[11]), m2, m0i, m1i);
			}
		}
		ASSERT(0 <= p1_values.Find(ROOT_K) && 0 <= p2_values.Find(HUMN_K));
		Cout() << "part1: root = " << p1_values.Get(ROOT_K) << EOL;
		Cout() << "part2: humn = " << p2_values.Get(HUMN_K) << EOL;
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
	for (const String & arg : CommandLine()) lines_loop(AoC2022Day21Task(), arg);
}
