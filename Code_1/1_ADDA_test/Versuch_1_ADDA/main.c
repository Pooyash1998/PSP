#include <avr/io.h>
#include <util/delay.h>
#include "os_input.h"

// function declaration
void manuell(void);
void tracking_wandler();
void sar_wandler();

int main(void) {
    /* Replace with your application code */
	os_initInput();
	//select the desired Subprogramm here by changing the char (s/m/t)!
	char subprog = 't';
	while (1) {
	switch(subprog)
	{
		case 'm': manuell();
		break;
		case 't': tracking_wandler();
		break;
		case 's': sar_wandler();
		break;
		default:  //do nothing
		break;
	}   
  }
}

void manuell()
{
	DDRD = 0; //Configures PORTD as Input for manual Conversion
	PORTD = 0xFF; //configures pull-up Resistors
	DDRA = 0xFF ; //output LEDs  
	DDRB = 0xFF ; //Output for R-2R-Network
	
	while(1)  //kontinuerliche Aktualisierung der Werten
	{	
	uint8_t signal_port_d = PIND; //reading the values of DIP-schalter
	
	PORTA = signal_port_d ; //updating LEDs (not inverting needed LEDS are also on when bit=0)
	PORTB = ~(signal_port_d) ; //updating R-2R values ( inverting because DIP 1 --> bit 0) 
	/*hinweis: bei (11111111) hat max Max_Volt = 255/256 * Vcc 
	  analog bei (00000000) ist die gemessene Spannung = 0 */ 
	}
}
void tracking_wandler()
{
	os_waitForInput(); //wait for user to press a button (here only C available)
	uint8_t ref = 0b10000000 ; //zu beginn initialisiere ref Var mit beliebigem Wert
	DDRB = 0xFF ; //Output for R-2R-Network
	DDRA = 0xFF ; //output LEDs
	DDRC &= 0b11111110 ;// portc pin C0 (Ergebnis des Komparators) als Eingang
	PORTC |= 0b00000001 ;//pullup C0 = 1
	
	PORTB = ref; //ref an R-2R-Netzwerk, das die Referencespannung erzeugt.
	PORTA = ~ref; //LED
	_delay_ms(50);
	uint8_t comp_out = PINC & 1 ; 
	uint8_t state = 0;  //benutze state fur Fallunterscheidung + das Ende der Wandlung erkennen
	
	/* : if U_ref < U_mess => Komp = 0
	     if U_ref > U_mess => Komp = 1 */
	if(comp_out == 1) state=1; 
	
	while(1) {
	PORTB = ref; 
	PORTA = ~ref; //LED
	_delay_ms(50);
	comp_out = PINC & 1 ; //aktualisiere comp_out
	
	// Ueberprüfe, ob Wandlung abgeschlossen ist
	if ((state==1 && comp_out==0) || (state==0 && comp_out==1))
		break;
		
	// vermeidet Ueberlauf falls U mehr als 5 volt ist)
	if((ref==0xFF) || (ref==0x0))
		break;
			
	//Vergleichsschritt 
	if(comp_out == 0)
	{ref++;}
	else if(comp_out == 1)
	{ref--;}
	
	 }
	 //Wandlung abgeschlossen warte auf btn_druck fuer naechste Wandlung (hier nur btn_C)
	os_waitForInput();
}

void sar_wandler()
{
	os_waitForInput(); //wait for user to press a button (here only C available)
	uint8_t ref = 0b10000000 ; //zu beginn initialisiere ref Var mit MSB=1
	DDRB = 0xFF ; //Output for R-2R-Network
	DDRA = 0xFF ; //output LEDs
	DDRC &= 0b11111110 ;// portc pin C0 (Ergebnis des Komparators) als Eingang
	PORTC |= 0b00000001 ;//pullup C0 = 1
	PORTB = ref; //ref an R-2R-Netzwerk, das die Referencespannung erzeugt.
	PORTA = ~ref; //LED
	_delay_ms(50);
	for(uint8_t iteration=1 ;iteration <= 8; iteration ++)
	{
	 uint8_t comp_out = PINC & 1 ;
	 
	 /* : if U_ref < U_mess => Komp = 0
	     if U_ref > U_mess => Komp = 1 */
	 if(comp_out == 0)
		{
		  ref |= 1 << (7 - iteration) ; // naeschtes hochwertiges Bit ->1 
		}
	  else if(comp_out == 1)
	  {	  
		  ref |= 1 << (7 - iteration) ; // naeschtes hochwertiges Bit auf 1 
		  ref &= ~(1 << (7 - iteration + 1)); // voriges bit wieder auf 0
	  }
	   PORTB = ref; //update ref spannung
	   PORTA = ~ref; //LED
	   _delay_ms(50);
	}
	   // Randfall : das letzte bit wird nie auf 0 gesetzt. falls immer noch gilt U_ref > U_mess 
	   uint8_t comp_out = PINC & 1 ;
	   if(comp_out==1) 
	   {ref=0; PORTB=ref; PORTA=~ref;} 
	  
	//Wandlung abgeschlossen, warte auf die Naechste
	os_waitForInput(); 
}