#include <Core/Core.h>

// expected answers: sample2.txt [28, 24], sample.txt [1651, 1707], input.txt [1857, 2536]

//Myopian:  It takes 81284 steps with my aggressive pruning constant,
// and the same answer with the more conservative pruning threshold, but does 1243846.

// Part 2 paths:

//New max: 1707 by
//AA,DD,DD,EE,FF,GG,HH,HH,GG,FF,EE,EE :
//AA,II,JJ,JJ,II,AA,BB,BB,CC,CC,DD
//most pressure you can release: 1707

//New max: 2536 by
//AA,KF,HR,HR,AW,DW,DW,HY,IU,XO,XO,MO,VI,VI,MO,XO,IU,HY,DW,CB,MW,MW,RQ,FQ,FQ :
//AA,OX,MA,MA,YC,II,II,RH,AS,AS,IR,BL,RU,RU,YZ,PM,PM,FR,KQ,KQ,FR,PM,YZ,RU
//most pressure you can release: 2536

using namespace Upp;

struct Node : Moveable<Node> {
	int flow;
	Vector<int16> tunnels;
	String name;

	Node() : flow(-1), name("xx") {}

	static int16 name2i(const char* name) {
		if ('x' == name[0]) return -1;
		ASSERT('A' <= name[0] && name[0] <= 'Z' && 'A' <= name[1] && name[1] <= 'Z');
		return (name[0] - 'A') + 26 * (name[1] - 'A');
	}

	String ToString() const {
		return Format("\n%s(%d) = %d: %s", name, name2i(~name), flow, tunnels.ToString());
	}
};

static Vector<Node> nodes;
static int total_flow;

struct Path : Moveable<Path> {
	constexpr static int TOTAL_TIME = 30;

	int released = 0, releasing = 0, could = 0, last_open = 0;
	String p = "";

	bool bigger(const Path & b) const {
		if (b.released < released) return true;
		if (released < b.released) return false;
		return b.released + b.could < released + could;
	}

	int16 node_i() const { return Node::name2i(~p + p.GetLength() - 2); }

	bool is_on(const Node & n) const {
		String name2x = n.name + "," + n.name;
		return -1 != p.Find(name2x);
	}

	int time_remaining() const {
		return TOTAL_TIME - (p.GetLength()/3);
	}

	Path grow(int16 ni, bool open_valve = false) const {
		Path r = *this;
		r.p += "," + nodes[ni].name;
		if (open_valve) {
			r.last_open = r.p.GetLength() - 2;
			r.releasing += nodes[ni].flow;
			r.released += nodes[ni].flow * r.time_remaining();
			r.could = (total_flow - r.releasing) * (r.time_remaining() - 1);
		} else {
			if (last_open <= p.ReverseFind(nodes[ni].name)) r.could = 0;
			else r.could = (total_flow - r.releasing) * (r.time_remaining() - 1);
		}
		return r;
	}
};

struct DualPath : Moveable<DualPath> {
	constexpr static int TOTAL_TIME = 26;

	int released = 0, releasing = 0, could[2] { 0, 0 }, last_open[2] { 0, 0 };
	String p[2] { "", "" };

	int get_could() const {
		return std::max(could[0], could[1]);
	}

	bool bigger(const DualPath & b) const {
		if (b.released < released) return true;
		if (released < b.released) return false;
		return b.released + b.could < released + could;
	}

	int16 node_i(int pi) const { return Node::name2i(~p[pi] + p[pi].GetLength() - 2); }

	bool is_on(const Node & n) const {
		String name2x = n.name + "," + n.name;
		for (const auto & pp : p) if (-1 != pp.Find(name2x)) return true;
		return false;
	}

	int time_remaining(int pi) const {
		return TOTAL_TIME - (p[pi].GetLength()/3);
	}

	DualPath grow(int pi, int16 ni, bool open_valve = false) const {
		DualPath r = *this;
		r.p[pi] += "," + nodes[ni].name;
		if (open_valve) {
			r.last_open[pi] = r.p[pi].GetLength() - 2;
			r.releasing += nodes[ni].flow;
			r.released += nodes[ni].flow * r.time_remaining(pi);
			r.could[pi] = (total_flow - r.releasing) * (r.time_remaining(pi) - 1);
			r.could[1-pi] = (total_flow - r.releasing) * (r.time_remaining(1-pi) - 1);
		} else {
			if (last_open[pi] <= p[pi].ReverseFind(nodes[ni].name)) r.could[pi] = 0;
			else r.could[pi] = (total_flow - r.releasing) * (r.time_remaining(pi) - 1);
		}
		return r;
	}
};

template<> struct StdGreater<Path> {
	bool operator () (const Path& a, const Path& b) const { return a.bigger(b); }
};

