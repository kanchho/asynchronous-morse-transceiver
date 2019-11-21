/*   
          Electronics Submodule - 5 cr
          Embedded Systems
          Fall 2018 / Spring 2019
          TUAS
          
          Lab 11 : Microcontrollers Lab
       
Pick any four numbers from the Morse Code table.
Implement a system that is able to transmit the Morse code
of those numbers that you just picked up in random order.
Arduino should send them to a led, which should blink
according to Morse code.

There should also be a light sensitive sensor in your device
so, that the sent code can be catched and shown in a seven
segment display.

Your device should work so, that it randomly is transmitting
the Morse code of one of those four numbers that you
selected in the beginning. The sensor should recognize the
code and after that Arduino should show the sent number in
a display.




+----+    +----+
|LED |    |LDR +--------+
|Bulb|    +----+        |
+--+-+                  |
   |                    |
   |         +----------+--------------------------+             +------+
   |         |                                     |             |      |
   |         |                                     |             |      |
   +---------+            Arduino                  |             | 7    |
             |                                     +-------------+ Seg  |
             |                                     |             | Display
             |                                     |             |      |
             +-------------------------------------+             +------+
       


-----------------------------------------------------------------------------------------
#########################################################################################
-----------------------------------------------------------------------------------------


 ## Author : Bibek Koirala
 ## Env : Atmel Studio
 ## Description :  This sketch transmits and decodes Morse code on a single microcontroller 
     without blocking. Encoding and decoding happens simultaneously.

  ** NOTE :-
   Transmission states : 
     * LED ON : dot or dash
     * LED OFF : Letter space or word space
   In this Sketch, each number/digit is assumed as a word. So, 
     * Letter Space here means delay between dots and dashes of same number
     * Word Space here means delay between recently transmitted number and next number to be transmiited
  **

*/

#include <Arduino.h>

/* Morse Code Specification*/
#define DOT_PERIOD 500                              // 1 unit = 500ms
#define DASH_PERIOD (DOT_PERIOD * 3)                // 3 units
#define LETTER_SPACE (DOT_PERIOD * 3)               // 3 units
#define WORD_SPACE  (DOT_PERIOD* 7)                 // 7 units 

#define THRESHOLD 894                               // LDR's threshold value for LED's On/Off state; depends on room lighting

/* I/O Pins */
const int ledPin =  13; 
const int ldrPin = A0;

/*state machine memory*/
int numTransmitState = 1;                           // decides whether to transmit dot or dash next in current transmission. 


/* Encoding and Decoding data */
const int morseNumbers[5] = {3, 4, 5, 6};           // random number will be chosen from these 4 numbers
int index = -1;
char storeMorse[6];                                 // stores decoded code i.e. dots and dashes
int sM_index = 0;
static char *decodeList[6] = {                               // Lookup table for decoding dots and dashes into numbers
                               "...--",       // '3' 
                               "....-",       // '4'
                               ".....",      // '5'
                               "-...."       // '6'
                             }; 

               
/* flags */
bool transmissionFinished = true;                    // set to True when a number is transmitted completely
bool genNextRandomNum = true;                        // generate next random number if true
bool dot_in_progress = false;
bool dash_in_progress = false;
bool letter_space_in_progress = false;
bool word_space_in_progress = false;


/* when dot, dash, letter space and word space should finish after start of their transmission, values in milliseconds */
unsigned long dotEndMillis = 0;
unsigned long dashEndMillis = 0;
unsigned long letterSpaceEndMillis = 0;
unsigned long wordSpaceEndMillis = 0;
unsigned long  currentMillis = 0;
unsigned long prevMillis = 0;


/* LED States */
int currentState = 0;                     // is set to 1 if LED is ON
int prevState = 0;                                    


/* state(1 or 0) of each segments of 7-seg-Display for displaying particular digit*/
byte seven_seg_digits[10][7] = {  
    { 0,0,0,0,0,0,1 }, // = 0
    { 1,0,0,1,1,1,1 }, // = 1
    { 0,0,1,0,0,1,0 }, // = 2
    { 0,0,0,0,1,1,0 }, // = 3
    { 1,0,0,1,1,0,0 }, // = 4
    { 0,1,0,0,1,0,0 }, // = 5
    { 0,1,0,0,0,0,0 }, // = 6
    { 0,0,0,1,1,1,1 }, // = 7
    { 0,0,0,0,0,0,0 }, // = 8
    { 0,0,0,0,1,0,0 }  // = 9
};   

/*Function Prototypes*/
void dot();
void dash();
void letter_space();
void word_space();
void chooseRandomNumber();
void transmitThree();
void transmitFour();
void transmitFive();
void transmitSix();
unsigned int decodeMorse(char* code);
void sevenSegWrite(int numToDisplay);

// Pin setup
void setup(){
    Serial.begin(9600);
    pinMode(ledPin, OUTPUT);
    pinMode(ldrPin, INPUT);
    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(7, OUTPUT);
    pinMode(8, OUTPUT);

}

