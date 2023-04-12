#include <Core/Core.h>

using namespace Upp;

void incString(String & s) {
	for (int i = s.GetCount(); i--;) {
		s.Set(i, s[i] + 1);
		if ('z' < s[i]) s.Set(i, 'a'); else return;
	}
}

bool isInvalid(const String & s) {
	int triplet = 0;
	char p1 = 0, p2 = 0;
	for (int i = s.GetCount(); i--;) {
		if ('i' == s[i] || 'o' == s[i] || 'l' == s[i]) return true;	// "iol" -> invalid
		if (i && s[i-1] == s[i]) {		// check pairs
			if (0 == p1) p1 = s[i];
			else if (s[i] != p1) p2 = s[i];
		}
		if (2 <= i && s[i-2] + 2 == s[i] && s[i-1] + 1 == s[i]) triplet = i;
	}
	return 0 == (triplet * p1 * p2);
}

CONSOLE_APP_MAIN {
	for (String s : { "abcdefgh", "ghijklmn", "hepxcrrq", "hepxxyzz" }) {
		Cout() << s << " -> ";
		do incString(s); while (isInvalid(s));
		Cout() << s << EOL;
	} // expected answer: hepxcrrq -> hepxxyzz, hepxxyzz -> heqaabcc
}
