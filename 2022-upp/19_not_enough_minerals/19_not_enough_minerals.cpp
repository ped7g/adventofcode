#include <Core/Core.h>

using namespace Upp;	// expected results: sample [33, 3472 (two blueprints)], input [994, 15960]

struct Resources : Moveable<Resources> {
	short ore = 0, clay = 0, obsidian = 0, geodes = 0;

	bool covers(const Resources cost) const { return cost.ore <= ore && cost.clay <= clay && cost.obsidian <= obsidian && cost.geodes <= geodes; }
	void operator += (const Resources & b) { ore += b.ore, clay += b.clay, obsidian += b.obsidian, geodes += b.geodes; }
	void operator -= (const Resources & b) { ore -= b.ore, clay -= b.clay, obsidian -= b.obsidian, geodes -= b.geodes; }
	String ToString() const { return Format("[ore %2>d, clay %2>d, obsidian %2>d, geodes %2>d]", ore, clay, obsidian, geodes); }
	String ToFirstRes() const { return ore ? "ore" : clay ? "clay" : obsidian ? "obsidian" : geodes ? "geodes" : "<none>"; }
};

static Resources operator * (const Resources & a, const uint8 k) {
	Resources r;
	r.ore = k * a.ore, r.clay = k * a.clay, r.obsidian = k * a.obsidian, r.geodes = k * a.geodes;
	return r;
}

struct RobotInfo : Moveable<RobotInfo> {
	Resources cost, produces;
	uint8 working = 0;

	String ToString() const {
		return Format(" - working %2>d, cost %s, produces %s\n", working, cost.ToString(), produces.ToFirstRes());
	}
};

struct Blueprint : Moveable<Blueprint> {
	uint8 id;
	Resources resources;
	RobotInfo robots[4];

	Blueprint() {
		robots[0].working = 1;
		robots[0].produces.ore = 1;
		robots[1].produces.clay = 1;
		robots[2].produces.obsidian = 1;
		robots[3].produces.geodes = 1;
	}
	String ToString() const {
		return Format("* [blueprint %d] %s, robots:\n%s%s%s%s", id, resources.ToString(), robots[0].ToString(), robots[1].ToString(), robots[2].ToString(), robots[3].ToString());
	}
};

class Aoc2022Day19Task {
	int p1_ql_sum = 0, p2_max_geodes = 1;

	void Simulate(Vector<Blueprint> & blueprints, int & max_geodes, int time) {
		Blueprint bp = blueprints.Top();
		// early exit test
		short upper_geodes = bp.resources.geodes + bp.robots[3].working * time;
		upper_geodes += (time - 1) * time / 2;
		if (upper_geodes <= max_geodes) return;
		// simulate
		--time;
		// check cost (availability) of robots
		bool can_build[4];
		bool can_build_all = true;
		for (int i = 0; i < 4; ++i) {
			can_build[i] = bp.resources.covers(bp.robots[i].cost);
			can_build_all &= can_build[i];
		}
		// collect resources with working robots
		for (const auto & r : bp.robots) bp.resources += r.produces * r.working;
		// refresh max_geodes
		if (max_geodes < bp.resources.geodes) {
			max_geodes = bp.resources.geodes;
//			Cout() << "** new max: " << max_geodes << ", state history:\n";
//			for (int i = 0; i < blueprints.GetCount(); ++i ) Cout() << "* t " << i+1 << " " << blueprints[i];
		}
		if (0 == time) return;			// no more time to build anything
		// try different simulation paths with building robots
		for (int i = 4; i--; ) {		// and try with building one new robot
			if (!can_build[i]) continue;
			bp.resources -= bp.robots[i].cost, ++bp.robots[i].working;
			blueprints.Add(bp); Simulate(blueprints, max_geodes, time); blueprints.Pop();
			bp.resources += bp.robots[i].cost, --bp.robots[i].working;
		}
		if (can_build_all) return;		// no reason to just wait if all can be built
		// nothing built at all
		blueprints.Add(bp); Simulate(blueprints, max_geodes, time); blueprints.Pop();
	}

public:

	void init() { Cout() << "**********"; }

	bool line(const String & line) {
		Blueprint bp;
		if (7 != sscanf(~line, "Blueprint %hhu: Each ore robot costs %hd ore. Each clay robot costs %hd ore. Each obsidian robot costs %hd ore and %hd clay. Each geode robot costs %hd ore and %hd obsidian.",
						&bp.id, &bp.robots[0].cost.ore, &bp.robots[1].cost.ore, &bp.robots[2].cost.ore, &bp.robots[2].cost.clay, &bp.robots[3].cost.ore, &bp.robots[3].cost.obsidian))
			return true;
		Cout() << "***" << bp;
		int max_geodes = 0;
		Vector<Blueprint> simulation { bp };
		Simulate(simulation, max_geodes, 24);
		Cout() << "*** 24 minutes max geodes: " << max_geodes << " -> quality level: " << bp.id * max_geodes << EOL;
		p1_ql_sum += bp.id * max_geodes;
		if (1 <= bp.id && bp.id <= 3) {			// part 2
			Simulate(simulation, max_geodes, 32);	// also reuse max_geodes from part one for pruning
			p2_max_geodes *= max_geodes;
			Cout() << "*** 32 minutes max geodes: " << max_geodes << EOL;
		}
		return false;							// not finished yet, try next line
	}

	void finish() {
		Cout() << "******* part1: " << p1_ql_sum << ", part2: " << p2_max_geodes << EOL;
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
	for (const String & arg : CommandLine()) lines_loop(Aoc2022Day19Task(), arg);
}
