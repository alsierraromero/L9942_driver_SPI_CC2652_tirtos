
/*
 *  ======== spimaster.c ========
 */
/* XDC module Headers */
#include <xdc/std.h>                                                            // XDC "base types" - must be included FIRST
#include <xdc/runtime/Types.h>                                                  // XDC constants/types used in xdc.runtime pkg
#include <xdc/cfg/global.h>                                                     // For all BIOS instances created statically in RTOS .cfg file
#include <xdc/runtime/Error.h>                                                  // For error handling (e.g. Error block)
#include <xdc/runtime/System.h>                                                 // XDC System functions (e.g. System_abort(), System_printf())
#include <xdc/runtime/Log.h>                                                    // For any Log_info() call

// TI-RTOS Kernel Header Files
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Mailbox.h>

#include <ti/drivers/Board.h>
#include "ti_drivers_config.h"                                                  // Syscfg Board/Drivers Header file
#include <ti/drivers/GPIO.h>
#include <ti/drivers/SPI.h>
#include <ti/drivers/PWM.h>
#include <ti/drivers/Timer.h>
#include <ti/drivers/Power.h>
#include <ti/display/Display.h>
#include <ti/display/DisplayUart.h>

// Standard C Header Files
#include <stdint.h>
#include <stddef.h>

//#define THREADSTACKSIZE (1024)

#define STALL_TH        0x00FF                                                    //Set this number for stall detection. When the sum of the rise times is less than this the ST bit (register 6 bit 8) is set to a logic “1”.
#define D10             (STALL_TH & 0x03)
#define D432            ((STALL_TH & 0x1c)>>2)
#define D765            ((STALL_TH & 0xE0)>>5)

#define CURRENT_PRF2    0x0C                                                    //Current profiles according to table of figure 16
#define CURRENT_PRF3    0x11
#define CURRENT_PRF4    0x16
#define CURRENT_PRF5    0x1A
#define CURRENT_PRF6    0x1D
#define CURRENT_PRF7    0x1E
#define CURRENT_PRF8    0x1F

//Registers Init values
#define REG0_VAL        0x99                                                    // counter-clockwise, micro-stepping, Auto decay, fast decay without delay time
#define REG1_VAL        0x700                                                   //1500 mA full DAC scale
#define REG2_VAL        0X600                                                   //Current profile based on table Figure 16
#define REG3_VAL        ((CURRENT_PRF3<<8) | (D10<<6) | (CURRENT_PRF2))
#define REG4_VAL        ((CURRENT_PRF5<<8) | (D432<<5) | (CURRENT_PRF4))
#define REG5_VAL        ((CURRENT_PRF7<<8) | (D765<<5) | (CURRENT_PRF6))
#define REG6_VAL        ((1<<11) | (CURRENT_PRF8))                              //Setting ST pin value to PWM pin

#define SPI_MSG_LENGTH      (16)
#define SPI_MAX_NUMB_MSG    (7)

#define STEPPER_FREQUENCY       6100

uint16_t masterRxBuffer[SPI_MAX_NUMB_MSG];
uint16_t masterTxBuffer[SPI_MAX_NUMB_MSG];
enum registers {reg0, reg1, reg2, reg3, reg4, reg5, reg6, reg7};                    //Register addresses

Mailbox_Handle stepEnMB;
Timer_Handle    timer0_h;
Timer_Params    tparams;
Semaphore_Handle semReport, dirgpio;

/* Write Op: Write buffer will start with register address and then the data
 * Read Op: Write buffer will need to have the register address OR'ed with 0x80 (to indicate a read op)
 *  */
void dataSPI(char msgsize, uint16_t* wbuff, uint16_t* rbuff){
    SPI_Handle      spi;
    SPI_Params      spiParams;
    SPI_Transaction spiTransaction;
    bool            transferOK;

    SPI_init();
    SPI_Params_init(&spiParams);
    spiParams.frameFormat = SPI_POL0_PHA0;                                      //Mode0
    spiParams.bitRate = 1000000;
    spiParams.dataSize = SPI_MSG_LENGTH;

    spi = SPI_open(CONFIG_SPI_MASTER, &spiParams);
    if (spi == NULL) {
        System_printf("Error initializing master SPI\n");
    }

    spiTransaction.count = msgsize;
    spiTransaction.txBuf = (void *)wbuff;
    spiTransaction.rxBuf = (void *)rbuff;
    transferOK = SPI_transfer(spi, &spiTransaction);
/*    if (!transferOK) {
        System_printf("Unsuccessful master SPI transfer\n");
    }
*/
    SPI_close(spi);
}

void InitMotorDriverTask(void){
    uint16_t data2send[]={REG0_VAL, REG1_VAL, REG2_VAL, REG3_VAL, REG4_VAL, REG5_VAL, REG6_VAL};
    uint8_t i=0;

    for(i=0;i<SPI_MAX_NUMB_MSG;i++){
        masterTxBuffer[0]=((i<<13) | data2send[i]);
        dataSPI(1,masterTxBuffer,masterRxBuffer);
    }
}

void timerFxnReport(void){
    Semaphore_post(semReport);
}

