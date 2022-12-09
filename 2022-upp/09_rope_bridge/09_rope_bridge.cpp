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
	FileIn in(GetDataFile(filename));
	if (!in) return;
	task.init();
	Cout() << " input filename: " << filename << EOL;
	while (!task.line(in.GetLine()) && !in.IsEof()) ;
	task.finish();
}

// helper Position type definition (2x int32 tuple)

typedef Tuple<int32, int32> Position;
void operator += (Position & p1, const Position & p2) { p1.a += p2.a, p1.b += p2.b; }
uint64 pos_hash(const Position & p) { return (uint64(p.b) << 32) | uint32(p.a); }

class RopeSimulator {	// expected results: sample [13, 1], sample2 [88, 36], input [6503, 2724]

	Vector<Position> rope { 10 };
	Index<uint64> p1_visited;		// hash map of visited locations for knot 1 (tail in part 1)
	Index<uint64> p2_visited;		// hash map of visited locations for knot 9 (tail in part 2)

	void add_tails_pos() {			// remember visited locations of knots 1 and 9
		p1_visited.FindAdd( pos_hash( rope[1] ) );
		p2_visited.FindAdd( pos_hash( rope[9] ) );
	}

public:

	void init() {
		Cout() << "***";
		add_tails_pos();			// remember also start position (even if no motion in input)
	}

	bool line(const String & line) {
		if (line.GetLength() < 3) return true;		// invalid/empty input, terminate
		// parse head motion
		int32 d = atoi(line.Mid(2));
		if (d < 1) return true;						// invalid/zero amount of steps, terminate
		const char m = line[0];						// expected motion char: L R U D
		const Position delta { -('L' == m) +('R' == m), -('U' == m) +('D' == m) };
		// do motion d many times
		while (d--) {
			rope[0] += delta;						// update head position
			// update 9 knots below head
			for (int i = 1; i < rope.GetCount(); ++i) {
				// delta to previous knot/head
				const Position knot_delta { rope[i-1].a - rope[i].a, rope[i-1].b - rope[i].b };
				// clamp to -1..+1 range
				const Position clamped { sgn(knot_delta.a), sgn(knot_delta.b) };
				// if delta != clamped, knot is further away than +-1, apply clamped motion
				if (knot_delta != clamped) rope[i] += clamped;
			}
			add_tails_pos();						// track knot 1 and 9 positions
		}
		return false;								// not finished yet, try next line
	}

	void finish() {
		Cout() << "part 1: knot 1 visited " << p1_visited.GetCount() << " position(s)\n";
		Cout() << "part 2: knot 9 visited " << p2_visited.GetCount() << " position(s)\n";
	}
};

CONSOLE_APP_MAIN
{
	for (const String & arg : CommandLine()) lines_loop(RopeSimulator(), arg);
}
