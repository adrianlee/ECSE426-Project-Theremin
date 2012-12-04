#include "echo2.h"

float* history; //History buffer
int pos;
int amp;
int delay;
int ms;

float f_amp;

void setDelay(int fms)
{
	int newDelay = fms * I2S_AudioFreq_44k/1000;
	
	//Create new history buffer
	float newHistory[newDelay];
	for(int i = 0; i < newDelay; i++)
	{
		newHistory[i] = 0.0f;
	}
	
	if(history)
	{
		int howMuch = delay-pos;
		howMuch = MIN(howMuch, newDelay);
		for(int i = 0, j = pos; i < howMuch; i++, j++)
		{
			newHistory[i] = history[j];
		}
		if(howMuch < newDelay)
		{
			int i = howMuch;
			howMuch = newDelay - howMuch;
			howMuch = MIN(howMuch, delay);
			howMuch = MIN(howMuch, pos);
			for(int j = 0; i < howMuch; i++, j++)
			{
				newHistory[i] = history[j];
			}
		}
	}
	else
	{
		history = (float*) malloc(newDelay + 1);
		for(int i = 0; i < newDelay; i++)
		{
			history[i] = newHistory[i];
		}
	}
	
	pos = 0
	delay = newDelay;
	ms = fms;
}

void setAmp(int famp)
{
	amp = famp;
	f_amp = (float) amp/256.0f;
}

void getDelay()
{
	return ms;
}

void process()
{
	
}
