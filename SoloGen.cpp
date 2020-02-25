// SoloGen.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <cstdlib>
#include <time.h>
#include <string>
#include <vector>
#include <cmath>
#include <math.h>
#include <map>
#include <stdlib.h>
#include <stdio.h>
#include <AudioFile/AudioFile.h>
#include <tclap/CmdLine.h>

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

const double SEMITONE_STEP = pow(2.0, 1.0/12);

const double LOW_E_FREQ = 82.4;

const int STRING_STEPS[5] = {5, 5, 5, 4, 5};

const int N_FRETS = 24;

const double SAMPLE_RATE = 44100.0;

const double SECONDS_PER_SAMPLE = 1 / SAMPLE_RATE;

const int BUFFER_SIZE = 512;

const string TIME_SIGNATURES[7] = {
	"2/2",
	"3/4",
	"4/4",
	"5/4",
	"7/4",
	"6/8",
	"8/8"
};

void convertWAVtoFLAC(string fileName);
double noteToFreq(int* note);
void createWAV(vector<int*> notes, map<int, string> givenOptions);
map<int, string> parseCMD(int argc, char** argv);
void outputTabChrom(map<int, string> givenOptions);
void outputTabPenta(map<int, string> givenOptions);
int randPos();
int randFret();
int randString();
int randNoteLength();
vector<int*> genRandNotes(int numBeats);
vector<int*> genRandNotesPos(int numBeats);
int* convPosToNote(int pos);