void myTimer_init(void){
    Timer_Params_init(&tparams);
    tparams.periodUnits = Timer_PERIOD_HZ;
    tparams.period = 1;
    tparams.timerMode  = Timer_CONTINUOUS_CALLBACK;
    tparams.timerCallback = timerFxnReport;
    timer0_h = Timer_open(CONFIG_TIMER_0, &tparams);
    Timer_start(timer0_h);
}

void taskReportFxn(void){
    while(1){
        Semaphore_pend(semReport, BIOS_WAIT_FOREVER);
        masterTxBuffer[0]=(reg7<<13);
        dataSPI(1,masterTxBuffer,masterRxBuffer);
    }
}


void taskPWMFxn (void){
    uint8_t stepStatus;

    PWM_Handle pwm;
    PWM_Params pwmParams;
    uint32_t   dutyValue;
    PWM_init();
    // Initialize the PWM parameters
    PWM_Params_init(&pwmParams);
    pwmParams.idleLevel = PWM_IDLE_LOW;
    pwmParams.periodUnits = PWM_PERIOD_HZ;
    pwmParams.periodValue = STEPPER_FREQUENCY;
    pwmParams.dutyUnits = PWM_DUTY_FRACTION; // Duty is in fractional percentage
    pwmParams.dutyValue = 0;                 // 0% initial duty cycle
    // Open the PWM instance
    pwm = PWM_open(CONFIG_PWM_1, &pwmParams);
    if (pwm == NULL) {
        System_printf("PWM handle create failed");
    }
    PWM_start(pwm);                          // start PWM with 0% duty cycle

    while(1){
        Mailbox_pend(stepEnMB, &stepStatus, BIOS_WAIT_FOREVER);
        if(!stepStatus){
            dutyValue = (uint32_t) (((uint64_t) PWM_DUTY_FRACTION_MAX * 50) / 100);
            PWM_setDuty(pwm, dutyValue);
        }
        else{
            dutyValue = (uint32_t) (((uint64_t) PWM_DUTY_FRACTION_MAX * 0) / 100);
            PWM_setDuty(pwm, dutyValue);
        }
    }

}

void taskSetDirFxn(){
    while(1){
        Semaphore_pend(dirgpio, BIOS_WAIT_FOREVER);
        masterTxBuffer[0]=0;
        dataSPI(1,masterTxBuffer,masterRxBuffer);                                               //Reading reg0
        masterTxBuffer[0] = ((masterRxBuffer[0] ^ 1) & 0x1FFF);                                 //masking to add address reg 0
        dataSPI(1,masterTxBuffer,masterRxBuffer);                                               //Writing to reg0
        GPIO_toggle(CONFIG_LED_DIR);
        }
}


void *main(void *arg0)
{
    Task_Handle stepEn_h, dirToggle_h, report_h;
    Task_Params taskParams;
    Mailbox_Params mbParams;

    /* Call driver init functions. */
    Board_init();
    GPIO_init();
    GPIO_enableInt(CONFIG_GPIO_STEP_EN);
    GPIO_enableInt(CONFIG_GPIO_MOTOR_DIR);
    GPIO_enableInt(PWM_Sense_Stall);

    SPI_init();
    InitMotorDriverTask();
//    myTimer_init();                                                                               //Remove comment to get register 7 status every 1 second

    // Create Task(s)
   Task_Params_init(&taskParams);
   taskParams.priority = 3;

   dirToggle_h = Task_create((Task_FuncPtr)taskSetDirFxn, &taskParams, Error_IGNORE);
   if (dirToggle_h == NULL) {
       System_abort("Task Direction Toggle create failed");
   }

   stepEn_h = Task_create((Task_FuncPtr)taskPWMFxn, &taskParams, Error_IGNORE);
   if (stepEn_h == NULL) {
      System_abort("Task PWM create failed");
   }

   taskParams.priority = 2;
   report_h = Task_create((Task_FuncPtr)taskReportFxn, &taskParams, Error_IGNORE);
      if (report_h == NULL) {
         System_abort("Task Report create failed");
      }

   semReport = Semaphore_create(0, NULL, Error_IGNORE);
   if (semReport == NULL) {
      System_abort("Semaphore Trigger create failed");
   }
   dirgpio = Semaphore_create(0, NULL, Error_IGNORE);
   if (dirgpio == NULL) {
     System_abort("Direction create failed");
   }

   Mailbox_Params_init(&mbParams);
   //Create Mailbox
   stepEnMB = Mailbox_create(sizeof(uint8_t), 2, &mbParams,Error_IGNORE);
   if (stepEnMB == NULL) {
     System_abort("Mailbox with step enable gpio status create failed");
   }

   BIOS_start();
   return (NULL);
}

void Callback_GPIO_Step_En(){
    uint8_t gpioStepStatus = GPIO_read(CONFIG_GPIO_STEP_EN);
    Mailbox_post(stepEnMB, &gpioStepStatus, BIOS_WAIT_FOREVER);
}

void Callback_GPIO_Dir(){
    Semaphore_post(dirgpio);
}

void Callback_GPIO_Stall_Det(){
    GPIO_write(CONFIG_LED_STALL,GPIO_read(PWM_Sense_Stall));
}


