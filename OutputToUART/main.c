#include <msp430g2553.h>
#include <stdbool.h>
////////////////Defines////////////////
#define LED1        BIT6
#define LED0        BIT0
#define Error       BIT0
#define GND         BIT4
#define DAT         BIT5
#define VCC         BIT0 //port P2.0
char buffer[4];
float temp;
int n_temp,r_temp;
////////////////Function Protos////////////////
void TX(char *tx_message);
static char *i2a(unsigned i, char *a, unsigned r);
char *itoa(int i, char *a, int r);
void ADC_init(void);

static char *i2a(unsigned i, char *a, unsigned r)
{
    if (i/r > 0) a = i2a(i/r,a,r);
    *a = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[i%r];
    return a+1;
}

char *itoa(int i, char *a, int r)
{
    if ((r < 2) || (r > 36)) r = 10;
    if (i < 0)
    {
        *a = '-';
        *i2a(-(unsigned)i,a+1,r) = 0;
    }
    else *i2a(i,a,r) = 0;
    return a;
}

void TX(char *tx_message)
{
    unsigned int i=0; //Define end of string loop int
    char *message; // message variable
    unsigned int message_num; // define ascii int version variable
    message = tx_message; // move tx_message into message
    while(1)
    {
        if(message[i]==0) // If end of input string is reached, break loop.
        {break;}
        message_num = (int)message[i]; //Cast string char into a int variable
        UCA0TXBUF = message_num; // write INT to TX buffer
        i++; // increase string index
        __delay_cycles(10000); //transmission delay
        if(i>50) //prevent infinite transmit
        {
            P1OUT |= Error;
            break;
        }
    } // End TX Main While Loop
} // End TX Function

void ADC_init()
{
    /* Configure ADC  Channel */
    ADC10CTL1 = INCH_5 + ADC10DIV_3 ;         // Channel 5, ADC10CLK/4
    ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON + ADC10IE;  //Vcc & Vss as reference
    ADC10AE0 |= BIT5;                         //P1.4 ADC option


}

////////////////////////////////MAIN PROGRAM LOOP//////////////////////////////////////////////////
void main(void)
{
    WDTCTL = WDTPW + WDTHOLD;         // Stop WDT
    BCSCTL1 = CALBC1_1MHZ;            // Set DCO to 1MHz
    DCOCTL = CALDCO_1MHZ;
    ////////////////USCI setup////////////////
    P1SEL = BIT1 + BIT2;            // Set P1.1 to RXD and P1.2 to TXD
    P1SEL2 = BIT1 + BIT2;            //
    UCA0CTL1 |= UCSSEL_2;            // Have USCI use SMCLK AKA 1MHz main CLK
    UCA0BR0 = 104;                  // Baud: 9600, N= CLK/Baud, N= 10^6 / 9600
    UCA0BR1 = 0;                  // Set upper half of baud select to 0
    UCA0MCTL = UCBRS_1;               // Modulation UCBRSx = 1
    UCA0CTL1 &= ~UCSWRST;             // Start USCI
    ////////////////General GPIO Defines////////////////
    P1DIR |= (LED1+GND+LED0);            //Define GPIOs as outputs else GPIOs are inputs
    P2DIR |=VCC;
    P1OUT =0;                     //All Outputs off except for Green_LED
    P2OUT|=VCC;
    P1OUT&=~GND;
    //////////////SETUP ADC10 TO READ FROM CH A4 (P1.4)/////////////////////////////////////////////
    ADC_init();
    __enable_interrupt();                     // Enable interrupts.
    int value=0;

////////////////Main Loop////////////////
    while(1)
    {
        __delay_cycles(1000);                   // Wait for ADC Ref to settle
        ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
        __bis_SR_register(CPUOFF + GIE);        // LPM0 with interrupts enabled
        value = ADC10MEM;
        temp=value*0.317;  //temperature as float
        //split the float into integer and fraction part
        //C = A % B is equivalent to C = A ? B * (A / B)
        r_temp=((temp*10)-10) * ((temp*10)/10);
        itoa(temp, buffer, 10);
        TX("$LM35:");
        TX(buffer);
        TX(".");
        itoa(r_temp, buffer, 10);
        TX(buffer);
        TX("!\r\n");
        P1OUT ^= LED0;
        _delay_cycles(500000);

    } // End While
} // End Main Program

// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR (void)
{
  __bic_SR_register_on_exit(CPUOFF);        // Return to active mode
  P1OUT ^= LED1;
}
