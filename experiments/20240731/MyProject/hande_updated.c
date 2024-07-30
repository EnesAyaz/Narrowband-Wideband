#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <F2837xD_device.h>
#include <F2837xD_Pie_defines.h>
#include <F2837xD_Examples.h>


#define LED1 GPIO52 // Class D
#define LED2 GPIO94 // Class B
#define SW1 GPIO18 // Class D
#define SW2 GPIO29 // Class A
#define TRIG GPIO32 // Class B
#define DUMMYIO GPIO14 // Class A
#define EN1 GPIO125 // Class B

#define ENCODERMAXTICKCOUNT 4095

#define OCP GPIO24 // Class A
#define SCP GPIO16 // Class A

#define RESULTS_BUFFER_SIZE 256
#define sysclk_frequency    200000000   // Hz
#define sysclk_period       5               // ns
#define pwmclk_frequency    200000000   // Hz
#define pwmclk_period       5               // ns
#define PI                  3.141592654
#define sqrt2               1.414213562
#define sample_window       65          // clock
#define adc_frequency       50000000    //ns
#define adc_period          200         //ns

#define dead_time           100             // ns

__interrupt void cpu_timer0_isr(void);

__interrupt void epwm1_isr(void);

void GpioSelect(void);
void InitEpwm1(); // PWM1 - Phase A
void InitEpwm2(); // PWM2 - Phase B
void InitEpwm4(); // PWM4 - Phase C
void InitEpwm5(); // PWM5 - Phase D
void InitEpwm6(); // PWM6 - Phase E
void InitEpwm8(); // ADC i�in

void ConfigureADC(void);
void SetupADCEpwm(void);
void EQEPSetup(void);


float dutya = 0.05;
float dutyb = 0;
float dutyc = 0;
float dutyd = 0;
float dutye = 0;

float Iq1_set_speed = 0;

unsigned int OP_MODE = 0;


float POSLAT_now = 0;
float POSLAT_prev = 0;
float SpeedMax = 0;
float SpeedMin = 0;


int STARTMODE = 0;
#define BUMPSTART 0
#define PWLBEMF 1
float STARTBEMF = 2.5;
float STARTFREQ = 5.0;
float phaseInc = 0;
float mPhase = 0;
float INPUTFREQUENCY = 0;


// Global Variables
Uint16 resultsIndex;
volatile Uint16 bufferFull;
volatile Uint16 AdcCurr1;
volatile Uint16 AdcCurr2;
volatile Uint16 AdcVin;
volatile Uint16 AdcVout;
volatile float Vout;
volatile float Vin;
float Curr1;
float Curr2;
float Curr3;
float Curr4;
volatile Uint16 CodeTrig = 0;
volatile Uint16 StartTrig = 0;
int CurrentReadingCounter = 0;
int ADCIntCounter = 0;
int CurrentLoopCounter = 0;
int SpeedLoopCounter = 0;
/*
float VoltsPerBit = 0.000050354;
float CurrSensorSensitivity = 89;
float VinpGain = -195.9868743/2.85;
float VoutGain = 147.6634885;
 */
float VinpGain = -61.7522049;
float VoltsPerBit = 0.0000503534;

float Epwm7Counter = 0;
float MOTORFREQUENCY = 35;
float SpeedLoopFreq = 2000;
float CurrentLoopFreq = 20000;
float ModulIndex = 0.9;

float switching_frequency=50000;
float PWMFREQUENCY = 50000;
float PWMFREQINCREASE = 0;
float MOTORFREQUENCYINCREASE = 0;
float PHASEINCREMENT = 0;
float SINEPHASE = 0;


float BACKEMF = 0;
float VT = 0;
float VDC = 20.0;
float i_d = 0;
float i_q = 0;
volatile float theta = 0;
float SinePWMCount = 0;
float Epwm1Duty = 0.5;
float Epwm2Duty = 0.5;
float Epwm4Duty = 0.5;
float Epwm5Duty = 0.5;
float Epwm6Duty = 0.5;

// LPF COEFFICIENTS (sum must equal to 1)

float coefPrev = 0.0;
float coefNow = 1.0;

int dummyvar = 0;
unsigned long int EPwm1_isr_counter = 0;
int ButtonCount = 0;
float RefVMag = 0;




