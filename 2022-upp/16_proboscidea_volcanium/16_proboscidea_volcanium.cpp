#include <Core/Core.h>

using namespace Upp;

// expected answers: sample2.txt [28, 24], sample.txt [1651, 1707], input.txt [1857, 2536]

// version 2 - reworked algorithm to use dynamic programming
// - does NOT provide the path directly, only pressure value (path *could* be reconstructed, but...)
// - total time could be slightly limited by first figuring out how far from AA is each node
//   and update the paths only for remaining 1,2,...,(time-distance_from_AA) minutes

constexpr static int TOTAL_TIME_PART_1 = 30, TOTAL_TIME_PART_2 = 26;

typedef Tuple<uint16, uint16> ValvesPressure;	// pair of opened valves and most pressure
typedef VectorMap<uint16, uint16> ValvesPath;	// key: open valves bitmask, value: released pressure

// convert name like "AA".."ZZ" into uint16 just by using the two chars as uint8 values
static uint16 name2i(const char* name) {
	ASSERT(('A' <= name[0] && name[0] <= 'Z') && ('A' <= name[1] && name[1] <= 'Z'));
	return *reinterpret_cast<const uint16*>(name);
}

struct Node : Moveable<Node> {
	uint16 name, flow, mask;		// name of node, possible flow of its valve, bit-mask of it
	Vector<uint16> tunnels;			// node names during init, then indices into nodes vector
	Vector<ValvesPath> paths_T;		// paths for remaining time: 1, 2, ... 30 minutes

	Node(uint16 name, uint16 flow) : name(name), flow(flow), mask(0) {
		tunnels.Reserve(20);
		paths_T.AddN(TOTAL_TIME_PART_1);
	}

	// keep maximum pressure for particular open_valves type of path at time t
	void update_max(int t, uint16 open_valves, uint16 pressure) {
		int old_i = paths_T[t].FindAdd(open_valves, pressure);
		if (paths_T[t][old_i] < pressure) paths_T[t][old_i] = pressure;
	}

	// update possible-paths for remaining t minutes (assumes t-1 and less are already set)
	void update(const Vector<Node> & nodes, int t) {
		for (auto tunnel : tunnels) {	// consider possible paths from neighbours' T-1
			const auto & n_t1 = nodes[tunnel].paths_T[t - 1];
			// but keep only maximum pressure one per each combination of open valves
			for (int i = n_t1.GetCount(); i--; ) update_max(t, n_t1.GetKey(i), n_t1[i]);
		}
		if (0 == flow) return;			// this valve has no pressure -> no option to open it
		ASSERT(mask);					// valve bit-mask to track open valves
		update_max(t, mask, t * flow);	// new path: opening valve at time T and staying here
		if (t < 2) return;				// T-2 paths doesn't exist yet, nothing else to add
		for (auto tunnel : tunnels) {	// consider neighbour's T-2 paths after opening this valve
			const auto & n_t2 = nodes[tunnel].paths_T[t - 2];
			for (int i = n_t2.GetCount(); i--; ) {
				const uint16 open_valves = n_t2.GetKey(i);
				if (open_valves & mask) continue;	// this valve is already open in that T-2 path
				update_max(t, open_valves | mask, n_t2[i] + t * flow);		// open new path
			}
		}
	}
};

class AoC2022Day16Task_v2 {

	Vector<Node> nodes;

public:

	void init() {
		Cout() << "***";
		nodes.Reserve(100);
	}

	bool line(const String & line) {
		if (line.GetLength() < 50) return true;			// shorter than shortest valid
		uint16 name = name2i(~line + 6), flow = atoi(~line + 23);
		auto & n = nodes.Add({ name, flow });
		const char* tunnels_in = ~line + 1 + line.FindAfter("to valve");	// +1 for trailing space
		if (' ' == *tunnels_in) ++tunnels_in;			// "valves" with "s" needs extra +1
		ASSERT(48 <= tunnels_in - ~line);
		for (; tunnels_in < line.End(); tunnels_in += 4) n.tunnels.Add(name2i(tunnels_in));
		return false;
	}