// execution starts here
void loop(){

    /* Encoding & Transmission - Start */
  
    // if transmitting dot just finished
    if ((dot_in_progress == true) && (millis() >= dotEndMillis)) { 
        digitalWrite (ledPin, LOW);
      dot_in_progress = false;
        letter_space();
    }
  
    // if transmitting dash just finished
    if ((dash_in_progress == true) && (millis() >= dashEndMillis)) {
        digitalWrite (ledPin, LOW);
      dash_in_progress = false;
        letter_space();
    }
  
    // if letter space just finished
    if ((letter_space_in_progress == true) && (millis() >= letterSpaceEndMillis)) { 
        letter_space_in_progress = false;
        numTransmitState++;    
        if (transmissionFinished == true){
            word_space();
            transmissionFinished = false;
        }
    }  
  
    // if word space just finished
    if ((word_space_in_progress == true) && (millis() >= wordSpaceEndMillis)) { 
        word_space_in_progress = false;
        genNextRandomNum = true;

    }  
  
    chooseRandomNumber();
    if (index == -1) {
         // failed generating number
        // pass
    }
    else {
        switch (index) {
            case 0 :                 // morseNumbers[0] = 3
                transmitThree();
                break;
            case 1 :                 //morseNumbers[1] = 4
                transmitFour();
                break;
            case 2 :                //morseNumbers[2] = 5
                transmitFive();
                break;
            case 3 :                //morseNumbers[3] = 6
                transmitSix();
                break; 
            default :
                break;     
        }
    } 

  /* Encoding & Transmission - End */
  
  
  /* Receiving & Decoding - Start */
    Serial.println(analogRead(ldrPin));
    if (analogRead(ldrPin) <= THRESHOLD) {
        currentState = 0;
    }
    else {
        currentState = 1; 
    }

    if (currentState != prevState)
    {
        currentMillis = millis();
        Serial.println(analogRead(ldrPin));
        if (prevState == HIGH){
            if ((currentMillis - prevMillis) < (DOT_PERIOD + DOT_PERIOD/10 ) )  {
          // Dot received
                storeMorse[sM_index] = '.';
            }
            else {
          // Dash received
                storeMorse[sM_index] = '-';
            }
            sM_index++;
            storeMorse[sM_index] = '\0';
        }
        else {

            if ((currentMillis - prevMillis) < (LETTER_SPACE + LETTER_SPACE/10) ){
                // finished a dot or a dash
                //pass
            }

            else if ((currentMillis - prevMillis) > (WORD_SPACE - WORD_SPACE/10)){
                // Transmitter finished transmission of a number, now decode received dots and dashes to number
                unsigned int num = decodeMorse(storeMorse);
                Serial.print("Num decoded : ");
                Serial.println(num);
                sevenSegWrite(num);
            }   
       }
     
    prevState = currentState;
    prevMillis = currentMillis;
  
  /* Receiving & Decoding - End */
 }
}

/* Functions*/

void dot(){
    if ((dot_in_progress && dash_in_progress && letter_space_in_progress && word_space_in_progress)== false) {
        dot_in_progress = true;
        dotEndMillis = millis() + DOT_PERIOD;
        digitalWrite (ledPin, HIGH);
   }
}

void dash(){
    if ((dash_in_progress &&  dot_in_progress && letter_space_in_progress && word_space_in_progress) == false){
        dash_in_progress = true;
        dashEndMillis = millis() + DASH_PERIOD;
        digitalWrite (ledPin, HIGH);
    }
}

void letter_space(){               
    letterSpaceEndMillis = millis() + LETTER_SPACE ;
    letter_space_in_progress = true;    
}

void word_space(){                           
    wordSpaceEndMillis = millis() + WORD_SPACE ;
    word_space_in_progress = true;     
  }
 
void chooseRandomNumber() {
    // Choose new number only if previously chosen random number has been transmitted completely
    if (genNextRandomNum == true) {    
        index = random(0, 4);
        Serial.print("Encoded : "); 
        Serial.println(morseNumbers[index]);        
        genNextRandomNum = false;
        numTransmitState = 1;                   // reset 
         }          
 }
  
void transmitThree () { 
    Serial.print("Transmitting 3");
    switch(numTransmitState) {
        case 1 :
           dot(); 
           break;
        case 2 : 
           dot(); 
           break;
        case 3: 
           dot();  
           break;
        case 4 : 
           dash();
           break;
        case 5 :
           dash(); 
           transmissionFinished = true;
           break;        
        default : 
           break;
  }
}


void transmitFour () { 
    Serial.print("Transmitting 4");
    switch(numTransmitState) {
        case 1 :
            dot(); 
            break;
        case 2 : 
            dot(); 
            break;
        case 3: 
            dot();  
            break;
        case 4 : 
            dot();
            break;
        case 5 :
            dash(); 
            transmissionFinished = true;
            break;        
        default : 
            break;
  }
}


void transmitFive () { 
    Serial.print("Transmitting 5");
    switch(numTransmitState) {
        case 1 :
            dot(); 
            break;
        case 2 : 
            dot(); 
            break;
        case 3: 
            dot(); 
            break;
        case 4 : 
            dot();
            break;
        case 5 :
            dot(); 
            transmissionFinished = true;
            break;        
        default : 
            break;
  }
}

void transmitSix () { 
    Serial.print("Transmitting 6");
    switch(numTransmitState) {
        case 1 :
            dash(); 
            break;
        case 2 : 
            dot(); 
            break;
        case 3: 
            dot();  
            break;
        case 4 : 
            dash();
            break;
        case 5 :
            dot(); 
            transmissionFinished = true;
            break;        
        default : 
           break;
  }
}

unsigned int decodeMorse(char *code){
    Serial.print("Decoded : ");
    Serial.print(code);
    Serial.print("\n");
    for (unsigned int i = 0; i < 4 ; i++){
        if (strcmp(decodeList[i], code) == 0) {
           return (i + 3);
        }
    }
    return 1;
}

void sevenSegWrite(int numToDisplay) {
    int pin = 2;
    for (byte segCount = 0; segCount < 7; ++segCount) {
        digitalWrite(pin, seven_seg_digits[numToDisplay][segCount]);
        ++pin;
  }
}