int main(void)
{

    InitSysCtrl();
    InitPeripheralClocks();
    InitPieCtrl();

    IER = 0x0000;
    IFR = 0x0000;

    InitPieVectTable();

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;   /*disable epwm clock to initialize epwm modules*/
    CpuSysRegs.PCLKCR0.bit.GTBCLKSYNC = 0;
    EDIS;

    EALLOW;
    ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV = 0; // EPWM Clock Divide Select: /1 of PLLSYSCLK
    EDIS;
    InitCpuTimers();   // For this example, only initialize the Cpu Timers
    //int timer0set = round(200000000/(PWMFREQUENCY));
    ConfigCpuTimer(&CpuTimer0, 200, 1000000); // 1Hz
    ConfigCpuTimer(&CpuTimer1, 200, 1000000); //1hz
    ConfigCpuTimer(&CpuTimer2, 200, 1000000); //1 seconds

    CpuTimer0Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0
    CpuTimer1Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0
    CpuTimer2Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0


    EALLOW;  // This is needed to write to EALLOW protected registers
    PieVectTable.TIMER0_INT = &cpu_timer0_isr;
    PieVectTable.EPWM1_INT = &epwm1_isr;
    EDIS;
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1; // Enable the PIE block
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1; //timer0
    PieCtrlRegs.PIEIER3.bit.INTx1 = 1; //epwm1

    IER |= M_INT1;
    IER |= M_INT10; // For ADC Int.
    IER |= M_INT3; //epwm1 isr
    //IER |= M_INT13;
    //IER |= M_INT14;


    GpioSelect();

    GpioDataRegs.GPDSET.bit.GPIO125 = 1;



    InitEpwm1();
    InitEpwm2();
    InitEpwm4();
    InitEpwm5();
    InitEpwm6();

    ConfigureADC(); // Configure ADC and power it up
    InitEpwm8(); // Initialize ePWM8
    SetupADCEpwm();
    EQEPSetup();

    EINT;  // Enable Global interrupt INTM
    ERTM;  // Enable Global realtime interrupt DBGM


    EALLOW;
    CpuSysRegs.PCLKCR2.bit.EPWM1 = 1;
    CpuSysRegs.PCLKCR2.bit.EPWM2 = 1;
    CpuSysRegs.PCLKCR2.bit.EPWM4 = 1;
    CpuSysRegs.PCLKCR2.bit.EPWM5 = 1;
    CpuSysRegs.PCLKCR2.bit.EPWM6 = 1;
    CpuSysRegs.PCLKCR2.bit.EPWM8 = 1;

    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;   /*enable epwm clock to initialize epwm modules*/
    CpuSysRegs.PCLKCR0.bit.GTBCLKSYNC =1;
    EDIS;

    //
    EPwm8Regs.ETSEL.bit.SOCAEN = 1;  //enable SOCA
    EPwm8Regs.TBCTL.bit.CTRMODE = 0; //unfreeze, and enter up count mode

    //
    //wait while ePWM causes ADC conversions, which then cause interrupts,
    //which fill the results buffer, eventually setting the bufferFull

    EALLOW;
    GpioCtrlRegs.GPBGMUX1.bit.GPIO42 = 3;
    GpioCtrlRegs.GPBMUX1.bit.GPIO42 = 3;
    GpioCtrlRegs.GPBGMUX1.bit.GPIO43 = 3;
    GpioCtrlRegs.GPBMUX1.bit.GPIO43 = 3;
    EDIS;



    while(1)
    {

    }

}

__interrupt void epwm1_isr(void)

{

    EPwm1_isr_counter++;

    if ((EPwm1_isr_counter%1000000)==0)
    {
        EPwm1_isr_counter = 0;
    }

    // Refresh the PWM frequency

    PWMFREQUENCY = switching_frequency + PWMFREQINCREASE;


    EPwm1Regs.TBPRD = (pwmclk_frequency) / (PWMFREQUENCY)* 0.5;
    EPwm2Regs.TBPRD = (pwmclk_frequency) / (PWMFREQUENCY)* 0.5;
    EPwm4Regs.TBPRD = (pwmclk_frequency) / (PWMFREQUENCY)* 0.5;
    EPwm5Regs.TBPRD = (pwmclk_frequency) / (PWMFREQUENCY)* 0.5;
    EPwm6Regs.TBPRD = (pwmclk_frequency) / (PWMFREQUENCY)* 0.5;

    Curr1 = ((float)(AdcaResultRegs.ADCRESULT1 - 50500))*VoltsPerBit*90.0;
    Curr2 = ((float)(AdcaResultRegs.ADCRESULT2 - 50540))*VoltsPerBit*90.0;
    Curr3 = ((float)(AdcbResultRegs.ADCRESULT0 - 50580))*VoltsPerBit*89.0;
    Curr4 = ((float)(AdcbResultRegs.ADCRESULT1 - 50540))*VoltsPerBit*89.0;




    EPwm1Regs.CMPA.bit.CMPA = Epwm1Duty * EPwm1Regs.TBPRD;
    EPwm2Regs.CMPA.bit.CMPA = Epwm2Duty * EPwm2Regs.TBPRD;
    EPwm4Regs.CMPA.bit.CMPA = Epwm4Duty * EPwm4Regs.TBPRD;
    EPwm5Regs.CMPA.bit.CMPA = Epwm5Duty * EPwm5Regs.TBPRD;
    EPwm6Regs.CMPA.bit.CMPA = Epwm6Duty * EPwm6Regs.TBPRD;

    EPwm1Regs.ETCLR.bit.INT = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
    AdcaRegs.ADCINTOVFCLR.bit.ADCINT2 = 1;

}


__interrupt void cpu_timer0_isr(void)
{

    CpuTimer0.InterruptCount++;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;

}



