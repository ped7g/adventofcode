#include <Core/Core.h>

// expect results:
// - sample2.txt -1 / 12173837295 ([12, -11, -2] / [9739069836, 1623178306, 811589153])
// - sample.txt  3 / 1623178306 ([4, -3, 2] / [811589153, 2434767459, -1623178306])
// - input.txt   13522 / 17113168880158 ([-483, 6451, 7554] / [6551959232169, 4219452006447, 6341757641542])

using namespace Upp;
static constexpr bool DBG_OUT = 0;

struct ValueWithPosition : Moveable<ValueWithPosition> {
	int64 v;
	int i;
	String ToString() const { return AsString(v) + "[" + AsString(i) + "]"; }
};

struct AoC2022Day20Task {

	Vector<ValueWithPosition> in;

	void init() { Cout() << "***"; }

	bool line(const String & line) {
		if (line.IsEmpty()) return true;
		return in.Add({ .v = atoi(~line), .i = in.GetCount() }), false;
	}

	void mix(Vector<ValueWithPosition> & data) {
		const int sz = data.GetCount(), mod = sz - 1;
		for (int i = 0, at, new_at; i < sz; ++i) {		// mix all items in original order
			if (DBG_OUT && sz < 40) Cout() << "mixing  " << data.ToString() << EOL;
			for (at = sz; at-- && data[at].i != i; ) ;	// find original order item
			new_at = (data[at].v + at) % mod;
			if (new_at < 0) new_at += mod;				// figure out where it will move
			ValueWithPosition temp = data[at];			// move it
			if (at < new_at) {
				memmove(data.Begin() + at, data.Begin() + at + 1, sizeof(ValueWithPosition) * (new_at - at));
				data[new_at] = temp;
			} else if (new_at < at) {
				memmove(data.Begin() + new_at + 1, data.Begin() + new_at, sizeof(ValueWithPosition) * (at - new_at));
				data[new_at] = temp;
			}
		}
		if (DBG_OUT && sz < 40) Cout() << "mix out " << data.ToString() << EOL;
	}

	String resultToString(const Vector<ValueWithPosition> & data) {
		Vector<int64> r_items { 1000, 2000, 3000 };		// positions in, values out
		int zero_at = data.GetCount();
		while (zero_at-- && data[zero_at].v) {}
		for (auto & r : r_items) r = data[(zero_at + r) % data.GetCount()].v;
		return r_items.ToString() + " = " + AsString(Sum(r_items));
	}

	void finish() {
		auto p1_data = clone(in);						// part 1
		mix(p1_data);
		Cout() << "part1: " << resultToString(p1_data) << EOL;
		auto p2_data = clone(in);						// part 2
		for (auto & vp : p2_data) vp.v *= 811589153L;
		for (int i = 10; i--; ) mix(p2_data);
		Cout() << "part2: " << resultToString(p2_data) << EOL;
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
	for (const String & arg : CommandLine()) lines_loop(AoC2022Day20Task(), arg);
}
