/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# "Insert system clock initialization code here" comment
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#include <asf.h>
#include "stdio_serial.h"
#include "conf_board.h"
#include "conf_clock.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Defines */
/** PWM frequency in Hz */
#define PWM_FREQUENCY      1000
/** Period value of PWM output waveform */
#define PERIOD_VALUE       100
/** Initial duty cycle value */
#define INIT_DUTY_VALUE    20
/** I/O Pins */
#define DIR	PIO_PC24_IDX		//Digital Pin 6
/** Interrupt Pins */
#define SENSOR_PIN PIO_PC25_IDX	//Digital Pin 5
/** TEST FOR SENDER TASK **/
#define MAX_LIMIT 100
#define MIN_LIMIT -10


/* Queue Data Structure*/
typedef struct {
	int32_t speed;	//Speed of the disk (Duty cycle of PWM)
	int dir;		//Direction of the disk (1 -> Right, 0 -> left)
} xMotorData;

typedef struct {
	int count1;
	int count2;
} xP_Count;

/* Global Variables and Handles */
pwm_channel_t pwm_opts;
xQueueHandle xMotorQueue;
xQueueHandle xP_CountQueue;
int g_count;

/* Prototypes */
static void motor_task( void *pvParameters );
static void motor_test( void *pvParameters );
static void sender_task( void *pvParameters );
static void direction_task( void *pvParameters );
void sensor_handler(const uint32_t id, const uint32_t index);
void encoder_handler(const uint32_t id, const uint32_t index);

/**
 *  Configure UART console.
 */
// [main_console_configure]
static void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
		#ifdef CONF_UART_CHAR_LENGTH
		.charlength = CONF_UART_CHAR_LENGTH,
		#endif
		.paritytype = CONF_UART_PARITY,
		#ifdef CONF_UART_STOP_BITS
		.stopbits = CONF_UART_STOP_BITS,
		#endif
	};
	
	/* Configure console UART. */
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
	stdio_serial_init(CONSOLE_UART, &uart_serial_options);
}


int main (void)
{
	/* Insert system clock initialization code here (sysclk_init()). */
	sysclk_init();
	board_init();

	/* Initialize the serial I/O(console ) */
	configure_console();

	/* Insert application code here, after the board has been initialized. */

	/*---PWM CODE---*/
	pwm_clock_t pwm_clock_opts = {
		.ul_clka = PWM_FREQUENCY * PERIOD_VALUE, //1Khz frequency = 1 ms steps
		.ul_clkb = 0,
		.ul_mck =  sysclk_get_cpu_hz()
	};

	//PWM Options
	pwm_opts.ul_prescaler = PWM_CMR_CPRE_CLKA;
	pwm_opts.ul_period = PERIOD_VALUE;
	pwm_opts.ul_duty = INIT_DUTY_VALUE;
	pwm_opts.channel = PWM_CHANNEL_6;

	//Enable PWM clock
	pmc_enable_periph_clk(ID_PWM);

	//Temporarily disable the PWM channel
	pwm_channel_disable(PWM, PWM_CHANNEL_6);

	//Initialize PWM
	pwm_init(PWM, &pwm_clock_opts);
	
	// Initialize the PWM Channel
	pwm_channel_init(PWM, &pwm_opts);
	
	// Enable the PWM channel
	pwm_channel_enable(PWM, PWM_CHANNEL_6);

	/*---I/O CODE---*/
	ioport_set_pin_dir(DIR, IOPORT_DIR_OUTPUT);
	ioport_set_pin_mode(DIR, IOPORT_MODE_PULLUP);

	/*---QUEUE CODE---*/
	xMotorQueue = xQueueCreate( 5, sizeof(xMotorData) );
	xP_CountQueue = xQueueCreate( 5, sizeof(xP_Count) );

	/*---SHAFT ENCODER INTERRUPT---*/
	
	ioport_set_pin_dir(PIO_PB25_IDX, IOPORT_DIR_INPUT);
	ioport_set_pin_mode(PIO_PB25_IDX, IOPORT_MODE_PULLUP);
	ioport_set_pin_dir(PIO_PC28_IDX, IOPORT_DIR_INPUT);
	ioport_set_pin_mode(PIO_PC28_IDX, IOPORT_MODE_PULLUP);
	
	pmc_enable_periph_clk(ID_PIOB);
	pmc_enable_periph_clk(ID_PIOC);
	pio_set_input(PIOB, PIO_PB25, PIO_PULLUP);
	pio_set_input(PIOC, PIO_PC28, PIO_PULLUP);
	pio_handler_set( PIOB, ID_PIOB, PIO_PB25, PIO_IT_EDGE, encoder_handler );
	pio_handler_set( PIOC, ID_PIOC, PIO_PC28, PIO_IT_EDGE, encoder_handler );
	pio_enable_interrupt(PIOB, PIO_PB25);
	pio_enable_interrupt(PIOC, PIO_PC28);
	NVIC_SetPriority(PIOB_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 2);
	NVIC_SetPriority(PIOC_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 2);
	NVIC_EnableIRQ(PIOB_IRQn);
	NVIC_EnableIRQ(PIOC_IRQn);

	/*---SENSOR CODE---*/
	ioport_set_pin_dir(SENSOR_PIN, IOPORT_DIR_INPUT);
	ioport_set_pin_mode(SENSOR_PIN, IOPORT_MODE_PULLUP);
	
	pmc_enable_periph_clk(ID_PIOA);
	pio_set_input(PIOA, PIO_PA14, PIO_PULLUP);
	pio_handler_set(PIOA, ID_PIOA, PIO_PA14, PIO_IT_EDGE, sensor_handler);
	pio_enable_interrupt(PIOA, PIO_PA14);
	NVIC_SetPriority(PIOA_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 4);
	NVIC_EnableIRQ(PIOA_IRQn);

	/*---TASK CODE---*/
	xTaskCreate(motor_task, "Motor Task", 1000, NULL, 2, NULL);
	xTaskCreate(direction_task,"Direction Task", 1000, NULL, 2, NULL);
	//xTaskCreate(sender_task, "Sender Task", 1000, NULL, 2, NULL);
	//xTaskCreate(motor_test, "Motor Test Task", 1000, NULL, 3, NULL);

	/* Start Task Scheduler */
	vTaskStartScheduler();
	for( ;; );
	return 0;
}

