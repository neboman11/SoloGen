#include <tclap/CmdLine.h>
#include <map>
#include <string>

using namespace std;

enum CmdOptions {
	LENGTH,
	TEMPO,
	OUTFILE,
	FLAC,
	CHROMATIC,
	PENTATONIC,
	TIME_SIGNATURE
};

const string TIME_SIGNATURES[7] = {
	"2/2",
	"3/4",
	"4/4",
	"5/4",
	"7/4",
	"6/8",
	"8/8"
};

map<int, string> parseCMD(int argc, char** argv);