/*

 Autore : 	Orlandi Giuseppe

 Company: 	Brahma S.p.a.

 Data	: 	25/09/02

 Revisione:	00

 Note:
 Software per l'attivazione di una resistenza che scalda
 l'acqua in un sistema a pannello solare, nel caso in cui,
 questo non riesca a soddisfare la temperatura impostata 
 dall'utente.
 Il sistema è costituito da due display per la visualizzazione
 della temperatura ed eventuali errori, da tre led, il primo verde connesso
 direttamente ai 5V per visualizza la presenza di rete, il secondo
 per visualizzazione l'attivazione automatica o antigelo ed 
 infine uno rosso per la visualizzazione della resistenza 
 attiva.
 Il sistema, quando non è in automatico si porta in antigelo,
 facendo lampeggare il led automatico e attivando la 
 resistenza, quando la temperatua va sotto dei 5°C 
 ed esce quando la temperatura supera i 10°C. 

*/

#INCLUDE <16F870.h>
#DEVICE PIC16F870 *=8
#USE DELAY (clock=4000000)
#FUSES XT, PROTECT, PUT, BROWNOUT, NOLVP, WDT
#USE FAST_IO(C)
#USE FAST_IO(B)
#USE FAST_IO(A)

// Allocazioni RAM  //

#BYTE ADCON0=0x1F
#BYTE ADCON1=0x9F
#BYTE ADRESH=0x1E
#BYTE PORTC =0X07
#BYTE PORTB =0X06
#BYTE PORTA =0X05
#BYTE TMR0 = 0x01
#BYTE STATUS=0x03
#BYTE INDF=0x00
#BYTE FSR=0x04
#BYTE TXREG=0x19

// DEFINIZIONI VARIABILI //

int FLAG_1 = 0;				/* Byte di stato */
int FLAG_2 = 0;				/* Byte di stato */
int BCD = 0;				/* Byte conversione BCD */
int scansione = 0;			/* Byte scansione display */
int digit_1 = 0;			/* Byte informazioni display N°1 */
int digit_2 = 0;			/* Byte informazioni display N°2 */
int decimale = 0;			/* Byte conversione BCD */
int display_hex = 0;			/* Byte per visualizzazione su display */
int time1 = 0;				/* Byte base tempi */
int time2 = 0;				/* Byte base tempi */
int time3 = 0;				/* Byte base tempi */
int time4 = 0;				/* Byte base tempi */
int tempo = 0;				/* Byte base tempi */
int tempo1 = 0;				/* Byte base tempi */
int tempo2 = 0;				/* Byte base tempi */
int tasti = 0;				/* Byte immagine dei tasti */
int filtro ;				/* Byte filtro pulsanti */
int ripetizione;			/* Byte ripetizione pulsanti */
int temporaneo;				/* Byte temporaneo */			
int display_1=0;			/* Byte cifra display */
int display_2=0;			/* Byte cifra display */			
int menu = 0;				/* Byte indice menù */
int sonda_pannello = 0;			/* Byte A/D sonda pannello filtrata */
int imp_temp_pan;			/* Byte impostazione temperatura pannello */
int temp_pannello = 0;			/* Byte temperatura pannello */
int temp_pannello_c = 0;		/* Byte copia temperatura pannello rinfresca ogni secondo per visualizzazione*/
int t_v_p = 0;				/* Byte contatore tempo visualizzazione pricipale */
int test_display = 0;			/* Byte contatore test display */
int tempo_blink = 0;			/* Byte contatore lampeggio */
int fil_res = 0; 			/* Contatore filtro resistenza */
int fil_visual_ant = 0;			/* Contatore filtro visualizzazione antigelo */

// DEFINIZIONE DEI BIT  //

