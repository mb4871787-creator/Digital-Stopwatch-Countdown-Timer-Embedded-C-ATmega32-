#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

volatile unsigned char sec_units = 0, sec_tens = 0;
volatile unsigned char min_units = 0, min_tens = 0;
volatile unsigned char hr_units  = 0, hr_tens  = 0;

volatile unsigned char mode = 0;          // 0=up, 1=down
volatile unsigned char pause_flag = 0;

unsigned char digits[6];

// one-shot flags for buttons
unsigned char flag_pb0 = 0;   // Hour -
unsigned char flag_pb1 = 0;   // Hour +
unsigned char flag_pb3 = 0;   // Min -
unsigned char flag_pb4 = 0;   // Min +
unsigned char flag_pb5 = 0;   // Sec -
unsigned char flag_pb6 = 0;   // Sec +
unsigned char flag_pb7 = 0;   // Mode toggle

/* ---------- Timer1: CTC, 1 Hz ---------- */
void Timer1_Init_ctc_Mode(void){
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;
    OCR1A  = 15624;  // 1 ثانية بدقة
    TIMSK |= (1<<OCIE1A);
    TCCR1B = (1<<WGM12) | (1<<CS12) | (1<<CS10);
}

/* ---------- INT0: Reset (PD2) ---------- */
void INT0_Init(void){
    DDRD  &= ~(1<<PD2);
    PORTD |=  (1<<PD2);
    MCUCR |=  (1<<ISC01);
    GICR  |=  (1<<INT0);
}
ISR(INT0_vect){
    sec_units=sec_tens=min_units=min_tens=hr_units=hr_tens=0;
    PORTD &= ~(1<<PD0);  // بuzzer OFF
}

/* ---------- INT1: Pause (PD3) ---------- */
void INT1_Init(void){
    DDRD  &= ~(1<<PD3);
    PORTD |=  (1<<PD3);
    MCUCR |=  (1<<ISC11);
    MCUCR &= ~(1<<ISC10);
    GICR  |=  (1<<INT1);
}
ISR(INT1_vect){
    pause_flag = 1;
}

/* ---------- INT2: Resume (PB2) ---------- */
void INT2_Init(void){
    DDRB   &= ~(1<<PB2);
    PORTB  |=  (1<<PB2);
    MCUCSR &= ~(1<<ISC2);
    GICR   |=  (1<<INT2);
}
ISR(INT2_vect){
    pause_flag = 0;
}

/* ---------- 1 Hz tick ---------- */
ISR(TIMER1_COMPA_vect){
    if(pause_flag) return;

    if(mode == 0){
        PORTD &= ~(1<<PD0);  // البزر OFF في وضع Stopwatch

        if(++sec_units == 10){ sec_units=0; sec_tens++; }
        if(sec_tens == 6){ sec_tens=0; min_units++; }
        if(min_units == 10){ min_units=0; min_tens++; }
        if(min_tens == 6){ min_tens=0; hr_units++; }
        if(hr_units == 10){ hr_units=0; hr_tens++; }
        if(hr_tens==2 && hr_units==4){ hr_tens=0; hr_units=0; }
    } else {
        if(sec_units>0){ sec_units--; }
        else if(sec_tens>0){ sec_tens--; sec_units=9; }
        else if(min_units>0){ min_units--; sec_tens=5; sec_units=9; }
        else if(min_tens>0){ min_tens--; min_units=9; sec_tens=5; sec_units=9; }
        else if(hr_units>0 || hr_tens>0){
            if(hr_units>0) hr_units--;
            else { hr_units=9; hr_tens--; }
            min_tens=5; min_units=9; sec_tens=5; sec_units=9;
        } else {
            PORTD |= (1<<PD0);  // البزر ON لما الوقت يخلص
        }
    }
}

/* ---------- Display ---------- */
static inline void update_digits(void){
    digits[0] = sec_units;
    digits[1] = sec_tens;
    digits[2] = min_units;
    digits[3] = min_tens;
    digits[4] = hr_units;
    digits[5] = hr_tens;
}

static inline void display_cycle_once(void){
    for(unsigned char i=0; i<6; i++){
        PORTA = (1<<i);
        PORTC = (PORTC & 0xF0) | (digits[i] & 0x0F);
        _delay_us(1000);
        PORTA = 0x00;
    }
}

int main(void){
    DDRC |= 0x0F;   PORTC &= 0xF0;
    DDRA |= 0x3F;   PORTA  = 0x00;
    DDRD |= (1<<PD0)|(1<<PD4)|(1<<PD5);
    DDRB &= ~0xFF;  PORTB  = 0xFF;

    PORTD &= ~(1<<PD0);
    PORTD |=  (1<<PD4);
    PORTD &= ~(1<<PD5);

    Timer1_Init_ctc_Mode();
    INT0_Init();
    INT1_Init();
    INT2_Init();

    sei();

    while(1){
        update_digits();
        display_cycle_once();

        /* Mode toggle */
        if(!(PINB & (1<<PB7))){
            if(!flag_pb7){ mode ^= 1; flag_pb7 = 1; }
        } else flag_pb7 = 0;

        /* LEDs */
        if(mode==0){ PORTD |= (1<<PD4); PORTD &= ~(1<<PD5); }
        else        { PORTD |= (1<<PD5); PORTD &= ~(1<<PD4); }

        /* Manual time adjust (pause only) */
        if(pause_flag){
            // Hour +
            if(!(PINB & (1<<PB1))){
                if(!flag_pb1){
                    if(++hr_units>9){ hr_units=0; ++hr_tens; }
                    if(hr_tens>2 || (hr_tens==2 && hr_units>3)){ hr_tens=0; hr_units=0; }
                    flag_pb1=1;
                }
            } else flag_pb1=0;

            // Hour -
            if(!(PINB & (1<<PB0))){
                if(!flag_pb0){
                    if(hr_units==0 && hr_tens==0){ hr_tens=2; hr_units=3; }
                    else if(hr_units==0){ hr_units=9; --hr_tens; }
                    else --hr_units;
                    flag_pb0=1;
                }
            } else flag_pb0=0;

            // Minute +
            if(!(PINB & (1<<PB4))){
                if(!flag_pb4){
                    if(++min_units>9){ min_units=0; ++min_tens; }
                    if(min_tens>5){ min_tens=0; }
                    flag_pb4=1;
                }
            } else flag_pb4=0;

            // Minute -
            if(!(PINB & (1<<PB3))){
                if(!flag_pb3){
                    if(min_units==0 && min_tens==0){ min_tens=5; min_units=9; }
                    else if(min_units==0){ min_units=9; --min_tens; }
                    else --min_units;
                    flag_pb3=1;
                }
            } else flag_pb3=0;

            // Second +
            if(!(PINB & (1<<PB6))){
                if(!flag_pb6){
                    if(++sec_units>9){ sec_units=0; ++sec_tens; }
                    if(sec_tens>5){ sec_tens=0; }
                    flag_pb6=1;
                }
            } else flag_pb6=0;

            // Second -
            if(!(PINB & (1<<PB5))){
                if(!flag_pb5){
                    if(sec_units==0 && sec_tens==0){ sec_tens=5; sec_units=9; }
                    else if(sec_units==0){ sec_units=9; --sec_tens; }
                    else --sec_units;
                    flag_pb5=1;
                }
            } else flag_pb5=0;
        }
    }
}
