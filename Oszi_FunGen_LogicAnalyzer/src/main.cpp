#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

  /* Pins and Constants */
#define BUTTON 12
#define ADC_PIN A0
#define SIGNAL_PIN 14                   // Function Generator pin
#define SERIAL_RX_PIN 15                // SERIAL RX PIN 
#define DEBOUNCE_MS 350                 // button debounce delay 

  /*  Variables  */

uint32_t current_ms       = 0 ;      
uint32_t previous_ms      = 0 ;
uint32_t wave_previous_ms = 0 ;
uint32_t wave_current_ms  = 0 ;
uint32_t osci_previous_ms = 0 ;
uint32_t osci_current_ms  = 0 ;
uint32_t logi_previous_us = 0 ;
uint32_t logi_current_us  = 0 ;

bool button_status = false;             // Flag for button input
bool button_V_level_current   = true ; 
bool button_V_level_previous  = true ; 
bool welcome_page_status  = false;       // flag to avoid reloading of welcome page repetitively
bool menu_page_status     = false;       // flag to avoid reloading of menu page repetitively
bool osci_page_status     = false;       // flag to avoid reloading of osci page repetitively
bool square_page_status   = false;       // flag to avoid reloading of square page repetitively
bool triangle_page_status = false;       // flag to avoid reloading of triangle page repetitively
bool sine_page_status     = false;       // flag to avoid reloading of sine page repetitively
bool logican_page_status  = false;       // flag to avoid reloading of logic analyzer page repetitively
bool triangle_ramp        = true;        // true - upward   false - downward
bool start_bit_detect     = false;
bool previous_rx_value = 1;              // hold the value of digital read of serial rx
bool current_rx_value  = 1;


int incomingByte = 0 ;                    // for incoming serial data
int serialvalue = 0;                      // holds value of first recived byte
int analog_temp = 0;                      // temporary variable used in analog read
uint32_t square_frequency   = 500;        // frequency of square wave in ms
uint32_t triangle_frequency = 200;        // frequency of triangle wave in us
uint32_t sine_frequency     = 200;        // frequency of sine wave     in us
uint8_t signal_pin_status   = 00;         // holds the status of signal (square wave)
uint8_t sine_radian     = 00;             // holds the radian variable of sine wave)
uint8_t analog_value    = 00;             // holds the value of analog read
uint8_t previous_value  = 36;             // holds the previous value display on oLED 36 is zero line
uint16_t x_value_OLED   = 00;             // x - asix of oled
uint32_t osci_sample_ms = 500;            // its is in us
uint32_t logican_sample_us = 104;         // sample time at 9600 bps 8N1
uint8_t decoded_byte  = 00  ;             // decode byte from serial
uint8_t bit_counter   = 00  ;             // counts the number of bits

/* Function Declerations */
void welcomePage();
void menuPage();
void osciPage();
void squarePage();
void trianglePage();
void sinePage();
void logicanPage();
void serialBufferClear();         // clear the recieve buffer 
void squareWave();
void triangleWave();
void sineWave();
void osciRead();
void logicanRead();
bool readButton();                // reads button status with leading edge and remove debouncing error
int serialRead();                 // read  first byte of serial buffer
void wave_value_OLED(int16_t val) ;  // shows the value of Voltage on OLED

void wave_OLED_Memory(int16_t x_val, int16_t val, int x_scale);   // display waveform on oled x- axis and Value of voltage

enum states
{
    WELCOME, MENU, OSCI, SQUARE, TRIANGLE, SINE, LOGIC_AN
};
states currentstate = WELCOME;


void setup() {
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address  for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.setTextColor(WHITE);
  pinMode(BUTTON,INPUT_PULLUP); 
  pinMode(SERIAL_RX_PIN,INPUT_PULLUP); 
  Serial.begin(9600);
  analogWriteResolution(8);
  analogReadResolution(8);
}

