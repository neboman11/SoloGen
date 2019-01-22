// SoloGen.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <cstdlib>
#include <time.h>
#include <string>
#include <vector>

using namespace std;

void outputTabBeat(int numBeats);
int randFret();
int randString();
int randNoteLength();
vector<int*> genRandNotes(int numBeats);

int main()
{
	srand(time(NULL));
	int fret = 0;
	int tempo = 80;
	char key = 'A';
	int numBeats = 0;

	cout << "Guitar Solo Generator for the Minor Pentatonic Scale, enter the number of beats: ";
	cin >> numBeats;
	fret = (int)(((double)(rand()) / RAND_MAX) * 22);
	//cout << fret << endl;
	outputTabBeat(numBeats);
	system("PAUSE");
}

void outputTabBeat(int numBeats)
{
	int fret;
	int guitarString;
	int noteLength;
	vector<int*> notes;
	vector<string*> tab;

	fret = randFret();
	guitarString = randString();
	noteLength = randNoteLength();

	for (int i = 0; i < 6; i++)
	{
		if (i == guitarString)
			cout << fret;
		else
			cout << "-";
		cout << endl;
	}
	notes = genRandNotes(numBeats);
	for (int i = 0; i < notes.size(); i++)
	{
		for (int j = 0; j < 3; j++)
		{
			cout << notes.at(i)[j] << " ";
		}
		cout << endl;
	}

	for (int i = 0; i < numBeats; i++)
	{
		tab.push_back(new string[6]);
		for (int j = 0; j < 6; j++)
		{
			tab.at(i)[j] = "- ";
		}
	}

	int beatPos = 0;
	for (int i = 0; i < notes.size(); i++)
	{
		if (notes.at(i)[0] > 9)
		{
			tab.at(beatPos)[notes.at(i)[1] - 1] = " " + to_string(notes.at(i)[0]);
			beatPos += notes.at(i)[2];
		}
		else if (notes.at(i)[0] < 10)
		{
			tab.at(beatPos)[notes.at(i)[1] - 1] = " " + to_string(notes.at(i)[0]) + " ";
			beatPos += notes.at(i)[2];
		}
	}

	for (int i = 0; i < 6; i++)
		for (int j = 0; j < numBeats; j++)
			cout << tab.at(j)[i];
}

int randFret()
{
	return (int)(((double)(rand()) / RAND_MAX) * 22);
}

int randString()
{
	return (int)(((double)(rand()) / RAND_MAX) * 6);
}

int randNoteLength()
{
	return pow(2, (int)(((double)(rand()) / RAND_MAX) * 4));
}

vector<int*> genRandNotes(int numBeats)
{
	int totalBeats = 0;
	int* genNotes = NULL;
	vector<int*> notes;

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