	void finish() {
		// sort nodes by flow in descending order
		Sort(nodes, [](const Node & a, const Node & b) -> bool { return b.flow < a.flow; });
		// translate names in node.tunnels to indices into sorted array
		uint8 node_name2i[('Z'<<8) + 'Z' + 1];			// sparse initialised array name -> index
		for (int i = 0; i < nodes.GetCount(); ++i) {
			ASSERT(i < 16 || 0 == nodes[i].flow);		// make sure uint16 is enough for bitmask
			node_name2i[ nodes[i].name ] = i;
			if (nodes[i].flow) nodes[i].mask = 1 << i;	// init bit-mask of valves with flow
		}
		for (auto & n : nodes) for (auto & t : n.tunnels) t = node_name2i[t];	// translate tunnels data
		const int node_aa_i = node_name2i[0x4141];		// nodes index of "AA" (0x4141 as 'A' is 0x41)
		// evaluate most-pressure paths for T=1,2,...,TOTAL_TIME
		for (int t = 1; t < TOTAL_TIME_PART_1; ++t)		// t = 0 makes no difference (to open valve is too late)
			for (auto & n : nodes)						// update all possible paths (with different valves open)
				n.update(nodes, t);						// for remaining minutes 2,3,4,...
		// part1: find path with maximum possible flow from AA for T = TOTAL_TIME_PART_1
		const auto & aa_paths_p1 = nodes[node_aa_i].paths_T[TOTAL_TIME_PART_1 - 1];
		uint16 most_pressure = aa_paths_p1.IsEmpty() ? 0 : *std::max_element(aa_paths_p1.Begin(), aa_paths_p1.End());
		Cout() << "*** part 1 * most pressure you can release: " << most_pressure << EOL;
		// part2: find pair of paths with maximum possible flow from AA for T = TOTAL_TIME_PART_2
		auto & aa_paths_p2 = nodes[node_aa_i].paths_T[TOTAL_TIME_PART_2 - 1];
		Vector<ValvesPressure> p2;						// copy all paths into vector as [valves, pressure] pairs
		p2.Reserve(aa_paths_p2.GetCount());				// to sort them in descending order by pressure
		for (int i = 0; i < aa_paths_p2.GetCount(); ++i) p2.Add({ aa_paths_p2.GetKey(i), aa_paths_p2[i] });
		// sort paths by most pressure (descending order)
		Sort(p2, [](const ValvesPressure & left, const ValvesPressure & right) { return right.b < left.b; } );
		// find most-pressure pair of paths from AA at T=26 (each path opening different valves)
		most_pressure = 0;
		int dualpath_1_i = -1, dualpath_2_i, max_j = p2.GetCount();
		for (int i = 0; i < max_j; ++i) {				// try new "left" path of pair
			for (int j = i + 1; j < max_j; ++j) {
				if (p2[i].a & p2[j].a) continue;		// same valve(s) open, can't pair
				max_j = j;								// this is new search limit in *any* case
				if (p2[i].b + p2[j].b <= most_pressure) break;	// less pressure with this "left"
				most_pressure = p2[i].b + p2[j].b;		// new max pressure found, remember it
				dualpath_1_i = i;						// "left" = i
				dualpath_2_i = j;						// "right" = j
			}
		}
		// if there can't be any pair of paths, deliver at least single path with most pressure (or none)
		if (-1 == dualpath_1_i) most_pressure = p2.IsEmpty() ? 0 : p2[0].b;
		Cout() << "*** part 2 * most pressure you can release: " << most_pressure << EOL;
		if (0 <= dualpath_1_i) {
			Cout() << Format(" Paths as (valves, pressure): (0x%04X, %5d) with (0x%04X, %5d) does (0x%04X, %5d)\n",
				p2[dualpath_1_i].a, p2[dualpath_1_i].b, p2[dualpath_2_i].a, p2[dualpath_2_i].b, p2[dualpath_1_i].a | p2[dualpath_2_i].a, most_pressure);
		} else {
			Cout() << " Pair of paths not found, one or no path used." << EOL;
		}
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
	for (const String & arg : CommandLine()) lines_loop(AoC2022Day16Task_v2(), arg);
}