void loop() {
 
  switch (currentstate)
  {
  case WELCOME:  
      welcomePage();
      if(readButton()) 
        {currentstate = MENU ;}
      break;

  case MENU:   
      menuPage(); 
      if(readButton())                    // way to welcome page
          {  currentstate = WELCOME;}

      /* selection of mode by serial value  */
      serialvalue = serialRead() ;
      if(serialvalue == '1'){   
        currentstate=OSCI;    }

      else if (serialvalue == '2'){
        currentstate=SQUARE;     }

      else if (serialvalue == '3'){
        currentstate=TRIANGLE;  }

      else if (serialvalue == '4'){
        currentstate=SINE;      }

      else if (serialvalue == '5'){
        currentstate=LOGIC_AN;  }
      else
      { }

      break;

  case OSCI:  
      if(readButton()) 
        {currentstate = MENU ;}
      osciPage();
      osciRead();
      break;

  case SQUARE:  
       if(readButton()) 
         {currentstate = MENU ;}
      squarePage();  
      squareWave();
      break;

  case TRIANGLE:  
      if(readButton()) 
        {currentstate = MENU ;}
      trianglePage();  
      triangleWave();
      break;

  case SINE:  
      if(readButton()) 
        {currentstate = MENU ;}
      sinePage(); 
      sineWave(); 
      break;

  case LOGIC_AN: 
      if(readButton()) 
        {currentstate = MENU ;}
      logicanPage();   
      logicanRead();
      break;
  
  default:
    break;
  }
}

void welcomePage()
{ if(!welcome_page_status) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(25,24);
    display.println("Welcome");
    display.startscrollright(0x03, 0x04);
    display.setTextSize(1);
    display.setCursor(30,55);
    display.println("Press Start");
    display.display();
    welcome_page_status = true;
    menu_page_status= false;
  }
}

void menuPage()
{ if(!menu_page_status) {
    display.clearDisplay();
    display.stopscroll();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.println("SELECTION MENU:  CODE");
    display.setCursor(0,10);
    display.println(" Oscillator        1");
    display.setCursor(0,19);
    display.println(" Function Generator");
    display.setCursor(0,28);
    display.println("  * Square Wave    2");
    display.setCursor(0,37);
    display.println("  * Triangle Wave  3");
    display.setCursor(0,46);
    display.println("  * Sine Wave      4");
    display.setCursor(0,55);
    display.println(" Logic Analyzer    5");
    display.display();
    
    welcome_page_status = false;
    menu_page_status= true;
    osci_page_status= false;
    square_page_status= false;
    triangle_page_status= false;
    sine_page_status= false;
    logican_page_status= false;
    signal_pin_status = 00;
    serialBufferClear();
  }
}

void osciPage()
{   if(!osci_page_status) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0,0);
      display.println("Oscilloscope ");
      display.display();
      int16_t iij = 0;
      for (iij=0; iij< 127;iij++)
        {
        display.drawPixel(iij, 36, SSD1306_WHITE);
        iij++;
      }
      display.display(); 
      menu_page_status= false;
      osci_page_status= true;
      x_value_OLED=0;
      previous_value = 36;                                 // center line is at 36
    }
}

void squarePage()
{   if(!square_page_status) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0,0);
      display.println("Square Wave");
      display.display();
      menu_page_status= false;
      square_page_status= true;
      x_value_OLED=0;
      previous_value = 36;                                 // center line is at 36
    }
}

void trianglePage()
{   if(!triangle_page_status) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0,0);
      display.println("Triangle Wave");
      display.display();
      menu_page_status= false;
      triangle_page_status= true;
      x_value_OLED=0;
      previous_value = 36;                                 // center line is at 36
    }
}

void sinePage()
{   if(!sine_page_status) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0,0);
      display.println("Sine Wave");
      display.display();
      menu_page_status= false;
      sine_page_status= true;
      x_value_OLED=0;
      previous_value = 36;                                 // center line is at 36

    }
}