#BIT era_premuto = FLAG_1.0		/* Bit filtro pulsanti */
#BIT premuto = FLAG_1.1			/* Bit filtro pulsanti */		
#BIT premuto_f = FLAG_1.2		/* Bit filtro pulsanti */			
#BIT blink = FLAG_1.3			/* Bit lampeggio ogni 500mS */
#BIT bit_abilita = FLAG_1.4
#BIT bit_antigelo = FLAG_1.5		/* Bit antigelo 1 in antigelo */
#BIT bit_resistenza = FLAG_1.6		/* Bit attivazione resistenza */
#BIT bit_visual_antigelo = FLAG_1.7     /* Bit ritardo visualizzazione antigelo */

#BIT auto_man = FLAG_2.0		/* Bit manuale o automatico a 1 in automatico */
//#BIT          = FLAG_2.1		/* Non usare file scritto su EEPROM */
//#BIT          = FLAG_2.2		/* Non usare file scritto su EEPROM */
//#BIT          = FLAG_2.3		/* Non usare file scritto su EEPROM */
//#BIT          = FLAG_2.4		/* Non usare file scritto su EEPROM */
//#BIT          = FLAG_2.5		/* Non usare file scritto su EEPROM */
//#BIT          = FLAG_2.6		/* Non usare file scritto su EEPROM */
//#BIT          = FLAG_2.7		/* Non usare file scritto su EEPROM */

#BIT resistenza = PORTC.3		/* Bit attivazione e disattivazione resistenza */
#BIT led_res = PORTA.1			/* Bit abilitazione led resistenza accesa */
#BIT led_auto_man = PORTA.2		/* Bit abilitazione led automatico/manuale */

// DEFINIZIONE DELLA COSTANTI//

Const int temp_vis_princ = 50;  	/* Tempo che passa dalla visualizzazione della temperatura impostata a quella letta */
Const int val_filtro = 40;	 	/* Tempo filtro pulsanti */ 
Const int val_ripetizione = 40;		/* Tempo filtro pulsanti */
Const int tempo_test_display = 20;	/* Tempo verifica segmenti display 10=1S */
Const int isteresi = 5;			/* Isteresi accensione e spegnimento resistenza */
Const int f_imp_temp_pan = 40;		/* Impostazione di fabbrica della temperatura pannello */
Const int chiave = 0b01101111;		/* Chiave di accesso alla prima scrittura sulla EEPROM */
Const int max_temp_imp = 60;		/* Massima temperatura impostabile */
Const int min_temp_imp = 30;		/* Minima temperatura impostabile */
Const int temp_antigelo = 10;		/* Temperatura antigelo con -5 di isteresi */
Const int filtro_res =20;		/* Fitro attivazione resistenza */
Const int filtro_visual_ant =20;        /* Fitro visualizzazione antigelo */

//*******************************************
// 	        Zona Interrupt
//*******************************************

#INT_TIMER0		// ISR PER BASE TEMPI

void isr_timer0(void) {
		tmr0 = 11;
		time1 = 1;
		++tempo;		
			if(tempo ==5){
			tempo = 0; 
			time2 = 1;
			++tempo1;
			}
				if(tempo1 ==10){
				tempo1 = 0; 
				time3 = 1;
				++tempo2;	
				}
					if(tempo2 ==10){
					tempo2 = 0; 
					time4 = 1;	
					}

}
//*******************************************
// 	     Fine zona Interrupt
//*******************************************

void tastiera(void);			/* Definizione routine tastiera */

void BIN_BCD(void);			/* Definizione routine conversione BCD */

void scansione_display(void);		/* Definizione routine scansione display */

void init_micro(void);			/* Definizione routine inizializzazione micro */

void filtro_pulsanti(void);		/* Definizione routine filtro pulsanti */

visualizzazione_display(void);		/* Definizione routine visualizzazione display */

void visualizzazione_menu(void);	/* Definizione routine visualizzazione menù */

void service_AD(void);			/* Definizione routine conversione A/D */

void richiesta(void);			/* Definzione routine richiesta accensione resistenza */

void lettura_eeprom(void);		/* Definizione routine lettura EEPROM */

void scrittura_eeprom(void);		/* Definizione routine scrittura EEPROM */

void errore(void);			/* Definizione routine visualizzazione 99,00 e E1 blink */

void visualizzazione_led(void);		/* Definizione routine visualizzazione led */

