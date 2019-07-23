#INCLUDE <16F870.h>
#DEVICE *=16
#IGNORE_WARNINGS 203,202
#USE DELAY (clock=4000000)
#FUSES 	XT, WDT, NOPROTECT, PUT, NODEBUG, BROWNOUT, NOLVP, NOCPD, NOWRT
//#USE rs232(baud=9600, xmit=PIN_C6,rcv=PIN_C7)
#USE FAST_IO(C)
#USE FAST_IO(B)
#USE FAST_IO(A)

//##########################################
//          DEFINIZIONE VARIABILI          #
//##########################################

short time1,time2,time3,time4;
int tempo2,tempo3,tempo4;

int TIME_SALVATAGGIO;

int UNITA, DECINE, VIS, TIME_VIS;
short SW_DISP;

short premuto, era_premuto, premuto_f;
int temporaneo, filtro, ripetizione, tasti;

int CONT_SINC, TIME_INNESCO;
int DATI_MEMO[5];


#BYTE PORTA = 0x05
#BYTE PORTB = 0X06	
#BYTE PORTC = 0x07
#BYTE ADCON0 = 0x1f
#BYTE ADCON1 = 0x9f
#BYTE ADRESH = 0x1e
#BYTE ADRESL = 0x9e
#BYTE TXREG = 0X19
#BYTE RCREG = 0X1A
#BYTE FSR   = 0X04
#BYTE INDF  = 0X00
#BYTE RCSTA = 0X18
#BYTE TXSTA = 0X98
#BYTE SPBRG = 0X99


#BIT DISP_UNITA = PORTC.4
#BIT DISP_DECINE = PORTC.5
#BIT SINC = PORTA.5
#BIT TRIAC = PORTC.3
#BIT LED_1 = PORTA.1
#BIT LED_2 = PORTA.2


#define CHIAVE_PRIMA_ACCENSIONE DATI_MEMO[0]
#define POTENZA					DATI_MEMO[1]
#define STEP					DATI_MEMO[2]
#define MINIMO					DATI_MEMO[3]
#define ON_OFF					DATI_MEMO[4]


#SEPARATE
void INIT_MICRO(void);
#SEPARATE
void GESTIONE_DISPLAY(int var);
#SEPARATE
void SET_DISPLAY(int numero);
#SEPARATE
void FILTRO_PULSANTI(void);
#SEPARATE
void TASTIERA(void);
#SEPARATE
void SALVA_DATI(void);
#SEPARATE
void LEGGI_DATI(void);
#SEPARATE
void PRE_SET(void);

//##########################################
//         INIZIO ZONA INTERRUPT           #
//##########################################




//******************************************
//            Interrupt TX RS232           *
//******************************************
#INT_RDA
void isr_rda(void){


}


//******************************************
//    Interrupt TMR0 8mS per base tempi    *
//******************************************

#INT_TIMER0		                                                                                               
void isr_timer0(void) {

if(ON_OFF){
		   TRIAC = 0;
		   ++CONT_SINC;
		   if(!SINC)CONT_SINC = 0;
		   if(CONT_SINC == TIME_INNESCO)TRIAC = 1;
		  }


		   			  set_timer0(101);
			          time1 = true;
			          ++tempo2;			             
				      if(tempo2 == 5){
				                      tempo2 = 0; 
				                      time2 = true;
				                      ++tempo3;	
				                      }
					                  if(tempo3 == 100){
					                                  tempo3 = 0; 
					                                  time3 = true;
													  ++tempo4;				
												      }
													  if(tempo4 == 10){
					                                 			  	  tempo4 = 0; 
					                                 			 	  time4 = true;				
												     				  }
}

//##########################################
//		              MAIN                 #
//##########################################

#ZERO_RAM

