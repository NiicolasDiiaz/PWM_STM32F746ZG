#include <stdio.h>
#include "STM32F7xx.h"


int dato=100;
int j=0;
char conteo=1;
char bandera=0;
char dato5,dato7;
char envio = 0x31;
int res = 80586;     //(3.3V/4095) (12 bits) -> (resolucion: 805,86uV)  -----> (80586,0 *10^ -8)

char canales[8] = {0x9,0xE,0xF,0x4,0x5,0x6,0x7,0x8};

int dato_adc1=0;
int dato_adc[8];     //(32 bits)

extern "C" {
		void ADC_IRQHandler(void){         //Interrupcion por final de conversion.
		dato_adc[conteo] = ADC3->DR;	   //lee el dato
	}
	
	void SysTick_Handler (void)
		{
		    bandera=1;
		}		
}

void conv_adc(){
	
	ADC3->SQR3 = canales[conteo];     //Define el orden de las conversiones
	ADC3->CR2 |= (1UL << 30);         //inicio conversión (set el bit SWSTART)
	while ((ADC3->SR &= 0x2)==1);     //esperar hasta que EOC sea 1 (termino la conversión canal 1)
	
}


int main(void)
{
	//**********************************************************
	//CONFIGURACION "CLOCK"
  RCC->AHB1ENR =0xFF; //Puertos A,B,C,D,E,F,G,H F4
	
	RCC->APB1ENR |= (1UL << 1);    //HABILITA CLOCK TIM3
	
	//*********************************************************************************************
	
	//CONFIGURACION ADC (en este caso ADC3)
	
	RCC->APB2ENR |= 0x400;   //Enable clock for the ADC3 (set el bit 10 =ADC3EN)

	GPIOF->MODER = 0xFC0;	//configurar los pines como analógicos PF3,PF4,PF5 (analogico = 11)
	
	ADC3->CR1 |= (1UL << 5);   	 //ADC3->CR1 se activa la interrupcion EOCIE (end of convertion interrupt enable)
	ADC3->CR2 |= (1UL << 0);   	//ADC3->CR2 - The ADC is powered on by setting the ADON bit in the ADC_CR2 register
  ADC3->CR2 |= (1UL << 10);   //Set to 1 the bit EOCS (The EOC bit is set in the ADC_SR register:
																											//At the end of each regular channel conversion) 													
	ADC3->SQR3 = canales[0];     //Define el orden de las conversiones
	
  NVIC_EnableIRQ(ADC_IRQn);      //Habilita interrupción del ADC
		
	//**********************************************************
	//CONFIGURACION DE PINES COMO SALIDA
	GPIOB->MODER = 0x10004001;       //PTB0, PTB7 y PTB 14 -> OUTPUT
	GPIOB->OTYPER = 0;               //PUSH PULL -> PTB0, PTB7 y PTB 14
	GPIOB->OSPEEDR = 0x10004001;     //MEDIUM SPEEED -> PTB0, PTB7 y PTB 14
	GPIOB->PUPDR = 0x10004001;       //PULL-UP -> PTB0, PTB7 y PTB 14
	
	//**********************************************************
	//CONFIGURACION DE PINES COMO ENTRADA
	GPIOC->MODER &=  ~(3UL << 2*13); //pulsador como entrada (PC13)
	
	
	//********************************************************************************************
	//CONFIGURACION PTA6 -> TIM3_CH1
	GPIOA->MODER &= ~(3UL<<2*6);     //PTA6 en MODO ALTERNO
	GPIOA->MODER |= (2UL<<2*6);      
	GPIOA->OTYPER = 0;               //PUSH PULL -> PTA6
	GPIOA->OSPEEDR &= ~(3UL<<2*6);     //MEDIUM SPEEED -> PTA6
	GPIOA->OSPEEDR |= (1UL<<2*6);
	GPIOA->PUPDR &= ~(3UL<<2*6);       //PULL-UP -> PTA6
	GPIOA->PUPDR |= (1UL<<2*6);
	GPIOA->AFR[0] = 0x2000000;       //PTA6 funcion alterna  AF2= TIM3_CH1
	
	
	//CONFIGURACION DEL TIM3_CH1
	TIM3->EGR |= (1UL<<0);           //UG = 1 ,  RE-inicializar el contador
	TIM3->PSC = 15;                  //señal de reloj HSI=16Mhz, se necesita generar 1Mhz por lo tanto PSC=15
	TIM3->ARR = 20000;               //con una frecuencia de 1Mhz -> T=1uS :  
	TIM3->DIER |= (1UL<<0);          //UIE = 1, update interrupt enable
	                                 //conteo hasta 20000 significa 20000*1uS = 20ms //periodo de la señal de control del servo
	TIM3->CR1 |= (1UL<<0);           //Enable counter
  TIM3->CCMR1 =	0x60;              //PWM modo 1, preload del CCR1 deshabilitado, CH1 configurado como salida
	TIM3->CCER |= (1UL<<0);          //OC1 signal is output on the corresponding output pin
	TIM3->CCR1 = 100;                //conteo hasta 510 significa 510*1uS = 0,51ms

	//**********************************************************
	
	SystemCoreClockUpdate(); //Actualiza la variable System Core Clock
	SysTick_Config(SystemCoreClock/10); //Inicializa y comienza SystickTimer, además de su interrupción. 
																				//Interrupcion definida cada 1ms.
 	
	while(true){        //bucle infinito
	
		if(bandera==1){  
							conv_adc();
			
		if(dato_adc[conteo]>=621 && dato_adc[conteo]<1240 ){        //evalua si se oprimio el pulsador  
			  
			  dato=5000;
				TIM3->CCR1 = dato;            //Actualiza el valor del PWM
				
			for(int i=0;i<200000;i++);      //anti-rebote por software		
		}
				if(dato_adc[conteo]>=1240 && dato_adc[conteo]<1861){        //evalua si se oprimio el pulsador  
			  
			  dato=10000;
				TIM3->CCR1 = dato;            //Actualiza el valor del PWM
				
			for(int i=0;i<200000;i++);      //anti-rebote por software		
		}
								if(dato_adc[conteo]>=1861 && dato_adc[conteo]<2482){        //evalua si se oprimio el pulsador  
			  
			  dato=15000;
				TIM3->CCR1 = dato;            //Actualiza el valor del PWM
				
			for(int i=0;i<200000;i++);      //anti-rebote por software		
		}
																if(dato_adc[conteo]>=2482){        //evalua si se oprimio el pulsador  
			  
			  dato=20000;
				TIM3->CCR1 = dato;            //Actualiza el valor del PWM
				
			for(int i=0;i<200000;i++);      //anti-rebote por software		
		}
                 //enviar dato al PC	
				bandera=0;
				
			}
			
	}//cierra while
	
}//cierra main