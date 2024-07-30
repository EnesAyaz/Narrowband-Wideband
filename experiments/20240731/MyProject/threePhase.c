#include <F2837xD_Device.h>
#include <math.h>
#include <F2837xD_Examples.h>



/*Constant definitions*/
#define UNIPOLAR 0
#define BIPOLAR 1
#define PI 					          3.141592654
#define SYSTEMCLOCKFREQUENCY 	200000000	  // Hz
#define SYSTEMCLOCKPERIOD 		5				    // ns
#define MOTORFREQUENCY        50         // Hz			/*need to define this as constant value rather than macro, for control loop*/

#if UNIPOLAR
#define SWITCHINGFREQUENCY    20000         // Hz
#endif

#if BIPOLAR  // also change PWM's
#define SWITCHINGFREQUENCY    40000         // Hz
#endif

#if 0
#define SWITCHINGFREQUENCY    40000         // Hz
#endif


#define LOOPFREQUENCY    100000         // Hz
#define DEADTIME 			        100	 			// ns
#define OPENLOOPCONTROLMODE 	1
#define CONTROLMODE           OPENLOOPCONTROLMODE //OPENLOOPCONTROLMODE /*Use this to define the control mode each time*/

/*External functions*/
extern void InitSysCtrl(void);
extern void InitPieCtrl(void);
extern void InitPieVectTable(void);
extern void InitCpuTimers(void);
extern void ConfigCpuTimer(struct CPUTIMER_VARS *, float, float);
extern void InitAdc(void);
extern void InitPeripheralClocks(void);
extern int16 GetTemperatureC(int16);
extern void InitTempSensor(float32);

/*Setup functions*/
void GPIOSetup();
void EPWM1Setup(); // Phase-A
void EPWM2Setup(); // Phase-B
void EPWM4Setup(); // Phase-C


/*Interrupt functions*/
__interrupt void cpu_timer0_isr(void);


/*Variables: Open Loop Control*/
float fModulationIndexModule1;
float fModulationIndexModule2;
float fModulationIndexModule4;
float fModulationIndexDefault=0.9;




/*Variables: Pulse Width Modulation*/
int iSinusoidalPWMCounter = 0;
float fEpwm1DutyCycle = 0.5;
float fEpwm2DutyCycle = 0.5;
float fEpwm4DutyCycle=0.5;

int main(void)
{

    InitSysCtrl();
    InitPeripheralClocks();
    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV = 0; // EPWM Clock Divide Select: /1 of PLLSYSCLK
    EDIS;

    GPIOSetup();
    DINT;

    InitPieCtrl();
    IER = 0x0000;
    IFR = 0x0000;
    InitPieVectTable();

    EALLOW;
    PieVectTable.TIMER0_INT = &cpu_timer0_isr;
    EDIS;

    InitCpuTimers();
    ConfigCpuTimer(&CpuTimer0, 200, (1000000/LOOPFREQUENCY)); // 100kHz

    EPWM1Setup();
    EPWM2Setup();
    EPWM4Setup();


    CpuTimer0Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0

    IER |= M_INT1;   // ADC-A1 & CPU1.TIMER0
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1; // TIMER0

    EINT;
    ERTM;

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1; /*Enable all the clocks.*/
    EDIS;

    while (1)
    {


    }
}

