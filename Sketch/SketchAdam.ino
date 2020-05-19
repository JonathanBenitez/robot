#include <Servo.h>

// Kompass
#include <LSM303D.h>
#include <Wire.h>
#include <SPI.h>


/** Information om states
    0: Halt - ingenting händer
    1: Initiate - roboten rullar in till mitten av arenan
    2: Spin & search - snurra runt och leta efter boll / vägg. Backa ifall man stöter på något. UTVECKLA VIDARE
    3: Pick-up & Time To Yeet - Bollen plockas upp och kastas. Ska via Position först
    4: Position - Positionerar oss rätt mot motståndarplanen
    5: Back & forth - testcase för att köra fram och tillbaka
    6:
    7:
    8:
    9: Testcase
    10:Testcase
 l
*/

Servo servo;
int begstate;
int state;



int spin;
int way;
boolean forward = true;
boolean backward = false;

int pos = 0;
int center;


int averagecm = 0;


/* Setup Distance meters - nu måste vi deklarera nya matningara för alla fyra, kanske lättare om vi sätter alla till samma */
long duration, distance, inches;

int distFront;
int distBall;
int distRight;
int distLeft;

int trigFront = 39;
int trigBall;
int trigRight = A9;
int trigLeft;

int echoFront = 37;
int echoBall;
int echoRight = A10;;
int echoLeft;



int accel[3];  // we'll store the raw acceleration values here
int mag[3];  // raw magnetometer values stored here
float realAccel[3];  // calculated acceleration values here
float heading, titleHeading;
#define SPI_CS 10



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



  /* Front */
  pinMode(trigFront, OUTPUT);
  pinMode(echoFront, INPUT);
  pinMode(41, OUTPUT);
  pinMode(35, OUTPUT);
  digitalWrite(41, HIGH);
  digitalWrite(35, LOW);

  /* Ball */

  /* Right */
  pinMode(trigRight, OUTPUT);
  pinMode(echoRight, INPUT);
  pinMode(A8, OUTPUT);
  pinMode(A11, OUTPUT);
  digitalWrite(A8, HIGH);
  digitalWrite(A11, LOW);

  /* Left */


  /* Beginning state */
  begstate = 1;
  state = begstate;
}

