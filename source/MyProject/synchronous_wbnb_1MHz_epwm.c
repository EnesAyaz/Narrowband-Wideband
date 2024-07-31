#include <F2837xD_Device.h>
#include <math.h>
#include <F2837xD_Examples.h>

//#define MATH_TYPE 1
//#include <IQmathLib.h>
//
//_iq iq_wt_1 = 0x00 ;
//_iq iq_wt_2 = 0x00 ;
//_iq randomnumber = 0x00 ;
/*Constant definitions*/
#define PI 					          3.141592654
#define SYSTEMCLOCKFREQUENCY 	200000000	  // 200 MHz
#define SYSTEMCLOCKPERIOD 		5			  // ns
#define MOTORFREQUENCY          50            // Hz			/*need to define this as constant value rather than macro, for control loop*/


#define SWITCHINGFREQUENCY      1020000       // 1 MHz


float operation_frequency = 1020000;  // 2 MHz

#define LOOPFREQUENCY    1020000 // 2 MHz
#define DEADTIME 			        30	 // ns
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
void EPWM3Setup(); // Phase-C

/* ADC Setup */
void ConfigureADC(void);
void ConfigureEPWM(void);
void SetupADCEpwm(void);

__interrupt void adca1_isr(void);
Uint16 AdcaResults;


/*Interrupt functions*/
//__interrupt void cpu_timer0_isr(void);
__interrupt void epwm3_isr(void);



/*Variables: Open Loop Control*/
float fModulationIndexModule1;
float fModulationIndexModule2;
float fModulationIndexModule4;
float fModulationIndexDefault=0;
float duty_cycle=0.5;
float phase_ref = 0;

uint32_t Epwm3Counter=0;


/*Variables: Pulse Width Modulation*/
int iSinusoidalPWMCounter = 0;
float fEpwm1DutyCycle = 0.5;
float fEpwm2DutyCycle = 0.5;
float fEpwm4DutyCycle = 0.0;

float VFMODE =0;
float ma_max= 0.8;
float f_max= 20.0;
float fundamental_freq=87500;
int sayi=1;
float phase = 180;

int duration = 0;
unsigned long t0 = 0;
int flag = 2;

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
//    PieVectTable.TIMER0_INT = &cpu_timer0_isr;
    PieVectTable.EPWM3_INT = &epwm3_isr; //function for EPWM3 interrupt 3
    EDIS;

    InitCpuTimers();
//    ConfigCpuTimer(&CpuTimer0, 200, (1000000/(float)(LOOPFREQUENCY)) ); // 2 MHz (1000000/LOOPFREQUENCY)

//    EALLOW;
//    PieVectTable.ADCA1_INT = &adca1_isr;  /*specify the interrupt service routine (ISR) address to the PIE table*/
//    EDIS;




    EPWM1Setup();
    EPWM2Setup();
    EPWM3Setup();

    //    EPWM4Setup();

//    EPwm4Regs.CMPA.bit.CMPA = EPwm4Regs.TBPRD * fEpwm4DutyCycle; // (ePWM2)


//    ConfigureADC();
//    ConfigureEPWM();
//    SetupADCEpwm();
    CpuSysRegs.PCLKCR2.bit.EPWM3=1;  //Enable PWM CLock
    //CpuTimer0Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0

    IER |= M_INT1;   // ADC-A1 & CPU1.TIMER0
    IER |= M_INT3;   //EPWM3
