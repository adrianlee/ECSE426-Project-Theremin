#include "sine.h"
#include "cmsis_os.h"

int main(void)
{
  init_sine_generator(I2S_AudioFreq_44k, 75);
	
	generate_sine(261.626);
	osDelay(1000);
	
	generate_sine(293.665);
	osDelay(1000);
	
	generate_sine(329.628);
	osDelay(1000);
	
	generate_sine(349.228);
	osDelay(1000);
	
	generate_sine(391.995);
	osDelay(1000);
	
	generate_sine(440.000);
	osDelay(1000);
	
	generate_sine(493.883);
	osDelay(1000);
	
	generate_sine(523.251);
	osDelay(1000);
	
	EVAL_AUDIO_PauseResume(AUDIO_PAUSE);
	Audio_MAL_Stop();
}
