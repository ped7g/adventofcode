#include <Core/Core.h>

using namespace Upp;	//expected answers sample.txt: [65536, 65536], input.txt: [956, 40149]

static const String COMMAND_TXT[] = { "<DiReCt>", "NOT", "AND", "OR", "LSHIFT", "RSHIFT" };
enum COMMAND { DIRECT, NOT, AND, OR, LSHIFT, RSHIFT };	// must match index into COMMAND_TXT
static constexpr auto COMMANDS_TO_PARSE = {NOT, AND, OR, LSHIFT, RSHIFT};
static_assert(5 == COMMAND::RSHIFT && DIRECT < AND && NOT < AND);
static constexpr int DBG_OUT = 0;

typedef VectorMap<uint32, uint32> values_t;

struct rule_t : public Moveable<rule_t> {

	static constexpr uint32 U16MASK = 0xFFFF, NAME = U16MASK + 1;

	static uint32 s2w(const String & s) {		// string to wire number
		ASSERT(1 <= s.GetCount() && s.GetCount() <= 2 && 'a' <= s[0] && s[0] <= 'z');
		return NAME + 1 + (s[0] - 'a') + 26 * (2 == s.GetCount() ? 1 + s[1] - 'a' : 0);
	}

	static String w2s(uint32 w) {
		if (w < NAME) return IntStr(w);
		if (NAME == w) return "?";
		const char name[2] { char((w - NAME - 1) % 26 + 'a'), char((w - NAME - 27) / 26 + 'a') };
		return String(name, w <= NAME + 26 ? 1 : 2);
	}

	uint32 a { NAME }, b { NAME }, r { NAME };	// wire names or literal values
	COMMAND cmd { DIRECT };

	void parseCmd(const String & s) {
		for (COMMAND c : COMMANDS_TO_PARSE) if (COMMAND_TXT[c] == s) { cmd = c; return; }
	}

	void assignValue(uint32 & pos, uint32 v) {
		switch (pos++) {
			case 0: a = v; return;
			case 1: b = v; return;
			case 2: r = v; return;
			default: ASSERT(0);
		}
	}
	uint32 eval(const values_t & values) const {
		uint32 av = (a < NAME) ? a : values.Get(a, NAME);			// fetch values if available
		uint32 bv = (b < NAME) ? b : values.Get(b, NAME);
		if (NAME <= av || (AND <= cmd && NAME <= bv)) return NAME;	// not fully defined yet
		uint32 result = 0;
		switch (cmd) {
			case NOT: result = uint16(~av);			break;
			case AND: result = uint16(av & bv);		break;
			case OR: result = uint16(av | bv);		break;
			case LSHIFT: result = uint16(av << bv);	ASSERT(1 <= bv && bv <= 15);	break;
			case RSHIFT: result = uint16(av >> bv);	ASSERT(1 <= bv && bv <= 15);	break;
			default: result = uint16(av);			break;
		}
		if (DBG_OUT) printf("%s(%s=%d,%s=%d) = %s(%d)\n", ~COMMAND_TXT[cmd], ~w2s(a), av, ~w2s(b), bv, ~w2s(r), result);
		return result;
	}
	String ToString() const { return Format("\n%s (%s, %s) -> %s", ~COMMAND_TXT[cmd], w2s(a), w2s(b), w2s(r)); }
};

class Part1 {

	BiVector<rule_t> rules;
	values_t values;

public:

	void init() { Cout() << "part1"; }

	bool line(const String & line) {
		rule_t rule;
		uint32 arg_n = 0;
		for (const String & word : Split(~line, ' ')) {
			if (IsUpper(word[0])) rule.parseCmd(word);
			else if ('-' == word[0]) arg_n = 2;
			else if (IsDigit(word[0])) rule.assignValue(arg_n, StrInt(~word));
			else rule.assignValue(arg_n, rule_t::s2w(word));
		}
		uint32 eval = rule.eval(values);		// check if value can be evaluated
		if (eval < rule_t::NAME) values.FindAdd(rule.r, eval);
		else rules.AddTail(rule);				// otherwise postpone the rule
		return false;							// not finished yet, try next line
	}

	void finish() {
		while (!rules.IsEmpty()) {				// evaluate all postponed rules
			auto r = rules.PopHead();			// evaluate first rule and set value
			uint32 v = r.eval(values);			// or postpone the rule again
			if (v < rule_t::NAME) values.FindAdd(r.r, v); else rules.AddTail(r);
		}
		Cout() << "part1: a = " << values.Get(rule_t::s2w("a"), rule_t::NAME) << EOL;
	}
};

class Part2 {
	Part1 p1;
public:		// identical to Part1, but value 956 is injected for "b" overriding input value
	void init() { Cout() << "part2 overriding b = 956 of "; p1.init(); p1.line("956 -> b"); }
	bool line(const String & line) { return p1.line(line); }
	void finish() { Cout() << "part2 overriding b = 956 of "; p1.finish(); }
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
