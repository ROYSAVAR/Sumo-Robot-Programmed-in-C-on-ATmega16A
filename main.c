
#define  F_CPU 1000000UL
#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "lcd.h"
//PROTOTIPADO DE FUNCIONES PARA PODER UTILIZARLAS DESDE CUALQUIER "LUGAR"
//*************************************************************************
uint8_t cero_en_bit(volatile uint8_t *LUGAR, uint8_t BIT);
uint8_t uno_en_bit(volatile uint8_t *LUGAR, uint8_t BIT);
void timer1_init();
void trigger_pulse();

void inizializar_componentes();

void buscar(); // GIRA HASTA ENCONTRAR ENEMIGO
void esquivar();  // GIRA AL ENCONTRAR LINEA BLANCA
void atacar(); // AVANZA HACIA ADELANTE PARA EMPUJAR 

// DECLARACION VARIABLES GLOBALES 
#define TRIGGER_PIN PA1
#define ECHO_PIN PA0
#define SOUND_SPEED 340.29 // en m/s

// Variables para la medición de tiempo y cálculo de distancia
volatile uint16_t timer_counter = 0;
volatile uint8_t measuring = 0;
volatile float distance = 0.0;

uint16_t distance_cm;
uint32_t duration;

int ya_paso = 0, ya_paso_medio = 0; 
int contador = 0, contador2 = 0; 

int aux1 = 0; 

int main(void)
{
	
	inizializar_componentes();
	
    while (1) 
    {	
		trigger_pulse();
		while(!(PINA & (1 << ECHO_PIN))) {}
		measuring = 1;
		TCNT1 = 0; // Reiniciar contador del timer
		timer_counter = 0;
		
		// Esperar a que Echo pin se ponga en bajo
		while(PINA & (1 << ECHO_PIN)) {}
		measuring = 0;
		
		duration = (timer_counter * 65536UL + TCNT1) * 8; // Duración en microsegundos
		distance_cm = duration / 58; // Distancia en centímetros
		
		ya_paso_medio = 0; 
	 	_delay_ms(100); // Esperar medio segundo para la siguiente medición
			 
	
		if ((distance_cm >= 0.0 && distance_cm < 60.0) || (distance_cm >= 1180.0 && distance_cm < 1210.0))
		{
			aux1 = 1;
		}

		else{ aux1 = 0; }
		
		if(aux1 == 1){	atacar(); }
		else if(aux1 == 0){ buscar(); }
		
    }
}

// ************************************  FUNCIONES ***************************************************

uint8_t cero_en_bit(volatile uint8_t *LUGAR, uint8_t BIT) {	return (!(*LUGAR&(1<<BIT))); }
uint8_t uno_en_bit(volatile uint8_t *LUGAR, uint8_t BIT){ return (*LUGAR&(1<<BIT)); }

void esquivar()
{
	// AL DETECTAR LA LINEA BLANCA GIRAR PARA NO CAERSE		 "OBSTACULO"
	if(cero_en_bit(&PINB, 7))
	{

		ya_paso = 0; 
		
		PORTD = 0b00000000;		// DETENER EL AVANCE HACIA ADELANTE
		
		PORTD = 0b00000101;		// HACERLO PARA ATRAS 2 SEGS
				
		if(ya_paso == 2)				// ESPERAR 2 SEGS
		{
			PORTD = 0b00001001;		// GIRAR 180 GRADOS+
			
			ya_paso = 0; 
			
		} // ESPERAR 2 SEGS
		if(ya_paso == 2)
		{
			PORTD = 0b00000000;		// DETENER EL GIRO
		}
		
	}
	
}

void buscar()
{
	// QUE SE QUEDE GIRANDO 
	PORTD = 0b00001001;		 
}

void atacar()
{
	// AVANZAR HACIA ADELANTE 
	PORTD = 0b00001010;
}

void inizializar_componentes()
{
	
	sei(); // HABILITAR INTERRUPCIONES 
	
	_delay_ms(5000); // MODO SEGURO, ESPERAR 5 SEGS
	
	DDRD = 255; // PUERTO D COMO SALIDA PARA PUENTE H
	
	DDRA |= (1 << TRIGGER_PIN); // PA1 como salida para el Trigger
	DDRA &= ~(1 << ECHO_PIN);   // PA0 como entrada para el Echo
	
	// INICIRALIZAR TIMER0
	TIFR=0b00000011;
	TIMSK = 0b00000010;   // HABILITAR INTERRUPCION POR COMP
	OCR0 = 243;
	
	// INICIRALIZAR TIMER1
	timer1_init();
}

ISR(TIMER0_COMP_vect)
{
	
	// PARA CONTAR DE A SEGUNDO 
	contador++;
	if(contador > 4 )
	{
		contador = 0;
		ya_paso = 1;  // paso un seg
	}
	
	// PARA CONTAR DE MEDIO SEG
	contador2++;
	if(contador > 2)
	{
		contador2 = 0;
		ya_paso_medio = 1 ;  // paso medio seg
	}
}

// Inicialización del Timer1
void timer1_init() {
	TCCR1A = 0; // Configurar como Normal mode
	TCCR1B = 0;
	TCNT1 = 0; // Inicializar contador en 0
	TCCR1B |= (1 << CS11); // Preescaler de 8
	TIMSK |= (1 << TOIE1); // Habilitar interrupción de overflow
	sei(); // Habilitar interrupciones globales
}

// Interrupción por overflow de Timer1 para contar el tiempo que el pin Echo está en alto
ISR(TIMER1_OVF_vect) 
{
	if (measuring) 
	{
		timer_counter++;
	}
}

// Función para iniciar la medición de distancia
void trigger_pulse() 
{
	PORTA |= (1 << TRIGGER_PIN); // Enviar pulso alto
	//_delay_us(10); // Mantener por 10 microsegundos
	PORTA &= ~(1 << TRIGGER_PIN); // Enviar pulso bajo para generar el ultrasonido
}