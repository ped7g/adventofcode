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
	FileIn in(FileExists(filename) ? filename : GetDataFile(filename)); // 2nd try exe-dir
	if (!in) return;
	task.init();
	Cout() << " input filename: " << filename << EOL;
	while (!task.line(in.GetLine()) && !in.IsEof()) ;
	task.finish();
}

static ConvertInt64 stoi64;

struct AocFile : Moveable<AocFile>  {
	String p;		// full path
	int64 s;		// size

	AocFile(const String & path, const String & name, int64 size)
		: p(path + "/" + name), s(size) {}

	// for sort of files by full path
	int operator <=> (const AocFile & b) const { return p.Compare(b.p); }
};

class BothParts {

	static constexpr int64 FS_SIZE		= 70'000'000;
	static constexpr int64 UPDATE_REQ	= 30'000'000;

	String parse_dir_;
	bool ignore_listed_files_ = true;
	Index<String> listed_;
	Vector<AocFile> files_;

public:

	void init() { Cout() << "parsing"; }

	bool line(const String & line) {
		if (line.IsEmpty()) return true;		// invalid input
		auto words = Split(line, ' ');
		if ('$' == line[0]) {					// user command
			ignore_listed_files_ = true;		// ignore any unexpected listing output
			if (!words[1].Compare("cd")) {
				// CD command, keep track of current directory (in parse_dir_ var)
				if (!words[2].Compare("/")) {
					parse_dir_.Trim(0);
				} else if (!words[2].Compare("..")) {	// remove last directory from parse_dir_
					int pd = parse_dir_.ReverseFind('/');	// find parent dir
					if (pd >= 0) parse_dir_.TrimLast(parse_dir_.GetLength() - pd);
				} else {								// append new directory to parse_dir_
					parse_dir_.Cat('/');
					parse_dir_.Cat(words[2]);
				}
			} else if (!words[1].Compare("ls")) {
				// protect against duplicate ls (not part of AoC inputs)
				ignore_listed_files_ = (0 <= listed_.Find(parse_dir_));
				if (!ignore_listed_files_) listed_.Add(parse_dir_);
			}
		} else if (!words[0].Compare("dir")) {	// listing of sub-directory
			// nothing to do, files are stored with full paths as flat vector, not using FS tree
		} else {								// file with size, add it to files_
			if (!ignore_listed_files_) {
				files_.Add(AocFile(parse_dir_, words[1], stoi64.Scan(words[0])));
			}
		}
		return false;							// not finished yet, try next line
	}

	void finish() {
		// sort files to make "group by directory" algorithm possible
		Sort(files_);
		// sum all files in "group by directory" way with cummulative nesting
		int64 dir_size = 0, at_most_100k_sum = 0;	// sum of <= 100k dirs (part 1)
		Vector<int64> dir_sizes;				// list of all possible directory sizes (part 2)
		Vector<int64> sizes;					// stack to hold size of parent directories
		String group_by = "/";
		files_.Add(AocFile("", "", 0));			// fake "/" file to finish summing of last dir
		for (const auto & f : files_) {
			while (!f.p.StartsWith(group_by)) {	// next file has different dir, finish group
				if (dir_size <= 100'000) at_most_100k_sum += dir_size;	// sum part 1 result
				dir_sizes.Add(dir_size);		// record dir size for part 2 result
				int slash = group_by.ReverseFind('/', group_by.GetLength() - 2);
				if (slash < 0) break;
				group_by.Trim(slash + 1);
				dir_size += sizes.Pop();		// update parent dir size with closed dir size
			}
			int next_dir, gbl;
			while (0 <= (gbl = group_by.GetLength(), next_dir = f.p.Find('/', gbl))) {
				sizes.Add(dir_size);			// preserve current dir size as new parent dir
				group_by = f.p.Left(next_dir + 1);		// update group_by to include new dir
				dir_size = 0;					// reset current dir size to zero
			}
			dir_size += f.s;					// add file size to current directory sum
		}
		ASSERT(sizes.IsEmpty());
		// all directories summed, find the smallest one to delete to have enough free space
		int64 to_free = UPDATE_REQ - (FS_SIZE - dir_size);
		Cout() << group_by << " = " << dir_size << " to free: " << to_free << EOL;	// debug
		// find the smallest directory size which will free enough space
		int64 min_dir = INT64_MAX;
		for (const auto size : dir_sizes) {
			if (size < to_free) continue;
			min_dir = std::min(min_dir, size);
		}
		// display results
		Cout() << "part1: " << at_most_100k_sum << EOL;
		Cout() << "part2: " << min_dir << EOL;
	}
};

// expected answers: sample [95437, 24933642], input [1886043, 3842121], tricky [95437, 24933642]
CONSOLE_APP_MAIN
{
	const Vector<String>& cmdline = CommandLine();
	for(int i = 0; i < cmdline.GetCount(); i++) {
		lines_loop(BothParts(), cmdline[i]);
	}
}
