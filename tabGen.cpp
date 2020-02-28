#include "tabGen.h"

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