void utility(void);			/* Definizione routine generica */ 				

//###########################################
//		INIZIO PROGRAMMA
//###########################################

#ZERO_RAM

void main(void) {


init_micro();				/* Chiamata per inizializzazione registri */
lettura_eeprom();			/* Chiamata lettura EEPROM */


while(1) {				/* Loop principale */

if(time1){ 
time1=0;
//*******************************************
//		Uscita 2mS
//*******************************************

restart_wdt();				/* Reset  watchdog */
filtro_pulsanti();			/* Routine filtro pulsanti */
visualizzazione_menu();			/* Routine menù */
service_AD();				/* Routine lettura A/D */

if(!(menu==0)){
	    tastiera();		        /* Routine lettura tasti */
	    visualizzazione_led();      /* Routine visualizzazione led */ 
	    }




if(time2){
time2=0;
//*******************************************
//		Uscita 10mS
//*******************************************







if(time3){
time3=0;
//*******************************************
//		Uscita 100mS
//*******************************************

utility();				/* Routine generica */





if(time4){
time4=0;
//*******************************************
//		Uscita 1S
//*******************************************

temp_pannello_c = temp_pannello;	/* Visualizzazione temperatura su display ogni secondo */

     }
    } 
   }		
  }	
 }
}

//###########################################
//		FINE PROGRAMMA
//###########################################
//*******************************************
//    Zona inizzializzazione registri
//*******************************************

void init_micro(void){

setup_adc_ports(RA0_ANALOG);
setup_adc(ADC_CLOCK_DIV_8);
set_adc_channel(0);
bit_clear(ADCON1,7);

set_tris_C(0b00000111);
set_tris_B(0b00000000);
set_tris_A(0b00000001);

FLAG_1 = 0B00000000;
FLAG_2 = 0B00000000;

PORTC = 0B00000000;
PORTB = 0B00000000;
PORTA = 0B00000000;

setup_wdt (WDT_18MS);

setup_timer_0(RTCC_INTERNAL|RTCC_DIV_8);
set_timer0(205);
enable_interrupts(INT_TIMER0);
enable_interrupts(GLOBAL);

filtro=val_filtro;
ripetizione= val_ripetizione;
imp_temp_pan = min_temp_imp;

}
//*******************************************
//    Fine zona inizzializzazione registri
//*******************************************

//*******************************************
//          Lettura EEPROM
//*******************************************

void lettura_eeprom(void){
	
	temporaneo=read_eeprom (0);
	if(chiave==temporaneo){   
			      imp_temp_pan = read_eeprom (1);
			      FLAG_2 = read_eeprom (2);	
			      }else{   
			      imp_temp_pan = f_imp_temp_pan;
			      FLAG_2 = 0;
			      }          
}

//*******************************************
//        Scrittura EEPROM
//*******************************************

void scrittura_eeprom(void){

int i=0;

	portb = 0B00000000;
	for (i=0;i<3;i++){
			if(i==0)temporaneo=chiave; 
			if(i==1)temporaneo=imp_temp_pan; 
			if(i==2)temporaneo=FLAG_2;
			write_eeprom(i,temporaneo); 				
		         }
}

//*******************************************
//           Inizio zona pulsanti
//*******************************************

void filtro_pulsanti(void){

	temporaneo = (portc & 0b00000111);
	
	if(temporaneo){
			if(era_premuto==false)filtro=val_filtro;
			era_premuto = true;
			--filtro;
			if(filtro==0){
			     	     premuto=true;
			     	     --ripetizione;
			             filtro=1;
			             if(ripetizione==0){ripetizione=val_ripetizione; 
						        premuto=false; 
						        era_premuto=false;
						        filtro=1;
						        }
			             }
	
		      }else{
		           ripetizione=0;	
		           if(era_premuto)filtro=val_filtro;
		           era_premuto = false;
		           --filtro;
		           if(filtro==0){premuto=false, filtro=1;}			
	              }
	
	tasti=temporaneo;				
}
//*******************************************
//           Fine zona pulsanti
//*******************************************