void GpioSelect(void)
{
    EALLOW;
    GpioCtrlRegs.GPAMUX1.all = 0;
    GpioCtrlRegs.GPAMUX2.all = 0;
    GpioCtrlRegs.GPAGMUX1.all = 0;
    GpioCtrlRegs.GPAGMUX2.all = 0;

    GpioCtrlRegs.GPBMUX1.all = 0;
    GpioCtrlRegs.GPBMUX2.all = 0;
    GpioCtrlRegs.GPBGMUX1.all = 0;
    GpioCtrlRegs.GPBGMUX2.all = 0;

    GpioCtrlRegs.GPCMUX1.all = 0;
    GpioCtrlRegs.GPCMUX2.all = 0;
    GpioCtrlRegs.GPCGMUX1.all = 0;
    GpioCtrlRegs.GPCGMUX2.all = 0;

    GpioCtrlRegs.GPDMUX1.all = 0;
    GpioCtrlRegs.GPDMUX2.all = 0;
    GpioCtrlRegs.GPDGMUX1.all = 0;
    GpioCtrlRegs.GPDGMUX2.all = 0;

    GpioCtrlRegs.GPEMUX1.all = 0;
    GpioCtrlRegs.GPEMUX2.all = 0;
    GpioCtrlRegs.GPEGMUX1.all = 0;
    GpioCtrlRegs.GPEGMUX2.all = 0;

    GpioCtrlRegs.GPFMUX1.all = 0;
    //    GpioCtrlRegs.GPFMUX2.all = 0;
    GpioCtrlRegs.GPFGMUX1.all = 0;
    //    GpioCtrlRegs.GPFGMUX2.all = 0;


    // LED GPIOs
    GpioCtrlRegs.GPBPUD.bit.LED1 = 0; // enable pull up
    GpioDataRegs.GPBSET.bit.LED1 = 1; // Load output latch. Recommended in rm
    GpioCtrlRegs.GPBDIR.bit.LED1 = 1; // set it as output

    GpioCtrlRegs.GPCPUD.bit.LED2 = 0; // enable pull up
    GpioDataRegs.GPCSET.bit.LED2 = 1; // Load output latch. Recommended in rm
    GpioCtrlRegs.GPCDIR.bit.LED2 = 1; // set it as output

    GpioDataRegs.GPBCLEAR.bit.LED1 = 1;
    GpioDataRegs.GPCCLEAR.bit.LED2 = 1;

    // TRIG GPIO
    GpioCtrlRegs.GPBPUD.bit.TRIG = 1; // enable pull up
    GpioCtrlRegs.GPBDIR.bit.TRIG = 1; // set it as output
    GpioDataRegs.GPBCLEAR.bit.TRIG = 1; // Clear data
    GpioCtrlRegs.GPAPUD.bit.DUMMYIO = 1; // enable pull up
    GpioCtrlRegs.GPADIR.bit.DUMMYIO = 1; // set it as output
    GpioDataRegs.GPACLEAR.bit.DUMMYIO = 1; // Clear data


    // Push Button GPIOs
    GpioCtrlRegs.GPAPUD.bit.SW1 = 1;
    GpioCtrlRegs.GPADIR.bit.SW1 = 0; // PB1
    GpioCtrlRegs.GPAQSEL2.bit.SW1 = 0; // XINT1 Synch to SYSCLKOUT only
    GpioCtrlRegs.GPACTRL.bit.QUALPRD2 = 0xFF;  //510*SYSCLOCK Low-pass Filter

    GpioCtrlRegs.GPAPUD.bit.SW2 = 1;
    GpioCtrlRegs.GPADIR.bit.SW2 = 0;
    GpioCtrlRegs.GPAQSEL2.bit.SW2 = 0;
    GpioCtrlRegs.GPACTRL.bit.QUALPRD3 = 0xFF;

    // Gate Driver Enable GPIOs
    GpioCtrlRegs.GPDPUD.bit.EN1 = 1; // disable pull up
    GpioCtrlRegs.GPDDIR.bit.EN1 = 1; // set it as output
    GpioDataRegs.GPDCLEAR.bit.EN1 = 1; // set 0

    //GpioCtrlRegs.GPBPUD.bit.EN2 = 1; // enable pull up
    //GpioCtrlRegs.GPBDIR.bit.EN2 = 1; // set it as output
    //GpioDataRegs.GPBCLEAR.bit.EN2 = 1; // set 0


    // PWM GPIOs
    GpioCtrlRegs.GPAPUD.bit.GPIO0 = 1;    // Disable pull-up on GPIO0 (EPWM1A)
    GpioCtrlRegs.GPAPUD.bit.GPIO1 = 1;    // Disable pull-up on GPIO1 (EPWM1B)

    GpioCtrlRegs.GPAGMUX1.bit.GPIO0 = 0;  // Configure GPIO0 as EPWM1A
    GpioCtrlRegs.GPAGMUX1.bit.GPIO1 = 0;  // Configure GPIO1 as EPWM1B
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;   // Configure GPIO0 as EPWM1A
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 1;   // Configure GPIO1 as EPWM1B

    GpioCtrlRegs.GPAGMUX1.bit.GPIO2 = 0;
    GpioCtrlRegs.GPAGMUX1.bit.GPIO3 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1;
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 1;

    GpioCtrlRegs.GPAGMUX1.bit.GPIO6 = 0;
    GpioCtrlRegs.GPAGMUX1.bit.GPIO7 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 1;
    GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 1;

    GpioCtrlRegs.GPAGMUX1.bit.GPIO8 = 0;
    GpioCtrlRegs.GPAGMUX1.bit.GPIO9 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO8 = 1;
    GpioCtrlRegs.GPAMUX1.bit.GPIO9 = 1;

    GpioCtrlRegs.GPAGMUX1.bit.GPIO10 = 0;
    GpioCtrlRegs.GPAGMUX1.bit.GPIO11 = 0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO10 = 1;
    GpioCtrlRegs.GPAMUX1.bit.GPIO11 = 1;

    GpioCtrlRegs.GPADIR.bit.GPIO0 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO1 = 1;
    GpioDataRegs.GPACLEAR.bit.GPIO0 = 1;
    GpioDataRegs.GPACLEAR.bit.GPIO1 = 1;

    GpioCtrlRegs.GPADIR.bit.GPIO2 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO3 = 1;
    GpioDataRegs.GPACLEAR.bit.GPIO2 = 1;
    GpioDataRegs.GPACLEAR.bit.GPIO3 = 1;

    GpioCtrlRegs.GPADIR.bit.GPIO6 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO7 = 1;
    GpioDataRegs.GPACLEAR.bit.GPIO6 = 1;
    GpioDataRegs.GPACLEAR.bit.GPIO7 = 1;

    GpioCtrlRegs.GPADIR.bit.GPIO8 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO9 = 1;
    GpioDataRegs.GPACLEAR.bit.GPIO8 = 1;
    GpioDataRegs.GPACLEAR.bit.GPIO9 = 1;

    GpioCtrlRegs.GPADIR.bit.GPIO10 = 1;
    GpioCtrlRegs.GPADIR.bit.GPIO11 = 1;
    GpioDataRegs.GPACLEAR.bit.GPIO10 = 1;
    GpioDataRegs.GPACLEAR.bit.GPIO11 = 1;

    // Over-current Protection GPIOs
    GpioCtrlRegs.GPAPUD.bit.OCP = 0; // Pull up is not disabled
    GpioCtrlRegs.GPADIR.bit.OCP = 0;
    GpioCtrlRegs.GPAQSEL2.bit.OCP = 3; // Asynch input
    InputXbarRegs.INPUT1SELECT = 16;

    GpioCtrlRegs.GPAPUD.bit.SCP = 0; // Pull up is not disabled
    GpioCtrlRegs.GPADIR.bit.SCP = 0;
    GpioCtrlRegs.GPAQSEL2.bit.SCP = 3; // Asynch input
    InputXbarRegs.INPUT2SELECT = 24;


    EDIS;
}