void GPIOSetup()
{

    EALLOW;

    // HB C PWM (ePWM4)
     GpioCtrlRegs.GPAPUD.bit.GPIO6 = 1;    // Disable pull-up on GPIO2 (EPWM2A)
     GpioCtrlRegs.GPAPUD.bit.GPIO7 = 1;    // Disable pull-up on GPIO3 (EPWM2B)
     GpioCtrlRegs.GPAGMUX1.bit.GPIO6 = 0;  // Configure GPIO2 as EPWM2A
     GpioCtrlRegs.GPAGMUX1.bit.GPIO7 = 0;  // Configure GPIO3 as EPWM2B
     GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 1;   // Configure GPIO2 as EPWM2A
     GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 1;   // Configure GPIO3 as EPWM2B


    // HB B PWM (ePWM2)
    GpioCtrlRegs.GPAPUD.bit.GPIO2 = 1;    // Disable pull-up on GPIO2 (EPWM2A)
    GpioCtrlRegs.GPAPUD.bit.GPIO3 = 1;    // Disable pull-up on GPIO3 (EPWM2B)
    GpioCtrlRegs.GPAGMUX1.bit.GPIO2 = 0;  // Configure GPIO2 as EPWM2A
    GpioCtrlRegs.GPAGMUX1.bit.GPIO3 = 0;  // Configure GPIO3 as EPWM2B
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1;   // Configure GPIO2 as EPWM2A
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 1;   // Configure GPIO3 as EPWM2B


    // HB A PWM PWM (ePWM1)
    GpioCtrlRegs.GPAPUD.bit.GPIO0 = 1;    // Disable pull-up on GPIO0 (EPWM1A)
    GpioCtrlRegs.GPAPUD.bit.GPIO1 = 1;    // Disable pull-up on GPIO1 (EPWM1B)
    GpioCtrlRegs.GPAGMUX1.bit.GPIO0 = 0;  // Configure GPIO0 as EPWM1A
    GpioCtrlRegs.GPAGMUX1.bit.GPIO1 = 0;  // Configure GPIO1 as EPWM1B
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;   // Configure GPIO0 as EPWM1A
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 1;   // Configure GPIO1 as EPWM1B


    GpioCtrlRegs.GPDPUD.bit.GPIO125 = 0; // enable pull up
    GpioDataRegs.GPDSET.bit.GPIO125 = 1; // Set 1 (ENABLE)
    GpioCtrlRegs.GPDDIR.bit.GPIO125 = 1; // set it as output

    GpioCtrlRegs.GPAPUD.bit.GPIO4 = 0; // enable pull up
    GpioDataRegs.GPACLEAR.bit.GPIO4 = 1; // Clear 0 (SOFTOFF)
    GpioCtrlRegs.GPADIR.bit.GPIO4 = 1; // set it as output

    EDIS;

}
__interrupt void cpu_timer0_isr(void)
{

    CpuTimer0.InterruptCount++;

#if CONTROLMODE == 0
        /* Open loop Control - Sinusoidal PWM*/
        fModulationIndexModule1 = fModulationIndexDefault;
        fModulationIndexModule2 = fModulationIndexDefault;
        fModulationIndexModule4 =fModulationIndexDefault;
        fEpwm1DutyCycle = (fModulationIndexModule1
                * sin(2 * PI * MOTORFREQUENCY
                        * iSinusoidalPWMCounter/ LOOPFREQUENCY) + 1) / 2;
        fEpwm2DutyCycle = (fModulationIndexModule2
                * sin(2 * PI * MOTORFREQUENCY * iSinusoidalPWMCounter
                        / LOOPFREQUENCY -PI ) + 1) / 2;

        fEpwm4DutyCycle = (fModulationIndexModule4
                        * sin(2 * PI * MOTORFREQUENCY * iSinusoidalPWMCounter
                                / LOOPFREQUENCY -PI ) + 1) / 2;
        iSinusoidalPWMCounter += 1;
        if (iSinusoidalPWMCounter > ((LOOPFREQUENCY / MOTORFREQUENCY) - 1))
        iSinusoidalPWMCounter = 0;
#endif

#if UNIPOLAR
        EPwm1Regs.CMPA.bit.CMPA = EPwm1Regs.TBPRD * fEpwm1DutyCycle; // (ePWM1)
        EPwm2Regs.CMPA.bit.CMPA = EPwm2Regs.TBPRD * fEpwm2DutyCycle; // (ePWM2)
#endif

#if 0
        EPwm1Regs.CMPA.bit.CMPA = EPwm1Regs.TBPRD * fEpwm1DutyCycle; // (ePWM1)
        EPwm2Regs.CMPA.bit.CMPA = EPwm2Regs.TBPRD * fEpwm2DutyCycle; // (ePWM2)
#endif

#if BIPOLAR
        EPwm1Regs.CMPA.bit.CMPA = EPwm1Regs.TBPRD * 0.5; // (ePWM1)
        EPwm2Regs.CMPA.bit.CMPA = EPwm2Regs.TBPRD * 0.5; // (ePWM2)
        EPwm4Regs.CMPA.bit.CMPA = EPwm4Regs.TBPRD * 0.5; // (ePWM2)
#endif


    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}