//*******************************************
//   	    Inizio zona display
//*******************************************

void scansione_display(void){
		++scansione;
		switch(scansione){
			case 1:
			display_hex=display_1;
			visualizzazione_display();
			bit_clear(portc,5);
			bit_set(portc,4);			
			break;

			case 2:
			display_hex=display_2;
			visualizzazione_display();
			bit_clear(portc,4);
			bit_set(portc,5);
			scansione=0;	
			break;
			}
	
}
//*******************************************
//           Fine zona display
//*******************************************
//*******************************************
//       Inizio conversione BCD
//*******************************************
void BIN_BCD(void){
			decimale = BCD;
			digit_1=0;
			digit_2=0;
			decimale=decimale+246;
				while(bit_test(status,0)){
					++digit_1;
					decimale=decimale+246;
					}
		digit_2=decimale+10;

}

//*******************************************
//       Fine zona conversione BCD
//*******************************************

//*******************************************
//           Inizio zona tasti
//*******************************************

void tastiera(void){

	if(premuto){	
		if(premuto_f==false)
		{
		
		if(tasti==0b00000100){
					tempo_blink=0;
					blink = true;
					menu=2;
					t_v_p=0;
					if(bit_abilita){
							if(imp_temp_pan<max_temp_imp){++imp_temp_pan;}else{imp_temp_pan=max_temp_imp;}
						       }
					bit_abilita = true;
				    }

		if(tasti==0b00000001){
					tempo_blink=0;
					blink = true;
					menu=2;
					t_v_p=0;
					if(bit_abilita){
							if(imp_temp_pan>min_temp_imp){--imp_temp_pan;}else{imp_temp_pan=min_temp_imp;}
						       }
				        bit_abilita = true;				    
				    }

		if(tasti==0b00000010){
					ripetizione = 0;
					if(auto_man){auto_man=false;led_auto_man=false;}else{auto_man=true;led_auto_man=true;}		
				        scrittura_eeprom();
				     }
		
		premuto_f=true;
		}
	}else{ 
	premuto_f=false;	
	}
}
//*******************************************
//           Fine zona tasti
//*******************************************

//*******************************************
//    Inizio zona visualizzazione display
//*******************************************

void visualizzazione_display(void){

		switch(display_hex) {
		case 0:portb=(0b11111100); break;		/* Numero 0 */
		case 1:portb=(0b01100000); break;		/* Numero 1 */
		case 2:portb=(0b11011010); break;		/* Numero 2 */
		case 3:portb=(0b11110010); break;		/* Numero 3 */
		case 4:portb=(0b01100110); break;		/* Numero 4 */
		case 5:portb=(0b10110110); break;       	/* Numero 5 */
		case 6:portb=(0b10111110); break;		/* Numero 6 */
		case 7:portb=(0b11100000); break;		/* Numero 7 */
		case 8:portb=(0b11111110); break;		/* Numero 8 */
		case 9:portb=(0b11110110); break;		/* Numero 9 */
		case 10:portb=(0b11101110); break;    		/* Lettera A */
		case 11:portb=(0b00111110); break;    		/* Lettera b */
		case 12:portb=(0b10011100); break;		/* Lettera C */
		case 13:portb=(0b01111010); break;		/* Lettera d */
		case 14:portb=(0b10011110); break;		/* Lettera E*/
		case 15:portb=(0b11111110); break;		/* Display tutto acceso */		
		}
}
//*******************************************
//    Fine zona visualizzazione display
//*******************************************

//*******************************************
// Inizio zona visualizzazione vari menù
//*******************************************