void InitEpwm1(void)
{


    EPwm1Regs.TBCTL.all = 0x00;
    EPwm1Regs.TBCTL.bit.CTRMODE = 2;   // Count up and douwn
    EPwm1Regs.TBCTL.bit.CLKDIV = 0;    // TBCLOK = EPWMCLOCK/(128*10) = 78125Hz
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = 0;


    EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO;
    EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm1Regs.TBPHS.bit.TBPHS = 0;

    EPwm1Regs.TBPRD = (pwmclk_frequency) / (PWMFREQUENCY)* 0.5; // PWM Block has half clock freq of system & up-down mode requires division by 2
    EPwm1Regs.TBCTR = 0x0000;          // Clear counter

    EPwm1Regs.CMPCTL.all = 0x00;
    EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;         //only active registers are used
    EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm1Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;         //only active registers are used
    EPwm1Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;


    EPwm1Regs.AQCTLA.all = 0x00;
    EPwm1Regs.AQCTLA.bit.CAU = 1; //set high
    EPwm1Regs.AQCTLA.bit.CAD = 2; //set low
    EPwm1Regs.AQCTLB.all = 0x00;
    //EPwm1Regs.AQCTLB.bit.CBU = 1; //set low
    //EPwm1Regs.AQCTLB.bit.CBD = 2; //set high

    EPwm1Regs.CMPA.bit.CMPA = EPwm1Regs.TBPRD/2;    // Set compare A value
    EPwm1Regs.CMPB.bit.CMPB = EPwm1Regs.TBPRD/2;    // Set Compare B value


    //EPwm1Regs.TBPHS.bit.TBPHS = 0x0000;          // Phase is 0

    EPwm1Regs.TBCTL2.all = 0x00;
    EPwm1Regs.CMPCTL2.all = 0x00;
    EPwm1Regs.DBCTL.all = 0x00;
    EPwm1Regs.DBCTL.bit.OUT_MODE = 3;
    EPwm1Regs.DBCTL.bit.POLSEL = 2;
    EPwm1Regs.DBFED.bit.DBFED = dead_time / pwmclk_period;
    EPwm1Regs.DBRED.bit.DBRED = dead_time / pwmclk_period;
    EPwm1Regs.DBCTL2.all = 0x00;
    //EPwm1Regs.TBCTL.bit.SYNCOSEL = 3;
    //EPwm1Regs.ETSEL.all = 0x00;

    EPwm1Regs.ETSEL.bit.INTSEL = 1; // When TBCTR == 0
    EPwm1Regs.ETSEL.bit.INTEN = 1;  // Enable INT
    //    EPwm1Regs.ETPS.all = 0x00;
    EPwm1Regs.ETPS.bit.INTPRD = 1;  // Generate INT on first event

}