void EPWM1Setup(void)
{
/*	EALLOW;
    EPwm1Regs.TBCTL.all = 0x00;
    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;   // Count up and douwn
    EPwm1Regs.TBCTL.bit.CLKDIV = 0;
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = 0;
    EPwm1Regs.TBPRD = SYSTEMCLOCKFREQUENCY / (SWITCHINGFREQUENCY * 2);
    EPwm1Regs.TBCTR = 0x0000;          // Clear counter
    EPwm1Regs.CMPCTL.all = 0x00;
    EPwm1Regs.CMPCTL.bit.SHDWAMODE = 0; // Shadow mode. Operates as a double buffer. All writes via the CPU access the shadow register
    EPwm1Regs.CMPCTL.bit.LOADAMODE = 1; // Load on CTR = PRD: Time-base counter equal to period
    EPwm1Regs.AQCTLA.all = 0x00;
    EPwm1Regs.AQCTLA.bit.CAU = 1; //set low
    EPwm1Regs.AQCTLA.bit.CAD = 2; //set high
    EPwm1Regs.CMPA.bit.CMPA = EPwm1Regs.TBPRD / 2;    // Set compare A value


    EPwm1Regs.TBCTL2.all = 0x00;
    EPwm1Regs.CMPCTL2.all = 0x00;
    EPwm1Regs.DBCTL.all = 0x00;
    EPwm1Regs.DBCTL.bit.OUT_MODE = 3;
    EPwm1Regs.DBCTL.bit.POLSEL = 2;
    EPwm1Regs.DBFED.bit.DBFED = DEADTIME / SYSTEMCLOCKPERIOD;
    EPwm1Regs.DBRED.bit.DBRED = DEADTIME / SYSTEMCLOCKPERIOD;
    EPwm1Regs.DBCTL2.all = 0x00;
    EPwm1Regs.ETSEL.all = 0x00;

   EPwm1Regs.TBCTL.bit.PHSEN = 0;
   EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO; //SYNCOUT
  // EPwm1Regs.TBPHS.bit.TBPHS = 0x0000;          // Phase is 0
EDIS;
*/
EALLOW;
EPwm1Regs.TBCTL.all = 0x00;
    EPwm1Regs.TBCTL.bit.CTRMODE = 2;   // Count up and douwn
    EPwm1Regs.TBCTL.bit.CLKDIV = 0;    // TBCLOK = EPWMCLOCK/(128*10) = 78125Hz
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = 0;

    EPwm1Regs.TBPRD = SYSTEMCLOCKFREQUENCY / (SWITCHINGFREQUENCY * 2); // PWM Block has half clock freq of system & up-down mode requires division by 2
    EPwm1Regs.TBCTR = 0x0000;          // Clear counter

    EPwm1Regs.CMPCTL.all = 0x00;
    EPwm1Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;         //only active registers are used
    EPwm1Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;
    //EPwm1Regs.CMPCTL.bit.SHDWBMODE = 1;//only active registers are used

    EPwm1Regs.AQCTLA.all = 0x00;
    EPwm1Regs.AQCTLA.bit.CAU = 1; //set high
    EPwm1Regs.AQCTLA.bit.CAD = 2; //set low


    EPwm1Regs.TBPHS.bit.TBPHS = 0x0000;          // Phase is 0

    EPwm1Regs.TBCTL2.all = 0x00;
    EPwm1Regs.CMPCTL2.all = 0x00;
    EPwm1Regs.DBCTL.all = 0x00;
    EPwm1Regs.DBCTL.bit.OUT_MODE = 3;
    EPwm1Regs.DBCTL.bit.POLSEL = 2;
    EPwm1Regs.DBFED.bit.DBFED = DEADTIME / SYSTEMCLOCKPERIOD;
    EPwm1Regs.DBRED.bit.DBRED = DEADTIME / SYSTEMCLOCKPERIOD;
    EPwm1Regs.DBCTL2.all = 0x00;
    //EPwm1Regs.TBCTL.bit.SYNCOSEL = 3;
    EPwm1Regs.ETSEL.all = 0x00;

    EPwm1Regs.TBCTL.bit.PHSEN = 0;
    EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO; //SYNCOUT
    EPwm2Regs.TBCTL.bit.PHSDIR = 0;

    EDIS;


}

