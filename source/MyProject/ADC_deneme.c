#include <F2837xD_Device.h>
#include <F2837xD_Examples.h>

/*Bu kodda pot ile phase shift ayarlanacak.potun de�erine g�re phashift 180 ve 0 aras�nda de�i�tirilecek.*/

void ConfigureADC(void);
void ConfigureEPWM(void);
void SetupADCEpwm(void);

#define Cur_Buf_Size 256
Uint16 Cb1[Cur_Buf_Size];
float cur_sum1=0;
unsigned int resultsIndex=0;

void EPwm1A(void);
void EPwm2A(void);

__interrupt void adca1_isr(void);
Uint16 AdcaResults;

int main(void)
{

    InitSysCtrl(); /*initialize the peripheral clocks*/

    InitPieCtrl();  /*initialize the PIE table (interrupt related)*/
    IER = 0x0000;   /*clear the Interrupt Enable Register   (IER)*/
    IFR = 0x0000;   /*clear the Interrupt Flag Register     (IFR)*/
    InitPieVectTable();

    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;   /*stop the TimeBase clock for later synchronization*/
    CpuSysRegs.PCLKCR0.bit.GTBCLKSYNC = 0;  /*stop the global TimeBase clock for later synchronization*/
    EDIS;

    /*Initialize cpu timers*/
   InitCpuTimers();
   CpuTimer0Regs.TCR.all = 0x4000; // enable cpu timer interrupt


   EALLOW;
   PieVectTable.ADCA1_INT = &adca1_isr;  /*specify the interrupt service routine (ISR) address to the PIE table*/
   EDIS;


    IER |= M_INT1;  /*Enable the PIE group of Cpu timer 0 interrupt*/
    IER |= M_INT3;
    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;  // Enable the PIE block
    PieCtrlRegs.PIEIER1.bit.INTx1 = 1;  //ADCA1 interrupt
    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;   /*start the TimeBase clock */
    CpuSysRegs.PCLKCR0.bit.GTBCLKSYNC = 1;  /*start the global TimeBase clock */
    EDIS;

    EINT;  // Enable Global interrupt INTM
    ERTM;  // Enable Global realtime interrupt DBGM

    ConfigureADC();
    ConfigureEPWM();
    SetupADCEpwm();
    EPwm1A();
    //EPwm2A();                                                //BURAYI UNUTMA ��MD�L�K DEAKT�VE ETT�N!!!

    EALLOW;
    GpioCtrlRegs.GPAPUD.bit.GPIO0=1;
    GpioCtrlRegs.GPAGMUX1.bit.GPIO0=0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO0=1; //Epwm1A gpio0 secildi

    GpioCtrlRegs.GPAPUD.bit.GPIO1=1;
    GpioCtrlRegs.GPAGMUX1.bit.GPIO1=0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO1=1; //Epwm1B gpio1 secildi

    GpioCtrlRegs.GPAPUD.bit.GPIO2=1;
    GpioCtrlRegs.GPAGMUX1.bit.GPIO2=0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO2=1; //Epwm2A gpio0 secildi

    GpioCtrlRegs.GPAPUD.bit.GPIO3=1;
    GpioCtrlRegs.GPAGMUX1.bit.GPIO3=0;
    GpioCtrlRegs.GPAMUX1.bit.GPIO3=1; //Epwm2B gpio3 secildi


    EDIS;




  while(1)
  {
	  EPwm3Regs.ETSEL.bit.SOCAEN = 1;  //enable SOCA
	  EPwm3Regs.TBCTL.bit.CTRMODE = 0; //unfreeze, and enter up count mode



  }
  }







