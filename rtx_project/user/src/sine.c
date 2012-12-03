#include "sine.h"
#include "cmsis_os.h"

#include <stdbool.h>
#include <math.h>

#define PI 3.14159265358979323846f
#define TRANSFER_COMPLETE_SIGNAL 0x1

static float sampling_period;
static uint8_t volume;

static osThreadId change_frequency_id;

static uint16_t sine_table[SINE_TABLE_SIZE];
static unsigned int num_samples;

static float new_freq;
static volatile bool freq_changed = false;

static void generate_sine_helper(float freq);
static void change_frequency(const void* arg);

osThreadDef(change_frequency, osPriorityNormal, 1, 0);

void init_sine_generator(uint32_t freq, uint8_t vol)
{
	sampling_period = 1.0f / freq;
	volume = vol;
	
	EVAL_AUDIO_SetAudioInterface(AUDIO_INTERFACE_I2S);
	EVAL_AUDIO_Init(OUTPUT_DEVICE_AUTO, volume, freq);
	
	change_frequency_id = osThreadCreate(osThread(change_frequency), NULL);
}

void generate_sine(float freq)
{
	static bool first = true;
	
	if (first)
	{
		generate_sine_helper(freq);
		Audio_MAL_Play((uint32_t) &sine_table, num_samples);
		first = false;
	}
	
	else
	{
		new_freq = freq;
		freq_changed = true;
	}
}

void EVAL_AUDIO_TransferComplete_CallBack(uint32_t buffer, uint32_t size)
{
	osSignalSet(change_frequency_id, TRANSFER_COMPLETE_SIGNAL);
}

static void generate_sine_helper(float freq)
{
	num_samples = (unsigned int) ((1 / freq) / sampling_period);
	const float period = 2 * PI / num_samples;
	
	for (unsigned int i = 0; i < num_samples; ++i)
		sine_table[i] = (uint16_t) (SINE_AMPLITUDE * sin(i * period) + SINE_AMPLITUDE);
}

static void change_frequency(const void* arg)
{
	while (true)
	{
		osSignalWait(TRANSFER_COMPLETE_SIGNAL, osWaitForever);
		
		if (freq_changed)
		{
			EVAL_AUDIO_PauseResume(AUDIO_PAUSE);
			Audio_MAL_Stop();
			
			generate_sine_helper(new_freq);
			
			Audio_MAL_Play((uint32_t) &sine_table, num_samples);
			EVAL_AUDIO_PauseResume(AUDIO_RESUME);
			
			freq_changed = false;
		}
	}
}