void InitEpwm2(void)
{


    EPwm2Regs.TBCTL.all = 0x00;
    EPwm2Regs.TBCTL.bit.CTRMODE = 2;   // Count up and douwn
    EPwm2Regs.TBCTL.bit.CLKDIV = 0;    // TBCLOK = EPWMCLOCK/(128*10) = 78125Hz
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = 0;


    EPwm2Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;
    EPwm2Regs.TBCTL.bit.PHSEN = TB_ENABLE;
    EPwm2Regs.TBPHS.bit.TBPHS = 0;

    EPwm2Regs.TBPRD = (pwmclk_frequency) / (PWMFREQUENCY)* 0.5; // PWM Block has half clock freq of system & up-down mode requires division by 2
    EPwm2Regs.TBCTR = 0x0000;          // Clear counter

    EPwm2Regs.CMPCTL.all = 0x00;
    EPwm2Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;         //only active registers are used
    EPwm2Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm2Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;         //only active registers are used
    EPwm2Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;


    EPwm2Regs.AQCTLA.all = 0x00;
    EPwm2Regs.AQCTLA.bit.CAU = 2; //set high
    EPwm2Regs.AQCTLA.bit.CAD = 1; //set low
    EPwm2Regs.AQCTLB.all = 0x00;
    //EPwm2Regs.AQCTLB.bit.CBU = 1; //set low
    //EPwm2Regs.AQCTLB.bit.CBD = 2; //set high

    EPwm2Regs.CMPA.bit.CMPA = EPwm2Regs.TBPRD/2;    // Set compare A value
    EPwm2Regs.CMPB.bit.CMPB = EPwm2Regs.TBPRD/2;    // Set Compare B value


    //EPwm2Regs.TBPHS.bit.TBPHS = 0x0000;          // Phase is 0

    EPwm2Regs.TBCTL2.all = 0x00;
    EPwm2Regs.CMPCTL2.all = 0x00;
    EPwm2Regs.DBCTL.all = 0x00;
    EPwm2Regs.DBCTL.bit.OUT_MODE = 3;
    EPwm2Regs.DBCTL.bit.POLSEL = 2;
    EPwm2Regs.DBFED.bit.DBFED = dead_time / pwmclk_period;
    EPwm2Regs.DBRED.bit.DBRED = dead_time / pwmclk_period;
    EPwm2Regs.DBCTL2.all = 0x00;
    //EPwm2Regs.TBCTL.bit.SYNCOSEL = 3;
    EPwm2Regs.ETSEL.all = 0x00;

    //    EPwm2Regs.ETSEL.bit.INTSEL = 1; // When TBCTR == 0
    //    EPwm2Regs.ETSEL.bit.INTEN = 1;  // Enable INT
    //    EPwm2Regs.ETPS.all = 0x00;
    //    EPwm2Regs.ETPS.bit.INTPRD = 1;  // Generate INT on first event

}

void InitEpwm4(void)
{


    EPwm4Regs.TBCTL.all = 0x00;
    EPwm4Regs.TBCTL.bit.CTRMODE = 2;   // Count up and douwn
    EPwm4Regs.TBCTL.bit.CLKDIV = 0;    // TBCLOK = EPWMCLOCK/(128*10) = 78125Hz
    EPwm4Regs.TBCTL.bit.HSPCLKDIV = 0;


    EPwm4Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;
    EPwm4Regs.TBCTL.bit.PHSEN = TB_ENABLE;
    EPwm4Regs.TBPHS.bit.TBPHS = 0;

    EPwm4Regs.TBPRD = (pwmclk_frequency) / (PWMFREQUENCY)* 0.5; // PWM Block has half clock freq of system & up-down mode requires division by 2
    EPwm4Regs.TBCTR = 0x0000;          // Clear counter

    EPwm4Regs.CMPCTL.all = 0x00;
    EPwm4Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;         //only active registers are used
    EPwm4Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm4Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;         //only active registers are used
    EPwm4Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;


    EPwm4Regs.AQCTLA.all = 0x00;
    EPwm4Regs.AQCTLA.bit.CAU = 1; //set high
    EPwm4Regs.AQCTLA.bit.CAD = 2; //set low
    EPwm4Regs.AQCTLB.all = 0x00;
    //EPwm4Regs.AQCTLB.bit.CBU = 1; //set low
    //EPwm4Regs.AQCTLB.bit.CBD = 2; //set high

    EPwm4Regs.CMPA.bit.CMPA = EPwm4Regs.TBPRD/2;    // Set compare A value
    EPwm4Regs.CMPB.bit.CMPB = EPwm4Regs.TBPRD/2;    // Set Compare B value


    //EPwm4Regs.TBPHS.bit.TBPHS = 0x0000;          // Phase is 0

    EPwm4Regs.TBCTL2.all = 0x00;
    EPwm4Regs.CMPCTL2.all = 0x00;
    EPwm4Regs.DBCTL.all = 0x00;
    EPwm4Regs.DBCTL.bit.OUT_MODE = 3;
    EPwm4Regs.DBCTL.bit.POLSEL = 2;
    EPwm4Regs.DBFED.bit.DBFED = dead_time / pwmclk_period;
    EPwm4Regs.DBRED.bit.DBRED = dead_time / pwmclk_period;
    EPwm4Regs.DBCTL2.all = 0x00;
    //EPwm4Regs.TBCTL.bit.SYNCOSEL = 3;
    EPwm4Regs.ETSEL.all = 0x00;

    //    EPwm4Regs.ETSEL.bit.INTSEL = 1; // When TBCTR == 0
    //    EPwm4Regs.ETSEL.bit.INTEN = 1;  // Enable INT
    //    EPwm4Regs.ETPS.all = 0x00;
    //    EPwm4Regs.ETPS.bit.INTPRD = 1;  // Generate INT on first event

}


