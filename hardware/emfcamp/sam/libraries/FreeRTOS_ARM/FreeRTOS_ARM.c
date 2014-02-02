/**
 * \file
 * \brief FreeRTOS for Due and Teensy 3.0
 */
#include <FreeRTOS_ARM.h>
#include <Arduino.h>
//------------------------------------------------------------------------------
/** calibration factor for delayMS */
#define CAL_FACTOR (F_CPU/7000)
/** delay between led error flashes
 * \param[in] millis milliseconds to delay
 */
static void delayMS(uint32_t millis) {
  uint32_t iterations = millis * CAL_FACTOR;
  int i;
  for(i = 0; i < iterations; ++i) {
    asm volatile("nop\n\t");
  }
}
//------------------------------------------------------------------------------
/** Blink error pattern
 *
 * \param[in] n  number of short pulses
 */
static void errorBlink(int n) {
	noInterrupts();
  pinMode(13, OUTPUT);
  for (;;) {
    int i;
    for (i = 0; i < n; i++) {
      digitalWrite(13, 1);
      delayMS(300);
      digitalWrite(13, 0);
      delayMS(300);
    }
    delayMS(2000);
  }
}
//------------------------------------------------------------------------------
// catch Teensy and Due exceptions
/** Hard fault - blink one short flash every two seconds */
void hard_fault_isr()	{errorBlink(1);}
/** Hard fault - blink one short flash every two seconds */
void HardFault_Handler() 	{errorBlink(1);}

/** Bus fault - blink two short flashes every two seconds */
void bus_fault_isr() {errorBlink(2);}
/** Bus fault - blink two short flashes every two seconds */
void BusFault_Handler() {errorBlink(2);}

/** Usage fault - blink three short flashes every two seconds */
void usage_fault_isr() {errorBlink(3);}
/** Usage fault - blink three short flashes every two seconds */
void UsageFault_Handler() {errorBlink(3);}
//------------------------------------------------------------------------------
	/** vApplicationMallocFailedHook()
   Blink four short pulses if malloc fails.

	will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue,
	timer or semaphore is created.  It is also called by various parts of the
	demo application.  If heap_1.c or heap_2.c are used, then the size of the
	heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
	FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
	to query the size of free heap space that remains (although it does not
	provide information on how the remaining heap might be fragmented). */
void vApplicationMallocFailedHook() {
  errorBlink(4);
}
//------------------------------------------------------------------------------

	/** vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
	to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
	task.  It is essential that code added to this hook function never attempts
	to block in any way (for example, call xQueueReceive() with a block time
	specified, or call vTaskDelay()).  If the application makes use of the
	vTaskDelete() API function (as this demo application does) then it is also
	important that vApplicationIdleHook() is permitted to return to its calling
	function, because it is the responsibility of the idle task to clean up
	memory allocated by the kernel to any task that has since been deleted. */
void  __attribute__((weak)) vApplicationIdleHook( void ) {
  void loop();
  loop();
}
/*-----------------------------------------------------------*/
	/**  Blink five short pulses if stack overflow is detected.
	Run time stack overflow checking is performed if
	configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected.
  \param[in] pxTask Task handle
  \param[in] pcTaskName Task name
  */
void vApplicationStackOverflowHook(xTaskHandle pxTask, char *pcTaskName) {
	(void) pcTaskName;
	(void) pxTask;
	errorBlink(5);
}
/*-----------------------------------------------------------*/
	/** This function will be called by each tick interrupt if
	configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
	added here, but the tick hook is called from an interrupt context, so
	code must not attempt to block, and only the interrupt safe FreeRTOS API
	functions can be used (those that end in FromISR()). */
void __attribute__((weak)) vApplicationTickHook() {
}
/*-----------------------------------------------------------*/
/** Dummy time stats gathering functions need to be defined to keep the
linker happy.  Could edit FreeRTOSConfig.h to remove these.*/
void vMainConfigureTimerForRunTimeStats( void ) {}
/** Dummy function
 *  \return zero
 */
unsigned long ulMainGetRunTimeCounterValue() {return 0UL;}