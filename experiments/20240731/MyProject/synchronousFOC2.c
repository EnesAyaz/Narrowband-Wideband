#include <F2837xD_Device.h>
#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <F2837xD_Examples.h>

/*Constant definitions*/
#define UNIPOLAR 0
#define BIPOLAR 1
#define PI 					          3.141592654
#define SYSTEMCLOCKFREQUENCY 	200000000	  // Hz
#define SYSTEMCLOCKPERIOD 		5				    // ns
#define MOTORFREQUENCY        50         // Hz			/*need to define this as constant value rather than macro, for control loop*/
#define ENCODERMAXTICKCOUNT 4095
#define RESULTS_BUFFER_SIZE 50

#define SWITCHINGFREQUENCY    40000       // Hz

#define sample_window       65          // clock
#define adc_frequency       50000000    //ns
#define adc_period          200         //ns

float operation_frequeuency=50000;
float RotorAngleMechanical=0;
float RotorAngleElectrical=0;
float POLEPAIRS=2;
float DaxisAngle=0;

float Curr1;
float Curr2;
float Curr3;
float Curr4;
float VoltsPerBit = 0.0000503534;
float maxCurr =10;
float IF_Ref=0.44;
float P_IF=0;
float I_IF=0;
float IF_ERROR_SUM=0;
float IF_ERROR=0;

#define LOOPFREQUENCY    30000         // Hz
#define DEADTIME 			        100	 			// ns
#define OPENLOOPCONTROLMODE 	1
float CONTROLMODE = 0; //OPENLOOPCONTROLMODE /*Use this to define the control mode each time*/
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
void EQEPSetup(void);


void InitEpwm8(); // ADC iin
void ConfigureADC(void);
void SetupADCEpwm(void);

/*Interrupt functions*/
__interrupt void cpu_timer0_isr(void);


/*Variables: Open Loop Control*/
float fModulationIndexModule1;
float fModulationIndexModule2;
float fModulationIndexModule4;
float fModulationIndexDefault=0;
float duty_cycle=0.5;
float  VF_constant =0;
float  VF_constant_early =0;

float ma1=0;
float ma2=0;
float ma4=0;
float maq=0;
float maq_ref=0;
float mad=0;
float maq_ramp=0;

float mdq=0;

float Curr1Results[RESULTS_BUFFER_SIZE];
float Curr4Results[RESULTS_BUFFER_SIZE];
float Curr2Results[RESULTS_BUFFER_SIZE];

int resultsIndex;
int xyz=0;
float Curr1ResultsAvg=0;
float Curr4ResultsAvg=0;
float Curr2ResultsAvg=0;

float Curr1ResultsSum=0;
float Curr4ResultsSum=0;
float Curr2ResultsSum=0;

/*Variables: Pulse Width Modulation*/
int iSinusoidalPWMCounter = 0;
float fEpwm1DutyCycle = 0;
float fEpwm2DutyCycle = 0;
float fEpwm4DutyCycle=0;

float VFMODE =0;
float ma_max= 0.5;
float f_max= 3;
float fundamental_freq=0;
float fundamental_freq2=0;
int sayi=1;
float phase =0;
float phasePI=0;
float Phase_TD=0;
float Phase_TD2=0;
float D1=0;
float D2=0;

