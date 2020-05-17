#include <Servo.h>

/*
Info om states
0 HALT sätter stop för roboten
1 APP_PRODUCE Startar allt innan normal drift
2 RUN för att köra normalt
3 THROW när bollen ska kastas
4 Back & forth
5 Initiate - roboten rullar in till mitten av arenan
6 Spin & search
*/

Servo servo;  
int state = 0;
int trigPin = 11;    // Trigger
int echoPin = 12;    // Echo
int spin;
int way;
long duration, cm, inches, averagecm;
int pos = 0; 
  
void setup() {
Serial.begin(9600);
servo.attach(5);

  //Setup Channel A and B
  pinMode(12, OUTPUT); //Initiates Motor Channel A pin
  pinMode(9, OUTPUT);  //Initiates Brake Channel A pin
  pinMode(13, OUTPUT); //Initiates Motor Channel B pin
  pinMode(8, OUTPUT);  //Initiates Brake Channel B pin
  
  //Setup Servo
  pinMode(4, OUTPUT); // Initiates Servo
  
  state = 7;

}

void loop() {
  // put your main code here, to run repeatedly:
  switch(state){
    
    case 1:
    runLeftWheel(200, true); //Startar motorn för den vänstra hjulet
    runRightWheel(200, true); //Startar motorn för den högra hjulet
    delay(2000);
    brakeRightWheel();
    brakeLeftWheel();
    
    delay(500);
    state = 6;
    break;

    //RUN
    case 2:
      //Tar snittet av 50 mätningar för avståndsmätaren
      for(int i = 0; i<50; ++i){
       averagecm +=avstandmatare();
       }
       averagecm=averagecm/50;

      //Ifall snittet ligger under 10 cm finns en boll och den går till state THROW
      if(averagecm<=10){
        state = 3;
        }
      //Annars om en vägg är inom 50 cm så svänger motorn i 1 sekund
      else if(averagecm<=50){
        runLeftWheel(255, false);
        delay(1000);
        runLeftWheel(255, true);
        }
      
    break;

    //Throw
    case 3:
  
      brakeLeftWheel();   //Stannar den vänstra motorn
      brakeRightWheel();  //Stannar den högra motorn
    
      servomotor();       //Kastar bollen

      runLeftWheel(255, true);  //Startar den högra motorn
      runRightWheel(255, true); //Startar den vänstra motorn
    
    
      state = 2;                //Går tillbaka till state RUN
    break;

    //HALT
    case 0:
    break;

    // Back & forth
    case 4:
    
  //forward @ full speed
  digitalWrite(12, HIGH); //Establishes forward direction of Channel A
  digitalWrite(9, LOW);   //Disengage the Brake for Channel A
  digitalWrite(13, HIGH); //Establishes forward direction of Channel B
  digitalWrite(8, LOW);   //Disengage the Brake for Channel B
  analogWrite(3, 200);   //Spins the motor on Channel A at full speed
  analogWrite(11, 200);   //Spins the motor on Channel B at full speed
  
  
  delay(3000);
  
  digitalWrite(9, HIGH); //Engage the Brake for Channel A
  digitalWrite(8, HIGH); //Engage the Brake for Channel B
  
  delay(1000);
  
  //backward @ half speed
  digitalWrite(12, LOW); //Establishes backward direction of Channel A
  digitalWrite(9, LOW);   //Disengage the Brake for Channel A
  digitalWrite(13, LOW); //Establishes backward direction of Channel B
  digitalWrite(8, LOW);   //Disengage the Brake for Channel B
  analogWrite(3, 200);   //Spins the motor on Channel A
  analogWrite(11, 200);   //Spins the motor on Channel B
    
  delay(3000);
  
  digitalWrite(9, HIGH); //Engage the Brake for Channel A
  digitalWrite(8, HIGH); //Engage the Brake for Channel B 

  delay(1000);
  break;

  // Initiate 
  case 5:
  digitalWrite(12, HIGH); //Establishes forward direction of Channel A
  digitalWrite(9, LOW);   //Disengage the Brake for Channel A
  digitalWrite(13, HIGH); //Establishes forward direction of Channel B
  digitalWrite(8, LOW);   //Disengage the Brake for Channel B
  analogWrite(3, 150);   //Spins the motor on Channel A at full speed
  analogWrite(11, 150);   //Spins the motor on Channel B at full speed
  
  delay(3000);
  
  digitalWrite(9, HIGH); //Engage the Brake for Channel A
  digitalWrite(8, HIGH); //Engage the Brake for Channel B

  analogWrite(3, 0);   
  analogWrite(11, 0); 

  delay(2000);
  state = 6;
  break;

  // Spin & search
  case 6:
  spin = random(500,2000);
  way = random(0,2);
  
  if((way & 2) == 0) {
    runRightWheel(200,true);
    delay(spin);
    brakeRightWheel();
  } else {
    runLeftWheel(200,true);
    delay(spin);
    brakeLeftWheel();
  }
 
  delay(50);
  runRightWheel(200,true);
  runLeftWheel(200, true);
  delay(3000);
  brakeRightWheel();
  brakeLeftWheel();
  delay(50);
  state = 7;
  break;

  // Pick-up & Time To Yeet
  case 7:
  
  digitalWrite(4, HIGH);
  Serial.println("Case 7");
  servo.write(95); // Stop

    // Testkod
//  pos = 0;
//  for(pos = 0;pos <= 150;pos++) {
//    Serial.println(pos);
//    servo.write(pos);
//    delay(20);
//  }


    // Koden här fungerar ifall man matar direkt från 5V  
//servo.write(96);
//delay(500);
//servo.write(95);
//delay(1000);
//
//servo.write(120);              
//delay(295);
//
//servo.write(95);
//delay(2000);
//
//servo.write(93);
//delay(1760);
//servo.write(95);
//delay(500);
  
  break;
  
 
  }
}


/**
 * Sets forward or backwards momentum for the left Wheel and disengages the brake
 * byte speed is the speed of the motor from 0-255
 * bool fwd is the direction of the motor where true is forward and false is backward momentum
 */
void runLeftWheel(byte speed, bool fwd){
  if(fwd==true){
    digitalWrite(13, HIGH); //Establishes forward directios of Channel B
  }
  else{
    digitalWrite(13, LOW); //Establishes backward directions of Channel B
  }
    digitalWrite(8, LOW); //Disengage the Brake for Channel B
    analogWrite(11, speed); //Spins the motor on Channel B at the speed of byte speed
  
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
    digitalWrite(12, HIGH); //Establishes forward directios of Channel B
  }
  else{
    digitalWrite(12, LOW); //Establishes backward directions of Channel B
  }
    digitalWrite(9, LOW); //Disengage the Brake for Channel B
    analogWrite(3, speed); //Spins the motor on Channel B at the speed of byte speed
}

/**
 * Turns on the brakes for the right wheel
 */
void brakeRightWheel(){
  digitalWrite(9, HIGH); //Engages the Brakes for Channel A
  analogWrite(3, 0);
  }










/**
 * Startar avståndsmätaren och mäter än gång distansen till väggen
 * returner värdet av distansen i cm för en mätning
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
