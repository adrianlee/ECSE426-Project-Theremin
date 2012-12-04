#include "sine.h"
#include "cmsis_os.h"

#include <stdbool.h>
#include <math.h>

#define PI 3.14159265358979323846f

#define GENERATE_SINE_SIGNAL 0x1
#define SWAP_BUFFERS_SIGNAL 0x2

#define XOR(x, y, type) ((type) (((uint32_t) (x)) ^ ((uint32_t) (y))))

static float sampling_period;
static uint8_t volume;

static uint16_t buffer0[SINE_TABLE_SIZE];
static uint16_t buffer1[SINE_TABLE_SIZE];

// Software double buffering
typedef uint16_t (*buffer_ptr)[SINE_TABLE_SIZE];
static buffer_ptr front_buffer = &buffer0;
static buffer_ptr back_buffer = &buffer1;

static osMutexId buffer_mutex;
osMutexDef(buffer_mutex);

static unsigned int num_samples;

static float new_freq;
static volatile bool freq_changed = false;

static void generate_sine_helper(float freq);

static void generator_thread(const void* arg);
static void swapper_thread(const void* arg);

osThreadDef(generator_thread, osPriorityNormal, 1, 0);
osThreadDef(swapper_thread, osPriorityNormal, 1, 0);

static osThreadId generator_id;
static osThreadId swapper_id;

/**
  * @brief Initializes the sine generator.
	* @param freq Sampling frequency
	* @param vol Initial volume
  * @retval None
  */
void init_sine_generator(uint32_t freq, uint8_t vol)
{
	sampling_period = 1.0f / freq;
	volume = vol;
	
	EVAL_AUDIO_SetAudioInterface(AUDIO_INTERFACE_I2S);
	EVAL_AUDIO_Init(OUTPUT_DEVICE_AUTO, volume, freq);
	
	buffer_mutex = osMutexCreate(osMutex(buffer_mutex));
	
	generator_id = osThreadCreate(osThread(generator_thread), NULL);
	swapper_id = osThreadCreate(osThread(swapper_thread), NULL);
}

/**
  * @brief Generates a sine wave with the given frequency.
	* @param freq Frequency to generate
  * @retval None
  */
void generate_sine(float freq)
{
	static bool first = true;
	
	if (first)
	{
		generate_sine_helper(freq);
		Audio_MAL_Play((uint32_t) *back_buffer, num_samples);
		first = false;
	}
	
	else
	{
		new_freq = freq;
		osSignalSet(generator_id, GENERATE_SINE_SIGNAL);
	}
}

/**
  * @brief Callback invoked when DMA transfer completes.
	* @param buffer Unused
	* @param size Unused
  * @retval None
  */
void EVAL_AUDIO_TransferComplete_CallBack(uint32_t buffer, uint32_t size)
{
	osSignalSet(swapper_id, SWAP_BUFFERS_SIGNAL);
}

/**
  * @brief Computes a sine wave with the given frequency and stores it in the back buffer.
	* @param freq Frequency of the sine wave
  * @retval None
  */
static void generate_sine_helper(float freq)
{
	num_samples = (unsigned int) ((1 / freq) / sampling_period);
	const float period = 2 * PI / num_samples;
	
	osMutexWait(buffer_mutex, osWaitForever);
	
	for (unsigned int i = 0; i < num_samples; ++i)
		(*back_buffer)[i] = (uint16_t) (SINE_AMPLITUDE * sin(i * period) + SINE_AMPLITUDE);
	
	osMutexRelease(buffer_mutex);
}

/**
  * @brief Generates sine wave on signal.
  * @retval None
  */
static void generator_thread(const void* arg)
{
	while (true)
	{
		osSignalWait(GENERATE_SINE_SIGNAL, osWaitForever);
		generate_sine_helper(new_freq);
		freq_changed = true;
	}
}

/**
  * @brief Swaps front and back buffers on signal.
  * @retval None
  */
static void swapper_thread(const void* arg)
{
	while (true)
	{
		osSignalWait(SWAP_BUFFERS_SIGNAL, osWaitForever);
		
		if (freq_changed)
		{
			Audio_MAL_Stop();
			osMutexWait(buffer_mutex, osWaitForever);
			
			// Swap buffers
			front_buffer = XOR(front_buffer, back_buffer, buffer_ptr);
			back_buffer = XOR(back_buffer, front_buffer, buffer_ptr);
			front_buffer = XOR(front_buffer, back_buffer, buffer_ptr);
			
			Audio_MAL_Play((uint32_t) *front_buffer, num_samples);
			osMutexRelease(buffer_mutex);
			
			freq_changed = false;
		}
	}
}
