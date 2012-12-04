#include "orientation.h"

#include "cmsis_os.h"
#include "stm32f4_discovery_lis302dl.h"

#include <stdbool.h>

#define ORIENTATION_TIMER_SIGNAL 0x1
#define ORIENTATION_DELAY 100 // Milliseconds between readings

#define MAILBOX_SIZE 32

// Calibration offsets
#define X_OFFSET 0
#define Y_OFFSET 0
#define Z_OFFSET 0

static void orientation_timer_isr(const void* arg);

static void read_orientation(const void* arg);
static void process_orientation(const void* arg);

osTimerDef(orientation_timer, orientation_timer_isr);

// Worker threads
osThreadDef(read_orientation, osPriorityNormal, 1, 0);
osThreadDef(process_orientation, osPriorityNormal, 1, 0);

osMailQDef(orientation_mailbox, MAILBOX_SIZE, orientation);
osMailQId orientation_mailbox;

static osThreadId read_orientation_id;

static orientation_callback callback;

/**
  * @brief Initializes the orientation module.
  * @retval None
  */
void init_orientation(orientation_callback c)
{
	callback = c;
	
	LIS302DL_InitTypeDef lis302dl;
	lis302dl.Power_Mode = LIS302DL_LOWPOWERMODE_ACTIVE;
	lis302dl.Output_DataRate = LIS302DL_DATARATE_400;
	lis302dl.Axes_Enable = LIS302DL_XYZ_ENABLE;
	lis302dl.Full_Scale = LIS302DL_FULLSCALE_2_3;
	lis302dl.Self_Test = LIS302DL_SELFTEST_NORMAL;
	
	LIS302DL_Init(&lis302dl);
	
	LIS302DL_InterruptConfigTypeDef lis302dl_int;
	lis302dl_int.Latch_Request = LIS302DL_INTERRUPTREQUEST_LATCHED;
	lis302dl_int.SingleClick_Axes = LIS302DL_CLICKINTERRUPT_XYZ_DISABLE;
	lis302dl_int.DoubleClick_Axes = LIS302DL_DOUBLECLICKINTERRUPT_XYZ_DISABLE;
	
	LIS302DL_InterruptConfig(&lis302dl_int);
	
	osDelay(30);
	
	// Initialize mailbox, threads and timer
	orientation_mailbox = osMailCreate(osMailQ(orientation_mailbox), NULL);
	
	read_orientation_id = osThreadCreate(osThread(read_orientation), NULL);
	osThreadCreate(osThread(process_orientation), NULL);
	
	osTimerId id = osTimerCreate(osTimer(orientation_timer), osTimerPeriodic, 0);
	osTimerStart(id, ORIENTATION_DELAY);
}

/**
  * @brief Handler for orientation timer interrupt.
  * @param arg Unused
  * @retval None
  */
static void orientation_timer_isr(const void* arg)
{
	osSignalSet(read_orientation_id, ORIENTATION_TIMER_SIGNAL);
}

/**
  * @brief Reads orientation on signal and stores it in mailbox.
  * @param arg Unused
  * @retval None
  */
static void read_orientation(const void* arg)
{
	while (true)
	{
		osSignalWait(ORIENTATION_TIMER_SIGNAL, osWaitForever);
		
		uint8_t buffer[6];
		LIS302DL_Read(buffer, LIS302DL_OUT_X_ADDR, 6);
		
		orientation* orient = (orientation*) osMailAlloc(orientation_mailbox, osWaitForever);
		orient->x = (int8_t) buffer[0] - X_OFFSET;
		orient->y = (int8_t) buffer[2] - Y_OFFSET;
		orient->z = (int8_t) buffer[4] - Z_OFFSET;
		
		osMailPut(orientation_mailbox, orient);
	}
}

/**
  * @brief Processes data from mailbox and invokes the callback.
  * @param arg Unused
  * @retval None
  */
static void process_orientation(const void* arg)
{
	while (true)
	{
		osEvent event = osMailGet(orientation_mailbox, osWaitForever);
		
		if (event.status == osEventMail)
		{
			orientation* orient = (orientation*) event.value.p;
			
			if (callback)
				callback(orient);
			
			osMailFree(orientation_mailbox, orient);
			osThreadYield();
		}
	}
}