void loop() {

  switch (state) {

    case 10:
    Serial.println(getPosition());
    delay(1000);
    break;

    /* Testcase 9: Samma som bnf med med metoderna */
    case 9:
      Serial.println("Fram 3s");
      runRightWheel(50, true);
      //runLeftWheel(50, true);
      delay(3000);
      Serial.println("Paus efter fram 3s");
      brakeRightWheel();
      //brakeLeftWheel();
      delay(3000);
      Serial.println("Bak 3s");
      runRightWheel(50, false);
      //runLeftWheel(50, false);
      delay(3000);
      Serial.println("Paus efter bak 3s");
      brakeRightWheel();
      //brakeLeftWheel();
      delay(3000);
      //state = 5;
      break;

    /* Testcase 5: Back & forth */
    case 5:

      Serial.println("Fram 3s");
      digitalWrite(12, HIGH); //Establishes forward direction of Channel A
      digitalWrite(9, LOW);   //Disengage the Brake for Channel A
      analogWrite(3, 50);   //Spins the motor on Channel A at full speed

      digitalWrite(13, HIGH); //Establishes forward direction of Channel B
      digitalWrite(8, LOW);   //Disengage the Brake for Channel 
      analogWrite(11, 50);   //Spins the motor on Channel B at full speed

      delay(3000);

      Serial.println("Paus efter fram 3s");
      digitalWrite(9, HIGH); //Eengage the Brake for Channel A
      digitalWrite(8, HIGH); //Eengage the Brake for Channel B
      analogWrite(3, 0);
      analogWrite(11, 0);

      delay(3000);

      Serial.println("Bak 3s");
      //digitalWrite(12, LOW); //Establishes forward direction of Channel A
      //digitalWrite(9, LOW);   //Disengage the Brake for Channel A
      analogWrite(3, 50);   //Spins the motor on Channel A at full speed

      //digitalWrite(13, LOW); //Establishes forward direction of Channel A
      //digitalWrite(8, LOW);   //Disengage the Brake for Channel A
      //analogWrite(11, 50);   //Spins the motor on Channel A at full speed

      delay(3000);

      Serial.println("Paus efter bak 3s");
      digitalWrite(9, HIGH); //Eengage the Brake for Channel A
      digitalWrite(8, HIGH); //Eengage the Brake for Channel B

      delay(3000);
      //state = 9;
      break;

    /* Case 0: Halt*/
    case 0:
      break;

    /* Case 1: Initiate */
    case 1:
      Serial.println("Case 1: Initiate");

      runRightWheel(25, true);
      runLeftWheel(25, true);
      delay(3000);
      brakeRightWheel();
      brakeLeftWheel();
      delay(2000);

      center = getPosition();
      Serial.print("Center is this many degrees of north: ");
      Serial.println(center);
      delay(1000);

      state = 2;
      break;

    /**
       Spin & search: Skapat en randomfunktion som väljer håll och tid som roboten snurrar,
       och sedan kör den rakt fram tills den slår en väg/boll.
    */
    case 2:
      Serial.println("Case 2: Spin & Search");

      while (true) {

        // Spin
        spin = random(500, 2000);
        way = random(0, 3); // Number betwenn 1 and 2 - decides which way to spin
        Serial.print("Spin: ");
        if ((way & 2) == 0) {
          Serial.println("Left");
          runRightWheel(50, true);
          delay(2 * spin);
          //brakeRightWheel();
        } else {
          Serial.println("Right");
          runLeftWheel(50, true);
          delay(2 * spin);
          //brakeLeftWheel();
        }

        // Run forward until hit wall - then back up for 1s and repeat
        getDistance(trigFront, echoFront);
        while (distance > 10) {
          getDistance(trigFront, echoFront);
          //Serial.println(distance);
          runRightWheel(50, true);
          runLeftWheel(50, true);
        }
        
        Serial.println("Stop");
        brakeRightWheel();
        brakeLeftWheel();
        delay(2000);
        Serial.print("Backa");
        runRightWheel(50, false);
        runLeftWheel(50, false);
        delay(1000);
        brakeRightWheel();
        brakeLeftWheel();
        delay(3000);
        Serial.print("");
        Serial.println("Börjar vi om igen");
        delay(2000);

      }

      state = 7;
      break;


    /**
       Pick-up & Time To Yeet. Bollen plockas upp, ska byta state till Position och sedan kasta. Koden fungerar ifall
       man matar från 5V men då snurrar den även när den inte ska. Matning från någon annan pin ger ett för stort
       spänningsfall misstänker jag?
    */
    case 3:
      Serial.println("Case 3: Pick-up & Time To Yeet");
      servo.attach(6);
      //digitalWrite(4, HIGH);

      // servo.write(360); // Stop

      // Testkod
      // pos = 0;
      //  for(pos = 0;pos <= 150;pos++) {
      //    Serial.println(pos);
      //    servo.write(pos);
      //    delay(20);
      //  }


      // Koden här fungerar ifall man matar direkt från 5V
      Serial.println("Hello");
      servo.write(96);
      delay(750);
      servo.write(95);
      delay(1000);

      servo.write(120);
      delay(300);

      servo.write(95);
      delay(2000);

      servo.write(93);
      delay(1760);
      servo.write(95);
      delay(500);

      break;

    /* Case 4: Position */
    case 4:
      Serial.println("Case 4: Position");
      Serial.print("We are this many degrees of center: ");
      Serial.println(getPosition());
      delay(2000);
      turnToCenter(getPosition());
      break;

  }
}


/**
   Sätter hastighet och rikting för högerhjulet. För tillfället fungerar inte bakriktingen vilket den har gjort tidigare. Kanske ska testa med en annan DC-motor?
*/
void runLeftWheel(byte speed, boolean direction) {
  //Serial.print("runLeftWheel(): ");
  if (direction == true) {
    //Serial.println("Forward");
    digitalWrite(13, HIGH); //Establishes forward directios of Channel B
  } else {
    //Serial.println("Backward");
    digitalWrite(13, LOW); //Establishes backward directions of Channel B
  }
  digitalWrite(8, LOW); //Disengage the Brake for Channel B
  analogWrite(11, speed); //Spins the motor on Channel B at the speed of byte speed

}