void main(void) { 

INIT_MICRO();
PRE_SET();

while(1) {

		                                                                                              
if(time1){ 
time1=false;
//******************************************
//		         Uscita 200 uS             *
//******************************************

restart_wdt();







if(time2){
time2=false;
//******************************************
//		           Uscita 1mS              *
//******************************************

FILTRO_PULSANTI();


TASTIERA();


switch(VIS){
			case 0: 
				   GESTIONE_DISPLAY(6-POTENZA); 
				   TIME_INNESCO = MINIMO +((POTENZA*STEP)-STEP);
			break;	

			case 1: 
				   GESTIONE_DISPLAY(MINIMO); 
				   TIME_INNESCO = MINIMO;
			break;

			case 2: 
				   GESTIONE_DISPLAY(STEP); 
			break;
		   }




if(time3){
time3=false;
//******************************************
//		           Uscita 100mS            *
//******************************************











if(time4){
time4=false;
//******************************************
//		           Uscita 1S               *
//******************************************



if(VIS){
		++TIME_VIS;
		if(TIME_VIS >= 60){VIS = 0; LED_1 = 0; LED_2 = 0;}
	   }else{
		TIME_VIS = 0;
	   }





if(TIME_SALVATAGGIO){
					 --TIME_SALVATAGGIO;
					 if(TIME_SALVATAGGIO == 1){
											   SALVA_DATI();
											  }
					}






	 }
    }
   }
  }  
 }
}

//###########################################
//		         END MAIN                   #
//###########################################


//###########################################
//         INIZIALIZZAZIONI MICRO           #
//###########################################

#SEPARATE
void INIT_MICRO(void){

setup_timer_0(RTCC_INTERNAL|RTCC_DIV_1);
set_timer0(10);
enable_interrupts(INT_TIMER0);
enable_interrupts(GLOBAL);
//enable_interrupts(INT_RDA);

set_tris_A(0b111001);
set_tris_B(0b00000000);
set_tris_C(0b11000111);
PORTA = 0;
PORTB = 0;
PORTC = 0;

setup_adc_ports(NO_ANALOGS);

//setup_adc(ADC_CLOCK_DIV_8);
//set_adc_channel(0);
//bit_clear(ADCON1,7);

}

//******************************************************************************************************************************

#SEPARATE
void GESTIONE_DISPLAY(int var){

if(ON_OFF){
			UNITA = var%10;
			DECINE = var/10;

			if(SW_DISP){
						SW_DISP = 0;
						DISP_DECINE = 0;
						SET_DISPLAY(UNITA);
						DISP_UNITA = 1;
					   }else{
						SW_DISP = 1;
						DISP_UNITA = 0;
						SET_DISPLAY(DECINE);
						DISP_DECINE = 1;
					   }
		  }else{
		   portb=(0b00000000);
		   DISP_UNITA = 0;
		   DISP_DECINE = 0;
		  }

}

//******************************************************************************************************************************

#SEPARATE
void SET_DISPLAY(int numero){

switch(numero){
				case 0:portb=(0b11111100); break;		/* Numero 0 */
				case 1:portb=(0b01100000); break;		/* Numero 1 */
				case 2:portb=(0b11011010); break;		/* Numero 2 */
				case 3:portb=(0b11110010); break;		/* Numero 3 */
				case 4:portb=(0b01100110); break;		/* Numero 4 */
				case 5:portb=(0b10110110); break;       /* Numero 5 */
				case 6:portb=(0b10111110); break;		/* Numero 6 */
				case 7:portb=(0b11100000); break;		/* Numero 7 */
				case 8:portb=(0b11111110); break;		/* Numero 8 */
				case 9:portb=(0b11110110); break;		/* Numero 9 */
				case 10:portb=(0b11101110); break;    	/* Lettera A */
				case 11:portb=(0b00111110); break;    	/* Lettera b */
				case 12:portb=(0b10011100); break;		/* Lettera C */
				case 13:portb=(0b01111010); break;		/* Lettera d */
				case 14:portb=(0b10011110); break;		/* Lettera E*/
				case 15:portb=(0b11111110); break;		/* Display tutto acceso */		
			 }
}

