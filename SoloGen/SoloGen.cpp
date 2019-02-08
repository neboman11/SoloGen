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
#include <stdio.h>
#include <stdlib.h>
#include <sndfile.h>
#include <cstdio>
#include <sndfile.hh>
#include "libsndfile/sfconfig.h"
#include <string.h>
#include <errno.h>

#if HAVE_UNISTD_H
#include <unistd.h>
#else
#include "libsndfile/sf_unistd.h"
#endif

#include "libsndfile/common.h"

#if HAVE_ALSA_ASOUNDLIB_H
#define ALSA_PCM_NEW_HW_PARAMS_API
#define ALSA_PCM_NEW_SW_PARAMS_API
#include <alsa/asoundlib.h>
#include <sys/time.h>
#endif

#if defined (__ANDROID__)

#elif defined (__linux__) || defined (__FreeBSD_kernel__) || defined (__FreeBSD__)
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>

#elif HAVE_SNDIO_H
#include <sndio.h>

#elif (defined (sun) && defined (unix))
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/auioio.h>

#elif _WIN32
#include <Windows.h>
#include <mmsystem.h>

#endif

#define SIGNED_SIZEOF(x)	((int) sizeof(x))
#define BUFFER_LEN 2048
#define MAX_CHANNELS 6

#if HAVE_ALSA_ASOUNDLIB_H

static snd_pcm_t * alsa_open(int channels, unsigned srate, int realtime);
static int alsa_write_float(snd_pcm_t *alsa_dev, float *data, int frames, int channels);

static void alsa_play(int argc, char *argv[])
{
	static float buffer[BUFFER_LEN];
	SNDFILE *sndfile;
	SF_INFO sfinfo;
	snd_pcm_t * alsa_dev;
	int k, readcount, subformat;

	for (k = 1; k < argc; k++)
	{
		memset(&sfinfo, 0, sizeof(sfinfo));

		printf("Playing %s\n", argv[k]);
		if (!(sndfile = sf_open(argv[k], SFM_READ, &sfinfo)))
		{
			puts(sf_strerror(NULL));
			continue;
		}

		if (sfinfo.channesl < 1 || sfinfo.channels > 2)
		{
			printf("Error : channels = %d.\n", sfinfo.channels);
			continue;
		}

		if ((alsa_dev = alsa_open(sfinfo.channels, (unsigned)sfinfo.samplerate, SF_FALSE)) == NULL)
			continue;

		subformat = sfinfo.format & SF_FORMAT_SUBMASK;

		if (subformat == SF_FORMAT_FLOAT || subformat == SF_FORMAT_DOUBLE)
		{
			double scale;
			int m;

			sf_command(sndfile, SFC_CALC_SIGNAL_MAX, &scale, sizeof(scale));
			if (scale > 1.0)
				scale = 1.0 / scale;
			else
				scale = 1.0;

			while (readcount = sf_read_float(sndfile, buffer, BUFFER_LEN))
			{
				for (m = 0; m < readcount; m++)
					buffer[m] *= scale;
				alsa_write_float(alsa_dev, buffer, BUFFER_LEN / sfinfo.channels, sfinfo.channels);
			}
		}
		else
		{
			while (readcount = sf_read_float(sndfile, buffer, BUFFER_LEN))
				alsa_write_float(alsa_dev, buffer, BUFFER_LEN / sfinfo.channels, sfinfo, channels);
		}

		snd_pcm_drain(alsa_dev);
		snd_pcm_close(alsa_dev);

		sf_close(sndfile);
	}
}

