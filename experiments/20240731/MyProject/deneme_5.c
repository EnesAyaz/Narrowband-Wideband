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
#define SWITCHINGFREQUENCY    80000         // Hz
#define LOOPFREQUENCY    10000         // Hz
#define DEADTIME 			        100	 			// ns
#define OPENLOOPCONTROLMODE 	1
#define CONTROLMODE           OPENLOOPCONTROLMODE //OPENLOOPCONTROLMODE /*Use this to define the control mode each time*/
#define sample_window       65          // clock


float operation_frequeuency=80000;
unsigned long int EPwm1_isr_counter = 0;

/*Variables: Open Loop Control*/
float fModulationIndexModule1;
float fModulationIndexModule2;
float fModulationIndexModule4;
float fModulationIndexDefault=0;
float duty_cycle=0.5;



/*Variables: Pulse Width Modulation*/
int iSinusoidalPWMCounter = 0;
float fEpwm1DutyCycle = 0.5;
float fEpwm2DutyCycle = 0.5;
float fEpwm4DutyCycle=0.5;

float VFMODE =0;
float ma_max= 0.8;
float f_max= 20.0;
float fundamental_freq=1.0;
int sayi=1;
float phase =40;


float VoltsPerBit = 0.000050354;


__interrupt void cpu_timer0_isr(void);
__interrupt void epwm1_isr(void);

// Functions
void GpioSelect(void);
void InitEpwm1(); // PWM1 - Phase A
void InitEpwm2(); // PWM2 - Phase B
void InitEpwm4(); // PWM4 - Phase C
void InitEpwm8(); // ADC in

void ConfigureADC(void);
void SetupADCEpwm(void);
void EQEPSetup(void);

volatile float Curr1;
volatile float Curr2;
volatile float Curr4;

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

	 InitCpuTimers();
	 ConfigCpuTimer(&CpuTimer0, 200, (1000000/LOOPFREQUENCY)); // 100kHz

	 CpuTimer0Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0

	 EALLOW;
	 CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1; /*Enable all the clocks.*/
	 EDIS;


   EALLOW;

   PieVectTable.TIMER0_INT = &cpu_timer0_isr;
   PieVectTable.EPWM1_INT = &epwm1_isr;

   PieCtrlRegs.PIECTRL.bit.ENPIE = 1; // Enable the PIE block

   PieCtrlRegs.PIEIER1.bit.INTx7 = 1; //timer0
   PieCtrlRegs.PIEIER3.bit.INTx1 = 1; //epwm1

   IER |= M_INT1; /*Enable the PIE group of Cpu timer 0 interrupt*/
   IER |= M_INT10; // For ADC Int.
   IER |= M_INT3; //epwm1 isr

   EDIS;


   GpioSelect();
   InitEpwm1();
   InitEpwm2();
   InitEpwm4();


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
     CpuSysRegs.PCLKCR2.bit.EPWM8 = 1;

     CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;   /*enable epwm clock to initialize epwm modules*/
     CpuSysRegs.PCLKCR0.bit.GTBCLKSYNC =1;

     EDIS;

     EPwm8Regs.ETSEL.bit.SOCAEN = 1;  //enable SOCA
     EPwm8Regs.TBCTL.bit.CTRMODE = 0; //unfreeze, and enter up count mode



  while(1)
  {

  	if (VFMODE==0)
  	    {
  	    	fEpwm1DutyCycle=0.5;
  	    	fEpwm2DutyCycle=0.5;
  	    	fEpwm4DutyCycle=0.5;
  	    	fModulationIndexDefault=0;
  	    }
  	    else if (VFMODE==1)
  	    {

  	    	if (fModulationIndexDefault<ma_max && fundamental_freq< f_max )
  	    	{
  	    			fModulationIndexDefault=fModulationIndexDefault+0.04;
  	    			DELAY_US(1000000);
  	    		    fundamental_freq=fundamental_freq+1;

  	    	}
  }
  	    else if (VFMODE==2)
  	    {
  	    	if (fModulationIndexDefault>0 && fundamental_freq>1 )
  	    	    	    	{
  	    	    	    			fModulationIndexDefault=fModulationIndexDefault-0.04;
  	    	    	    			DELAY_US(500000);
  	    	    	    			fundamental_freq=fundamental_freq-1;
  	    	    	    	}

  	    }

  	    else if (VFMODE==3)
  	       	    {
  	       	    	if (fModulationIndexDefault<0  )
  	       	    	    	    	{
  	       	    	    	    			fModulationIndexDefault=0;

  	       	    	    	    	}
  	       	    	else if (fModulationIndexDefault>1 )
	    	    	    	{
	    	    	    			fModulationIndexDefault=1;

	    	    	    	}

  	       	 	if (fundamental_freq<1  )
  	       	     	       	    {
  	       	 	fundamental_freq=1;
  	       	     	       	   }
  	       	    else if (fundamental_freq>2000 )
  	       	 	    	    	    	{
  	       	    	fundamental_freq=2000;
  	       	 	    	    	    	}

  	       	    }
  	    else if (VFMODE==5)
  	    	    {
  	    	    	if (fModulationIndexDefault<1 )
  	    	    	{
  	    	    			fModulationIndexDefault=fModulationIndexDefault+0.1;
  	    	    			DELAY_US(20000);
  	    	    		    fundamental_freq=400;
  	    	    	}
  	    }


  	if (phase<0)
  	{phase =0;}
  	else if (phase>60)
  	{phase=60;}

}

  }