//******************************************************************************************************************************

#SEPARATE
void FILTRO_PULSANTI(void){

	temporaneo = (portc & 0b00000111);
	
	if(temporaneo){
			if(era_premuto == 0)filtro = 80;
			era_premuto = 1;
			--filtro;
			if(filtro == 0){
			     	     premuto = 1;
			     	     --ripetizione;
			             filtro = 1;
			             if(ripetizione == 0){
											  ripetizione = 40; 
									          premuto = 0; 
									          era_premuto = 0;
									          filtro = 1;
									         }
			             }
	
		      }else{
		           ripetizione = 0;	
		           if(era_premuto)filtro = 80;
		           era_premuto = 0;
		           --filtro;
		           if(filtro == 0){premuto = 0, filtro = 1;}			
	              }
	
	tasti = temporaneo;				
}

//******************************************************************************************************************************

#SEPARATE
void TASTIERA(void){

	if(premuto){	
		        if(premuto_f==false){
		
									if((tasti==0b00000100)&&(ON_OFF)){

																	  TIME_VIS = 0;
			
																	  if(VIS == 0){
																				   if(POTENZA > 1)--POTENZA;
																				  }
			
																	  else if(VIS == 1){
																						if(MINIMO < 52)++MINIMO;
																					   }
			
																	  else if(VIS == 2){
																						if(STEP < 5)++STEP;
																					   }
																	
														    		 }
							
									else if((tasti==0b00000001)&&(ON_OFF)){

																		   TIME_VIS = 0;
			
																		   if(VIS == 0){
																				        if(POTENZA < 5)++POTENZA;
																					   }
				
																		   else if(VIS == 1){
																							 if(MINIMO > 0)--MINIMO;
																						    }
				
																		   else if(VIS == 2){
																							 if(STEP > 1)--STEP;
																						    }
			
														    		 	  }
							
									else if((tasti==0b00000010)&&(!VIS)){

																	     TIME_VIS = 0;
																	     ON_OFF = ~ON_OFF;
																	     ripetizione = 0;

													     		 	    }


									else if((tasti==0b00000101)&&(ON_OFF)){
																		   if(VIS == 0){
																						VIS = 1;
																						LED_1 = 1;
																						LED_2 = 0;
																					   }
			
																		   else if(VIS == 1){
																						     VIS = 2;
																							 LED_1 = 0;
																							 LED_2 = 1;
																					   		}
			
																		   else if(VIS == 2){
																						     VIS = 0;
																							 LED_1 = 0;
																							 LED_2 = 0;
																					   		}
																		  }
									
									premuto_f=true;

									TIME_SALVATAGGIO = 11;
									}
	}else{ 
	premuto_f=false;	
	}

}

//******************************************************************************************************************************

#SEPARATE
void SALVA_DATI(void){
int n_memo;

for(n_memo = 0; n_memo < 5; n_memo++){
								   	   write_eeprom (n_memo,DATI_MEMO[n_memo]);
									   restart_wdt();
									  }
}

//------------------------------------------------------------------------------------------------------------

#SEPARATE
void LEGGI_DATI(void){
int n_memo;

for(n_memo = 1; n_memo < 5; n_memo++){
									   DATI_MEMO[n_memo] = read_EEPROM (n_memo);
									   restart_wdt();
									  }
}

//------------------------------------------------------------------------------------------------------------

#SEPARATE
void PRE_SET(void){

CHIAVE_PRIMA_ACCENSIONE = read_EEPROM (0);

if(CHIAVE_PRIMA_ACCENSIONE == 170){
								   LEGGI_DATI();
								  }else{
								   CHIAVE_PRIMA_ACCENSIONE = 170;
								   POTENZA = 5;
								   STEP = 3;
								   MINIMO = 25;
								   ON_OFF = 0;

								   SALVA_DATI();
								  }


}

//------------------------------------------------------------------------------------------------------------