void EPwm1A(void)
{
	EALLOW;
	EPwm1Regs.TBCTL.all = 0x00;
	EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;  // Set counter to be up-down
	EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV1;
	EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
	EPwm1Regs.TBPRD = (200000000) / (10000)/2;  /*period is set to be 10khz (for up down count)*/

	EPwm1Regs.CMPA.bit.CMPA = EPwm1Regs.TBPRD / 2; //EPWM1A i�in se�ildi
	EPwm1Regs.CMPB.bit.CMPB = EPwm1Regs.TBPRD / 2; //EPWM1B i�in se�ildi

	EPwm1Regs.AQCTLA.all = 0x00;
	EPwm1Regs.AQCTLA.bit.CAU = AQ_SET;      //set high
	EPwm1Regs.AQCTLA.bit.CAD = AQ_CLEAR;    //Set low

	EPwm1Regs.AQCTLB.all = 0x00;
	EPwm1Regs.AQCTLB.bit.CAU = AQ_CLEAR;      //set low
	EPwm1Regs.AQCTLB.bit.CAD = AQ_SET;    //Set high

	EPwm1Regs.DBCTL.bit.OUT_MODE = 3;
	EPwm1Regs.DBCTL.bit.POLSEL = 2;
	EPwm1Regs.DBFED.bit.DBFED = 1000 / 5;
	EPwm1Regs.DBRED.bit.DBRED = 1000 / 5;
	EPwm1Regs.DBCTL2.all = 0x00;


	EPwm1Regs.TBCTL.bit.PHSEN = 0;
	EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO; //SYNCOUT
	EDIS;
}

void EPwm2A(void)
{
	EALLOW;
	EPwm2Regs.TBCTL.all = 0;
	EPwm2Regs.TBCTL.bit.CLKDIV =  0;	// CLKDIV = 1
	EPwm2Regs.TBCTL.bit.HSPCLKDIV = 0;	// HSPCLKDIV = 1
	EPwm2Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;	// up - down mode
	EPwm2Regs.TBCTL.bit.PHSEN = 1;		// enable phase shift for ePWM2

	EPwm2Regs.AQCTLA.bit.CAU = AQ_SET;      //set high
    EPwm2Regs.AQCTLA.bit.CAD = AQ_CLEAR;    // Set low
    EPwm2Regs.AQCTLB.bit.CAU = AQ_CLEAR;    //set low
    EPwm2Regs.AQCTLB.bit.CAD = AQ_SET;      //set high

    EPwm2Regs.DBCTL.bit.OUT_MODE = 3;
    EPwm2Regs.DBCTL.bit.POLSEL = 2;
    EPwm2Regs.DBFED.bit.DBFED = 1000 / 5;
    EPwm2Regs.DBRED.bit.DBRED = 1000 / 5;
    EPwm2Regs.DBCTL2.all = 0x00;


    EPwm2Regs.TBCTL.bit.SYNCOSEL =  TB_SYNC_IN;
    EPwm2Regs.TBCTL.bit.PHSDIR = TB_DOWN;
	EPwm2Regs.TBPRD = (200000000) / (10000)/2; 			// 10KHz - PWM signal

	EPwm2Regs.CMPA.bit.CMPA = EPwm2Regs.TBPRD/2; // EPWM2A i�in %50duty
	EPwm2Regs.CMPB.bit.CMPB = EPwm2Regs.TBPRD/2; // EPWM2B i�in %50duty

	EPwm2Regs.TBPHS.bit.TBPHS= EPwm2Regs.TBPRD/2;	// 1/3 phase shift
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
	AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 9; //SOC0 will begin conversion on ePWM3 SOCB
	//AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 1; //SOC0 will begin conversion on Tımer0
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

	Cb1[resultsIndex++] = AdcaResultRegs.ADCRESULT0;
	cur_sum1=cur_sum1+AdcaResultRegs.ADCRESULT0;
	cur_sum1=cur_sum1-Cb1[Cur_Buf_Size-resultsIndex];
	        if(Cur_Buf_Size <= resultsIndex)
	        {
	        	resultsIndex = 0;    }


	//EPwm1Regs.TBPRD=10000+AdcaResults*10;
	//EPwm1Regs.CMPA.bit.CMPA=EPwm1Regs.TBPRD/2;
	//EPwm1Regs.CMPB.bit.CMPB=EPwm1Regs.TBPRD/2;

	AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1; //clear INT1 flag
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP1;
}


