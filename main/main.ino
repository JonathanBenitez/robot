#include <Servo.h>

int state = 0;
/*
Info om states
0 HALT sätter stop för roboten
1 APP_PRODUCE Startar allt innan normal drift
2 RUN för att köra normalt
3 THROW när bollen ska kastas
5 DEBUG för alla enkel kod för att debuga
*/

int trigPin = 11;    // Trigger
int echoPin = 12;    // Echo
char way;
int spin;
long duration, cm, inches, averagecm;
float compassvalue;

Servo servo;  

#include <LSM303D.h>
#include <Wire.h>
#include <SPI.h>
#define SPI_CS 10
 
/* Global variables */
int accel[3];  // we'll store the raw acceleration values here
int mag[3];  // raw magnetometer values stored here
float realAccel[3];  // calculated acceleration values here
float heading, titleHeading;

// twelve servo objects can be created on most boards

int pos = 0;    
void setup() {
  //Serial Port begin
  Serial.begin (9600);
 
  // put your setup code here, to run once:
  //Setup Channel A
  pinMode(12, OUTPUT); //Initiates Motor Channel A pin
  pinMode(9, OUTPUT);  //Initiates Brake Channel A pin

  //Setup Channel B
  pinMode(13, OUTPUT); //Initiates Motor Channel B pin
  pinMode(8, OUTPUT);  //Initiates Brake Channel B pin
  
  state = 5;

//  servo.attach(9);  


  //Define inputs and outputs
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

    char rtn = 0;
    Serial.println("\r\npower on");
    rtn = Lsm303d.initI2C();
    //rtn = Lsm303d.initSPI(SPI_CS);
    if(rtn != 0)  // Initialize the LSM303, using a SCALE full-scale range
    {
        Serial.println("\r\nLSM303D is not found");
        while(1);
    }
    else
    {
        Serial.println("\r\nLSM303D is found");
    }

}
/**
 * Sets forward or backwards momentum for the left Wheel and disengages the brake
 * byte speed is the speed of the motor from 0-255
 * bool fwd is the direction of the motor where true is forward and false is backward momentum
 */
void runLeftWheel(byte speed, bool fwd){
  if(fwd==false){
    digitalWrite(13, HIGH);  //Establishes forward directios of Channel B
  }
  else{
    digitalWrite(13, LOW);   //Establishes backward directions of Channel B
  }
    digitalWrite(8, LOW);    //Disengage the Brake for Channel B
    analogWrite(11, speed);  //Spins the motor on Channel B at the speed of byte speed
  
}

/**
 * Turns on the breaks for the Left Wheel
 */
void brakeLeftWheel(){
  digitalWrite(8, HIGH); //Engages the Brakes for Channel B
  analogWrite(11, 0);
  }
 
/**
 * Sets forward or backwards momentum for the right Wheel and disengages the brake
 * byte speed is the speed of the motor from 0-255
 * bool fwd is the direction of the motor where true is forward and false is backward momentum
 */
void runRightWheel(byte speed, bool fwd){
  if(fwd==true){
    digitalWrite(12, HIGH); //Establishes forward directions of Channel A
    }
   else{ 
    digitalWrite(12, LOW);  //Establishes backward directions of Channel A
   }
  
    digitalWrite(9, LOW);  //Disengages the Brakes for Channel A
    analogWrite(3, speed); //Spins the motor on Channel A at the speed of byte speed
}

/**
 * Turns on the brakes for the right wheel
 */
void brakeRightWheel(){
  digitalWrite(9, HIGH); //Engages the Brakes for Channel A
  analogWrite(3, 0);
  }

/**
 * Starts the distansmeasurements and delays until it receives the signal
 * return- the value of the measurement
 */
