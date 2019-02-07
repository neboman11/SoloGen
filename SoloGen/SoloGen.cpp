// SoloGen.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <cstdlib>
#include <time.h>
#include <string>
#include <vector>
//#include <Windows.h>
//#include <mfapi.h>
//#include <Audioclient.h>
//#include <mmdeviceapi.h>
#include <fstream>


//#define REFTIMES_PER_SEC 10000000
//#define REFTIMES_PER_MILLISEC 10000
//
//#define EXIT_ON_ERROR(hres) \
//	if (FAILED(hres)) { goto Exit; }
//#define SAFE_RELEASE(punk) \
//	if ((punk) != NULL) \
//		{ (punk)->Release(); (punk) = NULL; }

//const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
//const IID IID_MMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
//const IID IID_IAudioClient = __uuidof(IAudioClient);
//const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

//HRESULT PlayAudioStream(int *pMySource)
//{
//	HRESULT hr;
//	REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
//	REFERENCE_TIME hnsActualDuration;
//	IMMDeviceEnumerator *pEnumerator = NULL;
//	IMMDevice *pDevice = NULL;
//	IAudioClient *pAudioClient = NULL;
//	IAudioRenderClient *pRenderClient = NULL;
//	WAVEFORMATEX *pwfx = NULL;
//	UINT32 bufferFrameCount;
//	UINT32 numFramesAvailable;
//	UINT32 numFramesPadding;
//	BYTE *pData;
//	DWORD flags = 0;
//
//	hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pEnumerator);
//	EXIT_ON_ERROR(hr)
//
//	hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
//	EXIT_ON_ERROR(hr)
//
//	hr = pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&pAudioClient);
//	EXIT_ON_ERROR(hr)
//
//	hr = pAudioClient->GetMixFormat(&pwfx);
//	EXIT_ON_ERROR(hr)
//
//	hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, 0, hnsRequestedDuration, 0, pwfx, NULL);
//	EXIT_ON_ERROR(hr)
//
//	// Tell the audio source which format to use
//	hr = pMySource->SetFormat(pwfx);
//	EXIT_ON_ERROR(hr)
//
//	// Get the actual size of the allocated buffer
//	hr = pAudioClient->GetBufferSize(&bufferFrameCount);
//	EXIT_ON_ERROR(hr)
//
//	hr = pAudioClient->GetService(IID_IAudioRenderClient, (void**)&pRenderClient);
//	EXIT_ON_ERROR(hr)
//
//	//Grab the entire buffer for the initial fill operation
//	hr = pMySource->LoadData(bufferFrameCount, pData, &flags);
//	EXIT_ON_ERROR(hr)
//
//	hr = pRenderClient->ReleaseBuffer(bufferFrameCount, flags);
//	EXIT_ON_ERROR(hr)
//
//	//Calculate the actual duration of the allocated buffer
//	hnsActualDuration = (double)REFTIMES_PER_SEC * bufferFrameCount / pwfx->nSamplesPerSec;
//
//	hr = pAudioClient->Start();	// Start playing
//	EXIT_ON_ERROR(hr)
//
//		// Each loop fills about half of the shared buffer
//		while (flags != AUDCLNT_BUFFERFLAGS_SILENT)
//		{
//			// Sleep for half the buffer duration
//			Sleep((DWORD)(hnsActualDuration / REFTIMES_PER_MILLISEC / 2));
//
//			// See how much buffer space is available
//			hr = pAudioClient->GetCurrentPadding(&numFramesPadding);
//			EXIT_ON_ERROR(hr)
//
//			numFramesAvailable = bufferFrameCount - numFramesPadding;
//
//			// Grab all the available space in the shared buffer
//			hr = pRenderClient->GetBuffer(numFramesAvailable, &pData);
//			EXIT_ON_ERROR(hr)
//
//			// Get next 1/2-second of data from the audio source
//			hr = pMySource->LoadData(numFramesAvailable, flags);
//			EXIT_ON_ERROR(hr)
//			hr = pRenderClient->ReleaseBuffer(numFramesAvailable, flags);
//			EXIT_ON_ERROR(hr)
//		}
//
//	// Wait for last data in buffer to play before stopping
//	Sleep((DWORD)(hnsActualDuration / REFTIMES_PER_MILLISEC / 2));
//
//	hr = pAudioClient->Stop();	// Stop playing
//	EXIT_ON_ERROR(hr)
//
//	Exit:
//		CoTaskMemFree(pwfx);
//		SAFE_RELEASE(pEnumerator)
//		SAFE_RELEASE(pDevice)
//		SAFE_RELEASE(pAudioClient)
//		SAFE_RELEASE(pRenderClient)
//
//	return hr;
//}

using namespace std;
using std::string;
using std::fstream;

typedef struct WAV_HEADER {
	char			ChunkID[4];		// RIFF Header
	unsigned long	ChunkSize;		// RIFF Chuck Size
	char			format[4];		// Format Header
	char			Subchunk1ID[4];	// ID of the fmt chunk
	unsigned long	Subchunk1Size;	// Size of the fmt chunk
	unsigned short	AudioFormat;	// Audio format 1=PCM, 6=Mu-Law, 7=A-Law, 257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
	unsigned short	NumOfChan;		// Number of channels 1=Mono 2=Stereo
	unsigned long	SamplesPerSec;	// Sampling Frequency in Hz
	unsigned long	bytesPerSec;	// bytes per second
	unsigned short	blockAlign;		// 2=16-bit mono, 4=16-bit stereo
	unsigned short	bitsPerSample;	// Number of bits per sample
	char			Subchunk2ID[4];	// "data" string
	unsigned long	Subchunk2Size;	// Sampled data length
}wav_hdr;