void visualizzazione_menu(void){	

	switch(menu) {
								/* TEST DISPLAY */
		case 0:display_1=15;				/* Accensione display 1 */			
		       display_2=15;				/* Accensione display 2 */
		       led_res = true;				/* Accensione led resistenza */			
                       led_auto_man = true;			/* Accensione led automatico */
		       bit_resistenza = false;			/* Spegnimento resistenza */	
		       bit_antigelo = false;			/* Disattiva antigelo */
		       scansione_display();			/* Scansione display_1 e_2 dui display */
		       break;
								/* VISUALIZZAZIONE TEMPERATURA PANNELLO SOLARE */
		case 1:errore();				/* Routine per verificare se 99,00 e E1 */
		       if(sonda_pannello>=18){richiesta();}	/* Routine per eventuale richiesta se non c'è sonda interrotta */ 				       							 		
		       BCD=temp_pannello_c;			/* Preparazione per visualizzare della temperatura pannello*/			
		       BIN_BCD();				/* Routine conversione BCD */
		       display_1=digit_2;			/* Riporto del valore convertito per scansione display */	
		       display_2=digit_1;			/* Riporto del valore convertito per scansione display */
		       scansione_display();			/* Routine visualizzazione e scansione display */	
		       break;
								/* VISUALIZZAZIONE TEMPERATURA IMPOSTATA */
		case 2:if(sonda_pannello>=18){richiesta();}	/* Routine per eventuale richiesta se non c'è sonda interrotta */  						      			
		       if(sonda_pannello<=15){  		/* Se durante l'impostazione andiamo in sonda interrotta viene disattivata la resistenza */
					      bit_resistenza=false;
					      bit_antigelo = false;
					     }
		       BCD=imp_temp_pan;			/* Preparazione per visualizzare della temperatura impostata*/
		       BIN_BCD();				/* Routine conversione BCD */
		       display_1=digit_2;			/* Riporto del valore convertito per scansione display */	
		       display_2=digit_1;			/* Riporto del valore convertito per scansione display */
		       if(blink){scansione_display();		/* Visualizzazione lampeggiante della tempeartura impostata */
					 }else{
					 portb=0b000000000;
					 } 
		       break;
								/* VISUALIZZAZIONE E1 PER SONDA INTERROTTA*/			
		case 3:errore();				/* Routine per verificare se 99,00 e E1 */		
		       display_1=1;				/* Impostazione display_1 con il numero 1 */							
		       display_2=14;				/* Impostazione display_2 con la lettera E */
		       bit_antigelo = false;			/* Disattivazione antigelo */			
                       bit_resistenza = false;			/* Disattivazione resistenza */			
		       if (sonda_pannello>=18)menu=1;		/* Se sonda pannello >=18 torna nella visualizzazione della tempeartura pannello */
		       		if(blink){scansione_display();  /* Visualizzazione lampeggiante di E1 */
					 }else{
					 portb=0b000000000;
					 } 
		       break;
								/* VISUALIZZAZIONE 00 QUANDO LA TEMPERATURA PANNELLO VA SOTTO 0°C*/
		case 4:errore();				/* Routine per verificare se 99,00 e E1 */				
		       richiesta();				/* Routine per eventuale richiesta */				
		       display_1=0;				/* Impostazione display_1 con il numero 0 */				
		       display_2=0;				/* Impostazione display_1 con il numero 0 */
		       if(sonda_pannello>=25)menu=1;		/* Se sonda pannello >=25 torna nella visualizzazione della tempeartura pannello */
				if(blink){scansione_display();	/* Visualizzazione lampeggiante di 00 */
					 }else{
					 portb=0b000000000;
					 }
			break; 
								/* VISUALIZZAZIONE 99 QUANDO LA TEMPERATURA PANNELLO VA SOPRA I 99°C*/
		case 5:errore();				/* Routine per verificare se 00 e E1 */					
		       richiesta();				/* Routine per eventuale richiesta */						       
		       display_1=9;				/* Impostazione display_1 con il numero 9 */			
		       display_2=9;				/* Impostazione display_1 con il numero 9 */		      
		       if(sonda_pannello<=191)menu=1;		/* Se sonda pannello <=191 torna nella visualizzazione della tempeartura pannello */		
				if(blink){scansione_display();	/* Visualizzazione lampeggiante di 99 */
					 }else{
					 portb=0b000000000;
					 }
			break; 				
		
		}
}

//*******************************************
// Fine zona visualizzazione vari menù
//*******************************************

