// SoloGen.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <cstdlib>
#include <time.h>
#include <string>
#include <vector>
#include <Windows.h>
#include <mfapi.h>
#include <Audioclient.h>
#include <mmdeviceapi.h>

#define REFTIMES_PER_SEC 10000000
#define REFTIMES_PER_MILLISEC 10000

#define EXIT_ON_ERROR(hres) \
	if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk) \
	if ((punk) != NULL) \
		{ (punk)->Release(); (punk) = NULL; }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_MMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

HRESULT PlayAudioStream(MyAudioSource *pMySource)
{
	HRESULT hr;
	REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
	REFERENCE_TIME hnsActualDuration;
	IMMDeviceEnumerator *pEnumerator = NULL;
	IMMDevice *pDevice = NULL;
	IAudioClient *pAudioClient = NULL;
	IAudioRenderClient *pRenderClient = NULL;
	WAVEFORMATX *pwfx = NULL;
	UINT32 bufferFrameCount;
	UINT32 numFramesAvailable;
	UINT32 numFramesPadding;
	BYTE *pData;
	DWORD flags = 0;

	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pEnumerator);
	EXIT_ON_ERROR(hr)

		hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
	EXIT_ON_ERROR(hr)

		hr = pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pAudioClient);
	EXIT_ON_ERROR(hr)

		hr = pAudioClient->GetMixFormat(&pwfx);
	EXIT_ON_ERROR(hr)

		hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, hnsRequestedDuration, 0, pwfx, NULL);
	EXIT_ON_ERROR(hr)

		// Tell the audio source which format to use
		hr = pMySource->SetFormat(pwfx);
	EXIT_ON_ERROR(hr)

		// Get the actual size of the allocated buffer
		hr = pAudioClient->GetBufferSize(&bufferFrameCount);
	EXIT_ON_ERROR(hr)

		hr = pAudioClient->GetService(IID_IAudioRenderClient, (void**)&pRenderClient);
	EXIT_ON_ERROR(hr)

		//Grab the entire buffer for the initial fill operation
		hr = pMySource->LoadData(bufferFrameCount, pData, &flags);
	EXIT_ON_ERROR(hr)

		hr = pRenderClient->ReleaseBuffer(bufferFrameCount, flags);
	EXIT_ON_ERROR(hr)

		//Calculate the actual duration of the allocated buffer
		hnsActualDuration = (double)REFTIMES_PER_SEC * bufferFrameCount / pwfx->nSamplesPerSec;

	hr = pAudioClient->Start();	// Start playing
	EXIT_ON_ERROR(hr)

		// Each loop fills about half of the shared buffer
		while (flags != AUDCLNT_BUFFERFLAGS_SILENT)
		{
			// Sleep for half the buffer duration
			Sleep((DWORD)(hnsActualDuration / REFTIMES_PER_MILLISEC / 2));

			// See how much buffer space is available
			hr = pAudioClient->GetCurrentPadding(&numFramesPadding);
			EXIT_ON_ERROR(hr)

				numFramesAvailable = bufferFrameCount - numFramesPadding;

			// Grab all the available space in the shared buffer
			hr = pRenderClient->GetBuffer(numFramesAvailable, &pData);
			EXIT_ON_ERROR(hr)

				// Get next 1/2-second of data from the audio source
				hr = pMySource->LoadData(numFramesAvailable, flags);
			EXIT_ON_ERROR(hr)
		}
}

using namespace std;

void outputTabChrom(int numBeats);
void outputTabPenta(int numBeats);
int randPos();
int randFret();
int randString();
int randNoteLength();
vector<int*> genRandNotes(int numBeats);
vector<int*> genRandNotesPos(int numBeats);
int* convPosToNote(int pos);

int main()
{
	srand(time(NULL));	// Set rand() seed to current time to give pseudorandom effect
	int tempo = 80;		// Desired output tempo
	char key = 'A';		// Key used for generating PentaScale
	int numBeats = 0;	// Total number of beats to generate for

	cout << "Guitar Solo Generator for the Minor Pentatonic Scale, enter the number of beats: ";
	cin >> numBeats;
	outputTabChrom(numBeats);
	cout << endl << endl;
	outputTabPenta(numBeats);
	system("PAUSE");
}

void outputTabChrom(int numBeats)
{
	vector<int*> notes;		// Vector of arrays of integers containing note info (pitch, length)
	vector<string*> tab;	// Vector of arrays of strings containing each note per beat

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
	for (int i = 0; i < notes.size(); i++)
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
}

void outputTabPenta(int numBeats)
{
	vector<int*> notes;		// Vector of arrays of integers containing note info (pitch, length)
	vector<string*> tab;	// Vector of arrays of strings containing each note per beat

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

	for (int i = 0; i < notes.size(); i++)
	{
		int* pitch = convPosToNote(notes.at(i)[0]);
		notes.at(i) = new int[3] {pitch[0], pitch[1], notes.at(i)[1]};
	}

	int beatPos = 0;	// Accumulator to keep position beat-wise in the tab
	// Loops through the arrays in vector <notes> and places the contained notes in position
	// in the tab
	for (int i = 0; i < notes.size(); i++)
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
}

// Returns a random integer 0 >= x > 30 to be converted into a PentaScale Position
int randPos()
{
	return (int)(((double)(rand()) / RAND_MAX) * 30);
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
	else if (pos == 30)
		return new int[2] {12, 1};
	else if (pos == 30)
		return new int[2] {12, 2};
	else if (pos == 30)
		return new int[2] {12, 3};
	else if (pos == 30)
		return new int[2] {12, 4};
	else if (pos == 30)
		return new int[2] {12, 5};
	return nullptr;
}