int phase_array[300];
int D1_array[300];
int phase_array_i=0;

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
    EQEPSetup();

    ConfigureADC(); // Configure ADC and power it up
    InitEpwm8(); // Initialize ePWM8
    SetupADCEpwm();

    CpuTimer0Regs.TCR.all = 0x4000; // Use write-only instruction to set TSS bit = 0

    IER |= M_INT1;   // ADC-A1 & CPU1.TIMER0
    PieCtrlRegs.PIEIER1.bit.INTx7 = 1; // TIMER0

    EINT;
    ERTM;

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1; /*Enable all the clocks.*/
    EDIS;

    EPwm8Regs.ETSEL.bit.SOCAEN = 1;  //enable SOCA
    EPwm8Regs.TBCTL.bit.CTRMODE = 0; //unfreeze, and enter up count mode


    for(resultsIndex = 0; resultsIndex < RESULTS_BUFFER_SIZE; resultsIndex++)
    {
        Curr1Results[resultsIndex] = 0;
        Curr4Results[resultsIndex] = 0;
        Curr2Results[resultsIndex] = 0;
    }
    resultsIndex = 0;



    while (1)
    {


    	if (VFMODE==0)
    	    	    {
    	    	    	fEpwm1DutyCycle=0;
    	    	    	fEpwm2DutyCycle=0;
    	    	    	fEpwm4DutyCycle=0;
    	    	    	phase=0;

    	    	    }
    	else if (VFMODE==1)
    	    	    {
    		if (mdq>0.2)
    		{mdq=0.2;}
    					  phase=40;
    		              fEpwm1DutyCycle=0.6;
    	    	    	  fEpwm2DutyCycle=0.5;
    	    	    	  fEpwm4DutyCycle=0.5;

    	    	    	  DELAY_US(5000000);

    	    	    	  DaxisAngle = (float)EQep1Regs.QPOSCNT;

    	    }

    	else if (VFMODE==2)
    	    	    	    {

    		if (maq>0.5) {maq=0.5;}


    		               ma1= cos(RotorAngleElectrical)*mad - sin(RotorAngleElectrical)*maq;
    	    	   	   	   ma2= cos(RotorAngleElectrical-2.0*PI/3)*mad - sin(RotorAngleElectrical-2.0*PI/3)*maq;
    	    	   	   	   ma4= cos(RotorAngleElectrical+2.0*PI/3)*mad - sin(RotorAngleElectrical+2.0*PI/3)*maq;

    	    	   		if (maq<0.10)
    	    	   	        	{phase =55;}
    	    	   	    	else if (maq<0.20)
    	    	   	        	{phase=56;}
    	    	   	    	else if (maq<0.25)
    	    	   	    	        	{phase=57;}
    	    	   	    	else if (maq<0.30)
    	    	   	    	        	{phase=58;}
    	    	   	    	else if (maq<0.35)
    	    	   	    	        	{phase=59;}
    	    	   	    	else if (maq<0.40)
    	    	   	    	        	{phase=60.5;}
    	    	   	    	else if (maq<0.45)
    	    	   	    	        	{phase=62;}
    	    	   	    	else if (maq<0.50)
    	    	   	    	        	{phase=64;}
    	    	   	    	else if (maq<0.55)
    	    	   	    	        	{phase=66;}
    	    	   	    	else if (maq<0.6)
    	    	   	    	    	     {phase=69;}

    	    	    	    }


       else if (VFMODE==3)
    	    	    {

    	   fEpwm1DutyCycle=0.5;
    	   fEpwm2DutyCycle=0.5;
    	   fEpwm4DutyCycle=0.5;

    	    	    }


    	else if (VFMODE==4)
    	    	       	    {
    		  if (fModulationIndexDefault>0 && fundamental_freq>0.1 )
    		    	       	    	{
    		    	       	    			fModulationIndexDefault=fModulationIndexDefault-0.01;
    		    	       	    		    fundamental_freq=fundamental_freq-0.15;
    		    	       	    		    DELAY_US(1000000);
    		    	       	    	}

    	    	       	    }

      	else if (VFMODE==5)
        	    	       	    {
      		fModulationIndexDefault=0;
        	    	       	    }

      	else if (VFMODE==6)
            	    	       	    {

            	    	       	    }

      	else if (VFMODE==7)
      	    	    	    	    {


      	    		if (maq>0.5) {maq=0.5;}


      	    		               ma1= cos(RotorAngleElectrical)*mad - sin(RotorAngleElectrical)*maq;
      	    	    	   	   	   ma2= cos(RotorAngleElectrical-2.0*PI/3)*mad - sin(RotorAngleElectrical-2.0*PI/3)*maq;
      	    	    	   	   	   ma4= cos(RotorAngleElectrical+2.0*PI/3)*mad - sin(RotorAngleElectrical+2.0*PI/3)*maq;

      	    	    	   		if (maq<0.10)
      	    	    	   	        	{phase =55;}
      	    	    	   	    	else if (maq<0.20)
      	    	    	   	        	{phase=56;}
      	    	    	   	    	else if (maq<0.25)
      	    	    	   	    	        	{phase=57;}
      	    	    	   	    	else if (maq<0.30)
      	    	    	   	    	        	{phase=58;}
      	    	    	   	    	else if (maq<0.35)
      	    	    	   	    	        	{phase=59;}
      	    	    	   	    	else if (maq<0.40)
      	    	    	   	    	        	{phase=60.5;}
      	    	    	   	    	else if (maq<0.45)
      	    	    	   	    	        	{phase=62;}
      	    	    	   	    	else if (maq<0.50)
      	    	    	   	    	        	{phase=64;}
      	    	    	   	    	else if (maq<0.55)
      	    	    	   	    	        	{phase=66;}
      	    	    	   	    	else if (maq<0.6)
      	    	    	   	    	    	     {phase=69;}

      	    	    	    	    }

      	else if (VFMODE==8)
      	    	    	    	    {

      	    		if (maq>0.5) {maq=0.5;}


      	    		               ma1= cos(RotorAngleElectrical)*mad - sin(RotorAngleElectrical)*maq;
      	    	    	   	   	   ma2= cos(RotorAngleElectrical-2.0*PI/3)*mad - sin(RotorAngleElectrical-2.0*PI/3)*maq;
      	    	    	   	   	   ma4= cos(RotorAngleElectrical+2.0*PI/3)*mad - sin(RotorAngleElectrical+2.0*PI/3)*maq;

      	    	    	    	    }

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
__interrupt void cpu_timer0_isr(void)
{

    CpuTimer0.InterruptCount++;

    if (fModulationIndexDefault<0 )
		{fModulationIndexDefault=0;}
    else if (fModulationIndexDefault>1 )
          {fModulationIndexDefault=1;}
    else
          {fModulationIndexDefault=fModulationIndexDefault;}


    if (operation_frequeuency>30000 && operation_frequeuency<90000)
            { 		EPwm1Regs.TBPRD = (SYSTEMCLOCKFREQUENCY) / (operation_frequeuency) /2;
            		EPwm2Regs.TBPRD = (SYSTEMCLOCKFREQUENCY) / (operation_frequeuency) /2;
            		EPwm4Regs.TBPRD = (SYSTEMCLOCKFREQUENCY) / (operation_frequeuency) /2;
            	  //  EPwm2Regs.TBPHS.bit.TBPHS=  (phase+phasePI+Phase_TD)*EPwm2Regs.TBPRD/180; // EPwm2Regs.TBPRD = 180 derece

             }

    RotorAngleMechanical = ((float)EQep1Regs.QPOSCNT-DaxisAngle)/((float)ENCODERMAXTICKCOUNT)* 2.0 * PI;
    RotorAngleElectrical =  RotorAngleMechanical*POLEPAIRS;

    if (RotorAngleElectrical>=2.0*PI)
            {RotorAngleElectrical=RotorAngleElectrical-2.0*PI;}


    if (abs(RotorAngleElectrical)<=32*PI/180 && abs(RotorAngleElectrical)>=28*PI/180  )
                {maq=maq_ref;}

if (CONTROLMODE == 1)
{

    	        fEpwm1DutyCycle = (ma1+ 1) / 2;
    	        fEpwm2DutyCycle = (ma2+ 1) / 2;
    	        fEpwm4DutyCycle = (ma4+ 1) / 2;

    	        D1=2*sin(PI*fEpwm1DutyCycle)/PI;
    	        D2=2*sin(PI*fEpwm2DutyCycle)/PI;

    	        Phase_TD=acos((-(IF_Ref*IF_Ref)+( D1*D1+D2*D2))/ (2*D1*D2))*180/PI;

    	           if (Phase_TD<0)
    	       	{Phase_TD=0;}
    	           else if (Phase_TD>180)
    	           {Phase_TD=180;}

}

if (CONTROLMODE==2)
{

    fModulationIndexModule1 = fModulationIndexDefault;
    fModulationIndexModule2 = fModulationIndexDefault;
    fModulationIndexModule4 = fModulationIndexDefault;
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

    D1=2*sin(PI*fEpwm1DutyCycle)/PI;
    D2=2*sin(PI*fEpwm2DutyCycle)/PI;

   // fEpwm1DutyCycle=fEpwm1DutyCycle+0.01;
//    fEpwm2DutyCycle=fEpwm2DutyCycle+0.01;
  //  fEpwm4DutyCycle=fEpwm4DutyCycle+0.01;


    Phase_TD=acos((-(IF_Ref*IF_Ref)+( D1*D1+D2*D2))/ (2*D1*D2))*180/PI;

    if (Phase_TD<0)
	{Phase_TD=0;}
    else if (Phase_TD>180)
    {Phase_TD=180;}


    phase_array[phase_array_i]=Phase_TD;
    D1_array[phase_array_i]=100*D1;
    phase_array_i++;
    if (phase_array_i==300) {phase_array_i=0;}


}



        EPwm1Regs.CMPA.bit.CMPA = EPwm1Regs.TBPRD * fEpwm1DutyCycle; // (ePWM1)
        EPwm2Regs.CMPA.bit.CMPA = EPwm2Regs.TBPRD * fEpwm2DutyCycle; // (ePWM2)
        EPwm4Regs.CMPA.bit.CMPA = EPwm4Regs.TBPRD * fEpwm4DutyCycle; // (ePWM2)
        EPwm1Regs.TBPHS.bit.TBPHS=  ((phase+phasePI+Phase_TD))*EPwm1Regs.TBPRD/180; // EPwm2Regs.TBPRD = 180 derece
        EPwm2Regs.TBPHS.bit.TBPHS=  ((Phase_TD2))*EPwm2Regs.TBPRD/180; // EPwm2Regs.TBPRD = 180 derece



        Curr1 = ((float)(AdcaResultRegs.ADCRESULT1 - 50540))*VoltsPerBit*90.0;
        Curr2 = ((float)(AdcaResultRegs.ADCRESULT2 - 50300))*VoltsPerBit*94.0;

      //  Curr3 = ((float)(AdcbResultRegs.ADCRESULT0 - 50580))*VoltsPerBit*89.0;
        Curr4 = ((float)(AdcbResultRegs.ADCRESULT1 - 50640))*VoltsPerBit*89.0;
        Curr3=-Curr1-Curr4;



        Curr1ResultsSum= Curr1ResultsSum-Curr1Results[resultsIndex]+Curr1;
        Curr1ResultsAvg=Curr1ResultsSum/RESULTS_BUFFER_SIZE;

        Curr4ResultsSum= Curr4ResultsSum-Curr4Results[resultsIndex]+Curr4;
        Curr4ResultsAvg=Curr4ResultsSum/RESULTS_BUFFER_SIZE;

        Curr2ResultsSum= Curr2ResultsSum-Curr2Results[resultsIndex]+Curr2;
        Curr2ResultsAvg=Curr2ResultsSum/RESULTS_BUFFER_SIZE;

      	Curr1Results[resultsIndex] = Curr1;
        Curr4Results[resultsIndex] = Curr4;
        Curr2Results[resultsIndex] = Curr2;
        resultsIndex++;
   	    if(resultsIndex==RESULTS_BUFFER_SIZE)
   	    {
      	  resultsIndex = 0;
   	    }

        if (Curr1> maxCurr || Curr4> maxCurr || Curr1<-maxCurr || Curr4<-maxCurr)
        {GpioDataRegs.GPDCLEAR.bit.GPIO125 = 1; } // Set 1 (DISABLE)


   /*     iSinusoidalPWMCounter += 1;
        if (maq<maq_ramp && iSinusoidalPWMCounter==2000 )
              	    	    		{
              	    	    			maq=maq+0.01;
              	    	    			iSinusoidalPWMCounter=0;

              	    	    		} */


       IF_ERROR= IF_Ref-Curr2;
       IF_ERROR_SUM=IF_ERROR_SUM+IF_ERROR;


       phasePI = (P_IF*IF_ERROR+I_IF*IF_ERROR_SUM);

       if (phasePI>10)
       {phasePI=10;}
       else if (phasePI<-10)
       {phasePI=-10;}


        PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}


void EPWM1Setup(void)
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
	    EPwm2Regs.TBPHS.bit.TBPHS=  2*EPwm2Regs.TBPRD/9; // EPwm2Regs.TBPRD = 180 derece
	    EPwm2Regs.TBCTL.bit.SWFSYNC = 1;

	    EDIS;

}

void EPWM4Setup(void)
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



/*void EncoderReading()

{

} */

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
