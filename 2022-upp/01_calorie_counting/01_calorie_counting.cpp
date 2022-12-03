#include <Core/Core.h>

using namespace Upp;

void part1(const String & filename) {
	FileIn in(GetDataFile(filename));
	if (!in) return;
	Cout() << "input filename: " << filename << EOL;
	int maxcal = 0, elfcal = 0;
	while (true) {
		String line = in.GetLine();
		if (!line.IsEmpty()) {
			elfcal += StrInt(line);
			continue;
		}
		maxcal = max(maxcal, elfcal);
		elfcal = 0;
		if (in.IsEof()) break;
	}
	Cout() << "max cal: " << maxcal << EOL;
}

void part2(const String & filename) {
	FileIn in(GetDataFile(filename));
	if (!in) return;
	Cout() << "input filename: " << filename << EOL;
	int maxcals[3] { 0, 0, 0 }, elfcal = 0;
	while (true) {
		String line = in.GetLine();
		if (!line.IsEmpty()) {
			elfcal += StrInt(line);
			continue;
		}
		for (int & maxcal : maxcals) {
			if (elfcal <= maxcal) continue;
			Swap(maxcal, elfcal);
		}
		elfcal = 0;
		if (in.IsEof()) break;
	}
	int maxcalsum = 0;
	for (const int maxcal : maxcals) maxcalsum += maxcal;
	Cout() << "Top 3 sum cal: " << maxcalsum << EOL;
}

CONSOLE_APP_MAIN
{
	const Vector<String>& cmdline = CommandLine();
	for(int i = 0; i < cmdline.GetCount(); i++) {
		part1(cmdline[i]);
		part2(cmdline[i]);
	}
}
