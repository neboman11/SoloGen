#ifndef TAB_GEN_H
#define TAB_GEN_H

#include <vector>
#include <cmath>

using namespace std;

int randPos();
int randFret();
int randString();
int randNoteLength();
vector<int*> genRandNotes(int numBeats);
vector<int*> genRandNotesPos(int numBeats);
int* convPosToNote(int pos);

#endif