void InitEpwm5(void)
{


    EPwm5Regs.TBCTL.all = 0x00;
    EPwm5Regs.TBCTL.bit.CTRMODE = 2;   // Count up and douwn
    EPwm5Regs.TBCTL.bit.CLKDIV = 0;    // TBCLOK = EPWMCLOCK/(128*10) = 78125Hz
    EPwm5Regs.TBCTL.bit.HSPCLKDIV = 0;


    EPwm5Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;
    EPwm5Regs.TBCTL.bit.PHSEN = TB_ENABLE;
    EPwm5Regs.TBPHS.bit.TBPHS = 0;

    EPwm5Regs.TBPRD = (pwmclk_frequency) / (PWMFREQUENCY)* 0.5; // PWM Block has half clock freq of system & up-down mode requires division by 2
    EPwm5Regs.TBCTR = 0x0000;          // Clear counter

    EPwm5Regs.CMPCTL.all = 0x00;
    EPwm5Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;         //only active registers are used
    EPwm5Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm5Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;         //only active registers are used
    EPwm5Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;


    EPwm5Regs.AQCTLA.all = 0x00;
    EPwm5Regs.AQCTLA.bit.CAU = 2; //set high
    EPwm5Regs.AQCTLA.bit.CAD = 1; //set low
    EPwm5Regs.AQCTLB.all = 0x00;
    //EPwm5Regs.AQCTLB.bit.CBU = 1; //set low
    //EPwm5Regs.AQCTLB.bit.CBD = 2; //set high

    EPwm5Regs.CMPA.bit.CMPA = EPwm5Regs.TBPRD/2;    // Set compare A value
    EPwm5Regs.CMPB.bit.CMPB = EPwm5Regs.TBPRD/2;    // Set Compare B value


    //EPwm5Regs.TBPHS.bit.TBPHS = 0x0000;          // Phase is 0

    EPwm5Regs.TBCTL2.all = 0x00;
    EPwm5Regs.CMPCTL2.all = 0x00;
    EPwm5Regs.DBCTL.all = 0x00;
    EPwm5Regs.DBCTL.bit.OUT_MODE = 3;
    EPwm5Regs.DBCTL.bit.POLSEL = 2;
    EPwm5Regs.DBFED.bit.DBFED = dead_time / pwmclk_period;
    EPwm5Regs.DBRED.bit.DBRED = dead_time / pwmclk_period;
    EPwm5Regs.DBCTL2.all = 0x00;
    //EPwm5Regs.TBCTL.bit.SYNCOSEL = 3;
    EPwm5Regs.ETSEL.all = 0x00;

    //    EPwm5Regs.ETSEL.bit.INTSEL = 1; // When TBCTR == 0
    //    EPwm5Regs.ETSEL.bit.INTEN = 1;  // Enable INT
    //    EPwm5Regs.ETPS.all = 0x00;
    //    EPwm5Regs.ETPS.bit.INTPRD = 1;  // Generate INT on first event

}

void InitEpwm6(void)
{


    EPwm6Regs.TBCTL.all = 0x00;
    EPwm6Regs.TBCTL.bit.CTRMODE = 2;   // Count up and douwn
    EPwm6Regs.TBCTL.bit.CLKDIV = 0;    // TBCLOK = EPWMCLOCK/(128*10) = 78125Hz
    EPwm6Regs.TBCTL.bit.HSPCLKDIV = 0;


    EPwm6Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN;
    EPwm6Regs.TBCTL.bit.PHSEN = TB_ENABLE;
    EPwm6Regs.TBPHS.bit.TBPHS = 0;

    EPwm6Regs.TBPRD = (pwmclk_frequency) / (PWMFREQUENCY)* 0.5; // PWM Block has half clock freq of system & up-down mode requires division by 2
    EPwm6Regs.TBCTR = 0x0000;          // Clear counter

    EPwm6Regs.CMPCTL.all = 0x00;
    EPwm6Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;         //only active registers are used
    EPwm6Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    EPwm6Regs.CMPCTL.bit.SHDWBMODE = CC_SHADOW;         //only active registers are used
    EPwm6Regs.CMPCTL.bit.LOADBMODE = CC_CTR_ZERO;


    EPwm6Regs.AQCTLA.all = 0x00;
    EPwm6Regs.AQCTLA.bit.CAU = 1; //set high
    EPwm6Regs.AQCTLA.bit.CAD = 2; //set low
    EPwm6Regs.AQCTLB.all = 0x00;
    //EPwm6Regs.AQCTLB.bit.CBU = 1; //set low
    //EPwm6Regs.AQCTLB.bit.CBD = 2; //set high

    EPwm6Regs.CMPA.bit.CMPA = EPwm6Regs.TBPRD/2;    // Set compare A value
    EPwm6Regs.CMPB.bit.CMPB = EPwm6Regs.TBPRD/2;    // Set Compare B value


    //EPwm6Regs.TBPHS.bit.TBPHS = 0x0000;          // Phase is 0

    EPwm6Regs.TBCTL2.all = 0x00;
    EPwm6Regs.CMPCTL2.all = 0x00;
    EPwm6Regs.DBCTL.all = 0x00;
    EPwm6Regs.DBCTL.bit.OUT_MODE = 3;
    EPwm6Regs.DBCTL.bit.POLSEL = 2;
    EPwm6Regs.DBFED.bit.DBFED = dead_time / pwmclk_period;
    EPwm6Regs.DBRED.bit.DBRED = dead_time / pwmclk_period;
    EPwm6Regs.DBCTL2.all = 0x00;
    //EPwm6Regs.TBCTL.bit.SYNCOSEL = 3;
    EPwm6Regs.ETSEL.all = 0x00;

    //    EPwm6Regs.ETSEL.bit.INTSEL = 1; // When TBCTR == 0
    //    EPwm6Regs.ETSEL.bit.INTEN = 1;  // Enable INT
    //    EPwm6Regs.ETPS.all = 0x00;
    //    EPwm6Regs.ETPS.bit.INTPRD = 1;  // Generate INT on first event

}