//    PieCtrlRegs.PIEIER1.bit.INTx7 = 1; // TIMER0

    EINT;
    ERTM;

    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;  // Enable the PIE block
    PieCtrlRegs.PIEIER3.bit.INTx3 = 1; // EPWM3 - Datasheet Page 95

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1; /*Enable all the clocks.*/
    CpuSysRegs.PCLKCR0.bit.GTBCLKSYNC = 1;  /*start the global TimeBase clock */
    EDIS;



    while (1)
    {

//    	if (VFMODE==0)
//    	    {
//    	    	fEpwm1DutyCycle=0.5;
//    	    	fEpwm2DutyCycle=0.5;
//    	    	fEpwm4DutyCycle=0.5;
//    	    	fModulationIndexDefault=0;
//    	    }
//    	    else if (VFMODE==1)
//    	    {
//
//    	    	if (fModulationIndexDefault<ma_max && fundamental_freq< f_max )
//    	    	{
//    	    			fModulationIndexDefault=fModulationIndexDefault+0.04;
//    	    			DELAY_US(1000000);
//    	    		    fundamental_freq=fundamental_freq+1;
//
//    	    	}
//    }
//    	    else if (VFMODE==2)
//    	    {
//    	    	if (fModulationIndexDefault>0 && fundamental_freq>1 )
//    	    	    	    	{
//    	    	    	    			fModulationIndexDefault=fModulationIndexDefault-0.04;
//    	    	    	    			DELAY_US(500000);
//    	    	    	    			fundamental_freq=fundamental_freq-1;
//    	    	    	    	}
//
//    	    }
//
//    	    else if (VFMODE==3)
//    	       	    {
//    	       	    	if (fModulationIndexDefault<0  )
//    	       	    	    	    	{
//    	       	    	    	    			fModulationIndexDefault=0;
//
//    	       	    	    	    	}
//    	       	    	else if (fModulationIndexDefault>1 )
//	    	    	    	{
//	    	    	    			fModulationIndexDefault=1;
//
//	    	    	    	}
//
//    	       	 	if (fundamental_freq<1  )
//    	       	     	       	    {
//    	       	 	fundamental_freq=1;
//    	       	     	       	   }
//    	       	    else if (fundamental_freq>2000 )
//    	       	 	    	    	    	{
//    	       	    	fundamental_freq=2000;
//    	       	 	    	    	    	}
//
//    	       	    }
//
//
//    	    else if (VFMODE==5)
//    	    	    {
//
//    	    	    	if (fModulationIndexDefault<1 )
//    	    	    	{
//    	    	    			fModulationIndexDefault=fModulationIndexDefault+0.1;
//    	    	    			DELAY_US(20000);
//    	    	    		    fundamental_freq=400;
//
//    	    	    	}
//    	    }

		//if (phase<0)
		//	{phase =0;}
		//else if (phase>60)
		//	{phase=60;}

		//if (phase == 120 && flag == 0){
		//	t0=CpuTimer0.InterruptCount;
		//	flag=1;
		//}
		//
		//if (flag==1 && CpuTimer0.InterruptCount > t0+4000){
		//	phase_ref=0;
		//}
		//if (phase_ref == 0.0){phase = 110;}


    	// BELOW USED IN NARROW WIDE
//			if (phase == 180 && flag == 0){
//			//	phase_ref = 0.05;
//				t0=CpuTimer0.InterruptCount;
//				flag=1;
//			}
//
//			if (flag==1 && CpuTimer0.InterruptCount > t0+4000){
//			//	phase = 60;
//				phase_ref=0.1667;
//			}
//			//if (phase_ref == 0.0){phase = 110;}

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
   // GpioDataRegs.GPDCLEAR.bit.GPIO125 = 1; // Set 1 (ENABLE)
    GpioCtrlRegs.GPDDIR.bit.GPIO125 = 1; // set it as output

    //GpioCtrlRegs.GPAPUD.bit.GPIO4 = 0; // enable pull up
    GpioDataRegs.GPACLEAR.bit.GPIO4 = 1; // Clear 0 (SOFTOFF)
   // GpioDataRegs.GPASET.bit.GPIO4 = 1; // Clear 0 (SOFTOFF)
    GpioCtrlRegs.GPADIR.bit.GPIO4 = 1; // set it as output

    EDIS;

}
__interrupt void epwm3_isr(void)
{
    Epwm3Counter++;

// DO NOT ENTER MOD INDEX > 1 , NO CHECK
//    if (fModulationIndexDefault<0 )
//		{fModulationIndexDefault=0;}
//    else if (fModulationIndexDefault>1 )
//          {fModulationIndexDefault=1;}
//    else
//          {fModulationIndexDefault=fModulationIndexDefault;}

// DO NOT NEED TO CHANGE FSW ACTIVELY
//    if (operation_frequency>1000 && operation_frequency<520000)
//            { 		EPwm1Regs.TBPRD = (SYSTEMCLOCKFREQUENCY) / (operation_frequency) /2;
//            		EPwm2Regs.TBPRD = (SYSTEMCLOCKFREQUENCY) / (operation_frequency) /2;
//            		EPwm4Regs.TBPRD = (SYSTEMCLOCKFREQUENCY) / (operation_frequency) /2;
//            	    EPwm2Regs.TBPHS.bit.TBPHS=  phase*EPwm2Regs.TBPRD/180; // EPwm2Regs.TBPRD = 180 derece
//             }

//#if CONTROLMODE == 0
//
//    iq_wt_1=_IQ(2 * PI * fundamental_freq* iSinusoidalPWMCounter/ LOOPFREQUENCY);
//
////    randomnumber=_IQsin(iq_wt_1);
//
////    iq_wt_2=_IQ(2 * PI * fundamental_freq* iSinusoidalPWMCounter/ LOOPFREQUENCY-2*PI*phase_ref);
////    fEpwm1DutyCycle=(fModulationIndexDefault*_IQtoF(_IQsin(iq_wt_1))+1)/2;
////    fEpwm2DutyCycle=(fModulationIndexDefault*_IQtoF(_IQsin(iq_wt_2))+1)/2;
////    fEpwm4DutyCycle = 0;
//    iSinusoidalPWMCounter += 1;
//	if (iSinusoidalPWMCounter > ((LOOPFREQUENCY / fundamental_freq) - 1))
//		iSinusoidalPWMCounter = 0;
//#endif

//if (phase_ref>0.3 && flag == 0){
//t0=CpuTimer0.InterruptCount;
//flag=1;
//}
//
//if (flag==1 && CpuTimer0.InterruptCount > t0+600){
//	phase=60;
//}


//	if (phase < 100 && flag == 0){
//		phase = 120;
//		t0=CpuTimer0.InterruptCount;
//		flag=1;
//	}

//	if (phase == 120 && flag == 0){
//		t0=CpuTimer0.InterruptCount;
//		flag=1;
//	}

//	if (flag==1 && CpuTimer0.InterruptCount > t0+10000){
//		phase_ref=0;
//	}




//#if CONTROLMODE == 1

	iSinusoidalPWMCounter ++;
	if (iSinusoidalPWMCounter == (float)(LOOPFREQUENCY/fundamental_freq)){ //((LOOPFREQUENCY / fundamental_freq) )) 50
	iSinusoidalPWMCounter = 0;}

//    	        fModulationIndexModule1 = fModulationIndexDefault;
//    	        fModulationIndexModule2 = fModulationIndexDefault;
//    fModulationIndexModule4 = fModulationIndexDefault/LOOPFREQUENCY;
    fEpwm1DutyCycle = (fModulationIndexDefault*__sinpuf32(iSinusoidalPWMCounter*fundamental_freq/LOOPFREQUENCY)+1)/2; // fundamental_freq*iSinusoidalPWMCounter/ LOOPFREQUENCY
    fEpwm2DutyCycle = (fModulationIndexDefault*__sinpuf32(iSinusoidalPWMCounter*fundamental_freq/LOOPFREQUENCY-phase_ref)+1)/2;//(__sinpuf32(iSinusoidalPWMCounter*0.05 - phase_ref ) + 1) / 2;

//    	        fEpwm4DutyCycle = (fModulationIndexModule4
//    	                        * __sinpuf32(fundamental_freq * iSinusoidalPWMCounter
//    	                                / LOOPFREQUENCY +2*PI/3 ) + 1) / 2;
//    	        fEpwm4DutyCycle = 0.5;


//#endif

    EPwm1Regs.CMPA.bit.CMPA = EPwm1Regs.TBPRD * fEpwm1DutyCycle; // (ePWM1)
    EPwm2Regs.CMPA.bit.CMPA = EPwm2Regs.TBPRD * fEpwm2DutyCycle; // (ePWM2)
    EPwm2Regs.TBPHS.bit.TBPHS=  phase*EPwm2Regs.TBPRD/180; // EPwm2Regs.TBPRD = 180 derece

//        EPwm4Regs.CMPA.bit.CMPA = EPwm4Regs.TBPRD * fEpwm4DutyCycle; // (ePWM2)
    EPwm3Regs.ETCLR.bit.INT = 1;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP3;
}

