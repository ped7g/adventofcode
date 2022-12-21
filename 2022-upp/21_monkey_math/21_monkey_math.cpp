#include <Core/Core.h>

using namespace Upp;
// expected answer:	sample.txt:	root = 152, humn = 301
//					input.txt:	root = 38731621732448, humn = 3848301405790

static constexpr int32 name2int(const char *s) {
	return s[0] | (s[1]<<8) | (s[2]<<16) | (s[3]<<24);
}

static constexpr int32 ROOT_K = name2int("root");
static constexpr int32 HUMN_K = name2int("humn");

void eval(VectorMap<int32, int64> & values, char op, int32 k0, int i1, int i2) {
	int64 value;
	switch (op) {
		case '+': value = values[i1] + values[i2]; break;
		case '-': value = values[i1] - values[i2]; break;
		case '*': value = values[i1] * values[i2]; break;
		case '/': value = values[i1] / values[i2]; break;
	}
	values.Add(k0, value);
}

class Part1 {
	VectorMap<int32, int64> p1_values;
	BiVector<String> p1_ops;

public:

	void init() { Cout() << "part1"; }

	bool line(const String & line) {
		if (line.GetLength() < 7) return true;
		int32 monkey = name2int(~line);
		if (IsDigit(line[6])) {
			int64 value = atoi(~line + 6);
			ASSERT(p1_values.Find(monkey) < 0);
			p1_values.Add(monkey, value);
		} else {
			p1_ops.AddTail(~line);
		}
		return false;							// not finished yet, try next line
	}

	void finish() {
		while (!p1_ops.IsEmpty()) {
			String op = p1_ops.PopHead();
			int m1i = p1_values.Find(name2int(~op + 6)), m2i;
			if (m1i < 0 || (m2i = p1_values.Find(name2int(~op + 13))) < 0) p1_ops.AddTail(op);
			else eval(p1_values, op[11], name2int(~op), m1i, m2i);
		}
		ASSERT(0 <= p1_values.Find(ROOT_K));
		Cout() << "part1: root = " << p1_values.Get(ROOT_K) << EOL;
	}
};

class Part2 {
	VectorMap<int32, int64> values;
	BiVector<String> operations;

public:

	void init() { Cout() << "part2"; }

	bool line(const String & line) {
		if (line.GetLength() < 7) return true;
		int32 monkey = name2int(~line);
		if (HUMN_K == monkey) return false;
		if (IsDigit(line[6])) {
			int64 value = atoi(~line + 6);
			ASSERT(values.Find(monkey) < 0);
			values.Add(monkey, value);
		} else {
			operations.AddTail(~line);
		}
		return false;							// not finished yet, try next line
	}

	void finish() {
		while (!operations.IsEmpty()) {
			String op = operations.PopHead();
			int64 value;
			char o = op[11];
			int32 m0 = name2int(~op), m1 = name2int(~op + 6), m2 = name2int(~op + 13);
			int m0i = values.Find(m0), m1i = values.Find(m1), m2i = values.Find(m2);
			const int known = (0 <= m0i) + (0 <= m1i) + (0 <= m2i);
			if (1 == known && ROOT_K == m0) {		// root special case -> defines other one
				if (0 <= m2i) Swap(m1i, m2i), Swap(m1, m2);
				values.Add(m2, values[m1i]);
				//Cout() << Format("%s ! [%d] = %lld\n", op, m2, values[m1i]);
				continue;
			}
			if (known < 2) {						// not enough known values
				operations.AddTail(op);
				continue;
			}
			if (m0i < 0) {							// both args are known
				ASSERT(0 <= m1i && 0 <= m2i);
				switch (o) {
					case '+': value = values[m1i] + values[m2i]; break;
					case '-': value = values[m1i] - values[m2i]; break;
					case '*': value = values[m1i] * values[m2i]; break;
					case '/': value = values[m1i] / values[m2i]; break;
				}
				values.Add(m0, value);
				//Cout() << Format("%s = %lld by %lld %c %lld\n", op, value, values[m1i], o, values[m2i]);
				continue;
			}
			ASSERT(0 <= m0i);
			if (0 <= m1i) {							// result and first arg is known
				switch (o) {
					case '+': value = values[m0i] - values[m1i]; break;	// b = r - a // r = a + b
					case '-': value = values[m1i] - values[m0i]; break;	// b = a - r // r = a - b
					case '*': value = values[m0i] / values[m1i]; break;	// b = r / a // r = a * b
					case '/': value = values[m1i] / values[m0i]; break; // b = a / r // r = a / b
				}
				values.Add(m2, value);
				//Cout() << Format("%s arg2 = %lld by %c (%lld, %lld)\n", op, value, o, values[m0i], values[m1i]);
			} else {								// result and second arg is known
				switch (o) {
					case '+': value = values[m0i] - values[m2i]; break;	// a = r - b // r = a + b
					case '-': value = values[m0i] + values[m2i]; break;	// a = r + b // r = a - b
					case '*': value = values[m0i] / values[m2i]; break;	// a = r / b // r = a * b
					case '/': value = values[m0i] * values[m2i]; break; // a = r * b // r = a / b
				}
				values.Add(m1, value);
				//Cout() << Format("%s arg1 = %lld by %c (%lld, %lld)\n", op, value, o, values[m0i], values[m2i]);
			}
		}
		ASSERT(0 <= values.Find(HUMN_K));
		Cout() << "part2: humn = " << values.Get(HUMN_K) << EOL;
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
	for (const String & arg : CommandLine()) {
		lines_loop(Part1(), arg);
		lines_loop(Part2(), arg);
	}
}