int avstandmatare(){
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
      // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
      digitalWrite(trigPin, LOW);
      delayMicroseconds(5);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);
 
      // Read the signal from the sensor: a HIGH pulse whose
      // duration is the time (in microseconds) from the sending
      // of the ping to the reception of its echo off of an object.
      pinMode(echoPin, INPUT);
      duration = pulseIn(echoPin, HIGH);
 
      // Convert the time into a distance
      cm = (duration/2) / 29.1;     // Divide by 29.1 or multiply by 0.0343
      inches = (duration/2) / 74;   // Divide by 74 or multiply by 0.0135
  
      Serial.print(inches);
      Serial.print("in, ");
      Serial.print(cm);
      Serial.print("cm");
      Serial.println();

      delay(250);

    return cm;
  }

/**
 * Startar servomotorn och kör till sitt max sen kör tillbaka
 */
void servomotor(){
  for (pos = 0; pos <= 180; pos += 1) { 
        // in steps of 1 degree
        servo.write(pos);              
        delay(5);                       
     }
      for (pos = 180; pos >= 0; pos -= 1) { 
        servo.write(pos);              
        delay(5);                       
    }
  
  }

  int compass(){

    while(!Lsm303d.isMagReady());// wait for the magnetometer readings to be ready
    Lsm303d.getMag(mag);  // get the magnetometer values, store them in mag

    Serial.println("The clockwise angle between the magnetic north and x-axis: ");
    Serial.print(Lsm303d.getHeading(mag), 3); // this only works if the sensor is level
    Serial.println(" degrees");
    return mag;
    }

  int getAccel(){
    
      Lsm303d.getAccel(accel);
       for (int i=0; i<3; i++)
    {
        realAccel[i] = accel[i] / pow(2, 15) * ACCELE_SCALE;  // calculate real acceleration values, in units of g
    }
       Serial.println("Acceleration of X,Y,Z is");
    for (int i=0; i<3; i++)
    {
        Serial.print(realAccel[i]);
        Serial.println("g");
    }
    //print both the level, and tilt-compensated headings below to compare

    return realAccel;
    }

    float averagedist(){
      int average;
      float averagec;

      for(int i = 0; i<50; i++){
        average = avstandsmatare();
        if(average<4500){
          averagec=average;
          }
        else{
          i--;
          }
        }
        averagec= averagec/50;
        
      return averagec;
      }

