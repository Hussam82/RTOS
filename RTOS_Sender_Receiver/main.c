#include <stdio.h>
#include <stdlib.h>
#include "diag/trace.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

#define CCM_RAM __attribute__((section(".ccmram")))


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"



int recieved =0;
int sent = 0;
int sentfailed = 0;
int Tsender[] = {100, 140, 180, 220, 260, 300};
int i = -1;
QueueHandle_t xMainQueue;
TimerHandle_t SenderTimer = NULL;
TimerHandle_t RecievedTimer = NULL;
SemaphoreHandle_t xSemaphoreSender = NULL;
SemaphoreHandle_t xSemaphoreReciever = NULL;
TaskHandle_t SenderHandle = NULL;
TaskHandle_t RecieverHandle = NULL;







void init (void)
{


	printf("The number of sent messages %d\r\n", sent);
	printf("The number of blocked messages %d\r\n",sentfailed);
	sent = 0;
	sentfailed = 0;
	recieved = 0;
	i++;
	if(i > 0)
	{
		xQueueReset(xMainQueue);
	}
	if(i < 6)
	{
		printf("The new timer is : %i msec \r\n",Tsender[i]);
		xTimerChangePeriod(SenderTimer, pdMS_TO_TICKS(Tsender[i]), 0);
		xTimerReset(RecievedTimer, 0);
	}
	if(i == 6)
	{
		printf("Game Over\r\n");
		xTimerDelete(SenderTimer,0);
		xTimerDelete(RecievedTimer,0);
	}
}

void SenderTask (void* p)
{
		char myTxBuff[30];
		int CurrentTime;
		xMainQueue = xQueueCreate(2,sizeof(myTxBuff));
			while(1)
			{
				if(xSemaphoreTake(xSemaphoreSender, (TickType_t) 10000) == pdTRUE)
				{
					CurrentTime = xTaskGetTickCount();
					sprintf(myTxBuff, "%d",CurrentTime);
					if(xQueueSend(xMainQueue,(void*) myTxBuff,(TickType_t) 5)== pdTRUE)
					{
						sent++;
					}
					else
						sentfailed++;
				}
			}
}


void RecieverTask (void* p)
{
	char myRxBuff[30];
		while(1)
		{
			if(xSemaphoreTake( xSemaphoreReciever, (TickType_t) 10000) == pdTRUE)
			{
				if(xQueueReceive(xMainQueue,(void*) myRxBuff,(TickType_t) 5))
				{
					recieved++;
				}
			}
		}
}


void SenderCallB (void* p)
{
	xSemaphoreGive(xSemaphoreSender);
}


void RecieverCallB (void* p)
{
	if(recieved < 500)
	{
		xSemaphoreGive(xSemaphoreReciever);
	}
	else
		init();
}



int main()
{

	SenderTimer = xTimerCreate("SenderStart",(pdMS_TO_TICKS(Tsender[i])),pdTRUE, (void*) 0 , SenderCallB);
	RecievedTimer = xTimerCreate("RecieverStart",(pdMS_TO_TICKS(200)),pdTRUE, (void*) 0 , RecieverCallB);
	xTaskCreate(SenderTask, "Sender", 200, (void*) 0, tskIDLE_PRIORITY, &SenderHandle  );
	xTaskCreate(RecieverTask, "Receiver", 200, (void*) 0, tskIDLE_PRIORITY , &RecieverHandle  );
	vSemaphoreCreateBinary(xSemaphoreSender);
	vSemaphoreCreateBinary(xSemaphoreReciever);

	xSemaphoreTake(xSemaphoreSender, (TickType_t) 10000);
	xSemaphoreTake(xSemaphoreReciever, (TickType_t) 10000);

	if((SenderTimer != NULL) && (RecievedTimer != NULL))
	{
		xTimerStart(SenderTimer,0);
		xTimerStart(RecievedTimer,0);
	}
	init();
	vTaskStartScheduler();

}












#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------


void vApplicationMallocFailedHook( void )
{
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
volatile size_t xFreeStackSpace;

	/* This function is called on each cycle of the idle task.  In this case it
	does nothing useful, other than report the amout of FreeRTOS heap that
	remains unallocated. */
	xFreeStackSpace = xPortGetFreeHeapSize();

	if( xFreeStackSpace > 100 )
	{
		/* By now, the kernel has allocated everything it is going to, so
		if there is a lot of heap remaining unallocated then
		the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
		reduced accordingly. */
	}
}

void vApplicationTickHook(void) {
}

StaticTask_t xIdleTaskTCB CCM_RAM;
StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE] CCM_RAM;

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
  /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
  state will be stored. */
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

  /* Pass out the array that will be used as the Idle task's stack. */
  *ppxIdleTaskStackBuffer = uxIdleTaskStack;

  /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
  Note that, as the array is necessarily of type StackType_t,
  configMINIMAL_STACK_SIZE is specified in words, not bytes. */
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

static StaticTask_t xTimerTaskTCB CCM_RAM;
static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH] CCM_RAM;

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize) {
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
  *ppxTimerTaskStackBuffer = uxTimerTaskStack;
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
