/*
 * QMGbat.c
 *
 * Created: 24.06.2019 21:24:24
 * Author : Benjamin Renz
 */ 

//Settings
#define onTimeInSec		900			//for how long to enable the led's =15min
#define maxVoltageRatio 770			//minimal allowed value for (V_status_led/VCC)*1024   V_status_led 1.8V typical red led, was 387 on 5V, should warn when lower 2.4V
#define F_CPU			1000000UL	//1MHZ cpu clock

//Fuses
/*
PUD off //important for voltage reference with external led

*/

#define irLEDpin1    3	//Ir led is split in two, because each pin can only source absolute max 40mA and the whole package 60mA
#define irLEDpin2    4
#define statusLED    2  //Status led is used as voltage reference, must be adc capable pin therefore
#define statusLEDadc 1	//is pb2
#define interruptIn  0

#include <avr/io.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
int secondsPassed=onTimeInSec;
 

int16_t readVCC() {
	PRR&=~(1<<PRADC);
	ADCSRA|=(1<<ADEN);
	ADCSRA|=(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);			  //Enable adc
	ADMUX|=statusLEDadc;									//Set reference to Vin
	//set led as input pullup to get a voltage reference
	DDRB&=~(1<<statusLED);		  //set status led as input
	PORTB|=(1<<statusLED);		  //pullup status led pin to generate small current an therefore get led voltage on pin
	ADCSRA |=(1 << ADSC);        //Start conversion
	while (ADCSRA & (1 << ADSC)); //Wait for end of first conversion
	ADCSRA |=(1<<ADIF);
	ADCSRA |=(1<<ADSC);			  //Start new conversion
	while (ADCSRA & (1 << ADSC)); //Wait for end of second conversion
	int16_t result=ADC;
	ADCSRA |= (1<<ADIF);
	PORTB&=~(1<<statusLED);		  //disable pullup status led pin
	DDRB|=(1<<statusLED);		  //set status led as output
	ADCSRA&=~(1<<ADEN);			  //disable adc
	return result;
}

void enableIR(){
	PORTB|= (1<<irLEDpin1)|(1<<irLEDpin2); //needs to be done in one instruction because pins are connected
}

void disableIR(){
	PORTB&= ~((1<<irLEDpin1)|(1<<irLEDpin2)); //needs to be done in one instruction same as enable
}

void enableStatus(){
	PORTB|= (1<<statusLED);
}

void disableStatus(){
	PORTB&= ~(1<<statusLED);
}

void configurePINS(){
	DDRB=(1<<irLEDpin1)|(1<<irLEDpin2)|(1<<statusLED); //set all led pins as output
}


void goSleep(){
	GIFR|=(1<<PCIF);		//Clear previousely pending pin change interrupts
	GIMSK|=(1<<PCIE);		//Enable pin change interrupts
	PCMSK=(1<<interruptIn); //Enable only the interrupts for interruptPin
	ADCSRA&=~(1<<ADEN);		//disable ADC
	PRR|=(1<<PRTIM0)|(1<<PRADC); //turn off ADC and TIMER0 to save power
	cli();					//No intrerrupts from here, the bod disable sequence is timed
	BODCR = (1<<BODS) | (1<<BODSE);
	BODCR = (1<<BODS);
	sei();					//Enable interrupts for sleep
	sleep_bod_disable();
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();
	sleep_cpu();
}

ISR(PCINT0_vect){ //Interrupt function
	GIMSK&=~(1<<PCIE);//Disable any additional pin change interrupt
	secondsPassed=0;
}

int main(void)
{
    configurePINS();
	disableStatus();
	//goSleep();
    while (1) 
    {	
		if((secondsPassed++)>onTimeInSec){
			disableStatus(); //power off status led to save power
			disableIR();     //disable IR led
			goSleep();       //go to sleep and setup wakeup interrupts	
			int16_t voltage=readVCC();
			if(voltage>maxVoltageRatio){ //warn user of low battery
				for(int i=0;i<20;i++){
					enableStatus();
					_delay_ms(500);
					disableStatus();
					_delay_ms(500);
				}
				secondsPassed=onTimeInSec; //power off directely
			}else{
				enableIR();
				enableStatus();
			}
		}
		if(!(PINB&(1<<interruptIn))){
			secondsPassed=0; //reset counter, when pin goes low/stays low
		}
		_delay_ms(1000);
    }
}

