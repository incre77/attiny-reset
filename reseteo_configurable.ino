#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#define RESETPIN          PB0   
#define OPCION1           PB4 
#define OPCION2           PB3 

/* 
PINOUT USADO PARA NUESTRO ATTINY13 
--------------------------------------
PB0  5-  /--4 GND (PIN4 PUENTEADO A PIN6)
GND  6--/  -3 PB4 OPCION1
     7-    -2 PB3 OPCION2 
VCC  8-    -1                       
--------------------------------------
*/


#define WDT_SLEEP_MS      8000UL 

unsigned long tiempos_ms[4] = {
  86400000UL, // 24 HORAS: Sin Jumpers 
  43200000UL, // 12 HORAS: OPCION1
  21600000UL, //  6 HORAS: OPCION2
    60000UL  //   1 MIN : OPCION1 + OPCION2
};

unsigned long tiempo_acumulado_ms = 0;
unsigned long tiempo_reseteo; 

// --- Funciones de Bajo Nivel para WDT Sleep (sin cambios) ---

void setup_wdt() {
  cli(); 
  wdt_reset(); 
    MCUSR &= ~_BV(WDRF);
    WDTCR |= _BV(WDCE) | _BV(WDE); 
    WDTCR = _BV(WDTIE) | _BV(WDP3) | _BV(WDP0);
  sei(); 
}


void sleep_wdt() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); 
  sleep_enable();                      
  sleep_mode(); 
  sleep_disable(); 
}

ISR(WDT_vect) {} // Despertar

// --- Setup y Loop (Lógica Corregida) ---

void setup() {
  int jumpers = 0;
  pinMode(RESETPIN, OUTPUT);
  digitalWrite(RESETPIN, HIGH);       
    
  pinMode(OPCION1, INPUT_PULLUP);
  pinMode(OPCION2, INPUT_PULLUP);

  if (digitalRead(OPCION1) == LOW) { jumpers += 1; }
  if (digitalRead(OPCION2) == LOW) { jumpers += 2; }
  tiempo_reseteo = tiempos_ms[jumpers];

  setup_wdt();
  //muestra_config(jumpers);
}

void muestra_config(int op){
  op = op +1;
  while(op>=1){
    op = op-1;
    digitalWrite(RESETPIN, HIGH); 
    delay(200); 
    digitalWrite(RESETPIN, LOW); 
    delay(200); 
  }
  delay(5000); 
  digitalWrite(RESETPIN, HIGH); 
}

void loop() {
  sleep_wdt();
  
  tiempo_acumulado_ms += WDT_SLEEP_MS; 

  if (tiempo_acumulado_ms >= tiempo_reseteo) {
    digitalWrite(RESETPIN, LOW); 
    delay(200); 
    digitalWrite(RESETPIN, HIGH); 
    tiempo_acumulado_ms = 0; 
  }
}