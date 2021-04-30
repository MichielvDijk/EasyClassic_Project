// Michiel van Dijk and Marton Varga - 2021
// EasyClassic - Dashboard Particle
#include <elapsedMillis.h>
#include "Keypad_Particle.h"


//Keypad Setup + variables


// set up keypad buttons
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
byte rowPins[ROWS] = { D3, D2, D1, D0 };
byte colPins[COLS] = { D6, D5, D4 };

// create Keypad object variable called "keypad"
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

String engineCode = "1234"; // Initial engine code
String codeEntered = ""; // String of keys entered
int codeLength = 0; // Amount of keys pressed
// Keypad End


// Status Led setup
int ledgreen = A5;
int ledred = A4;
int ledblue = A3;
String notifyLed = "off";
elapsedMillis ledTimer;
// Status Led End



//Buzzer
int buzzer = A1;
String buzzerstate = "nosound";
//buzzer sound timers for different interval sounds
elapsedMillis buzzTimer;



// Engine off button
int enginebutton = A2;
String engineState = "off";



//Servo declaration
Servo myservo;  // create servo object to control a servo 
int pos = 0;
String password = "1234";
int lockState = 0;



// Engine off button
int doorbutton = A0;
String doorState = "closed";
elapsedMillis doorTimer;


//Auto lock timer
elapsedMillis autolockTimer;


//SETUP
void setup(){
    //LED Setup
    pinMode(ledgreen, OUTPUT);
    pinMode(ledred, OUTPUT);
    pinMode(ledblue, OUTPUT);
    
    //Buzzer Setup
    pinMode(buzzer,OUTPUT);
    buzzTimer = 0;
    
    //Engine off button
    pinMode(enginebutton, INPUT_PULLUP);
    
    //Servo Setup
    myservo.attach(D8);
    myservo.write(pos);
    
    Particle.function("carlock", doorUnlock); 
    
    Particle.variable("lockstate", lockState);
    
    //Engine off button
    pinMode(doorbutton, INPUT_PULLUP);
    
    //Subscribe to receive a new engine code
    Particle.subscribe("engine_start_code", updateEnginecode); // Comes from Android device
    
    //Subscribe to the parking sensor
    Particle.subscribe("front proximity", parkingFront);
    Particle.subscribe("rear proximity", parkingFront);
    
    //Subscribe to the motion detector (PIR Sensor) on the sensors argon
    Particle.subscribe("Motion Detected", alarmFunction);
}



//LOOP
void loop(){
 keyEntry();
 
 engineOff();
 
 doorCheck();
 
 ledControl();
 
 buzzerFunction();
 
 autolockFunction(); 
 
}



//FUNCTIONS

//Keypad functionality + code regonition
void keyEntry(){
      char key = keypad.getKey();
  
  if (key){
    delay(50);
    codeEntered = codeEntered+key; // Store key entries
    Particle.publish(codeEntered);
    
    
    //# allows for a new passcode entry, wiping the old data
    if (String(key) == "#"){
        notifyLed = "blue";
        codeEntered = "";
        ledTimer=0;
        
    }
    
  } 
  
  
//When 4 keys are entered, the function will check if the code is correct 
  codeLength = strlen(codeEntered);

  if (codeLength == 4){
      if (codeEntered == engineCode){
          engineState = "on";
          Particle.publish("Engine State", engineState);//engine state notifies the other particle that the engine has been started, so the parking sensors can be started.
          notifyLed = "green";
      }
      else{
            Particle.publish("wrong code", "A wrong code was entered");
            notifyLed = "red";
      }
      ledTimer=0;
      codeEntered = "";
  }
  
}


//Code to controll the LED and signal with the correct color
void ledControl(){
    
    if (notifyLed == "red"){
        
        digitalWrite(ledred, HIGH);
        tone(buzzer, 500);
        if (ledTimer > 499){
            digitalWrite(ledred, LOW);
            notifyLed = "off";
            noTone(buzzer);
        }
    }
    if (notifyLed == "blue"){
        digitalWrite(ledblue, HIGH);
        
        if (ledTimer > 499){
            digitalWrite(ledblue, LOW);
            notifyLed = "off";
        }
        
    }
    if (notifyLed == "green"){
        digitalWrite(ledgreen, HIGH);
        
        if (ledTimer > 499){
            digitalWrite(ledgreen, LOW);
            notifyLed = "off";
            
        }
        
    }
    
}


//Turn the engine off - this function is needed to signal the other particle to suspend its parking sensor activity

void engineOff(){
    int buttonState = digitalRead(enginebutton);
    
    //LOW means the button is pressed
    if(buttonState == LOW){
            if (engineState == "on"){
            engineState = "off";
            notifyLed = "blue";
            Particle.publish("Engine State", engineState);
            ledTimer=0;
            buzzerstate = "nosound";
        }
    }
}


