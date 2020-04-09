#include "cmdParse.h"


map<int, string> parseCMD(int argc, char** argv)
{
	map<int, string> givenOptions;

	try
	{
		TCLAP::CmdLine cmd("SoloGen", ' ', "0.1");

		TCLAP::ValueArg<int> length("l", "length", "The number of measures to generate a solo over", true, 5, "measures");

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

		if (flacConvert.getValue())
		{
			givenOptions[FLAC] = "true";
		}

		else
		{
			givenOptions[FLAC] = "false";
		}

		// Check if converting to flac if output file extension matches flac instead of wav
		if (givenOptions[FLAC] == "false")
		{
			if (outFile.getValue().substr(outFile.getValue().length() - 4, outFile.getValue().length()) == ".wav")
			{
				givenOptions[OUTFILE] = outFile.getValue();
			}
			
			else
			{
				givenOptions[OUTFILE] = outFile.getValue() + ".wav";
			}
		}

		else
		{
			if (outFile.getValue().substr(outFile.getValue().length() - 5, outFile.getValue().length()) == ".flac")
			{
				givenOptions[OUTFILE] = outFile.getValue();
			}

			else
			{
				givenOptions[OUTFILE] = outFile.getValue() + ".flac";
			}
			
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
			givenOptions[TIME_SIG_UPPER] = timeSig.getValue().substr(0, 1);
			givenOptions[TIME_SIG_LOWER] = timeSig.getValue().substr(2, 1);
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