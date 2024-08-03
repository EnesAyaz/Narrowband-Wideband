#include <math.h>
#include <F2837xD_Device.h>
#include <F2837xD_Examples.h>

#define PI 					          3.141592654
#define SYSTEMCLOCKFREQUENCY 	200000000	  // Hz
#define SYSTEMCLOCKPERIOD 		5				    // ns
#define SWITCHINGFREQUENCY    800         // Hz
#define DEADTIME 			        100	 			// ns
#define sample_window       65          // clock

float epwm1counter=0;

__interrupt void epwm1_isr(void);

void InitializeEpwm1Registers(void);
void ConfigureADC(void);
void SetupADCEpwm();
void SetupGPIOs(void);

Uint16 AdcaResults;


int main(void)
{

    InitSysCtrl(); /*initialize the peripheral clocks*/

    EALLOW;
    ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV = 0; // EPWM Clock Divide Select: /1 of PLLSYSCLK
    EDIS;

    InitPieCtrl(); /*initialize the PIE table (interrupt related)*/
    IER = 0x0000;  /*clear the Interrupt Enable Register   (IER)*/
    IFR = 0x0000;  /*clear the Interrupt Flag Register     (IFR)*/
    InitPieVectTable();

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;  /*stop the TimeBase clock for later synchronization*/
    CpuSysRegs.PCLKCR0.bit.GTBCLKSYNC = 0; /*stop the global TimeBase clock for later synchronization*/
    EDIS;

    DINT;

    EALLOW;
    PieVectTable.EPWM1_INT = &epwm1_isr;        /*specify the interrupt service routine (ISR) address to the PIE table*/
    EDIS;

    IER |= M_INT3;  /*Enable the PIE group of Epwm1 interrupt*/

    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;  // Enable the PIE block
    PieCtrlRegs.PIEIER3.bit.INTx1 = 1;  /*Enable the 1st interrupt of the Group 3, which is for epwm1 interrupt*/

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;  /*start the TimeBase clock */
    CpuSysRegs.PCLKCR0.bit.GTBCLKSYNC = 1; /*start the global TimeBase clock */
    EDIS;


    EALLOW;
    EINT; // Enable Global interrupt INTM
    ERTM; // Enable Global realtime interrupt DBGM
    EDIS;


    while (1)
    {

    }
}


void InitializeEpwm1Registers(void)
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

	    EALLOW;
	        EPwm1Regs.ETSEL.bit.SOCAEN = 1; /*enable EPWMxSOCA signal*/
	        EPwm1Regs.ETSEL.bit.SOCASEL = 2;    /*ADC sampling is done when TBCTR==TBPRD*/
	        EPwm1Regs.ETSEL.bit.INTEN = 1;  /*enable pwm interrupt*/
	        EPwm1Regs.ETSEL.bit.INTSEL = 1; /*interrupt occurs when TBCTR = 0*/

	        EPwm1Regs.ETPS.all = 0x00;
	        EPwm1Regs.ETPS.bit.INTPRD = 1;  // Generate INT on first event
	        EPwm1Regs.ETPS.bit.SOCAPRD = 1; // Generate SOC on first event
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

    EDIS;
}

void SetupADCEpwm()
{

    EALLOW;
    AdcaRegs.ADCSOC1CTL.bit.CHSEL = 2;  //SOC1 will convert (ADCINA2 - ADCINA3)
    AdcaRegs.ADCSOC1CTL.bit.ACQPS = sample_window; // 1.1MSPS -> 909 ns sampling window -> 910ns/5ns = 182 -> acqps = 182 - 1
    AdcaRegs.ADCSOC1CTL.bit.TRIGSEL = 5; // ePWM1 for Trig Signal

	AdcaRegs.ADCINTSEL1N2.bit.INT2SEL = 0; //end of SOC0 will set INT1 flag
	AdcaRegs.ADCINTSEL1N2.bit.INT2E = 1;   //enable INT1 flag
	AdcaRegs.ADCINTFLGCLR.bit.ADCINT2 = 1; //make sure INT1 flag is cleared

    EDIS;

}

void SetupGPIOs(void)
{
    EALLOW;
    GpioCtrlRegs.GPAPUD.bit.GPIO0 = 1;   // Disable pull-up on GPIO0 (EPWM1A)
    GpioCtrlRegs.GPAPUD.bit.GPIO1 = 1;   // Disable pull-up on GPIO1 (EPWM1B)
    GpioCtrlRegs.GPAGMUX1.bit.GPIO0 = 0; // Configure GPIO0 as EPWM1A
    GpioCtrlRegs.GPAGMUX1.bit.GPIO1 = 0; // Configure GPIO1 as EPWM1B
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;  // Configure GPIO0 as EPWM1A
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 1;  // Configure GPIO1 as EPWM1B
    EDIS;

    EALLOW;
    GpioCtrlRegs.GPDPUD.bit.GPIO125 = 0; // enable pull up
    GpioDataRegs.GPDSET.bit.GPIO125 = 1; // Set 1 (ENABLE)
    GpioCtrlRegs.GPDDIR.bit.GPIO125 = 1; // set it as output
    GpioDataRegs.GPACLEAR.bit.GPIO4 = 1; // Clear 0 (SOFTOFF)
    GpioCtrlRegs.GPADIR.bit.GPIO4 = 1; // set it as output
    EDIS;


}


__interrupt void epwm1_isr(void)
{
	epwm1counter++;
	if (epwm1counter>1000000)
	{epwm1counter =0;}

	AdcaResults=AdcaResultRegs.ADCRESULT1;
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
    PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}