int main(int argc, char** argv)
{
	map<int, string> givenOptions;
	string answer;
	srand(time(NULL));	// Set rand() seed to current time to give pseudorandom effect
	// int tempo = 80;		// Desired output tempo
	// char key = 'A';		// Key used for generating PentaScale

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
	cout << "flac -f -o " + fileName.substr(0, fileName.length() - 3) + "flac " + fileName << endl;

	// Open the pipe with the given command in read mode
	pipeTown = popen(("flac -f -o " + fileName.substr(0, fileName.length() - 3) + "flac " + fileName).c_str(), "r");

	// If the pipe failed to open
	if (!pipeTown)
	{
		// Let the user know
		cerr << "Failed to run command: " << "flac -f -o " + fileName.substr(0, fileName.length() - 3) + "flac " + fileName << endl;
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

map<int, string> parseCMD(int argc, char** argv)
{
	map<int, string> givenOptions;

	try
	{
		TCLAP::CmdLine cmd("SoloGen", ' ', "0.1");

		TCLAP::ValueArg<int> length("l", "length", "The length of the solo to generate in seconds", true, 5, "time (s)");

		cmd.add(length);

		TCLAP::ValueArg<int> tempo("t", "tempo", "The tempo of the resulting audio file", true, 120, "tempo (bpm)");

		cmd.add(tempo);

		TCLAP::ValueArg<string> outFile("o", "output", "The name of the output file", false, "output.wav", "filename");

		cmd.add(outFile);

		TCLAP::SwitchArg flacConvert("f", "flac", "Output the audio file in flac format. Flac must be installed for this to work", false);

		cmd.add(flacConvert);

		TCLAP::SwitchArg chromatic("c", "chromatic", "Generate a solo using the chromatic scale", false);

		cmd.add(chromatic);

		TCLAP::SwitchArg pentatonic("p", "pentatonic", "Generate a solo using the minor pentatonic scale", false);

		cmd.add(pentatonic);

		TCLAP::ValueArg<string> timeSig("s", "time-signature", "The time signature to generate for", true, "4/4", "time signature (#/#)");

		cmd.add(timeSig);

		cmd.parse(argc, argv);

		givenOptions[LENGTH] = to_string(length.getValue());

		givenOptions[TEMPO] = to_string(tempo.getValue());

		// Check if converting to flac if output file extension matches flac instead of wav
		if (outFile.getValue().substr(outFile.getValue().length() - 4, outFile.getValue().length()) == ".wav")
		{
			givenOptions[OUTFILE] = outFile.getValue();
		}

		else
		{
			givenOptions[OUTFILE] = outFile.getValue() + ".wav";
		}
		

		if (flacConvert.getValue())
		{
			givenOptions[FLAC] = "true";
		}

		else
		{
			givenOptions[FLAC] = "false";
		}

		if (chromatic.getValue() && pentatonic.getValue() && pentatonic.isSet())
		{
			cerr << "Only one scale option may be given." << endl;
			throw new exception();
		}

		if (chromatic.getValue())
		{
			givenOptions[CHROMATIC] = "true";
			givenOptions[PENTATONIC] = "false";
		}

		else
		{
			givenOptions[CHROMATIC] = "false";
			givenOptions[PENTATONIC] = "true";
		}

		bool validSig = false;

		for (string s : TIME_SIGNATURES)
		{
			if (s == timeSig.getValue())
			{
				validSig = true;
			}
		}

		if (validSig)
		{
			givenOptions[TIME_SIGNATURE] = timeSig.getValue();
		}

		else
		{
			cerr << "Invalid or unknown time signature given." << endl;
			throw new exception();
		}
		
	}

	catch(TCLAP::ArgException& e)
	{
		std::cerr << e.argId() << " threw error " << e.error() << endl;
	}
	
	return givenOptions;
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

// Returns a random integer 0 >= x > 30 to be converted into a PentaScale Position
int randPos()
{
	return (int)(((double)(rand()) / RAND_MAX) * 35);
}

// Returns a random integer 0 >= x > 22 to be converted to a guitar fret
int randFret()
{
	return (int)(((double)(rand()) / RAND_MAX) * 22);
}

// Returns a random integer 0 >= x > 6 to be converted into a guitar string
int randString()
{
	return (int)(((double)(rand()) / RAND_MAX) * 6);
}

// Returns a random integer 2 ^ (0 >= x > 4; where x is an integer) to be interpretted as a beat length, ie. whole note, half note, quarter note, etc.
int randNoteLength()
{
	return pow(2, (int)(((double)(rand()) / RAND_MAX) * 4));
}

// Takes the max number of beats for it to generate and returns a vector of pointers to
// arrays of notes (pitch (string and fret) and length). Gets a random fret, string, and
// note length, assigns them to an array and then adds the array to a vector. Checks to
// make sure length of notes in vector are not longer than number of beats passed to it
vector<int*> genRandNotes(int numBeats)
{
	int totalBeats = 0;		// Accumulator of generated note lengths
	int* genNotes = NULL;	// Pointer for dynamic array creation that holds newly generated notes
	vector<int*> notes;		// Vector of arrays that hold all the generated notes

	// Loop until the number of beats for all the notes meets or exceeds the max given
	for (totalBeats = 0; totalBeats < numBeats;)
	{
		int fret = randFret();
		int guitarString = randString();
		int beats = randNoteLength();
		genNotes = new int[3];
		genNotes[0] = fret;
		genNotes[1] = guitarString;
		genNotes[2] = beats;
		notes.push_back(genNotes);
		totalBeats += beats;
		if (totalBeats == numBeats)
			break;
		else if (totalBeats > numBeats)
			notes.pop_back();
	}
	return notes;
}

// Takes the max number of beats for it to generate and returns a vector of pointers to
// arrays of PentaScale positions and notes. Gets a random position and note length,
// assigns them to an array and then adds the array to a vector. Checks to make sure
// length of notes in vector are not longer than number of beats passed to it
vector<int*> genRandNotesPos(int numBeats)
{
	int totalBeats = 0;		// Accumulator of generated note lengths
	int* genNotes = NULL;	// Pointer for dynamic array creation that holds newly generated notes
	vector<int*> notes;		// Vector of arrays that hold all the generated notes

	// Loop until the number of beats for all the notes meets or exceeds the max given
	for (totalBeats = 0; totalBeats < numBeats;)
	{
		int pos = randPos();
		int beats = randNoteLength();
		genNotes = new int[2];
		genNotes[0] = pos;
		genNotes[1] = beats;
		notes.push_back(genNotes);
		totalBeats += beats;
		if (totalBeats == numBeats)
			break;
		else if (totalBeats > numBeats)
			notes.pop_back();
	}
	return notes;
}

// Takes an integer representing the position in the PentaScale and returns a pointer to an array
// containing the note pitch (string and fret)
int * convPosToNote(int pos)
{
	if (pos == 0)
		return new int[2] {0, 0};
	else if (pos == 1)
		return new int[2] {0, 1};
	else if (pos == 2)
		return new int[2] {0, 2};
	else if (pos == 3)
		return new int[2] {0, 3};
	else if (pos == 4)
		return new int[2] {0, 4};
	else if (pos == 5)
		return new int[2] {0, 5};
	else if (pos == 6)
		return new int[2] {3, 0};
	else if (pos == 7)
		return new int[2] {2, 1};
	else if (pos == 8)
		return new int[2] {2, 2};
	else if (pos == 9)
		return new int[2] {2, 3};
	else if (pos == 10)
		return new int[2] {3, 4};
	else if (pos == 11)
		return new int[2] {3, 5};
	else if (pos == 12)
		return new int[2] {5, 0};
	else if (pos == 13)
		return new int[2] {5, 1};
	else if (pos == 14)
		return new int[2] {5, 2};
	else if (pos == 15)
		return new int[2] {4, 3};
	else if (pos == 16)
		return new int[2] {5, 4};
	else if (pos == 17)
		return new int[2] {5, 5};
	else if (pos == 18)
		return new int[2] {7, 0};
	else if (pos == 19)
		return new int[2] {7, 1};
	else if (pos == 20)
		return new int[2] {7, 2};
	else if (pos == 21)
		return new int[2] {7, 3};
	else if (pos == 22)
		return new int[2] {8, 4};
	else if (pos == 23)
		return new int[2] {7, 5};
	else if (pos == 24)
		return new int[2] {10, 0};
	else if (pos == 25)
		return new int[2] {10, 1};
	else if (pos == 26)
		return new int[2] {9, 2};
	else if (pos == 27)
		return new int[2] {9, 3};
	else if (pos == 28)
		return new int[2] {10, 4};
	else if (pos == 29)
		return new int[2] {10, 5};
	else if (pos == 30)
		return new int[2] {12, 0};
	else if (pos == 31)
		return new int[2] {12, 1};
	else if (pos == 32)
		return new int[2] {12, 2};
	else if (pos == 33)
		return new int[2] {12, 3};
	else if (pos == 34)
		return new int[2] {12, 4};
	else if (pos == 35)
		return new int[2] {12, 5};
	return nullptr;
}