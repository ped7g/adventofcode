#include <Core/Core.h>

using namespace Upp;

struct Position : Moveable<Position> {
	int x,y,z;
	Position(int x, int y, int z) : x(x), y(y), z(z) {}
	void operator += (const Position & b) { x += b.x, y += b.y, z += b.z; }
	void min(const Position &b) { x = std::min(x, b.x), y = std::min(y, b.y), z = std::min(z, b.z); }
	void max(const Position &b) { x = std::max(x, b.x), y = std::max(y, b.y), z = std::max(z, b.z); }
	const uint16 hash() const { return (z << 10) | (y << 5) | x; }
};

class BoilingBouldersTask {		// expected answers: sample.txt [64, 58], input.txt [3500, 2048]
	static const Position ORIGIN_OFS;
	static const Vector<Position> neighbours_ofs;

	Vector<Position> droplet;
	Index<uint16> hashed;
	Position boundary_min { 31, 31, 31 }, boundary_max { 0, 0, 0 };

public:

	void init() { Cout() << "boiling boulders"; }

	bool line(const String & line) {
		Position cube {-1, -1, -1};
		if (3 != sscanf(~line, "%d,%d,%d", &cube.x, &cube.y, &cube.z)) return true;
		cube += ORIGIN_OFS;						// leave space at coordinates 1 and 30 for fill
		boundary_min.min(cube), boundary_max.max(cube);
		droplet.Add(cube), hashed.Add(cube.hash());
		return false;
	}

	void finish() {
		boundary_min += { -1, -1, -1 }, boundary_max += { 1, 1, 1 };
		// must fit uint16 hash even after +-1 -> only 1..30 are valid boundary coordinates
		ASSERT( 1 <= boundary_min.x &&  1 <= boundary_min.y &&  1 <= boundary_min.z);
		ASSERT(boundary_max.x <= 30 && boundary_max.y <= 30 && boundary_max.z <= 30);
		// 3D flood-fill the "outside of droplet" volume in boundary_min -> boundary_max
		Index<uint16> outside { boundary_min.hash() };		// hash-marks of all "outside" air
		BiVector<Position> outside_to_fill { boundary_min };
		while (!outside_to_fill.IsEmpty()) {
			const auto cube = outside_to_fill.PopHead();
			if (cube.x < boundary_min.x || cube.y < boundary_min.y || cube.z < boundary_min.z) continue;
			if (boundary_max.x < cube.x || boundary_max.y < cube.y || boundary_max.z < cube.z) continue;
			for (auto ofs : neighbours_ofs) {	// flood-fill all free cubes around current one
				ofs += cube;
				auto hash = ofs.hash();
				if (0 <= hashed.Find(hash) || 0 <= outside.Find(hash)) continue;	// lava/marked-already
				outside_to_fill.AddTail(ofs), outside.Add(hash);	// enqueue and mark it
			}
		}
		// calculate full and "outside" surface of droplet
		int full_surface = 0, outside_surface = 0;
		for (const auto & cube : droplet) for (auto ofs : neighbours_ofs) {
			ofs += cube;
			full_surface += (hashed.Find(ofs.hash()) < 0);
			outside_surface += (0 <= outside.Find(ofs.hash()));
		}
		Cout()	<< "droplet cubes: " << hashed.GetCount() << ", full surface: " << full_surface
				<< ", outside surface: " << outside_surface << EOL;
	}
};

const Position BoilingBouldersTask::ORIGIN_OFS { +2, +2, +2 };
const Vector<Position> BoilingBouldersTask::neighbours_ofs {
	{-1,0,0}, {+1,0,0}, {0,-1,0}, {0,+1,0}, {0,0,-1}, {0,0,+1}
};

// concept of part1/part2 task having init/line(line)/finish interface
template <typename T> concept TaskProcessingLines = requires(T task, const String & line) {
	task.init();
	{ task.line(line) } -> std::same_as<bool>;	// return true finished early
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
	for (const String & arg : CommandLine()) lines_loop(BoilingBouldersTask(), arg);
}