void EPWM2Setup(void)
{
/*EALLOW;
    EPwm2Regs.TBCTL.all = 0x00;
    EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;   // Count up and douwn
    EPwm2Regs.TBCTL.bit.CLKDIV = 0;    // TBCLOK = EPWMCLOCK/(128*10) = 78125Hz
    EPwm2Regs.TBCTL.bit.HSPCLKDIV = 0;
    EPwm2Regs.TBPRD = SYSTEMCLOCKFREQUENCY / (SWITCHINGFREQUENCY * 2);
    EPwm2Regs.TBCTR = 0x0000;          // Clear counter
    EPwm2Regs.CMPCTL.all = 0x00;
    EPwm2Regs.CMPCTL.bit.SHDWAMODE = 0; // Shadow mode. Operates as a double buffer. All writes via the CPU access the shadow register
    EPwm2Regs.CMPCTL.bit.LOADAMODE = 1; // Load on CTR = PRD: Time-base counter equal to period
    EPwm2Regs.AQCTLA.all = 0x00;
    EPwm2Regs.AQCTLA.bit.CAU = 2; //set low
    EPwm2Regs.AQCTLA.bit.CAD = 1; //set high
    EPwm2Regs.CMPA.bit.CMPA = EPwm2Regs.TBPRD / 2;    // Set compare A value
#if 0
    EPwm2Regs.TBCTL.bit.PHSEN = 1;
    EPwm2Regs.TBPHS.bit.TBPHS = EPwm2Regs.TBPRD;          // Phase is 180
#endif
#if UNIPOLAR
    EPwm2Regs.TBCTL.bit.PHSEN = 1;
    EPwm2Regs.TBPHS.bit.TBPHS = 0x0000;          // Phase is 0
#endif

#if BIPOLAR
      EPwm2Regs.TBCTL.bit.PHSEN = 1;
      EPwm2Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN; //SYNCIN
      EPwm2Regs.TBCTL.bit.PHSDIR = 0;
      EPwm2Regs.TBPHS.bit.TBPHS= 450;
      EPwm2Regs.TBCTL.bit.SWFSYNC = 1;
#endif

    EPwm2Regs.TBCTL2.all = 0x00;
    EPwm2Regs.CMPCTL2.all = 0x00;
    EPwm2Regs.DBCTL.all = 0x00;
    EPwm2Regs.DBCTL.bit.OUT_MODE = 3;
    EPwm2Regs.DBCTL.bit.POLSEL = 2;
    EPwm2Regs.DBFED.bit.DBFED = DEADTIME / SYSTEMCLOCKPERIOD;
    EPwm2Regs.DBRED.bit.DBRED = DEADTIME / SYSTEMCLOCKPERIOD;
    EPwm2Regs.DBCTL2.all = 0x00;

EDIS; */

	EALLOW;
	    EPwm2Regs.TBCTL.all = 0x00;
	    EPwm2Regs.TBCTL.bit.CTRMODE = 2;   // Count up and douwn
	    EPwm2Regs.TBCTL.bit.CLKDIV = 0;    // TBCLOK = EPWMCLOCK/(128*10) = 78125Hz
	    EPwm2Regs.TBCTL.bit.HSPCLKDIV = 0;


	    EPwm2Regs.TBPRD = SYSTEMCLOCKFREQUENCY / (SWITCHINGFREQUENCY * 2); // PWM Block has half clock freq of system & up-down mode requires division by 2
	    EPwm2Regs.TBCTR = 0x0000;          // Clear counter

	    EPwm2Regs.CMPCTL.all = 0x00;
	    EPwm2Regs.CMPCTL.bit.SHDWAMODE = 1;         //only active registers are used
	    EPwm2Regs.CMPCTL.bit.LOADAMODE = 1;
	    //EPwm2Regs.CMPCTL.bit.SHDWBMODE = 1;//only active registers are used

	    EPwm2Regs.AQCTLA.all = 0x00;

	    EPwm2Regs.AQCTLA.bit.CAU = 2; //set low
	    EPwm2Regs.AQCTLA.bit.CAD = 1; //set high


	    EPwm2Regs.TBCTL2.all = 0x00;
	    EPwm2Regs.CMPCTL2.all = 0x00;
	    EPwm2Regs.DBCTL.all = 0x00;
	    EPwm2Regs.DBCTL.bit.OUT_MODE = 3;
	    EPwm2Regs.DBCTL.bit.POLSEL = 2;
	    EPwm2Regs.DBFED.bit.DBFED = DEADTIME / SYSTEMCLOCKPERIOD;
	    EPwm2Regs.DBRED.bit.DBRED =DEADTIME / SYSTEMCLOCKPERIOD;
	    EPwm2Regs.DBCTL2.all = 0x00;

	    EPwm2Regs.TBCTL.bit.PHSEN = 1;
	    EPwm2Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN; //SYNCIN
	    EPwm2Regs.TBCTL.bit.PHSDIR = 0;
	    EPwm2Regs.TBPHS.bit.TBPHS= 0;
	    EPwm2Regs.TBCTL.bit.SWFSYNC = 1;

	    EDIS;

}


