#include <msp430.h> 

#define START  0
#define STAGE1 1
#define STAGE2 2
#define STAGE3 3
#define WIN    4

volatile int state = START;

int main(void){

    WDTCTL = WDTPW | WDTHOLD;  // Stop watchdog timer
                                //Default MCLK speed 1MHz

    //-- Setup Ports
        //LEDs
      P1DIR |= BIT0;              // Set P1.0 (LED1) as output
      P1OUT &=~ BIT0;             // Start LED1 off

        P1DIR |= BIT6;              // Set P1.6 (LED2) as output
      P1OUT &=~ BIT6;             // Start LED2 off

        //Switch
        P1DIR &=~ BIT3;             // Set P1.3 (SW1) as input
      P1REN |= BIT3;              // Enable resistor for SW1
      P1OUT |= BIT3;              // Config resistor as pull-up
      P1IES |= BIT3;              // Interrupt edge select from H-to-L
      P1IE |= BIT3;               // Local interrupt enable
      P1IFG &=~ BIT3;             // Clear SW1 interrupt flag

    //-- Setup TimerA0
        TA0CTL |= TACLR;            // Clear TA0
        TA0CTL |= TASSEL_2;         // SMCLK is source for TA0
        TA0CTL |= ID_3;             // No input divide
        TA0CTL |= MC_1;             // Up mode

    //-- Setup CCR0
        TA0CCR0 = 0x8000;           // Timer resets at 0x8000
        TA0CCTL0 |= CCIE;           // CCR0 local interrupt enable
        TA0CCTL0 &=~ CCIFG;         // Clear CCR0 interrupt flag

    __enable_interrupt();       // global interrupt enable



    while(1)
    {

    }

}

//----------Interrupt Service Requests------------------------------//

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMERA0_CCR0_ISR(void){
    switch (state)
    {
    case START:
        P1OUT ^= BIT0 | BIT6;
        TA0CCR0 = 0x8000;
        break;

    case STAGE1:
        P1OUT &=~ BIT6;
        P1OUT ^= BIT0;
        TA0CCR0 = 0x8000;
        break;

    case STAGE2:
        P1OUT ^= BIT6;
        P1OUT |= BIT0;
        TA0CCR0 = 0x4000;
        break;

    case STAGE3:
        P1OUT ^= BIT0 | BIT6;
        TA0CCR0 = 0x2000;
        break;

    case WIN:
        P1OUT |= BIT0 | BIT6;
        break;

    default:
        break;
    }
    TA0CCTL0 &=~ CCIFG;          // Clear CCR0 interrupt flag
}

#pragma vector = PORT1_VECTOR
__interrupt void SWITCH1_ISR(void){

    switch (state)
    {
    case START:
        state = STAGE1;
        break;

    case STAGE1:
        if(P1OUT & BIT0){
            state = STAGE2;
        }
        else {
            state=STAGE1
        }
        break;

    case STAGE2:
        if(P1OUT & BIT6){
            state = STAGE3;
        }
        else {
            state=STAGE1
        }
        break;

    case STAGE3:
        if((P1OUT & BIT6) && (P1OUT & BIT0)){
            state = WIN;
            P1OUT |= BIT0;
            P1OUT &=~BIT6;
        }
        else {
              state=STAGE1
        }
        break;

    case WIN:
        state = START;
        break;

    default:
        break;
    }

    TA0CTL |= TACLR;            // Clear TA0
    P1IFG &=~ BIT3;             // Clear SW1 interrupt flag
}