__interrupt void epwm1_isr(void)

{
    // Check whether isr operates
    EPwm1_isr_counter++;
    // Prevent computational problems (overflow etc)
    if ((EPwm1_isr_counter%1000000)==0)
    {  EPwm1_isr_counter = 0; }

    Curr1 = ((float)(AdcaResultRegs.ADCRESULT1 - 50500))*VoltsPerBit*90.0;
    Curr2 = ((float)(AdcaResultRegs.ADCRESULT2 - 50540))*VoltsPerBit*90.0;
    Curr4 = ((float)(AdcbResultRegs.ADCRESULT1 - 50540))*VoltsPerBit*89.0;


/*    if (fModulationIndexDefault<0 )
		{fModulationIndexDefault=0;}
    else if (fModulationIndexDefault>1 )
          {fModulationIndexDefault=1;}
    else
          {fModulationIndexDefault=fModulationIndexDefault;}


    if (operation_frequeuency>70000 && operation_frequeuency<90000)
            { 		EPwm1Regs.TBPRD = (SYSTEMCLOCKFREQUENCY) / (operation_frequeuency) /2;
            		EPwm2Regs.TBPRD = (SYSTEMCLOCKFREQUENCY) / (operation_frequeuency) /2;
            		EPwm4Regs.TBPRD = (SYSTEMCLOCKFREQUENCY) / (operation_frequeuency) /2;
            	    EPwm2Regs.TBPHS.bit.TBPHS=  phase*EPwm2Regs.TBPRD/180; // EPwm2Regs.TBPRD = 180 derece

             }

#if CONTROLMODE == 1

    	        fModulationIndexModule1 = fModulationIndexDefault;
    	        fModulationIndexModule2 = fModulationIndexDefault;
    	        fModulationIndexModule4 =fModulationIndexDefault;
    	        fEpwm1DutyCycle = (fModulationIndexModule1
    	                * sin(2 * PI * fundamental_freq
    	                        * iSinusoidalPWMCounter/ LOOPFREQUENCY) + 1) / 2;
    	        fEpwm2DutyCycle = (fModulationIndexModule2
    	                * sin(2 * PI * fundamental_freq * iSinusoidalPWMCounter
    	                        / LOOPFREQUENCY -2*PI/3 ) + 1) / 2;

    	        fEpwm4DutyCycle = (fModulationIndexModule4
    	                        * sin(2 * PI * fundamental_freq * iSinusoidalPWMCounter
    	                                / LOOPFREQUENCY +2*PI/3 ) + 1) / 2;
    	        iSinusoidalPWMCounter += 1;
    	        if (iSinusoidalPWMCounter > ((LOOPFREQUENCY / fundamental_freq) - 1))
    	        iSinusoidalPWMCounter = 0;

#endif


        EPwm1Regs.CMPA.bit.CMPA = EPwm1Regs.TBPRD * fEpwm1DutyCycle; // (ePWM1)
        EPwm2Regs.CMPA.bit.CMPA = EPwm2Regs.TBPRD * fEpwm2DutyCycle; // (ePWM2)
        EPwm4Regs.CMPA.bit.CMPA = EPwm4Regs.TBPRD * fEpwm4DutyCycle; // (ePWM2) */




}

__interrupt void cpu_timer0_isr(void)
{
    CpuTimer0.InterruptCount++;
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}



void GpioSelect(void)
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

	// Gate Driver Enable
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


void InitEpwm1(void)
{
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

void InitEpwm2(void)
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
	    EPwm2Regs.TBPHS.bit.TBPHS=  2*EPwm2Regs.TBPRD/9; // EPwm2Regs.TBPRD = 180 derece
	    EPwm2Regs.TBCTL.bit.SWFSYNC = 1;

	    EDIS;

}

void InitEpwm4(void)
{
	EALLOW;
		    EPwm4Regs.TBCTL.all = 0x00;
		    EPwm4Regs.TBCTL.bit.CTRMODE = 2;   // Count up and down
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
		    EPwm4Regs.TBCTL.bit.PHSDIR = 1;
		    EPwm4Regs.TBPHS.bit.TBPHS= 0;
		    EPwm4Regs.TBCTL.bit.SWFSYNC = 1;

		    EDIS;

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