static void motor_test( void *pvParameters ){
	int check = 0;
	xMotorData itemToSend;
	itemToSend.speed = 20;
	portBASE_TYPE xStatus;
	
	for(;;){
		//printf("Motor_test has woken\n");
		if(check){
			itemToSend.dir = 1;
			check = 0;
		}
		else{
			itemToSend.dir = 0;
			check = 1;
		}
		xStatus = xQueueSendToBack( xMotorQueue, &itemToSend, 0);
		if(xStatus != pdPASS){
			printf("Could not send to Queue!\n");
		}
		//printf("Motor_test is going to sleeep!\n");
		
				
		const portTickType xDelay = 2000 / portTICK_RATE_MS;
		vTaskDelay( xDelay );
	}
}

static void motor_task( void *pvParameters ){
	xMotorData ReceivedMotorData;
	portBASE_TYPE xStatus;
	for(;;){
		//Check if data is received
		xStatus = xQueueReceive(xMotorQueue,&ReceivedMotorData,portMAX_DELAY);
		if(xStatus != pdPASS){
			printf("Could not retrieve from Queue.\n");
		}
		else{
			pwm_opts.channel = PWM_CHANNEL_6;
			pwm_channel_update_duty(PWM, &pwm_opts, ReceivedMotorData.speed);
			switch(ReceivedMotorData.dir){
				case 0:
					ioport_set_pin_level(DIR, LOW);
				break;
				case 1:
					ioport_set_pin_level(DIR, HIGH);
				break;
			}
		}
		taskYIELD();
	}
}

static void sender_task( void *pvParameters ){
	int count;
	portBASE_TYPE xStatus;
	xMotorData toSend;
	toSend.speed = 10;
	
	for(;;){
		/* //taskENTER_CRITICAL();
		count = g_count;
		//taskEXIT_CRITICAL();
		if(count >= MAX_LIMIT){
			toSend.dir = 1;
			printf("Turning Right! Count is: %d\n",count);
		}
		else if(count <= MIN_LIMIT){
			toSend.dir = 0;
			printf("Turning Left! Count is %d\n",count);
		}
		xStatus = xQueueSendToBack(xMotorQueue,&toSend,0);
		taskYIELD();
		*/
		taskENTER_CRITICAL();
		count = g_count;
		taskEXIT_CRITICAL();
		//printf("Count is: %d\n",count);
	}
}

