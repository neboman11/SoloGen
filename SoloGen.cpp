#include <AudioFile/AudioFile.h>
#include "tabGen.h"
#include "cmdParse.h"
#include "guitarConsts.h"

using namespace std;

const double SAMPLE_RATE = 44100.0;

const double SECONDS_PER_SAMPLE = 1 / SAMPLE_RATE;

const int BUFFER_SIZE = 512;

const string FLAC_COMMAND = "flac -s -f -o ";

void convertWAVtoFLAC(string fileName);
double noteToFreq(int* note);
void createWAV(vector<int*> notes, map<int, string> givenOptions);
void outputTabChrom(map<int, string> givenOptions);
void outputTabPenta(map<int, string> givenOptions);

int main(int argc, char** argv)
{
	map<int, string> givenOptions;
	string answer;
	srand(time(NULL));	// Set rand() seed to current time to give pseudorandom effect

	givenOptions = parseCMD(argc, argv);

	if (givenOptions[CHROMATIC] == "true")
	{
		cout << "Now generating a chromatic solo..."  << endl;
		outputTabChrom(givenOptions);
	}
	cout << endl;

	if (givenOptions[PENTATONIC] == "true")
	{
		cout << "Now generating a pentatonic solo..."  << endl;
		outputTabPenta(givenOptions);
	}

	if (givenOptions[FLAC] == "true")
	{
		convertWAVtoFLAC(givenOptions[OUTFILE]);
	}
}

void convertWAVtoFLAC(string fileName)
{
	// FILE object for reading from the pipe for the gawk command                                                                                                                                                                               
	FILE* pipeTown;
	// Array of characters to use as a buffer for reading from the pipe                                                                                                                                                                         
	char buffer[BUFFER_SIZE];
	// Pointer to the currently read in line from the pipe                                                                                                                                                                                      
	char* workingLine;

	// Show the command being run
	cout << FLAC_COMMAND + fileName.substr(0, fileName.length() - 3) + "flac " + fileName << endl;

	// Open the pipe with the given command in read mode
	pipeTown = popen((FLAC_COMMAND + fileName.substr(0, fileName.length() - 3) + "flac " + fileName).c_str(), "r");

	// If the pipe failed to open
	if (!pipeTown)
	{
		// Let the user know
		cerr << "Failed to run command: " << FLAC_COMMAND + fileName.substr(0, fileName.length() - 3) + "flac " + fileName << endl;
		return;
	}

	// Read in the first buffer space of the pipe output                                                                                                                                                                                        
	workingLine = fgets(buffer, BUFFER_SIZE, pipeTown);

	// Loop until there is nothing more to be read from the buffer                                                                                                                                                                              
	while (workingLine != NULL)
	{
		// Print out the contents of the current working line                                                                                                                                                                                     
		printf("%s", workingLine);
		// Read in the next buffer space of the pipe output                                                                                                                                                                                       
		workingLine = fgets(buffer, BUFFER_SIZE, pipeTown);
	}

	// Close the pipe                                                                                                                                                                                                                           
	pclose(pipeTown);

	// Show the command being run
	cout << "rm -f " + fileName << endl;

	// Open the pipe with the given command in read mode
	pipeTown = popen(("rm -f " + fileName).c_str(), "r");

	// If the pipe failed to open
	if (!pipeTown)
	{
		// Let the user know
		cerr << "Failed to run command: " << "rm -f " + fileName << endl;
		return;
	}

	// Read in the first buffer space of the pipe output                                                                                                                                                                                        
	workingLine = fgets(buffer, BUFFER_SIZE, pipeTown);

	// Loop until there is nothing more to be read from the buffer                                                                                                                                                                              
	while (workingLine != NULL)
	{
		// Print out the contents of the current working line                                                                                                                                                                                     
		printf("%s", workingLine);
		// Read in the next buffer space of the pipe output                                                                                                                                                                                       
		workingLine = fgets(buffer, BUFFER_SIZE, pipeTown);
	}

	// Close the pipe                                                                                                                                                                                                                           
	pclose(pipeTown);
}

double noteToFreq(int* note)
{
	int noteString = note[1];
	int noteFret = note[0];

	double base_freq = LOW_E_FREQ * pow(SEMITONE_STEP, STRING_STEPS[noteString]);

	double noteFreq = base_freq * pow(SEMITONE_STEP, noteFret);

	return noteFreq;
}