void InitEpwm8(void)
{
    // after enabling the ePWM clock
    EALLOW;
    EPwm8Regs.ETSEL.bit.SOCAEN = 0;
    EPwm8Regs.ETSEL.bit.SOCASEL = 4;
    EPwm8Regs.ETPS.bit.SOCAPRD = 1;
    EPwm8Regs.CMPA.bit.CMPA = (EPwm1Regs.TBPRD - 1)/2;
    EPwm8Regs.TBPRD = EPwm1Regs.TBPRD;
    EPwm8Regs.TBCTL.bit.CTRMODE = 3; // freeze counter
    EDIS;
}

void ConfigureADC(void)
{
    EALLOW;
    AdcaRegs.ADCCTL2.bit.PRESCALE = 6;
    AdcSetMode(ADC_ADCA, ADC_RESOLUTION_16BIT, ADC_SIGNALMODE_DIFFERENTIAL);
    AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1; // Set pulse pos. to late
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1; // Power up ADC block
    DELAY_US(1000);

    AdcbRegs.ADCCTL2.bit.PRESCALE = 6;
    AdcSetMode(ADC_ADCB, ADC_RESOLUTION_16BIT, ADC_SIGNALMODE_DIFFERENTIAL);
    AdcbRegs.ADCCTL1.bit.INTPULSEPOS = 1; // Set pulse pos. to late
    AdcbRegs.ADCCTL1.bit.ADCPWDNZ = 1; // Power up ADC block
    DELAY_US(1000);
    EDIS;
}

void SetupADCEpwm()
{

    EALLOW;
    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 0;  //SOC0 will convert (ADCINA0 - ADCINA1)
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = sample_window; // 1.1MSPS -> 909 ns sampling window -> 910ns/5ns = 182 -> acqps = 182 - 1
    AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 19; // ePWM8 for Trig Signal
    AdcaRegs.ADCSOC1CTL.bit.CHSEL = 2;  //SOC1 will convert (ADCINA2 - ADCINA3)
    AdcaRegs.ADCSOC1CTL.bit.ACQPS = sample_window; // 1.1MSPS -> 909 ns sampling window -> 910ns/5ns = 182 -> acqps = 182 - 1
    AdcaRegs.ADCSOC1CTL.bit.TRIGSEL = 19; // ePWM8 for Trig Signal
    AdcaRegs.ADCSOC2CTL.bit.CHSEL = 4;  //SOC2 will convert (ADCINA4 - ADCINA5)
    AdcaRegs.ADCSOC2CTL.bit.ACQPS = sample_window; // 1.1MSPS -> 909 ns sampling window -> 910ns/5ns = 182 -> acqps = 182 - 1
    AdcaRegs.ADCSOC2CTL.bit.TRIGSEL = 19; // ePWM8 for Trig Signal

    AdcaRegs.ADCINTSEL1N2.bit.INT2SEL = 0; // end of SOC2 will set INT2 flag
    AdcaRegs.ADCINTSEL1N2.bit.INT2E = 1; // enable INT2 flag
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT2 = 1; // re-clear the flag
    // It is assumed all channels will be sampled in 910 nsec in order. Therefore, the results registers can be read after EOC2 signal./
    AdcbRegs.ADCSOC0CTL.bit.CHSEL = 2; //SOC0 will convert (ADCINB2 - ADCINB3)
    AdcbRegs.ADCSOC0CTL.bit.ACQPS = sample_window; // 1.1MSPS -> 909 ns sampling window -> 910ns/5ns = 182 -> acqps = 182 - 1
    AdcbRegs.ADCSOC0CTL.bit.TRIGSEL = 19; // ePWM8 for Trig Signal
    AdcbRegs.ADCSOC1CTL.bit.CHSEL = 4; //SOC1 will convert (ADCINB4 - ADCINB5)
    AdcbRegs.ADCSOC1CTL.bit.ACQPS = sample_window; // 1.1MSPS -> 909 ns sampling window -> 910ns/5ns = 182 -> acqps = 182 - 1
    AdcbRegs.ADCSOC1CTL.bit.TRIGSEL = 19; // ePWM8 for Trig Signal

    AdcbRegs.ADCINTSEL1N2.bit.INT2SEL = 0; // end of SOC0 will set INT2 flag
    AdcbRegs.ADCINTSEL1N2.bit.INT2E = 1; // enable INT2 flag
    AdcbRegs.ADCINTFLGCLR.bit.ADCINT2 = 1; // re-clear the flag

    // For removing the offset or for flagging high/low threshold PPB block can be configured accordingly
    EDIS;

}