static void direction_task( void *pvParameters ){
	for(;;){
		
	}
}

void sensor_handler(const uint32_t id, const uint32_t index){
	static int counter = 0;
	unsigned int pinVal = 0;
	unsigned int priority = 0;
	static int edge_check = 0;
	static int count_check = 0;
	static int prev_diff;
	xP_Count p_count;
	
	static uint32_t prevTick;
	static uint32_t currentTick;
	
	portBASE_TYPE xStatus;
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	if ((id == ID_PIOA) && (index == PIO_PA14)){
		priority = NVIC_GetPriority(PIOA_IRQn);
		pinVal = ioport_get_pin_level(SENSOR_PIN);
		//printf("Interrupt PIOB detected!. Priority: %d. Pin Value: %d\n", priority, pinVal);
		
		if(edge_check == 0){
			//This means that the edge has not risen yet.
			edge_check = 1;
			prevTick = xTaskGetTickCountFromISR();
		}
		else{
			//This means that th edge has already risen
			edge_check = 0;
			currentTick = xTaskGetTickCountFromISR();
			int diff = currentTick - prevTick;
			//printf("Diff: %d\n",diff);
			if(count_check >= 1){
				count_check = 0;
				p_count.count1 = prev_diff;
				p_count.count2 = diff;
				printf("p_count.count1 = %d\tp_count.count2 = %d\n",p_count.count1,p_count.count2);
			}
			else{
				count_check++;
				prev_diff = diff;
			}
		}
		
	}
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}

void encoder_handler(const uint32_t id, const uint32_t index){
	int pinValA;
	int pinValB;
	int state;
	int priority;
	int pinVal;
	static int prevState = 1;

	static int count;

	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	
	if( (id == ID_PIOB) && (index == PIO_PB25) ) {
		//printf("Motor A Triggered.\n");
	}
	if( (id == ID_PIOC) && (index == PIO_PC28) ){
		//printf("Motor B Triggered.\n");
	}

	//Check if correct interrupt is triggered
	if(( (id == ID_PIOB) && (index == PIO_PB25) ) || ( (id == ID_PIOC) && (index == PIO_PC28) )){



	//Read pin values
	pinValA = ioport_get_pin_level(PIO_PC28_IDX);
	pinValB = ioport_get_pin_level(PIO_PB25_IDX);

	//Find out the state of the motor
	if(pinValA == 0 && pinValB == 0){
		state = 1;
	}
	else if(pinValA == 0 && pinValB == 1){
		state = 2; 
	}
	else if(pinValA == 1 && pinValB == 1){
		state = 3;
	}
	else if(pinValA == 1 && pinValB == 0){
		state = 4;
	}

	//Determine count incrementing based on state 
	switch(state){
		case 1:
			if(prevState == 2)count++;		//Clockwise Rotation
			else count--;
			break;
		case 2:
			if(prevState == 3)count++;		//Clockwise Rotation
			else count--;
			break;
		case 3:
			if(prevState == 4)count++;		//Clockwise Rotation
			else count--;
			break;
		case 4:
			if(prevState == 1)count++;		//Clockwise Rotation
			else count--;
			break;
	}

	//printf("State: %d\tPrevState: %d\tCount: %d\n",state,prevState,count);
	
	prevState = state;

	/*if(abs(count) >= 244){
		count=0;
		//Release binary semaphore here
		xSemaphoreGiveFromISR( xBinarySemaphore, &xHigherPriorityTaskWoken );
		portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
	}*/

		/*count_g = count;
		if(abs(count) >= MAX_COUNT){
			count = 0;
		}*/
		//printf("Count: %d\n", count);
		//portENTER_CRITICAL();
		g_count = count;
		printf("Count is: %d\n",count);
		//portEXIT_CRITICAL();

	}
}

void vApplicationMallocFailedHook( void )
{
	/* This function will only be called if an API call to create a task, queue
	or semaphore fails because there is too little heap RAM remaining. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed char *pcTaskName )
{
	/* This function will only be called if a task overflows its stack.  Note
	that stack overflow checking does slow down the context switch
	implementation. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
	/* This example does not use the idle hook to perform any processing. */
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
	/* This example does not use the tick hook to perform any processing. */
}