void createWAV(vector<int*> notes, map<int, string> givenOptions)
{
	AudioFile<double> audioFile;

	AudioFile<double>::AudioBuffer buffer;

	int wavLength = SAMPLE_RATE * stoi(givenOptions[LENGTH]);
	int numBeats = stoi(givenOptions[LENGTH]) * stoi(givenOptions[TEMPO]) / 60;

	buffer.resize(2);

	buffer[0].resize(wavLength);
	buffer[1].resize(wavLength);

	int numChannels = 2;
	int numSamplesPerChannel = wavLength;
	int numSamplesPerBeat = wavLength / numBeats;
	int noteLength = 0;
	int currentNote = 0;

	for (int i = 0; i < numSamplesPerChannel; i++)
	{
		float sample = sinf(2. * M_PI * ((float) i / SAMPLE_RATE) * noteToFreq(notes.at(currentNote)));

		for (int channel = 0; channel < numChannels; channel++)
		{
			buffer[channel][i] = sample * 0.5;
		}

		if (i > (notes.at(currentNote)[2] * numSamplesPerBeat) + noteLength)
		{
			noteLength += notes.at(currentNote)[2] * numSamplesPerBeat;
			currentNote++;
		}

		if (currentNote >= (int)notes.size())
		{
			currentNote--;
		}
	}

	audioFile.setAudioBuffer(buffer);

	audioFile.setAudioBufferSize(numChannels, numSamplesPerChannel);

	audioFile.setNumSamplesPerChannel(numSamplesPerChannel);

	audioFile.setNumChannels(numChannels);

	audioFile.setBitDepth(24);
	audioFile.setSampleRate(44100);

	audioFile.save(givenOptions[OUTFILE]);
}

void outputTabChrom(map<int, string> givenOptions)
{
	vector<int*> notes;		// Vector of arrays of integers containing note info (pitch, length)
	vector<string*> tab;	// Vector of arrays of strings containing each note per beat

	int numBeats = stoi(givenOptions[LENGTH]) * stoi(givenOptions[TEMPO]) / 60;

	// Generate random notes and place them into vector <notes>
	notes = genRandNotes(numBeats);

	// Initialize vector <tab> with hyphens to indicate each string
	for (int i = 0; i < numBeats; i++)
	{
		tab.push_back(new string[6]);
		for (int j = 0; j < 6; j++)
		{
			tab.at(i)[j] = " - ";
		}
	}

	int beatPos = 0;	// Accumulator to keep position beat-wise in the tab
	// Loops through the arrays in vector <notes> and places the contained notes in position
	// in the tab
	for (int i = 0; i < (int)notes.size(); i++)
	{
		if (notes.at(i)[0] > 9)
		{
			tab.at(beatPos)[notes.at(i)[1]] = " " + to_string(notes.at(i)[0]);
			beatPos += notes.at(i)[2];
		}
		else if (notes.at(i)[0] < 10)
		{
			tab.at(beatPos)[notes.at(i)[1]] = " " + to_string(notes.at(i)[0]) + " ";
			beatPos += notes.at(i)[2];
		}
	}

	// Output of the final tab
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < numBeats; j++)
			cout << tab.at(j)[i];
		cout << endl;
	}

	createWAV(notes, givenOptions);
}

void outputTabPenta(map<int, string> givenOptions)
{
	vector<int*> notes;		// Vector of arrays of integers containing note info (pitch, length)
	vector<string*> tab;	// Vector of arrays of strings containing each note per beat

	int numBeats = stoi(givenOptions[LENGTH]) * stoi(givenOptions[TEMPO]) / 60;

	// Generate random notes and place them into vector <notes>
	notes = genRandNotesPos(numBeats);

	// Initialize vector <tab> with hyphens to indicate each string
	for (int i = 0; i < numBeats; i++)
	{
		tab.push_back(new string[6]);
		for (int j = 0; j < 6; j++)
		{
			tab.at(i)[j] = " - ";
		}
	}

	for (int i = 0; i < (int)notes.size(); i++)
	{
		int* pitch = convPosToNote(notes.at(i)[0]);
		notes.at(i) = new int[3] {pitch[0], pitch[1], notes.at(i)[1]};
	}

	int beatPos = 0;	// Accumulator to keep position beat-wise in the tab
	// Loops through the arrays in vector <notes> and places the contained notes in position
	// in the tab
	for (int i = 0; i < (int)notes.size(); i++)
	{
		if (notes.at(i)[0] > 9)
		{
			tab.at(beatPos)[notes.at(i)[1]] = " " + to_string(notes.at(i)[0]);
			beatPos += notes.at(i)[2];
		}
		else if (notes.at(i)[0] < 10)
		{
			tab.at(beatPos)[notes.at(i)[1]] = " " + to_string(notes.at(i)[0]) + " ";
			beatPos += notes.at(i)[2];
		}
	}

	// Output of the final tab
	for (int i = 0; i < 6; i++)
	{
		for (int j = 0; j < numBeats; j++)
			cout << tab.at(j)[i];
		cout << endl;
	}

	createWAV(notes, givenOptions);
}