void EPWM1Setup(void)
{
EALLOW;
    EPwm1Regs.TBCTL.all = 0x00;
    EPwm1Regs.TBCTL.bit.CTRMODE = 2;   // Count up and douwn
    EPwm1Regs.TBCTL.bit.CLKDIV = 0;    // TBCLOK = EPWMCLOCK/(128*10) = 78125Hz
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = 0;

    EPwm1Regs.TBPRD = SYSTEMCLOCKFREQUENCY / (SWITCHINGFREQUENCY) / 2; // PWM Block has half clock freq of system & up-down mode requires division by 2
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
	    EPwm2Regs.TBPHS.bit.TBPHS=  phase*EPwm2Regs.TBPRD/180; // EPwm2Regs.TBPRD = 180 derece
	    EPwm2Regs.TBCTL.bit.SWFSYNC = 1;

	    EDIS;

}

void EPWM3Setup(void)
   {EALLOW;
    EPwm3Regs.TBCTL.all = 0x00;
    EPwm3Regs.TBCTL.bit.CTRMODE = 2;   // Count up and douwn
    EPwm3Regs.TBCTL.bit.CLKDIV = 0;    // TBCLOK = EPWMCLOCK/(128*10) = 78125Hz
    EPwm3Regs.TBCTL.bit.HSPCLKDIV = 0;

    EPwm3Regs.TBPRD = SYSTEMCLOCKFREQUENCY / (SWITCHINGFREQUENCY * 2); // PWM Block has half clock freq of system & up-down mode requires division by 2 (sysclk_frequency/2)/(1000)/2;
    EPwm3Regs.TBCTR = 0x0000;          // Clear counter

    EPwm3Regs.CMPCTL.all = 0x00;
    EPwm3Regs.CMPCTL.bit.SHDWAMODE = CC_SHADOW;         //only active registers are used
    EPwm3Regs.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO;

    EPwm3Regs.AQCTLA.all = 0x00;
    EPwm3Regs.AQCTLA.bit.CAU = 2; //set high
    EPwm3Regs.AQCTLA.bit.CAD = 1; //set low

    EPwm3Regs.CMPA.bit.CMPA = EPwm3Regs.TBPRD /2;    // Set compare A value
    EPwm3Regs.TBPHS.bit.TBPHS = 0x0000;          // Phase is 0

    EPwm3Regs.TBCTL2.all = 0x00;
    EPwm3Regs.CMPCTL2.all = 0x00;

    EPwm3Regs.DBCTL.all = 0x00;
    EPwm3Regs.DBCTL.bit.OUT_MODE = 3;
    EPwm3Regs.DBCTL.bit.POLSEL = 2;
    EPwm3Regs.DBFED.bit.DBFED = DEADTIME / SYSTEMCLOCKPERIOD;
    EPwm3Regs.DBRED.bit.DBRED = DEADTIME / SYSTEMCLOCKPERIOD;
    EPwm3Regs.DBCTL2.all = 0x00;

    EPwm3Regs.ETSEL.all = 0x00;

    EPwm3Regs.TBCTL.bit.PHSEN = 0;
    EPwm3Regs.TBCTL.bit.SYNCOSEL = 3; //SYNCOUT

    EPwm3Regs.ETSEL.bit.INTSEL = 1; // When TBCTR == 0
    EPwm3Regs.ETSEL.bit.INTEN = 1;  // Enable INT
    EPwm3Regs.ETPS.all = 0x00;
    EPwm3Regs.ETPS.bit.INTPRD = 1;  // Generate INT on first event
    EDIS;
    }