void logicanPage()
{   if(!logican_page_status) {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0,0);
      display.println("Logic Analyzer");
      display.display();
      menu_page_status= false;
      logican_page_status= true;
      x_value_OLED=0;
      previous_rx_value = true;
      current_rx_value = true;
      start_bit_detect = 0;
    }
}

void osciRead()
{   
    
    osci_current_ms=  micros();
    if((osci_current_ms - osci_previous_ms)>= osci_sample_ms){
      osci_previous_ms = osci_current_ms;
      analog_temp = analogRead(ADC_PIN);
      analog_value = map(analog_temp , 0 , 255, 0, 33);
      x_value_OLED++;
      wave_OLED_Memory(x_value_OLED , analog_value,1) ;
      wave_value_OLED(analog_value) ;
      if( x_value_OLED > 126 )
      {x_value_OLED=0;}
    }
      Serial.println(analog_temp);                        // sends the ADC Converstion 0-3.3 V (0-255)
      Serial.flush();    
}

void wave_OLED_Memory (int16_t x_val, int16_t val, int x_scale )
  {   x_val = x_val/ x_scale ;
      val = (33-val)+19 ;
      display.drawPixel(x_val, val, SSD1306_WHITE);
      if (previous_value > val ){
        for (uint8_t i = previous_value; i > val; i--){
          display.drawPixel(x_val, i, SSD1306_WHITE);
        }
      }

      if (previous_value < val ){
        for (uint8_t i = previous_value; i < val; i++){
          display.drawPixel(x_val, i, SSD1306_WHITE);
        }
      }
      previous_value = val;
      if((x_val%2) == 0){                                     // for dotted lines
        display.drawPixel(x_val, 36, SSD1306_WHITE);}
      else{
        display.drawPixel(x_val, 36, SSD1306_BLACK);}
      
      //x_val & 0b01111111   --- Make roll over after 127
      for(int j=12; j<63; j++)              // Black horzontal sweep loop
        { display.drawPixel(((x_val+1) & 0b01111111),j, SSD1306_BLACK);    
          display.drawPixel(((x_val+2) & 0b01111111),j, SSD1306_BLACK);
          display.drawPixel(((x_val+3) & 0b01111111),j, SSD1306_BLACK);
        }
  } 

void wave_value_OLED(int16_t val)                     // shows the value of Voltage on OLED
{
          /* convert Votage value into volts and display*/
          for (int x= 90; x <= 117; x++){              // clear the last value
            for(int y=0; y<=7; y++){ 
              display.drawPixel(x,y, SSD1306_BLACK);  }
          }
          int a =0, b=0 ;             // digit extraction in base form
          a = val / 10;
          b = val % 10;
          display.setCursor(90,0);
          display.print(a);
          display.drawPixel(98, 6, SSD1306_WHITE);
          display.drawPixel(99, 6, SSD1306_WHITE);
          display.setCursor(102,0);
          display.print(b);
          display.print(" V");
          display.display();
  }

void squareWave()
 {  wave_current_ms=  millis();
    if((wave_current_ms - wave_previous_ms)>= square_frequency)
      { signal_pin_status = ~signal_pin_status ;
        analogWrite(SIGNAL_PIN, signal_pin_status);
        wave_previous_ms = wave_current_ms;
      }
          analog_value = map(signal_pin_status , 0 , 255, 0, 33);
          x_value_OLED++;
          wave_OLED_Memory(x_value_OLED,analog_value, 1);
          wave_value_OLED(analog_value) ;
          if( x_value_OLED >= 127 )
          {x_value_OLED=0;}
      Serial.println(signal_pin_status);
      Serial.flush();
}