void EQEPSetup(void)
{
    EALLOW;
    CpuSysRegs.PCLKCR4.bit.EQEP1 = 1;
    EDIS;
    EALLOW;                              /* Enable EALLOW*/

    /* Enable internal pull-up for the selected pins */
    GpioCtrlRegs.GPAPUD.bit.GPIO20 = 0;  /* Enable pull-up on GPIO20 (EQEP1A)*/
    GpioCtrlRegs.GPAPUD.bit.GPIO21 = 0;  /* Enable pull-up on GPIO21 (EQEP1B)*/
    GpioCtrlRegs.GPDPUD.bit.GPIO99 = 0;  /* Enable pull-up on GPIO99 (EQEP1I)*/

    /* Configure eQEP-1 pins using GPIO regs*/
    GpioCtrlRegs.GPAMUX2.bit.GPIO20 = 1; /* Configure GPIO20 as EQEP1A*/
    GpioCtrlRegs.GPAGMUX2.bit.GPIO20 = 0;
    GpioCtrlRegs.GPAMUX2.bit.GPIO21 = 1; /* Configure GPIO21 as EQEP1B  */
    GpioCtrlRegs.GPAGMUX2.bit.GPIO21 = 0;
    GpioCtrlRegs.GPDMUX1.bit.GPIO99 = 1; /* Configure GPIO99 as EQEP1I*/
    GpioCtrlRegs.GPDGMUX1.bit.GPIO99 = 1;
    EDIS;

    /*the formula will be X/(t(k)-t(k-1)) at low  speeds, can be used with UPEVNT */
    /*the formula will be (x(k)-x(k-1))/T at high speeds, can be used with eqep unit timer or CAPCLK */

    EQep1Regs.QUPRD = 2000000; // Unit Timer for 100 Hz at 200 MHz

    // Quadrature Decoder Unit (QDU) Registers
    EQep1Regs.QDECCTL.all = 0x00;    // Quadrature Decoder Control
    EQep1Regs.QDECCTL.bit.QSRC = 0;  // Position-counter source selection: Quadrature count mode (QCLK = iCLK, QDIR = iDIR)
    // hakansrc QSRC=2 girmis -- "0"
    EQep1Regs.QDECCTL.bit.SOEN = 0;  // Disable position-compare sync output
    EQep1Regs.QDECCTL.bit.SPSEL = 1; // Strobe pin is used for sync output: Don't care
    EQep1Regs.QDECCTL.bit.XCR = 0;   // External Clock Rate: 2x resolution: Count the rising/falling edge
    EQep1Regs.QDECCTL.bit.SWAP = 0;  // CLK/DIR Signal Source for Position Counter: Quadrature-clock inputs are not swapped
    EQep1Regs.QDECCTL.bit.IGATE = 0; // Disable gating of Index pulse
    EQep1Regs.QDECCTL.bit.QAP = 0;   // QEPA input polarity: No effect
    EQep1Regs.QDECCTL.bit.QBP = 0;   // QEPB input polarity: No effect
    EQep1Regs.QDECCTL.bit.QIP = 0;   // QEPI input polarity: No effect
    EQep1Regs.QDECCTL.bit.QSP = 0;   // QEPS input polarity: No effect

    // Position Counter and Control Unit (PCCU) Registers
    EQep1Regs.QEPCTL.all = 0x00;        // QEP Control
    EQep1Regs.QEPCTL.bit.FREE_SOFT = 0; // Emulation mode: Position counter stops immediately on emulation suspend
    EQep1Regs.QEPCTL.bit.PCRM = 1;      // Position counter reset on the maximum position
    EQep1Regs.QEPCTL.bit.IEI = 0;       // With 2, initializes the position counter on the rising edge of the QEPI signal
    EQep1Regs.QEPCTL.bit.IEL = 0;       // With 1, Latches position counter on rising edge of the index signal
    EQep1Regs.QEPCTL.bit.QPEN = 0;      // Reset the eQEP peripheral internal operating flags/read-only registers.
    //EQep1Regs.QEPCTL.bit.QCLM = 0; // QEP capture latch mode: Latch on position counter read by CPU
    EQep1Regs.QEPCTL.bit.QCLM = 1;      // Latch on unit time out

    EQep1Regs.QEPCTL.bit.UTE = 1; // QEP unit timer enable: Enable unit timer
    EQep1Regs.QEPCTL.bit.WDE = 1; // Enable the eQEP watchdog timer

    EQep1Regs.QPOSINIT = 0;    // Initial QPOSCNT , QPOSCNT set to zero on index event (or strobe or software if desired)
    EQep1Regs.QPOSMAX = ENCODERMAXTICKCOUNT; // Max value of QPOSCNT    /*better check this value*/

    // Quadrature edge-capture unit for low-speed measurement (QCAP)
    EQep1Regs.QCAPCTL.all = 0x00;
    EQep1Regs.QCAPCTL.bit.CEN = 1;  // eQEP capture unit is enabled
    EQep1Regs.QCAPCTL.bit.CCPS = 6; // eQEP capture timer clock prescaler: // CAPCLK = SYSCLKOUT/1
    EQep1Regs.QCAPCTL.bit.UPPS = 5; // Unit position event prescaler: UPEVNT = QCLK/1 , QCLK is triggered in every rising or falling edge of A or B
    // UPPS reiz veri important

    // Position Compare Control
    EQep1Regs.QPOSCTL.all = 0x0000; // Position Compare Control: Disabled

    EQep1Regs.QEINT.all = 0x00;

    EQep1Regs.QFLG.all = 0; // Interrupts are flagged here
    EQep1Regs.QCLR.all = 0; // QEP Interrupt Clear

    EQep1Regs.QCTMR = 0;    // This register provides time base for edge capture unit. 16 bit
    EQep1Regs.QCPRD = 0;    // This register holds the period count value between the last successive eQEP position events
    EQep1Regs.QCTMRLAT = 0; // QCTMR latch register, latching can be stopped by clearing QEPCTL[QCLM] register
    EQep1Regs.QCPRDLAT = 0; // QCPRD latch register, latching can be stopped by clearing QEPCTL[QCLM] register

    EQep1Regs.QEPCTL.bit.QPEN = 1; // eQEP position counter is enabled

}

