#include "sine.h"
#include "cmsis_os.h"
#include "orientation.h"

#include <math.h>

#define PI 3.14159265358979323846f

#define NUM_PITCHES 8
#define DEFAULT_VOLUME 75
#define VOLUME_RANGE 50

static void on_orientation_changed(const orientation* orient);

static const float PITCHES[NUM_PITCHES] =
{
	261.626,
	293.665,
	329.628,
	349.228,
	391.995,
	440.000,
	493.883,
	523.251
};

static unsigned int pitch = 0;
static uint8_t volume = DEFAULT_VOLUME;

/**
  * @brief Entry point of program.
  * @retval None
  */
int main(void)
{
  init_sine_generator(I2S_AudioFreq_44k, volume);
	init_orientation(on_orientation_changed);
	
	generate_sine(PITCHES[pitch]);
	osDelay(osWaitForever);
}

/**
  * @brief Callback invoked when the orientation changes.
	* @param orient Current orientation
  * @retval None
  */
static void on_orientation_changed(const orientation* orient)
{
	if (orient->z < 0) // Ignore when upside down
		return;
	
	float alpha_den = sqrtf(orient->y * orient->y + orient->z * orient->z);
	float alpha = atan2(orient->x, alpha_den) / PI * 180;
	
	float beta_den = sqrtf(orient->x * orient->x + orient->z * orient->z);
	float beta = atan2(orient->y, beta_den) / PI * 180;
	
	pitch = (alpha + 90) / 180 * NUM_PITCHES;
	generate_sine(PITCHES[pitch]);
	
	volume = DEFAULT_VOLUME + (beta + 90) / 180 * VOLUME_RANGE - 0.5 * VOLUME_RANGE;
	EVAL_AUDIO_VolumeCtl(volume);
}
