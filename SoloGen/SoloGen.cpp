// SoloGen.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <cstdlib>
#include <time.h>

using namespace std;

void outputTabBeat();
int randFret();
int randString();

int main()
{
	srand(time(NULL));
	int fret = 0;
	int tempo = 80;
	char key = 'A';

	cout << "Guitar Solo Generator for the Minor Pentatonic Scale, enter a tempo :";
	cin >> tempo;
	fret = (int)(((double)(rand()) / RAND_MAX) * 22);
	cout << fret << endl;
	outputTabBeat();
	system("PAUSE");
}

void outputTabBeat()
{
	int fret;
	int guitarString;

	fret = randFret();
	guitarString = randString();

	for (int i = 0; i < 6; i++)
	{
		if (i == guitarString)
			cout << fret;
		else
			cout << "-";
		cout << endl;
	}
}

int randFret()
{
	return (int)(((double)(rand()) / RAND_MAX) * 22);
}

int randString()
{
	return (int)(((double)(rand()) / RAND_MAX) * 6);
}