//The Servo code, works only on being called by the function, controlled from the phone.
int doorUnlock(String lockcode) { 


  if(lockcode==password){
      if (lockState == 0){
        pos = 90;
        myservo.write(pos);
        buzzerstate = "nosound";
        lockState = 1;
        Particle.publish("lockstate", String(lockState));
        autolockTimer = 0;
        return 1;
        }
        
        else {
        pos = 0;
        myservo.write(pos);
        lockState = 0;
        Particle.publish("lockstate", String(lockState));
        return 1;
        }

    }
    
    else if (lockcode!=password){
        return 0;
        
    }
    
    else{
        return -1;
    }
    
}

void doorCheck(){
    int doorRead = digitalRead(doorbutton);
    
    //LOW means the button is pressed
    if(doorTimer > 499 && doorRead == LOW){
            if (doorState == "closed"){
            doorState = "open";
            Particle.publish("Door", doorState);
            }
            else if (doorState == "open"){
                doorState = "closed";
                Particle.publish("Door", doorState);
                autolockTimer = 0;
            }
            doorTimer = 0;
    }
}   

//Update the engine code with the new password
void updateEnginecode(const char *event, const char *data)
{
    engineCode = data;
    Particle.publish(engineCode);
}




//Parking Sensor event handlers
//Front event handler
void parkingFront(const char *event, const char *data)
{

  if (strcmp(data,"close")==0) {
   if (buzzerstate != "closesound"){ 
        buzzerstate = "closesound";
    }}
  else if (strcmp(data,"medium")==0) {
   if (buzzerstate != "mediumsound"){ 
        buzzerstate = "mediumsound";
    }}
  else if (strcmp(data,"far")==0) {
    if (buzzerstate != "farsound"){ 
        buzzerstate = "farsound";
    }}
  else if (strcmp(data,"clear")==0) {
    if (buzzerstate != "nosound"){ 
        buzzerstate = "nosound";
    }}
  else{}
}


//Rear event handler
void parkingRear(const char *event, const char *data){

  if (strcmp(data,"close")==0) {
    if (buzzerstate != "closesound"){ 
        buzzerstate = "closesound";
    }}
  else if (strcmp(data,"medium")==0) {
    if (buzzerstate != "mediumsound"){ 
        buzzerstate = "mediumsound";
    }}
  else if (strcmp(data,"far")==0) {
    if (buzzerstate != "farsound"){ 
        buzzerstate = "farsound";
    }}
  else if (strcmp(data,"clear")==0) {
   if (buzzerstate != "nosound"){ 
        buzzerstate = "nosound";
    }}
  else{}
}




// Buzzer functionality
void buzzerFunction(){
    if (buzzerstate=="nosound"){
        noTone(buzzer);
        } 
        else if (buzzerstate=="farsound"){
            if (buzzTimer > 0){
                    tone(buzzer, 1000);
                }
            if (buzzTimer > 249){
                    noTone(buzzer);
                }
            if (buzzTimer > 999){
                    buzzTimer = 0;
                }
        }
        else if (buzzerstate=="mediumsound"){
            if (buzzTimer > 0){
                    tone(buzzer, 1000);
                }
            if (buzzTimer > 249){
                    noTone(buzzer);
                }
            if (buzzTimer > 749){
                    buzzTimer = 0;
                }
        }
        else if (buzzerstate=="closesound"){
            
            if (buzzTimer > 0){
                    tone(buzzer, 1000);
                }
            if (buzzTimer > 249){
                    noTone(buzzer);
                    buzzTimer = 0;
                }
            if (buzzTimer > 499){
                    buzzTimer = 0;
                }
        }
        else if (buzzerstate=="alarmsound"){
            
            if (buzzTimer > 0 && buzzTimer < 500){
                    tone(buzzer, 2000);
                }
            if (buzzTimer > 500 && buzzTimer < 750){
                    noTone(buzzer);
                }
            if (buzzTimer > 750 && buzzTimer < 1249){
                    tone(buzzer, 2000);
                }
            if (buzzTimer > 1499){
                    buzzTimer = 0;
                }
        }
    
}


void alarmFunction(const char *event, const char *data) {
 
    if (strcmp(data,"Motion")==0) {
        buzzerstate="alarmsound";
    }
    else  {
      // Do nothing
    }
}


void autolockFunction(){
    
    if(autolockTimer >= 15000 && lockState == 1 && engineState == "off" && doorState == "closed"){
        pos = 0;
        myservo.write(pos);
        lockState = 0;
        Particle.publish("lockstate", String(lockState));
    }
    
}
