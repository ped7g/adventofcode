#include <Core/Core.h>

// expected result: sample [10605, 2713310158], input [67830, 15305381442]

using namespace Upp;

struct Monkey : public Moveable<Monkey> {
	int64 inspections, op_mul, op_add, test_by, test_true, test_false;
	Vector<int64> items;

	template<int PART>
	void Inspect(Vector<Monkey> & target_monkeys, const int64 worry_wear = 3) {
		for (auto item : items) {
			++inspections;
			item *= op_mul < 0 ? item : op_mul;
			item += op_add < 0 ? item : op_add;
			// part 1 divides worry by three vs part 2 keeps modulo by product of all test_by
			if constexpr (PART == 1) item /= worry_wear; else item %= worry_wear;
			if (item % test_by) target_monkeys[test_false].items.Add(item);
			else target_monkeys[test_true].items.Add(item);
		}
		items.Clear();
	}

	Monkey() : inspections(0), op_mul(1), op_add(0) {}

	Monkey(const Monkey & b) :	// deep copy constructor for part2=clone(part1) during init
		inspections(b.inspections), op_mul(b.op_mul), op_add(b.op_add), test_by(b.test_by),
		test_true(b.test_true), test_false(b.test_false), items(clone(b.items)) {}
};

class AocTask {

	Vector<Monkey> monkeys_p1, monkeys_p2;
	int64 worry_wear = 1;		// product of all monkey.test_by values

	void results(int part, int rounds, const Vector<Monkey> & monkeys) {
		Cout() << "part " << part << " state after round " << rounds << ":" << EOL;
		int m1 = 0, m2 = 1;		// find two most active monkeys (starting with pair [0, 1])
		for (int i = 0; i < monkeys.GetCount(); ++i) {
			Cout() << Format("Monkey %d inspected items %ld times.\n", i, monkeys[i].inspections);
			if (monkeys[m1].inspections < monkeys[i].inspections) m1 = i;
			if (monkeys[m2].inspections < monkeys[m1].inspections) Swap(m1, m2);	// m2 is more active
		}
		Cout() << Format("Two most active monkeys: %d, %d, level of monkey business is %lld\n\n",
							m2, m1, monkeys[m1].inspections * monkeys[m2].inspections);
	}

public:

	void init() { Cout() << "***"; }

	bool line(const String & line) {
		if (line.StartsWith("Monkey")) return monkeys_p1.Add(), false;	// new monkey added
		auto & m = monkeys_p1.Top();					// top monkey being parsed
		if (line.StartsWith("  Starting")) {			// parse items
			for (int i = 18; i < line.GetLength(); ++i) {
				if (IsDigit(line[i-1]) || !IsDigit(line[i])) continue;
				m.items.Add(atoi(~line + i));			// first digit -> add item
			}
		} else if (line.StartsWith("  Operation")) {
			if (line.StartsWith("  Operation: new = old * ")) {
				m.op_mul = ('o' == line[25]) ? -1 : atoi(~line + 25);
			} else if (line.StartsWith("  Operation: new = old + ")) {
				m.op_add = ('o' == line[25]) ? -1 : atoi(~line + 25);
			} else throw Exc("unexpected Op: " + line);
		} else if (line.StartsWith("  Test")) {
			if (line.StartsWith("  Test: divisible by ")) {
				m.test_by = atoi(~line + 21);
				worry_wear *= m.test_by;
			} else throw Exc("unexpected Test: " + line);
		} else if (line.StartsWith("    If true")) {
			if (line.StartsWith("    If true: throw to monkey ")) {
				m.test_true = atoi(~line + 29);
			} else throw Exc("unexpected true: " + line);
		} else if (line.StartsWith("    If false")) {
			if (line.StartsWith("    If false: throw to monkey ")) {
				m.test_false = atoi(~line + 30);
			} else throw Exc("unexpected false: " + line);
		}
		return false;							// not finished yet, try next line
	}

	void finish() {
		if (monkeys_p1.GetCount() < 2) return;
		monkeys_p2 = clone(monkeys_p1);			// use same initial state also for part 2
		// part 1 and 2 rounds loops
		int round_p1 = 0, round_p2 = 0;
		while (++round_p1 <= 20   ) for (auto & m : monkeys_p1) m.Inspect<1>(monkeys_p1);
		while (++round_p2 <= 10000) for (auto & m : monkeys_p2) m.Inspect<2>(monkeys_p2, worry_wear);
		// output resulting state and find two max-inspections monkeys
		results(1, round_p1 - 1, monkeys_p1);
		results(2, round_p2 - 1, monkeys_p2);
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

CONSOLE_APP_MAIN
{
	for (const String & arg : CommandLine()) lines_loop(AocTask(), arg);
}
