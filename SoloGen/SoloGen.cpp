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
	fret = (int)(((double)(rand()) / RAND_MAX) * 22);
	cout << fret << endl;
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
	}
}

int randFret()
{
	return (int)(((double)(rand()) / RAND_MAX) * 22);
}

int randString()
{
	return 0;
}