void loop() {
  // put your main code here, to run repeatedly:
  switch(state){

    //APP_PRODUCE
    case 1:
    compassvalue = compass(); //Takes a measurement of the compass and saves it in compassvalue
    runLeftWheel(255, true);  //Starts the left wheel
    runRightWheel(255, true); //Starts the right wheel
    delay(2000);              //Delays for 3 seconds
    brakeLeftWheel();         //Stops the wheel
    brakeRightWheel();        //Stops the wheel

    delay(500);
    state = 2;                //Goes to Run
    
    //state=5;                //Goes to Debug
    break;

    //RUN
    case 2:
      runLeftWheel(255, true);  //Starts the left wheel
      runRightWheel(255, true); //Starts the right wheel
      delay(2000);              //Delays for 3 
      
      //Takes the average of 50 measurements
      for(int i = 0; i<50; ++i){
       averagecm +=avstandmatare();
       }
       averagecm=averagecm/50;

      //If the average is less than 10 cm it goes to state Throw
      if(averagecm<=10){
        state = 3;
        }
      //Else if a wall is in less than 50 cm it turns the wheel for 1 second
      else if(averagecm<=50){
        spin = random(250,1001);
        way = random(0,2);
  
      if((way & 2) == 0) {
          runRightWheel(255,true);
          runLeftWheel(255, false);

          delay(spin);
          brakeRightWheel();
          brakeLeftWheel();
          } else {
          runLeftWheel(255, true);
          runRightWheel(255, false);
          delay(spin);
          brakeLeftWheel();
          brakeRightWheel();
        }
 
     delay(50);
        
        }
      
    break;

    //Throw
    case 3:

      
      brakeLeftWheel();          //Stops the left wheel
      brakeRightWheel();         //Stops the right wheel

      //Takes the average value of 50 measurements
      for(int i = 0; i<50; ++i){
       averagecm +=avstandmatare();
       }
       averagecm=averagecm/50;
       
      float kompass = compass();
      
      if(averagecm<50){
        state = 4; 
      } else{
         runRightWheel(255, true); //Starts both motors
         runLeftWheel(255, true);

          delay(200);              //Delay i 200 ms
        }
       

     
    break;

      case 4: 
      if(compassvalue-kompass<5 & compassvalue-kompass<-5){ //Checks if the degree is acceptable
         servomotor();              //Throws ball
      
          state = 2;
        }
        else if(compassvalue-kompass>5){
           runRightWheel(255, true); //Start Left wheel
           delay(100);               //delay for 100 ms
           brakeRightWheel();        //brake right wheel
           delay(100);               //delay for 100 ms
          } else {
           runLeftWheel(255, true); //Start right wheel
           delay(100);              //delay for 100 ms
           brakeLeftWheel();         //brake Left wheel
           delay(100);              //Delay for 100 ms
            }
        

    break;
    //HALT
    case 0:
    break;
    
    //Debug
    case 5:
    runRightWheel(255, true);
    runLeftWheel(255, true);
    delay(3000);
    runRightWheel(255, false);
    runLeftWheel(255, false);
    delay(3000);

    brakeRightWheel();
    brakeLeftWheel();
    delay(1000);

    Serial.println(avstandsmatare());
    Serial.println(averagedist());
    Serial.println(compass());
    state = 6;
    break;
    //Debug 2
    case 6:
     digitalWrite(12, HIGH); //Establishes forward direction of Channel A
     digitalWrite(9, LOW);   //Disengage the Brake for Channel A
     analogWrite(3, 255);   //Spins the motor on Channel A at full speed
  
     digitalWrite(13, HIGH); //Establishes forward direction of Channel B
     digitalWrite(8, LOW);   //Disengage the Brake for Channel B
     analogWrite(11, 255);   //Spins the motor on Channel B at full speed

     delay(3000);
  
     digitalWrite(9, HIGH); //Eengage the Brake for Channel A
     digitalWrite(8, HIGH); //Eengage the Brake for Channel B


     delay(1000);
  
  //backward @ half speed
    digitalWrite(12, LOW); //Establishes backward direction of Channel A
    digitalWrite(9, LOW);   //Disengage the Brake for Channel A
    analogWrite(3, 123);   //Spins the motor on Channel A at half speed
  
    digitalWrite(13, LOW); //Establishes backward direction of Channel B
    digitalWrite(8, LOW);   //Disengage the Brake for Channel B
    analogWrite(11, 123);   //Spins the motor on Channel B at half speed
  
    delay(3000);
  
    digitalWrite(9, HIGH); //Eengage the Brake for Channel A
    digitalWrite(8, HIGH); //Eengage the Brake for Channel B
  
    delay(1000);

    // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
      digitalWrite(trigPin, LOW);
      delayMicroseconds(5);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);
 
      // Read the signal from the sensor: a HIGH pulse whose
      // duration is the time (in microseconds) from the sending
      // of the ping to the reception of its echo off of an object.
      pinMode(echoPin, INPUT);
      duration = pulseIn(echoPin, HIGH);
 
      // Convert the time into a distance
      cm = (duration/2) / 29.1;     // Divide by 29.1 or multiply by 0.0343
      inches = (duration/2) / 74;   // Divide by 74 or multiply by 0.0135
  
      Serial.print(inches);
      Serial.print("in, ");
      Serial.print(cm);
      Serial.print("cm");
      Serial.println();

      delay(250);

      analogWrite(5, 123);
      delay(3000);

      Serial.print("startar servomotorn");
      for (pos = 0; pos <= 180; pos += 1) { 
       // in steps of 1 degree
        servo.write(pos);              
        delay(5);                       
      }
      
      for (pos = 180; pos >= 0; pos -= 1) { 
        servo.write(pos);              
        delay(5);                       
      }
    break;
    }
  }
