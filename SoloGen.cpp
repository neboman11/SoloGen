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
#include <AudioFile/AudioFile.h>
#include <tclap/CmdLine.h>

using namespace std;

enum CmdOptions {
	LENGTH,
	TEMPO
};

const double SEMITONE_STEP = pow(2.0, 1.0/12);

const double LOW_E_FREQ = 82.4;

const int STRING_STEPS[5] = {5, 5, 5, 4, 5};

const int N_FRETS = 24;

const double SAMPLE_RATE = 44100.0;

const double SECONDS_PER_SAMPLE = 1 / SAMPLE_RATE;

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

	cout << "Guitar Solo Generator for the Minor Pentatonic Scale, enter the number of beats:"  << endl;
	outputTabChrom(givenOptions);
	cout << endl << endl;
	outputTabPenta(givenOptions);

	// createWAV();
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

	audioFile.save("./test.wav");
}

map<int, string> parseCMD(int argc, char** argv)
{
	map<int, string> givenOptions;

	try
	{
		TCLAP::CmdLine cmd("SoloGen", ' ', "0.1");

		TCLAP::ValueArg<int> length("l", "length", "The length of the solo to generate in seconds", true, 5, "time (s)");

		cmd.add(length);

		TCLAP::ValueArg<int> tempo("t", "tempo", "The tempo of the resulting audio file", true, 120, "temp (bpm)");

		cmd.add(tempo);

		cmd.parse(argc, argv);

		givenOptions[LENGTH] = to_string(length.getValue());

		givenOptions[TEMPO] = to_string(tempo.getValue());
	}

	catch(TCLAP::ArgException& e)
	{
		std::cerr << e.argId() << " threw error " << e.error() << '\n';
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