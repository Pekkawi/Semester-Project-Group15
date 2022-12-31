#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#include "usart.h"
#include <stdbool.h>
#define PI 3.141592659
void setup_octocoupler();
void setup_motor();
void setup_adc0();
unsigned int MotorOnOff();
unsigned int get_time();
unsigned int read_adc();
void display_speed(int);
void display_distance(int);
void display_voltage(unsigned int);
void set_speed_phase_one();
void set_speed_phase_two();
void get_both_times();
unsigned int buffer_reader(char a[10]);
unsigned int time_phase_one=0;
unsigned int time_phase_two=0;
bool speed_setter_one=false;
bool speed_setter_two=false;
int motor_state=0;
float distance_went=0;
int min_time=18;
int main(){
uart_init();
io_redirect();
setup_motor();
setup_adc0();
setup_octocoupler();
while(1){
 unsigned int adc = read_adc();
 if(adc==66||adc==65){
  display_voltage(adc);
  break;
 }
unsigned int intermediate = MotorOnOff();
if(intermediate==1){
 if(motor_state==0)motor_state=1;
}
else if(intermediate==2){
  if(motor_state==1){
  motor_state=0;
 speed_setter_one=false;
    speed_setter_two=false;
    distance_went=0;
  }
}

if(motor_state==1){
  if(speed_setter_one==false){
    speed_setter_one=true;
    set_speed_phase_one();
  }else if(speed_setter_two==false&&distance_went>=100){
    speed_setter_two=true;
    set_speed_phase_two();
  }else if(distance_went>=410){
    OCR0A=0;
    motor_state = 0;
    speed_setter_one=false;
    speed_setter_two=false;
    distance_went=0;
    continue;
 }

unsigned int time = 0;
time  = 0;
time = get_time();
if(time==0){continue;}
float relative_speed = 3.15 * ((1000.0)/(time*1.0*6.0))*PI;
float real_distance = (time*1.0 / 1000.0)*relative_speed;
distance_went+=real_distance;
int int_distance= (int)(distance_went*10);
display_speed((int)relative_speed*100);
display_distance(int_distance);
display_voltage(adc);
}
else if(motor_state==0){ 
  OCR0A = 0;
  }
 }

}

void setup_octocoupler(){
TCCR1A = 0x00;
TCCR1B = 0xC5; 
DDRB &= ~0x01;
PORTB |= 0x01; //enable pull-up
TCNT1=0;
}
void setup_motor(){
DDRD = DDRD|(1<<6); //ENA
DDRD = DDRD|(1<<7);//IN1
DDRB = DDRB |(1<<1); //IN2
TCCR0A = 0xA3; 
TCCR0B |= 0x05;
PORTB |= (1<<1);//IN2 5V
}
void setup_adc0(){
ADMUX = ADMUX | 0x40;
ADCSRB=ADCSRB & (0xF8);
ADCSRA = ADCSRA | 0xE7; 
 }

unsigned int read_adc(void)
{
 unsigned int adclow = ADCL;
adclow = (adclow + ((ADCH & 0x03) << 8));
float vm = ((adclow*1.0)/1023.0)*15.0;
vm*=10;
int voltage = (int)vm;
return voltage+2;
}

unsigned int get_time(){
unsigned int time=0;
while( (!(TIFR1 & (1<<ICF1)))&&ICR1<31250){}
if(ICR1>=31250){return 0;}
TIFR1 = TIFR1|(1<<ICF1);
time = ICR1;
TCNT1=0;
time/=(15.625);
return time;
}

unsigned int MotorOnOff(){
uint32_t readValue;
char readBuffer[10];
 unsigned int intemm;
if(motor_state==0)
{
  printf("get %s.val%c%c%c","page1.bt0",255,255,255);
  for(int i=0;i<8;i++){
  scanf("%c",&readBuffer[i]);
}
  if(readBuffer[0]==0x71){readValue = readBuffer[1] | (readBuffer[2] << 8) | (readBuffer[3] << 16)| (readBuffer[4] << 24);
   intemm=(unsigned int)readValue;
  if(intemm==1){
    get_both_times();
    return 1;
    }
  }
  return 0;
}
else if(motor_state==1)
{
  printf("get %s.val%c%c%c","page2.bt1",255,255,255);
  for(int i=0;i<8;i++){
  scanf("%c",&readBuffer[i]);
}
if(readBuffer[0]==0x71){
   readValue = readBuffer[1] | (readBuffer[2] << 8) | (readBuffer[3] << 16)| (readBuffer[4] << 24);
  intemm=(unsigned int)readValue;
 if(intemm==1){return 2;}
 }
   return 0;
 }
}
void set_speed_phase_one(){
float a = 255*1.0*(min_time*1.0 /time_phase_one*1.0);
OCR0A = (unsigned int)a;
}
void set_speed_phase_two(){
  float b = 255*1.0*(min_time*1.0/time_phase_two*1.0);
  OCR0A = (unsigned int)b;
}
unsigned int buffer_reader(char a[10]){
 uint32_t reading = a[1] | (a[2] << 8) | (a[3] << 16)| (a[4] << 24);
 unsigned int intemm=(unsigned int)reading;
return intemm;
}
void get_both_times(){
  char readBuffer[10];
  printf("get %s.val%c%c%c","page1.n0",255,255,255);
  uint32_t readValue=0;
  unsigned int intemm;
  for(int i=0;i<8;i++)scanf("%c",&readBuffer[i]);
  readValue = buffer_reader(readBuffer);
  intemm = (unsigned int )readValue;
  time_phase_one = intemm;
  readValue=0;
    printf("get %s.val%c%c%c","page1.n1",255,255,255);
    for(int i=0;i<8;i++)scanf("%c",&readBuffer[i]);
    readValue = buffer_reader(readBuffer);
    intemm = (unsigned int)readValue;
  time_phase_two = intemm;
}

void display_distance(int distanco){
printf("page2.x1.val=%d%c%c%c",distanco,255,255,255);
}
void display_speed(int speedo){
printf("page2.x2.val=%d%c%c%c",speedo,255,255,255);
}
void display_voltage(unsigned int value){
printf("page2.x0.val=%d%c%c%c",value,255,255,255);
}