void ConfigureEPWM(void)
{
	EPwm3Regs.TBCTL.bit.CLKDIV = TB_DIV1;
	EPwm3Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EALLOW;
    EPwm3Regs.ETSEL.bit.SOCAEN    = 0;    // Disable SOC on A group
    EPwm3Regs.ETSEL.bit.SOCASEL    = 4;   // Select SOC on up-count
    EPwm3Regs.ETPS.bit.SOCAPRD = 1;       // Generate pulse on 1st event
    EPwm3Regs.CMPA.bit.CMPA = 0x0800;     // Set compare A value to 2048 counts
    EPwm3Regs.TBPRD = 0x1000;             // Set period to 4096 counts
    EPwm3Regs.TBCTL.bit.CTRMODE = 3;      // freeze counter
    EDIS;
}

void SetupADCEpwm(void)
{
	EALLOW;
	AdcaRegs.ADCSOC0CTL.bit.CHSEL = 1; //SOC0 will convert ADCINA1
	AdcaRegs.ADCSOC0CTL.bit.ACQPS = 25; //SOC0 will use sample duration of 20 SYSCLK cycles
	// AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 9; //SOC0 will begin conversion on ePWM3 SOCB
	AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 1; //SOC2 will begin conversion on TIMER0

	AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 0; //end of SOC0 will set INT1 flag
	AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;   //enable INT1 flag
	AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //make sure INT1 flag is cleared
	EDIS;

}
void ConfigureADC(void)
{
    EALLOW;
    AdcaRegs.ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
    AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    DELAY_US(1000);
    EDIS;
}


__interrupt void adca1_isr(void)
{
	AdcaResults=AdcaResultRegs.ADCRESULT0;
	AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}

