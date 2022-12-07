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

struct AocFile : Moveable<AocFile>  {
	String p;	// full path
	int64 s;		// size

	AocFile(const String & path, const String & name, int64 size) :
		p(path + name), s(size) {}

	bool operator <(const AocFile & b) const {
		//Cout() << "compare " << p << " vs " << b.p << " = " << p.Compare(b.p) << EOL;
		return p.Compare(b.p) < 0;
	}

	String ToString() const {
		return Format("%s %d", p, Value(s));
	}
};

static ConvertInt64 stoi64;

class Part1 {	// expected answers: sample 95437, input 1886043

	String pwd_;
	Vector<AocFile> files_;

public:

	void init() { Cout() << "part1"; }

	bool line(const String & line) {
		if (line.IsEmpty()) return true;
		auto words = Split(line, ' ');
		if ('$' == line[0]) {
			if (0 == words[1].Compare("cd")) {
				if (0 == words[2].Compare("/")) {
					pwd_ = "/";
				} else if (0 == words[2].Compare("..")) {
					int pd = pwd_.ReverseFind('/', pwd_.GetLength() - 2);
					if (pd >= 0) pwd_.TrimLast(pwd_.GetLength() - pd - 1);
				} else {
					pwd_.Cat(words[2]);
					pwd_.Cat("/");
				}
			} else if (0 == words[1].Compare("ls")) {
				// Cout() << "ls " << pwd_ << EOL;
			}
		} else if (0 == words[0].Compare("dir")) {
		} else {
			files_.Add(AocFile(pwd_, words[1], stoi64.Scan(words[0])));
		}
		return false;							// not finished yet, try next line
	}

	void finish() {
		// sort files to make "group by path" algorithm possible
		Sort(files_);
		// sum all folders in "group by folder" way with cummulative nesting
		Vector<int64> sizes;
		String pwd = "/";
		int64 sz = 0, at_most_100k_sum = 0;
		files_.Add(AocFile("/", "", 0));		// fake empty file to close summing of root size
		for (const auto & f : files_) {
			while (!f.p.StartsWith(pwd)) {
				//Cout() << pwd << " = " << sz << EOL;
				if (sz <= 100000) at_most_100k_sum += sz;
				int slash = pwd.ReverseFind('/', pwd.GetLength() - 2);
				if (slash < 0) break;
				pwd.Trim(slash + 1);
				sz += sizes.Pop();
			}
			int nextdir, pwdl;
			while (0 <= (pwdl = pwd.GetLength(), nextdir = f.p.Find('/', pwdl))) {
				sizes.Add(sz);
				pwd.Cat(f.p.Mid(pwdl, nextdir - pwdl + 1));
				sz = 0;
			}
			sz += f.s;
		}
		ASSERT(sizes.IsEmpty());
		Cout() << pwd << " = " << sz << EOL;
		Cout() << "part1: " << at_most_100k_sum << EOL;
	}
};

class Part2 {	// expected answers: sample 24933642, input 3842121

	static constexpr int64 FS_SIZE = 70000000;
	static constexpr int64 UPDATE_REQ = 30000000;

	String pwd_;
	Vector<AocFile> files_;

public:

	void init() { Cout() << "part2"; }

	bool line(const String & line) {
		if (line.IsEmpty()) return true;
		auto words = Split(line, ' ');
		if ('$' == line[0]) {
			if (0 == words[1].Compare("cd")) {
				if (0 == words[2].Compare("/")) {
					pwd_ = "/";
				} else if (0 == words[2].Compare("..")) {
					int pd = pwd_.ReverseFind('/', pwd_.GetLength() - 2);
					if (pd >= 0) pwd_.TrimLast(pwd_.GetLength() - pd - 1);
				} else {
					pwd_.Cat(words[2]);
					pwd_.Cat("/");
				}
			} else if (0 == words[1].Compare("ls")) {
				// Cout() << "ls " << pwd_ << EOL;
			}
		} else if (0 == words[0].Compare("dir")) {
		} else {
			files_.Add(AocFile(pwd_, words[1], stoi64.Scan(words[0])));
		}
		return false;							// not finished yet, try next line
	}

	void finish() {
		// sort files to make "group by path" algorithm possible
		Sort(files_);
		// sum all folders in "group by folder" way with cummulative nesting
		Vector<int64> dir_sizes;
		Vector<int64> sizes;
		String pwd = "/";
		int64 sz = 0;
		files_.Add(AocFile("/", "", 0));		// fake empty file to close summing of root size
		for (const auto & f : files_) {
			while (!f.p.StartsWith(pwd)) {
				//Cout() << pwd << " = " << sz << EOL;
				dir_sizes.Add(sz);
				int slash = pwd.ReverseFind('/', pwd.GetLength() - 2);
				if (slash < 0) break;
				pwd.Trim(slash + 1);
				sz += sizes.Pop();
			}
			int nextdir, pwdl;
			while (0 <= (pwdl = pwd.GetLength(), nextdir = f.p.Find('/', pwdl))) {
				sizes.Add(sz);
				pwd.Cat(f.p.Mid(pwdl, nextdir - pwdl + 1));
				sz = 0;
			}
			sz += f.s;
		}
		ASSERT(sizes.IsEmpty());
		int64 to_free = UPDATE_REQ - (FS_SIZE - sz);
		Cout() << pwd << " = " << sz << " to free: " << to_free << EOL;
		// find the smallest directory size which will free enough space
		int64 min_dir = INT64_MAX;
		for (const auto dsz : dir_sizes) {
			if (dsz < to_free) continue;
			min_dir = std::min(min_dir, dsz);
		}
		Cout() << "part2: " << min_dir << EOL;
	}
};

CONSOLE_APP_MAIN
{
	const Vector<String>& cmdline = CommandLine();
	for(int i = 0; i < cmdline.GetCount(); i++) {
		lines_loop(Part1(), cmdline[i]);
		lines_loop(Part2(), cmdline[i]);
	}
}
