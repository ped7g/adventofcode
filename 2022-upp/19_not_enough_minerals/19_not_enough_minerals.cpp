#include <Core/Core.h>

using namespace Upp;	// expected results: sample [33, 3472 (two blueprints)], input [994, 15960]

static constexpr int ORE = 0, CLAY = 1, OBSIDIAN = 2, GEODES = 3, RES_NUM = 4;
typedef std::array<short, RES_NUM> resources_t;		// ore, clay, obsidian, geodes
typedef std::array<resources_t, RES_NUM> costs_t;	// four types of robots, each has full four-set for costs

static resources_t operator * (const resources_t & a, const int k) {
	resources_t r;
	for (int i = a.size(); i--; ) r[i] = a[i] * k;
	return r;
}

static void operator += (resources_t & a, const resources_t & b) {
	for (int i = a.size(); i--; ) a[i] += b[i];
}

static void operator -= (resources_t & a, const resources_t & b) {
	for (int i = a.size(); i--; ) a[i] -= b[i];
}

class v2Aoc2022Day19Task {
	int p1_ql_sum = 0, p2_max_geodes = 1;

	static String resourcesToString(const resources_t &r) {
		return Format("[ore %2d, clay %2d, obsidian %2d, geodes %2d]", r[ORE], r[CLAY], r[OBSIDIAN], r[GEODES]);
	}

	static String blueprintToString(uint8 id, const resources_t & r, const resources_t & w, const costs_t & c, const resources_t & max_r) {
		static constexpr const char* res2txt[RES_NUM] = { "ore", "clay", "obsidian", "geodes" };
		String txt = Format("* [blueprint %2d] %s, robots:\n", id, resourcesToString(r));
		for (int i = 0; i < RES_NUM; ++i)
			txt.Cat(Format(" - working %2d (max %5d), cost %s, produces %s\n", w[i], max_r[i], resourcesToString(c[i]), res2txt[i]));
		return txt;
	}

	class Simulator {
		const uint8 id;
		const costs_t & costs;							// costs of robots
		resources_t resources { 0, 0, 0, 0 };			// mined resources for next turn
		resources_t working { 1, 0, 0, 0 };				// working robots for next turn
		// effective max of particular robot type (more of them can't change outcome)
		resources_t max_robots { 0, 0, 0, std::numeric_limits<short>::max() };
		int to_minutes;

		void search_max_impl(short & max_geodes, int time_left) {
			ASSERT(1 < time_left);
			// PRUNE #4 - early exit when it's already impossible to reach current max_geodes
			short upper_geodes = resources[GEODES] + time_left * working[GEODES];
			upper_geodes += (time_left - 1) * time_left / 2;	// if every turn new geodes robot was added
			if (upper_geodes <= max_geodes) return;	// can't reach current max_geodes
			// try to build all types of robots as next step, use recursion if some is built
			for (int i = RES_NUM; i--; ) {			// four types of robot (DESC from geodes)
				if (max_robots[i] <= working[i]) continue;	// no further need for this type		// PRUNE #1
				// PRUNE #3 - enough resources till end of times, don't bother to build it
				if ((time_left - 1) * max_robots[i] <= resources[i] + (time_left - 1) * working[i]) continue;
				// calculate how many turns until this type of robot can be built
				int ttb = 1;						// turns required to build this robot
				for (int r = RES_NUM; r--; ) {		// resource type check
					if (costs[i][r] <= 0) continue;	// resource not required for this robot
					if (0 == working[r]) {			// no robot for this type of resource, assume zero resources
						ttb = time_left;			// can't be produced in reasonable time
						break;
					}
					ttb = max(ttb, (costs[i][r] - resources[r] + working[r] - 1) / working[r] + 1);
				}
				// update max_geodes if this is new geodes robot
				if (GEODES == i) {
					short end_geodes = resources[GEODES] + time_left * working[GEODES] + (time_left - ttb);
					if (max_geodes < end_geodes) max_geodes = end_geodes;
				}
				if (time_left <= ttb + 1) continue;	// can't be produced in time to affect result	// PRUNE #2
				// wait required turns and produce the robot, then try next step recursively
				resources += working * ttb;			// mined resources during wait and build turn
				resources -= costs[i], ++working[i];// build robot
				// try to build more robots recursively
				ASSERT(1 < time_left - ttb);
				search_max_impl(max_geodes, time_left - ttb);
				// revert last robot build
				resources += costs[i], --working[i];
				resources -= working * ttb;
			}
		}

	public:
		Simulator(uint8 id, const costs_t & costs) : id(id), costs(costs) {
			// max amount of robots of particular type is the maximum cost per resource,
			// that's enough to refill every turn for any next robot
			for (int i = RES_NUM; i--; ) for (int j = RES_NUM; j--; )
				max_robots[i] = max(max_robots[i], costs[j][i]);
		}

		short search_max(int time, bool show_state = true, short geodes_lower_bound = 0) {
			to_minutes = time + 1;
			short max_geodes = geodes_lower_bound;
			if (show_state) Cout() << blueprintToString(id, resources, working, costs, max_robots);
			search_max_impl(max_geodes, time);
			return max_geodes;
		}
	};

public:

	void init() { Cout() << "**********"; }

	bool line(const String & line) {
		uint8 blueprint_id;
		costs_t robot_costs {};
		if (7 != sscanf(~line, "Blueprint %hhu: Each ore robot costs %hd ore. Each clay robot costs %hd ore. Each obsidian robot costs %hd ore and %hd clay. Each geode robot costs %hd ore and %hd obsidian.",
						&blueprint_id, &robot_costs[0][ORE], &robot_costs[1][ORE], &robot_costs[2][ORE], &robot_costs[2][CLAY], &robot_costs[3][ORE], &robot_costs[3][OBSIDIAN]))
			return true;
		Simulator sim(blueprint_id, robot_costs);
		short max_geodes = sim.search_max(24, false);
		Cout() << Format("%2d: 24 minutes max geodes: %d -> quality level: %d\n", blueprint_id, max_geodes, blueprint_id * max_geodes);
		p1_ql_sum += blueprint_id * max_geodes;
		if (1 <= blueprint_id && blueprint_id <= 3) {		// part 2
			max_geodes = sim.search_max(32, false, max_geodes);	// max_geodes from part one for pruning
			p2_max_geodes *= max_geodes;
			Cout() << "    32 minutes max geodes: " << max_geodes << EOL;
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
	for (const String & arg : CommandLine()) lines_loop(v2Aoc2022Day19Task(), arg);
}
