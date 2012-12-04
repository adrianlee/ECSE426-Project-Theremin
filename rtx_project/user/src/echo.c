#include "echo.h"

//Private variables
short* delayBuffer;
int delaySamples;
int delayBufferPos;
float decay;

void addecho(uint16_t* buffer, int length, int phase, double damping)
{
	uint16_t buffer_echo[length+phase];
	unsigned int i,j;
	for(i = 0; i < phase; i++)
	{
		buffer_echo[i] = buffer[i];
	}
	for(j = 0; i < length;i++,j++)
	{
		buffer_echo[i] = buffer[i] + (int)((double)buffer[j]*damping);
	}
	for(;j < length;i++, j++)
	{
		buffer_echo[i] = (int) ((double)buffer[j]*damping);
	}
	
	Audio_MAL_Play((uint32_t) buffer_echo, length+phase);
}

/**
* @brief Initializes the Echo Filter.
* @param numDelaySamples Specifies how long before the echo is initially heard.
* @param dec Is value of how much the echo has decayed from the source.
*/
void init_Echo_Filter(int numDelaySamples, float dec)
{
	delaySamples = numDelaySamples;
	decay = dec;
	delayBuffer = (short*) malloc (numDelaySamples+1); //Allocate the array.
}

/**
* @brief Gets the remaining size, in bytes, of samples that this
* filter can echo after playing the sound. Ensures that the sound
* will have decayed to below 1% of the max vol (amplitude).
* @return Returns the remaining size in bytes.
*/
int getRemainingSize(void)
{
	float finalDecay = 0.01f;
	int numRemainingBuffers = (int) ceil(log(finalDecay))/log(decay);
	int bufferSize = delaySamples * 2;
	
	return bufferSize * numRemainingBuffers;
}

/**
* @brief Clears the Echo Filter's internal delay buffer.
*/
void reset(void)
{
	for(int i = 0; i < delaySamples; i++)
	{
		delayBuffer[i] = 0;
	}
	delayBufferPos = 0;
}

/**
* @brief Filters the sound smaples to add an echo.
* @param samples The sound samples array.
* @param offset Where to add the echo.
* @param length The length of the sound samples array.
*/
void filter(uint16_t* samples, int offset, int length)
{
	for(int i = offset; i < offset+length; i +=2)
	{
		short oldSample = getSample(samples, i);
		short newSample = (short) (oldSample + decay * delayBuffer[delayBufferPos]);
		setSample(samples, i, newSample);
		
		//Update the delay buffer
		delayBuffer[delayBufferPos] = newSample;
		delayBufferPos++;
		if(delayBufferPos == delaySamples)
			delayBufferPos = 0;
	}
}

/**
* @brief Gets the 16-bit sample.
* @param buffer Pointer to the samples.
* @param pos The position of the sample in the buffer.
*/
short getSample(uint16_t* buffer, int pos)
{
	return (short) ((buffer[pos + 1] & 0xff << 8) | (buffer[pos] & 0xff));
}

/**
* @brief Sets a 16-bit sample in the array.
* @param buffer The samples array.
* @param pos The position to insert.
* @param sample The sample to be inserted.
*/
void setSample(uint16_t* buffer, int pos, short sample)
{
	buffer[pos] = (uint16_t) (sample & 0xff);
	buffer[pos + 1] = (uint16_t) ((sample >> 8) & 0xff);
}