template<> struct StdGreater<DualPath> {
	bool operator () (const DualPath& a, const DualPath& b) const { return a.bigger(b); }
};

class Part1 {

	char buf[3] { "xx" };

	static String path2str(const Vector<int16> & p) {
		String r = "[";
		for (auto i : p) {
			if (1 < r.GetLength()) r += ", ";
			r += 'A' + (i%26);
			r += 'A' + (i/26);
		}
		r += "]";
		return r;
	}

public:

	void init() {
		Cout() << "***";
		total_flow = 0;
		nodes.Clear();
		nodes.AddN(26*26);
	}

	bool line(const String & line) {
		int flow;
		if (3 != sscanf(~line, "Valve %c%c has flow rate=%d;", buf+0, buf+1, &flow)) return true;
		int16 i = Node::name2i(buf);
		Node & n = nodes[i];
		n.flow = flow;
		n.name.Set(buf, 2);
		int tunnels_in = line.FindAfter("to valve");
		ASSERT(-1 != tunnels_in);
		tunnels_in += 1 + ('s' == line[tunnels_in]);	// skip "s " or " "
		while (tunnels_in < line.GetLength()) {
			n.tunnels.Add(Node::name2i(~line + tunnels_in));
			tunnels_in += 4;
		}
		total_flow += flow;
		return false;							// not finished yet, try next line
	}

	void part1() {
		int most_pressure = 0;
		SortedIndex<Path, StdGreater<Path>> paths;
		SortedIndex<Path, StdGreater<Path>> solutions;
		Path starting_path;
		starting_path.p = "AA";
		starting_path.could = total_flow * (Path::TOTAL_TIME - 1);
		paths.Add(starting_path);

		while (paths.GetCount()) {
			ASSERT(paths[0].could);
			Path p0 = paths[0];
			paths.Remove(0);
			int ni = p0.node_i();
			const Node & n = nodes[ni];
			if (0 < n.flow && !p0.is_on(n)) {
				Path np = p0.grow(ni, true);
				if (most_pressure < np.released + np.could) {
					if (np.could) paths.Add(np);
					else solutions.Add(np);
					if (most_pressure < np.released) {
						most_pressure = np.released;
						Cout() << "- new max: " << most_pressure << " by\n  " << np.p << EOL;
					}
				}
				if (0 == np.could) continue;	// this was last valve to open
			}
			for (auto ti : n.tunnels) {
				Path np = p0.grow(ti);
				if (most_pressure < np.released + np.could) paths.Add(np);
			}
		}
		ASSERT(!solutions.IsEmpty());

		Cout() << "*** part 1 * most pressure you can release: " << most_pressure << EOL;
	}

	void part2() {
		int most_pressure = 0;
		SortedIndex<DualPath, StdGreater<DualPath>> paths;
		SortedIndex<DualPath, StdGreater<DualPath>> solutions;
		DualPath starting_path;
		starting_path.p[0] = "AA";
		starting_path.p[1] = "AA";
		starting_path.could[0] = starting_path.could[1] = total_flow * (DualPath::TOTAL_TIME - 1);
		paths.Add(starting_path);

		while (paths.GetCount()) {
			ASSERT(paths[0].could);
			DualPath p0 = paths[0];
			paths.Remove(0);
			int pi = p0.time_remaining(0) < p0.time_remaining(1);	// alter 0/1 (elephant/you)
			int ni = p0.node_i(pi);
			const Node & n = nodes[ni];
			if (0 < n.flow && !p0.is_on(n)) {
				DualPath np = p0.grow(pi, ni, true);
				if (most_pressure < np.released + np.get_could()) {
					if (np.get_could()) paths.Add(np);
					else solutions.Add(np);
					if (most_pressure < np.released) {
						most_pressure = np.released;
						Cout() << "- new max: " << most_pressure << " by\n  " << np.p[0] << " :\n  " << np.p[1] << EOL;
					}
				}
				if (0 == np.get_could()) continue;	// this was last valve to open
			}
			if (0 == pi) {
				for (auto ti : n.tunnels) {
					DualPath np = p0.grow(pi, ti);
					if (most_pressure < np.released + np.could[pi]) paths.Add(np);
				}
			} else {
				for (auto ti : ReverseRange(n.tunnels)) {
					DualPath np = p0.grow(pi, ti);
					if (most_pressure < np.released + np.could[pi]) paths.Add(np);
				}
			}
		}
		ASSERT(!solutions.IsEmpty());

		Cout() << "*** part 2 * most pressure you can release: " << most_pressure << EOL;
	}

	void finish() {
		Cout() << "- maximum possible flow " << total_flow << EOL;
		part1();
		part2();
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
	for (const String & arg : CommandLine()) lines_loop(Part1(), arg);
}