void EPWM4Setup(void)
{
	EALLOW;
		    EPwm4Regs.TBCTL.all = 0x00;
		    EPwm4Regs.TBCTL.bit.CTRMODE = 2;   // Count up and douwn
		    EPwm4Regs.TBCTL.bit.CLKDIV = 0;    // TBCLOK = EPWMCLOCK/(128*10) = 78125Hz
		    EPwm4Regs.TBCTL.bit.HSPCLKDIV = 0;


		    EPwm4Regs.TBPRD = SYSTEMCLOCKFREQUENCY / (SWITCHINGFREQUENCY * 2); // PWM Block has half clock freq of system & up-down mode requires division by 2
		    EPwm4Regs.TBCTR = 0x0000;          // Clear counter

		    EPwm4Regs.CMPCTL.all = 0x00;
		    EPwm4Regs.CMPCTL.bit.SHDWAMODE = 1;         //only active registers are used
		    EPwm4Regs.CMPCTL.bit.LOADAMODE = 1;
		    //EPwm2Regs.CMPCTL.bit.SHDWBMODE = 1;//only active registers are used

		    EPwm4Regs.AQCTLA.all = 0x00;

		    EPwm4Regs.AQCTLA.bit.CAU = 1; //set low
		    EPwm4Regs.AQCTLA.bit.CAD = 2; //set high

		    EPwm4Regs.TBCTL2.all = 0x00;
		    EPwm4Regs.CMPCTL2.all = 0x00;
		    EPwm4Regs.DBCTL.all = 0x00;
		    EPwm4Regs.DBCTL.bit.OUT_MODE = 3;
		    EPwm4Regs.DBCTL.bit.POLSEL = 2;
		    EPwm4Regs.DBFED.bit.DBFED = DEADTIME / SYSTEMCLOCKPERIOD;
		    EPwm4Regs.DBRED.bit.DBRED =DEADTIME / SYSTEMCLOCKPERIOD;
		    EPwm4Regs.DBCTL2.all = 0x00;

		    EPwm4Regs.TBCTL.bit.PHSEN = 1;
		    EPwm4Regs.TBCTL.bit.SYNCOSEL = TB_SYNC_IN; //SYNCIN
		    EPwm4Regs.TBCTL.bit.PHSDIR = 0;
		    EPwm4Regs.TBPHS.bit.TBPHS= 0;
		    EPwm4Regs.TBCTL.bit.SWFSYNC = 1;

		    EDIS;

}