//*******************************************
//       Inizio zona conversione A/D
//*******************************************

void service_AD(void){
	
	bit_set(ADCON0,2);
	while(bit_test(ADCON0,2));
	if(ADRESH>sonda_pannello)++sonda_pannello;
	if(ADRESH<sonda_pannello)--sonda_pannello;
	if(sonda_pannello>254)sonda_pannello=255;
	if(sonda_pannello<=24)temp_pannello = 0;
	if((sonda_pannello>=25)&&(sonda_pannello<=61)) temp_pannello=(((sonda_pannello-23)*2)/3);
	if((sonda_pannello>=62)&&(sonda_pannello<=149)) temp_pannello=((sonda_pannello-11)/2);
	if((sonda_pannello>=150)&&(sonda_pannello<=191)) temp_pannello=(((long)(sonda_pannello-44)*2)/3);	
	if(sonda_pannello>=192){temp_pannello = 99;}
			      
}
//*******************************************
//       Fine zona conversione A/D
//*******************************************
//*******************************************
//         Attivazione resistenza		
//*******************************************

void richiesta(void){

		if(auto_man){
		      	
			    bit_antigelo=false;
			    if(temp_pannello>=imp_temp_pan){bit_resistenza=false;}		      
		      	    if(temp_pannello<=(imp_temp_pan-isteresi)){bit_resistenza=true;}	
		      		         
		            }else{

			    if(!(bit_antigelo)){bit_resistenza=false;}
 			    if(temp_pannello>=temp_antigelo){bit_resistenza=false;bit_antigelo=false;}			      		      		  
		            if(temp_pannello<=(temp_antigelo-isteresi)){bit_resistenza=true;bit_antigelo=true;}

		            } 
		       
		
}

//*******************************************
//       Fine zona attivazione resistenza
//*******************************************

//*******************************************
// Zona controllo errore o limitazione temp.
//*******************************************

void errore(void){

	if(sonda_pannello<=15){menu=3;}						/* Se vero al menù E1 */
			
	if((sonda_pannello>=18)&&(sonda_pannello<=24)){menu=4;}			/* Se vero al menù 00 lampeggiante */

	if(sonda_pannello>=192)menu=5;						/* Se vero al menù 99 lampeggiante */						

	if((sonda_pannello>=16)&&(sonda_pannello<=17)&&(menu==1)) menu=3;	
	
}

//*******************************************
// Fine zona controllo errore o limit. temp.
//*******************************************

//*******************************************
//        Zona visualizzazione led
//*******************************************

void visualizzazione_led(void){

if(auto_man){
	    led_auto_man = true;
	    }else{ 
	    if(bit_visual_antigelo){ 
			    if(blink){
			  	     led_auto_man=true;
			  	     }else{
				     led_auto_man=false;
				     }  
	    		    }else{
			    led_auto_man=false;
			    } 

	    }

if(resistenza){led_res=true;}else{led_res=false;}

}

//*******************************************
// Utilità lampeggio, test display e menù 
//*******************************************

void utility(void){

if(menu==0){
	   if(test_display<tempo_test_display){++test_display;}else{test_display=0;menu=1;}
	   }

if(menu==2){
	   if(t_v_p<temp_vis_princ){++t_v_p;}else{t_v_p=0; menu=1;scrittura_eeprom();bit_abilita = false;}
	   }

if(tempo_blink<4){++tempo_blink;
		  }else{
		  tempo_blink=0;
		  if(blink){blink=false;}else{blink=true;}	
		  }


if(bit_resistenza){
		  if(fil_res<filtro_res){++fil_res;}else{resistenza=true;}

		  }else{
		  fil_res = 0;
		  resistenza = false;

		  }

if(bit_antigelo){
		  if(fil_visual_ant<filtro_visual_ant){++fil_visual_ant;}else{bit_visual_antigelo=true;}

		  }else{
		  fil_visual_ant = 0;
		  bit_visual_antigelo = false;

		  }

}

//*******************************************
// Fine utilità lamp., test display e menù 
//*******************************************