void triangleWave()
 {  wave_current_ms=  micros();
    if((wave_current_ms - wave_previous_ms)>= triangle_frequency)
      { 
        if(triangle_ramp) {
        signal_pin_status++ ; }
        if(!triangle_ramp) {
        signal_pin_status-- ; }
        analogWrite(SIGNAL_PIN, signal_pin_status);
        wave_previous_ms = wave_current_ms;
        if(signal_pin_status == 255){
          triangle_ramp = false ;}
        if(signal_pin_status == 0){
          triangle_ramp = true ;}
        
        analog_value = map(signal_pin_status , 0 , 255, 0, 33);
          x_value_OLED++;
          wave_OLED_Memory(x_value_OLED,analog_value, 4);     // one-fourth the x scale
          wave_value_OLED(analog_value) ;
           if( x_value_OLED >= 511 )
          {x_value_OLED=0;}
      }
      Serial.println(signal_pin_status);
      Serial.flush();
}

void sineWave(){
    wave_current_ms=  micros();
    if((wave_current_ms - wave_previous_ms)>= sine_frequency){
      //signal_pin_status = (sin(3.1415 * (sine_radian/100.0) )) * 127 + 127;
      signal_pin_status = (sin(3.1415 * ((map(sine_radian,0,255,0,200))/100.0) )) * 127 + 127;
      sine_radian++;
      analogWrite(SIGNAL_PIN, signal_pin_status);
      wave_previous_ms = wave_current_ms;
        if(sine_radian>=255){
          sine_radian=00; }

      analog_value = map(signal_pin_status , 0 , 255, 0, 33);
      x_value_OLED++;
      wave_OLED_Memory(x_value_OLED,analog_value,2);      // half the x scale 
      wave_value_OLED(analog_value) ;
      if( x_value_OLED > 255 )
       {x_value_OLED=0;}
    }
    Serial.println(signal_pin_status);
    Serial.flush();
}

/* --------------- Not Tested Loop ---- Logically it should work 9600 bps No parity 1 stop bit------------------------- */
/* --------------- Was not able to find serial pin/ But tested on slow transition by changing hardwiring------------------------- */
void logicanRead(){
    current_rx_value = digitalRead(SERIAL_RX_PIN);
    if((current_rx_value == 0) & (previous_rx_value == 1) & (start_bit_detect == 0))
    {    logi_previous_us = micros();  
          start_bit_detect = 1;
          decoded_byte =0;
          bit_counter = 0;     
    }

    logi_current_us = micros();
    if((start_bit_detect == 1) & ((logi_current_us - logi_previous_us)  > logican_sample_us) & (bit_counter < 8))
    { bool rx_bit = digitalRead(SERIAL_RX_PIN);		/* Keep for check acknowledge	*/
          decoded_byte = decoded_byte<<1;		
	        decoded_byte = decoded_byte | rx_bit;	/* Keep bit data in dat */
          bit_counter ++ ;
          logi_previous_us = logi_current_us ;         
      }

    if(bit_counter == 8)
    { Serial.print(decoded_byte) ;
      display.setTextSize(1);
      display.setCursor(0,10);
      display.print(decoded_byte);
      display.println();
      start_bit_detect = 0;
      decoded_byte =0;
      bit_counter = 0;
      previous_rx_value = 1;
    }
      display.display();
}
/* --------------------------------------------------------- */

bool readButton(){
    current_ms= millis();
    if((button_V_level_previous) & (!digitalRead(BUTTON)) & ((current_ms - previous_ms) > DEBOUNCE_MS) ) {
      button_status = true;
      previous_ms = current_ms;
      button_V_level_previous = digitalRead(BUTTON);
    }
    else{
      button_status = false;
      button_V_level_previous = digitalRead(BUTTON);
    }
    return button_status;
}

int serialRead(){
    if (Serial.available() > 0) {
        incomingByte= Serial.read();
        serialBufferClear();
     } 
     else{
        incomingByte = 0; }
  return incomingByte;
}

void serialBufferClear()  
    {  int temp =0;  
      while(Serial.available() > 0) 
      { 
        temp = Serial.read();
        Serial.print("I rx: ");
        Serial.println(temp, DEC);
      }
    }