/**
   Bromsar högerhjulet och sätter hastigheten till 0
*/
void brakeLeftWheel() {
  //Serial.println("breakLeftWheel()");
  digitalWrite(8, HIGH); //Engages the Brakes for Channel B
  analogWrite(11, 0);
}

/**
   Sätter hastighet och rikting för högerhjulet. För tillfället fungerar inte bakriktingen vilket den har gjort tidigare. Kanske ska testa med en annan DC-motor?
*/
void runRightWheel(byte speed, boolean direction) {
  //  Serial.print("runRightWheel(): ");
  if (direction == true) {
    //Serial.println("Forward");
    digitalWrite(12, HIGH); //Establishes forward directios of Channel B
  } else {
    //Serial.println("Backward");
    digitalWrite(12, LOW); //Establishes backward directions of Channel B
  }
  digitalWrite(9, LOW); //Disengage the Brake for Channel B
  analogWrite(3, speed); //Spins the motor on Channel B at the speed of byte speed
}

/**
   Bromsar högerhjulet och sätter hastigheten till 0
*/
void brakeRightWheel() {
  //  Serial.println("breakRightWheel()");
  digitalWrite(9, HIGH);
  analogWrite(3, 0);
}



/**
   Startar avståndsmätarna och räknar ut avstånden. Funderar på om man ska göra så den räknar ut alla avstånden. Smidigt om den går konstant men drar kanske en del ström
*/
void getDistance(int trig, int echo) {

  digitalWrite(trig, LOW); // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  delayMicroseconds(5);
  digitalWrite(trig, HIGH); // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  pinMode(echo, INPUT); // Read the signal from the sensor: a HIGH pulse whose duration is the time (in microseconds) from the sending of the ping to the reception of its echo.
  duration = pulseIn(echo, HIGH);

  // Convert the time into a distance
  distance = (duration / 2) / 29.1;   // Divide by 29.1 or multiply by 0.0343
  delay(50);

  return distance;
}


int getPosition() {
  char rtn = Lsm303d.initI2C();
  Lsm303d.getMag(mag);  // get the magnetometer values, store them in mag
  return Lsm303d.getHeading(mag);
}

void turnToCenter(int degrees) {
  Serial.println("turnToCenter()");
  int time;
  time =  1000; // Skapa en funktion av grader ifrån norr
  delay(time);
  brakeRightWheel();
  brakeLeftWheel();
  Serial.println("End");
  state = 0; // Kasta bollen
}

void OGCompass() {
  // Setup kompass
  char rtn = 0;
  Serial.begin(9600);  // Serial is used for debugging
  Serial.println("\r\npower on");
  rtn = Lsm303d.initI2C();
  //rtn = Lsm303d.initSPI(SPI_CS);
  if (rtn != 0) // Initialize the LSM303, using a SCALE full-scale range
  {
    Serial.println("\r\nLSM303D is not found");
    while (1);
  }
  else
  {
    Serial.println("\r\nLSM303D is found");
  }

  Serial.println("\r\n**************");
  //getLSM303_accel(accel);  // get the acceleration values and store them in the accel array
  Lsm303d.getAccel(accel);
  while (!Lsm303d.isMagReady()); // wait for the magnetometer readings to be ready
  Lsm303d.getMag(mag);  // get the magnetometer values, store them in mag

  for (int i = 0; i < 3; i++)
  {
    realAccel[i] = accel[i] / pow(2, 15) * ACCELE_SCALE;  // calculate real acceleration values, in units of g
  }
  heading = Lsm303d.getHeading(mag);
  titleHeading = Lsm303d.getTiltHeading(mag, realAccel);

  printValues();

  delay(2000);  // delay for serial readability
}

void printValues() {
  Serial.println("Acceleration of X,Y,Z is");
  for (int i = 0; i < 3; i++)
  {
    Serial.print(realAccel[i]);
    Serial.println("g");
  }
  //print both the level, and tilt-compensated headings below to compare
  Serial.println("The clockwise angle between the magnetic north and x-axis: ");
  Serial.print(heading, 3); // this only works if the sensor is level
  Serial.println(" degrees");
  Serial.print("The clockwise angle between the magnetic north and the projection");
  Serial.println(" of the positive x-axis in the horizontal plane: ");
  Serial.print(titleHeading, 3);  // see how awesome tilt compensation is?!
  Serial.println(" degrees");
}
