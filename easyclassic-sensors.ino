// Michiel van Dijk and Marton Varga - 2021
// This #include statement was automatically added by the Particle IDE.
#include <elapsedMillis.h>

// This #include statement was automatically added by the Particle IDE.
#include <HC_SR04.h>


//rangefinder front
int trigPinFront = D4;
int echoPinFront = D5;
double cmFront = 0.0;

//rangefinder rear
int trigPinRear = D3;
int echoPinRear = D6;
double cmRear = 0.0;

//Overwrite the rangefinder to change the minimum distance from 10cm in the library to 5 cm, and the max range is reduced to 150cm we don't need more range for this project.
HC_SR04 rangefinderFront = HC_SR04(trigPinFront, echoPinFront, 5.0, 150.0);

HC_SR04 rangefinderRear = HC_SR04(trigPinRear, echoPinRear, 5.0, 150.0);




//Timers
elapsedMillis parkingTimer;
elapsedMillis motionTimer;
String proximityFront = "clear";
String proximityRear = "clear";


//Engine State
String engineState = "off";

//Lock State
String lockState = "locked";


//Setup for IR sensor
int irinput = D8;
int calibrateTime = 5000;
bool motionDetected = false;
int irled = D2;


//Gearbox simulation
int gearboxbutton = A0;
String gearboxstate = "drive";
elapsedMillis gearboxTimer;



//Setup
void setup() {
    //Subrscribe to the other argon to receive updates on what data should be published
    Particle.subscribe("Engine State", enginestateFunction);
    Particle.subscribe("lockstate", mylockHandler);
    //Make variables available for the android app
    Particle.variable("cmFront", &cmFront, DOUBLE);
    Particle.variable("cmRear", &cmRear, DOUBLE);
    
    //IR pin Setup
    pinMode(irinput, INPUT);
    pinMode(irled, OUTPUT);
    
    //gearbox drive select button
    pinMode(gearboxbutton, INPUT_PULLUP);
    
    
    //soft delay timer setup
    parkingTimer = 0;
    motionTimer = 0;
    gearboxTimer = 0;
}


//Loop
void loop() {
    gearboxCheck();
    cmFront = rangefinderFront.getDistanceCM();
    cmRear = rangefinderRear.getDistanceCM();
    
    if (engineState == "on" && parkingTimer > 999 && gearboxstate == "drive"){
        parkingSensorFront();
        parkingTimer = 0;
    }
    
    if (engineState == "on" && parkingTimer > 999 && gearboxstate == "reverse"){
        parkingSensorRear();
        parkingTimer = 0;
    }
    
    
    
    if (motionTimer > 4999){
        
        if (calibrated()){
            motionEvent();
            }
        motionTimer = 0;
    }
}


//Engine function
void enginestateFunction(const char *event, const char *data)
{

  if (strcmp(data,"on")==0) {
    // if the sensor's threshold is breached, then turn your LED on
    engineState = "on";
  }
  else if (strcmp(data,"off")==0) {
    // if your sensor's beam is all clear, turn your board LED off
    engineState = "off";
  }
}


//Lock function
void mylockHandler(const char *event, const char *data)
{

  if (strcmp(data,"0")==0) {
    // if the sensor's threshold is breached, then turn your LED on
    lockState = "locked";
  }
  else if (strcmp(data,"1")==0) {
    // if your sensor's beam is all clear, turn your board LED off
    lockState = "unlocked";
  }
}






//Front parkingsensor function
void parkingSensorFront (){     
    
    

    if (cmFront<10 && cmFront>0){
        
        if (proximityFront != "close"){
                proximityFront = "close";
                Particle.publish("front proximity", proximityFront);
                }
        } else if (cmFront<20 && cmFront>10){
            if (proximityFront != "medium"){
                proximityFront = "medium";
                Particle.publish("front proximity", proximityFront);
                }
        } else if (cmFront<30 && cmFront>20){
            if (proximityFront != "far"){
            proximityFront = "far";
            Particle.publish("front proximity", proximityFront);
            }
        } else if (cmFront > 30){
            if (proximityFront != "clear"){
                proximityFront = "clear";
                Particle.publish("front proximity", proximityFront);
                }
        
    } else {
    }
}

 

void parkingSensorRear(){     
    
    

    if (cmRear<10 && cmRear>0){
       if (proximityRear != "close"){
                proximityRear = "close";
                Particle.publish("rear proximity", proximityRear);
                }
 
        } else if (cmRear<20 && cmRear>10){
            if (proximityRear != "medium"){
                proximityRear = "medium";
                Particle.publish("rear proximity", proximityRear);
                }


        } else if (cmRear<30 && cmRear>20){
            if (proximityRear != "far"){
                proximityRear = "far";
                Particle.publish("rear proximity", proximityRear);
                }
            
        } else if (cmFront > 30){   
            if (proximityRear != "clear"){
                proximityRear = "clear";
                Particle.publish("rear proximity", proximityRear);
                }
    } else {
    }
}



//IR motion sensor + calibrate timer function

bool calibrated() {
    return millis() - calibrateTime > 0;
}


void motionEvent(){
    
    if (lockState == "locked" && engineState == "off"){
    if (digitalRead(irinput) == HIGH) {
         motionDetected = true;
         digitalWrite(irled, HIGH);
         Particle.publish("Motion Detected", "Motion");
        }
        
       else {
           
          motionDetected = false;
          digitalWrite(irled, LOW);
          
    }
    }
}



//A button used to simulate the car direction, stops un neccesary publishes from occuring.
void gearboxCheck(){
    int gearboxRead = digitalRead(gearboxbutton);
    
    //LOW means the button is pressed
    if(gearboxTimer > 499 && gearboxRead == LOW){
            if (gearboxstate == "drive"){
            gearboxstate = "reverse";
            }
            else if (gearboxstate == "reverse"){
                gearboxstate = "drive";
            }
            
            gearboxTimer = 0;
    }
}