static snd_pcm_t * alsa_open(int channels, unsigned samplerate, int realtime)
{
	const char * device = "default";
	snd_pcm_t *alsa_dev = NULL;
	snd_pcm_hw_params_t *hw_params;
	snd_pcm_uframes_t buffer_size;
	snd_pcm_uframes_t alsa_period_size, alsa_buffer_frames;
	snd_pcm_sw_params_t *sw_params;

	int err;

	if (realtime)
	{
		alsa_period_size = 256;
		alsa_buffer_frames = 3 * alsa_period_size;
	}
	else
	{
		alsa_period_size = 1024;
		alsa_buffer_frames = 4 * alsa_period_size;
	}

	if ((err = snd_pcm_open(&alsa_dev, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
	{
		fprintf(stderr, "cannot open audio device \"%s\" (%s)\n", device, snd_strerror(err));
		goto catch_error;
	}

	snd_pcmnonblock(alsa_dev, 0);

	if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0)
	{
		fprintf(stderr, "cannot allocate hardware parameter structure (%s)\n", snd_strerror(err));
		goto catch_error;
	}

	if ((err = snd_pcm_hw_params_any(alsa_dev, hw_params)) < 0)
	{
		fprintf(stderr, "cannot initialize hardware parameter structure (%s)\n", snd_strerror(err));
		goto catch_error;
	}

	if ((err = snd_pcm_hw_params_set_access(alsa_dev, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
	{
		fprintf(stderr, "cannot set access type(%s)\n", snd_strerror(err));
		goto catch_error;
	}

	if ((err = snd_pcm_hw_params_set_format(alsa_dev, hw_params, SND_PCM_FORMAT_FLOAT)) < 0)
	{
		fprintf(stderr, "cannot set sample format (%s)\n", snd_strerror(err));
		goto catch_error;
	}

	if ((err = snd_pcm_hw_params_set_rate_near(alsa_dev, hw_params, &samplerate, 0)) < 0)
	{
		fprintf(stderr, "cannot set sample rate (%s)\n", snd_strerror(err));
		goto catch_error;
	}

	if ((err = snd_pcm_hw_params_set_channels(alsa_dev, hw_params, channels)) < 0)
	{
		fprintf(stderr, "cannot set channel count (%s)\n", snd_strerror(err));
		goto catch_error;
	}

	if ((err = snd_pcm_hw_params_set_period_size_near(alsa_dev, hw_params, &alsa_period_size, 0)) < 0)
	{
		fprintf(stderr, "cannot set period size (%s)\n", snd_strerror(err));
		goto catch_error;
	}

	if ((err = snd_pcm_hw_params(alsa_dev, hw_params)) < 0)
	{
		fprintf(stderr, "cannot set parameters (%s)\n", snd_strerror(err));
		goto catch_error;
	}

	snd_pcm_hw_params_get_period_size(hw_params, &alsa_period_size, 0);
	snd_pcm_hw_params_get_buffer_size(hw_params, &buffer_size);
	if (alsa_period_size == buffer_size)
	{
		fprintf(stderr, "Can't use period equal to buffer size (%lu == %lu)", alsa_period_size, buffer_size);
		goto catch_error;
	}

	snd_pcm_hw_params_free(hw_params);

	if ((err = snd_pcm_sw_params_malloc(&sw_params)) != 0)
	{
		fprintf(stderr, "%s: snd_pcm_sw_params_current: %s", __func__, snd_strerror(err));
		goto catch_error;
	}

	snd_pcm_sw_params_current(alsa_dev, sw_params);

	if ((err = snd_pcm_sw_params(alsa_dev, sw_params)) != 0)
	{
		fprintf(stderr, "%s: snd_pcm_sw_params: %s", __func__, snd_strerror(err));
		goto catch_error;
	}

	snd_pcm_sw_params_free(sw_params);

	snd_pcm_reset(alsa_dev);

catch_error :

	if (err < 0 && alsa_dev != NULL)
	{
		snd_pcm_close(alsa_dev);
		return NULL;
	}

	return alsa_dev;
}

static int alsa_write_float(snd_pcm_t *alsa_dev, float *data, int frames, int channels)
{
	static int epipe_count = 0;

	int total = 0;
	int retval;

	if (epipe_count > 0)
		epipe_count--;

	while (total < frames)
	{
		retval = snd_pcm_writei(alsa_dev, data + total * channels, frames - total);

		if (retval >= 0)
		{
			total += retval;
			if (total == frames)
				return total;

			continue;
		}

		switch (retval)
		{
		case -EAGAIN:
			puts("alsa_write_float: EAGAIN");
			continue;
			break;

		case -EPIPE:
			if (epipe_count > 0)
			{
				prinf("alsa_write_float: EPIPE %d\n", epipe_count);
				if (epipe_count > 140)
					return retval;
			}
			epipe_count += 100;

#if 0
			if (0)
			{
				snd_pcm_status_t *status;

				snd_pcm_status_alloca(&status);
				if ((retval = snd_pcm_status(alsa_dev, status)) < 0)
					fprintf(stderr, "alsa_out: xrun. can't determine length\n");
				else if (snd_pcm_status_get_state(status) == SND_PCM_STATE_XRUN)
				{
					struct timeval now, diff, tstamp;

					gettimeofday(&now, 0);
					snd_pcm_status_get_trigger_tstamp(status, &tstamp);
					timersub(&now, &tstamp, &diff);

					fprintf(stderr, "alsa_write_float xrun: of at leas %.3f mses. resetting stream\n", diff.tv_sec * 1000 + diff.tv_usec / 1000.0);
				}
				else
					fprintf(stderr, "alsa_write_float: xrun. can't determine length\n");
			}
#endif

			snd_pcm_prepare(alsa_dev);
			break;

		case -EBADFD:
			fprintf(stderr, "alsa_write_float: Bad PCM state.n");
			return 0;
			break;

		case -ESTRPIPE:
			fprintf(stderr, "alsa_write_float: Suspend event.n");
			return 0;
			break;

		case -EIO:
			puts("alsa_write_float: EIO");
			return 0;

		default:
			fprintf(stderr, "alsa_write_float: retval = %d\n", retval);
			return 0;
			break;
		}
	}

	return total;
}

#endif

#if _WIN32

#define WIN32_BUFFER_LEN	(1 << 15)

typedef struct
{
	HWAVEOUT	hwave;
	WAVEHDR	whdr[2];

	CRITICAL_SECTION	mutex;
	HANDLE	Event;

	short	buffer[WIN32_BUFFER_LEN / sizeof(short)];
	int		current, bufferlen;
	int		BuffersInUse;

	SNDFILE *sndfile;
	SF_INFO	sfinfo;

	sf_count_t	remaining;
} Win32_Audio_Data;

static void win32_play_data(Win32_Audio_Data *audio_data)
{
	int thisread, readcount;

	readcount = (audio_data->remaining > audio_data->bufferlen) ? audio_data->bufferlen : (int)audio_data->remaining;

	thisread = (int)sf_read_short(audio_data->sndfile, (short *)(audio_data->whdr[audio_data->current].lpData), readcount);

	audio_data->remaining -= thisread;

	if (thisread > 0)
	{
		if (thisread < audio_data->bufferlen)
			audio_data->whdr[audio_data->current].dwBufferLength = thisread * sizeof(short);

		waveOutWrite(audio_data->hwave, (LPWAVEHDR) &(audio_data->whdr[audio_data->current]), sizeof(WAVEHDR));

		EnterCriticalSection(&audio_data->mutex);
		audio_data->BuffersInUse++;
		LeaveCriticalSection(&audio_data->mutex);

		audio_data->current = (audio_data->current + 1) % 2;
	}
}

static void CALLBACK win32_audio_out_callback(HWAVEOUT hwave, UINT msg, DWORD_PTR data, DWORD param1, DWORD param2)
{
	Win32_Audio_Data *audio_data;

	(void)hwave;
	(void)param1;
	(void)param2;

	if (data == 0)
		return;

	audio_data = (Win32_Audio_Data*)data;

	if (msg == MM_WOM_DONE)
	{
		EnterCriticalSection(&audio_data->mutex);
		audio_data->BuffersInUse--;
		LeaveCriticalSection(&audio_data->mutex);
		SetEvent(audio_data->Event);
	}
}

static void win32_play(int argc, char *argv[])
{
	Win32_Audio_Data audio_data;

	WAVEFORMATEX wf;
	int k, error;

	audio_data.sndfile = NULL;
	audio_data.hwave = 0;

	for (k = 1; k < argc; k++)
	{
		printf("Playing %s\n", argv[k]);

		if (!(audio_data.sndfile = sf_open(argv[k], SFM_READ, &(audio_data.sfinfo))))
		{
			puts(sf_strerror(NULL));
			continue;
		}

		audio_data.remaining = audio_data.sfinfo.frames * audio_data.sfinfo.channels;
		audio_data.current = 0;

		InitializeCriticalSection(&audio_data.mutex);
		audio_data.Event = CreateEvent(0, FALSE, FALSE, 0);

		wf.nChannels = audio_data.sfinfo.channels;
		wf.wFormatTag = WAVE_FORMAT_PCM;
		wf.cbSize = 0;
		wf.wBitsPerSample = 16;

		wf.nSamplesPerSec = audio_data.sfinfo.samplerate;

		wf.nBlockAlign = audio_data.sfinfo.channels * sizeof(short);

		wf.nAvgBytesPerSec = wf.nBlockAlign * wf.nSamplesPerSec;

		error = waveOutOpen(&(audio_data.hwave), WAVE_MAPPER, &wf, (DWORD_PTR)win32_audio_out_callback, (DWORD_PTR)&audio_data, CALLBACK_FUNCTION);
		if (error)
		{
			puts("waveOutOpen failed.");
			audio_data.hwave = 0;
			continue;
		}

		audio_data.whdr[0].lpData = (char*)audio_data.buffer;
		audio_data.whdr[1].lpData = ((char*)audio_data.buffer) + sizeof(audio_data.buffer) / 2;

		audio_data.whdr[0].dwBufferLength = sizeof(audio_data.buffer) / 2;
		audio_data.whdr[1].dwBufferLength = sizeof(audio_data.buffer) / 2;

		audio_data.whdr[0].dwFlags = 0;
		audio_data.whdr[1].dwFlags = 0;

		audio_data.bufferlen = sizeof(audio_data.buffer) / 2 / sizeof(short);

		if ((error = waveOutPrepareHeader(audio_data.hwave, &(audio_data.whdr[0]), sizeof(WAVEHDR))))
		{
			printf("waveOutPrepareHeader [0] failed : %08X\n", error);
			waveOutClose(audio_data.hwave);
			continue;
		}

		if ((error = waveOutPrepareHeader(audio_data.hwave, &(audio_data.whdr[1]), sizeof(WAVEHDR))))
		{
			printf("waveOutPrepareHeader [1] failed : %08X\n", error);
			waveOutUnprepareHeader(audio_data.hwave, &(audio_data.whdr[0]), sizeof(WAVEHDR));
			waveOutClose(audio_data.hwave);
			continue;
		}

		audio_data.BuffersInUse = 0;
		win32_play_data(&audio_data);
		win32_play_data(&audio_data);

		while (audio_data.BuffersInUse > 0)
		{
			WaitForSingleObject(audio_data.Event, INFINITE);

			win32_play_data(&audio_data);
		}

		waveOutUnprepareHeader(audio_data.hwave, &(audio_data.whdr[0]), sizeof(WAVEHDR));
		waveOutUnprepareHeader(audio_data.hwave, &(audio_data.whdr[1]), sizeof(WAVEHDR));

		waveOutClose(audio_data.hwave);
		audio_data.hwave = 0;

		DeleteCriticalSection(&audio_data.mutex);

		sf_close(audio_data.sndfile);
	}
}

#endif

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
static void create_file(const char * fname, int format);
static void read_file(const char * fname);
static void process_data(double *data, int count, int channels);

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

	if (argc < 2)
	{
		printf("\nUsage : Hot dog <input sound file>\n\n");
		printf("Using %s.\n\n", sf_version_string());
#if _WIN32
		printf("This is a Unix style command line application which\n"
			"should be run in a MSDOS box or Command Shell windows.\n\n");
		printf("Sleeping for 5 seconds before exiting.\n\n");

		Sleep(5 * 1000);
#endif
		return 1;
	}

#if _WIN32
	win32_play(argc, argv);
#else
	puts("*** Playing sound not supported on this platform.");
	puts("*** Please feel free to submit a patch.");
	return 1;
#endif
	return 0;

	static double data[BUFFER_LEN];

	SNDFILE *infile, *outfile;

	SF_INFO sfinfo;
	int readcount;
	const char *infilename = "C:\\Windows\\media\\Alarm01.wav";
	const char *outfilename = "output.wav";

	memset(&sfinfo, 0, sizeof(sfinfo));

	if (!(infile = sf_open(infilename, SFM_READ, &sfinfo)))
	{
		printf("Not able to open file %s.\n", infilename);
		puts(sf_strerror(NULL));
		return 1;
	}

	if (sfinfo.channels > MAX_CHANNELS)
	{
		printf("Not able to process more than %d channels\n", MAX_CHANNELS);
		return 1;
	}

	if (!(outfile = sf_open(outfilename, SFM_WRITE, &sfinfo)))
	{
		printf("Not able to open output file %s.\n", outfilename);
		puts(sf_strerror(NULL));
		return 1;
	}

	while (readcount = sf_read_double(infile, data, BUFFER_LEN))
	{
		process_data(data, readcount, sfinfo.channels);
		sf_write_double(outfile, data, readcount);
	}

	sf_close(infile);
	sf_close(outfile);

	const char * fname = "C:\\Windows\\media\\Alarm01.wav";

	puts("\nSimple example showing usage of the c++ SndfileHandle object.\n");

	read_file(fname);

	puts("Done.\n");

	system("PAUSE");
	return 0;

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

void create_file(const char * fname, int format)
{
	static short buffer[BUFFER_LEN];

	SndfileHandle file;
	int channels = 2;
	int srate = 48000;

	printf("Creating file named '%s'\n", fname);

	file = SndfileHandle(fname, SFM_WRITE, format, channels, srate);

	memset(buffer, 0, sizeof(buffer));

	file.write(buffer, BUFFER_LEN);

	puts("");
}

void read_file(const char * fname)
{
	static short buffer[BUFFER_LEN];

	SndfileHandle file;

	file = SndfileHandle(fname);

	printf("Opened file '%s'\n", fname);
	printf("	Sample rate	: %d\n", file.samplerate());
	printf("	Channels	: %d\n", file.channels());

	file.read(buffer, BUFFER_LEN);

	puts("");
}

void process_data(double * data, int count, int channels)
{
	double channel_gain[MAX_CHANNELS] = { 0.5, 0.8, 0.1, 0.4, 0.4, 0.9 };
	int k, chan;

	for (chan = 0; chan < channels; chan++)
		for (k = chan; k < count; k += channels)
			data[k] *= channel_gain[chan];
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