void outputTabChrom(int numBeats);
void outputTabPenta(int numBeats);
int randPos();
int randFret();
int randString();
int randNoteLength();
vector<int*> genRandNotes(int numBeats);
vector<int*> genRandNotesPos(int numBeats);
int* convPosToNote(int pos);
int getFileSize(FILE *inFile);

int main(int argc, char* argv[])
{
	//wav_hdr wavHeader;
	int headerSize = sizeof(wav_hdr), filelength = 0;
	string answer;
	ifstream wavFileIn;
	srand(time(NULL));	// Set rand() seed to current time to give pseudorandom effect
	int tempo = 80;		// Desired output tempo
	char key = 'A';		// Key used for generating PentaScale
	int numBeats = 0;	// Total number of beats to generate for

	//do
	//{
	//	string input;
	//	string answer;

	//	const char* filePath;

	//	cout << "Pick wave file from the Windows Media File: ";
	//	cin >> input;
	//	cin.get();

	//	cout << endl;

	//	string path = "C:\\Windows\\Media\\" + input + ".wav";
	//	filePath = path.c_str();

	//	wavFileIn.open(path);

	//	if (!wavFileIn)
	//	{
	//		printf("Can not able to open wave file\n");
	//		exit(EXIT_FAILURE);
	//	}

	//	wavFileIn.read(wavHeader.ChunkID, 4);
	//	wavFileIn.read(reinterpret_cast<char *>(&wavHeader.ChunkSize), 4);
	//	wavFileIn.read(wavHeader.format, 4);
	//	wavFileIn.read(wavHeader.Subchunk1ID, 4);
	//	wavFileIn.read(reinterpret_cast<char *>(&wavHeader.Subchunk1Size), 4);
	//	wavFileIn.read(reinterpret_cast<char *>(&wavHeader.AudioFormat), 2);
	//	wavFileIn.read(reinterpret_cast<char *>(&wavHeader.NumOfChan), 2);
	//	wavFileIn.read(reinterpret_cast<char *>(&wavHeader.SamplesPerSec), 4);
	//	wavFileIn.read(reinterpret_cast<char *>(&wavHeader.bytesPerSec), 4);
	//	wavFileIn.read(reinterpret_cast<char *>(&wavHeader.blockAlign), 2);
	//	wavFileIn.read(reinterpret_cast<char *>(&wavHeader.bitsPerSample), 2);
	//	wavFileIn.read(wavHeader.Subchunk2ID, 4);
	//	wavFileIn.read(reinterpret_cast<char *>(&wavHeader.Subchunk2Size), 4);

	//	wavFileIn.close();

	//	cout << "File is				:" << filelength << " bytes." << endl;

	//	// RIFF chunk
	//	cout << "Chunk ID			:" << wavHeader.ChunkID[0] << wavHeader.ChunkID[1] << wavHeader.ChunkID[2] << wavHeader.ChunkID[3] << endl;
	//	cout << "Chunk size			:" << wavHeader.ChunkSize << endl;
	//	cout << "Format				:" << wavHeader.format[0] << wavHeader.format[1] << wavHeader.format[2] << wavHeader.format[3] << endl;

	//	// Format subchunk
	//	cout << "Sub-chunk 1 ID			:" << wavHeader.Subchunk1ID[0] << wavHeader.Subchunk1ID[1] << wavHeader.Subchunk1ID[2] << wavHeader.Subchunk1ID[3] << endl;
	//	cout << "Sub-chunk 1 Size		:" << wavHeader.Subchunk1Size << endl;
	//	cout << "Audio Format			:" << wavHeader.AudioFormat << endl;
	//	cout << "Number of channels		:" << wavHeader.NumOfChan << endl;
	//	cout << "Sampling Rate			:" << wavHeader.SamplesPerSec << endl;
	//	cout << "Number of bytes per sec		:" << wavHeader.bytesPerSec << endl;
	//	cout << "Block align			:" << wavHeader.blockAlign << endl;
	//	cout << "Number of bits used		:" << wavHeader.bitsPerSample << endl;

	//	// Data subchunk
	//	cout << "Data string			:" << wavHeader.Subchunk2ID[0] << wavHeader.Subchunk2ID[1] << wavHeader.Subchunk2ID[2] << wavHeader.Subchunk2ID[3] << endl;
	//	cout << "Data length			:" << wavHeader.Subchunk2Size << endl;


	//	cout << endl << endl << "Try something else? (y/n)";
	//	cin >> answer;
	//	cout << endl << endl;
	//} while (answer == "y");

	//getchar();
	//return 0;



	cout << "Guitar Solo Generator for the Minor Pentatonic Scale, enter the number of beats: ";
	cin >> numBeats;
	outputTabChrom(numBeats);
	cout << endl << endl;
	outputTabPenta(numBeats);
	system("PAUSE");
}

int getFileSize(FILE *inFile)
{
	int fileSize = 0;
	fseek(inFile, 0, SEEK_END);

	fileSize = ftell(inFile);

	fseek(inFile, 0, SEEK_SET);
	return fileSize;
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