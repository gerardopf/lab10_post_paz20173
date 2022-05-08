/*
 * Lab10
 * Gerardo Paz Fuentes
 * 
 * Potenciómetro en RA0
 * Terminal conectada en puerto C RX/TX
 * Leds en puerto D para mostrar el ASCII ingresado
 *
 * Fecha de creación: 07/05/22
 */

#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>         // registros del PIC
#include <stdio.h>
#include <stdlib.h>

/* -------------CONSTANTES ------------ */
#define _XTAL_FREQ 1000000
#define LEN_MSG 9       // largo de mensaje

/* ------------- VARIABLES ------------ */
uint8_t res = 0b0;
int op = 0;     // guarda la opción
int pot = 0;
char pot_ascii[] = "";
char input_ascii[] = "";
/* ------------- PROTOTIPOS DE FUNCIÓN ------------ */
void setup(void);
void imprimir(char texto[]);

/* -------------CÓDIGO PRINCIPAL  ------------ */

void __interrupt() isr(void) {
    if(RCIF){   // si se reciben datos
        res = RCREG;
        if(op == 2){
            input_ascii[0] = RCREG;
            TXREG = input_ascii[0];
            PORTD = input_ascii[0];
            imprimir("\n\r\n\rFin...\n\r");
            imprimir("---------------\n\r");
            op = 0;
        }
        else{
            if(res == 49){      // opcion 1, leer Potenciómetro
                itoa(pot_ascii, pot, 10);
                imprimir("\n\r");
                imprimir(pot_ascii);
                imprimir("\n\r\n\rFin...\n\r");
                imprimir("---------------\n\r");
                op = 0;
            }
            if(res == 50){ // opción 2, pedir Ascii
                imprimir("\n\rIngrese el caracter\n\r");
                op = 2;
            }
        }
        
    }
    if(ADIF){
        if(ADCON0bits.CHS == 0){
            pot = ADRESH;   // guardar valor del potenciómetro 0-255
        }
        ADIF = 0;   // limpiar bandera
    }
    return;
}

// PIC - Terminal
void main(void) {
    setup();
    while (1) {
        if(ADCON0bits.GO == 0)
            ADCON0bits.GO = 1;  // iniciar conversión si no hay una
        if(op == 0){
            imprimir("Ingrese una opcion: \n\r1.Leer pot \n\r2.Enviar ascii \n\r");
            op = 1;         // modo espera de respuesta
        }
    }
}

void imprimir(char texto[]){
    int i = 0;
    while(texto[i] != '\0'){
        if(PIR1bits.TXIF){      // si el módulo está libre para recibir datos
            TXREG = texto[i];   // imprimir caracter
            i++;
        }
    }

    return;
}

void setup(void) {
    ANSEL = 0b00000001; // AN0
    ANSELH = 0;

    TRISA = 0b00000001; // AN0 in
    TRISD = 0;  // D out
    
    PORTA = 0;
    PORTD = 0;  // puertos limpios

    // INTERRUPCIONES
    INTCONbits.GIE = 1;     // globales
    INTCONbits.PEIE = 1;    // periféricos
    PIE1bits.RCIE = 1;      // recepción
    PIE1bits.ADIE = 1;      // ADC
    
    PIR1bits.ADIF = 0;      // bandera ADC limpia

    // Oscilador 
    OSCCONbits.IRCF = 0b100;    // 1MHz
    OSCCONbits.SCS = 1;         // Oscilador interno
    
    // COMUNICACIÓN SERIAL
    TXSTAbits.SYNC = 0;     // asíncroma (full duplex)
    TXSTAbits.BRGH = 1;     // Baud rate de alta velocidad
    BAUDCTLbits.BRG16 = 1;  // 16 bits para el Baud rate
    
    SPBRG = 25;
    SPBRGH = 0; // Baud rate 9600, error 0.16%
    
    RCSTAbits.SPEN = 1; // comunicación habilitada
    TXSTAbits.TX9 = 0;  // solo 8 bits
    TXSTAbits.TXEN = 1; // transmisor habilitado
    RCSTAbits.CREN = 1; // receptor habilitado
    
    // ADC
    ADCON0bits.ADCS = 0b00; // Fosc/2
    ADCON1bits.VCFG0 = 0;   // VDD
    ADCON1bits.VCFG1 = 0;   // VSS
    ADCON0bits.CHS = 0;     // AN0
    ADCON1bits.ADFM = 0;    // justificado a la izquierda
    ADCON0bits.ADON = 1;    // ADC habilitado
    __delay_us(40);         // tiempo para carga del capacitor

    return;
}