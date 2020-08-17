
#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>


/*
INTERRUPT vectors
USART_TXC_vect - przerwanie wysylania danych USART
ADC_vect - przerwanie zakonczenia konwersji ADC
TIMER_OVF_vect - przerwanie przepelnienia zegara/licznika nr 0


As per ATMega328 Datasheet Section 16.9.1 page 123, setting the timer
to Normal mode causes the counter to count up until it reaches 0xFF
at which point it will overrun and start back at 0.  To configure this
timer/counter to produce a period of 1ms we need to start counting
at a value that causes it to reach 0xFF in 1ms.

z odpowiednio ustawionym prescalerem możliwe jest uzyskanie czestotliwosci timera 976 Hz, co jest zbliżoną wartością do 1 kHz.
prescaler należy ustawić na 64, wtedy

16 MHz /64 /256 (bo 8 bitowy timer0) = 976 Hz

TIFR0 - timer interrupt flag register
TOV0 - timer overflow flag
The counter simply overruns when it passes its maximum 8-bit value (TOP = 0xFF) and then restarts from the bottom (0x00). 
In normal operation the Timer/Counter overflow flag (TOV0) will be set in the same timer clock cycle as the TCNT0 becomes zero. 
The TOV0 Flag in this case behaves like a ninth bit, except that it is only set, not cleared.
*/

static inline void initADC(void)
{
	ADMUX |= (1<<REFS0)|(1<<ADLAR); // ref=AVcc, przesuniecie bitow do lewej
	ADCSRA |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0) | (1<<ADEN); // dzielnik /128, wlaczenie ADC
}

void initUSART(void){
	UBRR0=103;		//baudrate 9600 dla 16 MHz
	
	UCSR0B = (1 << TXEN0) | (1 << RXEN0) | (1<<TXCIE0)|(1<<RXCIE0); // wlacz Tx i Rx oraz ich przerwania
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);   //8bitow danych 1 bit stopu
}

void transmitByte(uint8_t data) {
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = data;
}



int main(void){
	//inicjalizacja
	initADC();
	initUSART();
	
	TIMSK0 |= (1<<TOIE0);	//wlaczenie przerwan dla timera 0
	sei();
	TCCR0B |= (1<<CS01)|(1<<CS00);	//prescaler  64 
	TCNT0=0; //zerowanie timera0
    while (1) 
    {		
    }
}

ISR(TIMER0_OVF_vect)
{
	ADCSRA |= (1<<ADSC);
	loop_until_bit_is_clear(ADCSRA,ADSC);
	transmitByte(ADCH);
	TIFR0 |= (0<<TOV0);
}
