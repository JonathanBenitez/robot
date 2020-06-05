#include <Servo.h>

// Kompass
#include <LSM303D.h>
#include <Wire.h>
#include <SPI.h>

/** Information om states
    0: Halt - ingenting händer
    1: Initiate - roboten rullar in till mitten av arenanb
    2: Spin & search - snurra runt och leta efter boll / vägg. Backa ifall man stöter på något. UTVECKLA VIDARE
    3: Pick-up & Time To Yeet - Bollen plockas upp och kastas. Ska via Position först
    4: Position - Positionerar oss rätt mot motståndarplanen
    5: Back & forth - testcase för att köra fram och tillbaka
    6:
    7:
    8:
    10:Testcase
  l
*/

Servo servo;
int begstate;
int state;


int degrees;
int spin;
int way;
boolean forward = true;
boolean backward = false;

int pos = 0;
int center = 0;



int averagecm = 0;


/* Setup Distance meters - nu måste vi deklarera nya matningara för alla fyra, kanske lättare om vi sätter alla till samma */
long duration, distance, inches;

int distFront = 0;
int distBall = 0;
int distRight = 0;
int distLeft = 0;
int distRatio = 0;
int distvector[5]  = {distFront, distBall, distRight, distLeft, distRatio};


int trigFront = 41;
int trigBall = 38;
int trigRight = A11;
int trigLeft = 20;
int trig[4] = {trigFront, trigBall, trigRight, trigLeft};

int echoFront = 39;
int echoBall = 40;
int echoRight = A12;
int echoLeft = 19;
int echo[4] = {echoFront, echoBall, echoRight, echoLeft};



/* Global variables */
int accel[3];  // we'll store the raw acceleration values here
int mag[3];  // raw magnetometer values stored here
float realAccel[3];  // calculated acceleration values here
float heading, titleHeading;
 
#define SPI_CS 10

void setup() {
  Serial.begin(9600);
  servo.attach(5);

  /* Setup Channel A and B */
  pinMode(12, OUTPUT); //Initiates Motor Channel A pin
  pinMode(9, OUTPUT);  //Initiates Brake Channel A pin
  pinMode(13, OUTPUT); //Initiates Motor Channel B pin
  pinMode(8, OUTPUT);  //Initiates Brake Channel B pin

  /* Setup Servo */
  pinMode(4, OUTPUT); // Initiates Servo
  digitalWrite(4, HIGH);

  /* Front */
  pinMode(trigFront, OUTPUT);
  pinMode(echoFront, INPUT);
  pinMode(43, OUTPUT);
  pinMode(37, OUTPUT);
  digitalWrite(43, HIGH);
  digitalWrite(37, LOW);

  /* Ball */
  pinMode(trigBall, OUTPUT);
  pinMode(echoBall, INPUT);
  pinMode(36, OUTPUT);
  pinMode(42, OUTPUT);
  digitalWrite(36, HIGH);
  digitalWrite(42, LOW);


  /* Right */
  pinMode(trigRight, OUTPUT);
  pinMode(echoRight, INPUT);
  pinMode(A10, OUTPUT);
  pinMode(A13, OUTPUT);
  digitalWrite(A10, HIGH);
  digitalWrite(A13, LOW);

  /* Left */
  pinMode(trigLeft, OUTPUT);
  pinMode(echoLeft, INPUT);
  pinMode(21, OUTPUT);
  pinMode(18, OUTPUT);
  digitalWrite(21, HIGH);
  digitalWrite(18, LOW);



  /* Beginning state */
  begstate = 14;
  state = begstate;
}

void loop() {

  switch (state) {

  case 14:
  OGCompass();
  delay(5000);
  break;

    case 13:
      degrees = random(-360, 360);
      Serial.println(degrees);
      delay(200);
      break;


    /* Testcase 11: Testa metoden turnDegrees()*/
    case 11:
      turnDegrees(-360);
      break;

    /* Testcase 10: Testa metoderna för avståndsmätning samt detektering av boll */
    case 10:
      Serial.print("Distance front: ");
      Serial.println(getDistance(trigFront, echoFront));
      Serial.print("Distance ball: ");
      Serial.println(getDistance(trigBall, echoBall));
      Serial.print("Ratio: ");
      Serial.println(distFront - distBall);
      Serial.print("Distance left: ");
      Serial.println(getDistance(trigLeft, echoLeft));
      Serial.print("Distance Right: ");
      Serial.println(getDistance(trigRight, echoRight));
      Serial.println("------------------------------------------------------");
      delay(2000);
      break;


    /* Case 0: Halt*/
    case 0:
      Serial.println("Halt");
      break;

    /* Case 1: Initiate */
    case 1:
      Serial.println("Case 1: Initiate");
      runWheels(200, forward);
      delay(3000);
      brakeWheels();
      delay(2000);


      
      Serial.print("Center is this many degrees of north: ");
      Serial.println(getPosition());
      delay(2000);

      state = 2;
      break;

    /**
       Spin & search: Skapat en randomfunktion som väljer håll och tid som roboten snurrar,
       och sedan kör den rakt fram tills den slår en väg/boll.
    */
    case 2:
      Serial.println("Case 2: Spin & Search");

      while (state == 2) {

        // Spin
        degrees = random(-360, 360); // Number between -360  and 360 - decides which way to spin
        turnDegrees(degrees);

        // Run forward until hit wall - then back up for 1s and repeat. Måste fixa så den märker när den har en boll -> kasta sen.
        while (getDistance(trigFront, echoFront) > 25) {
          //Serial.println(getDistance(trigFront, echoFront));
          //Serial.println(getDistance(trigBall, echoBall));
          runWheels(200, forward);
        }

        Serial.println(distFront - distBall);
        if (distFront - distBall > 8 || distFront - distBall < 0 || distBall < 0) {
          Serial.println("Vi har bollen");
          brakeWheels();
          state = 3;
        }

        if (state != 3) {

          Serial.println("Stop");
          brakeWheels();
          delay(1000);
          Serial.print("Backa");
          runWheels(200, backward);
          delay(2000);
          brakeWheels();
          delay(500);

          // Om vi är nära väggarna
          if (getDistance(trigLeft, echoLeft) < 10 || getDistance(trigRight, echoRight) < 10) {
            if (getDistance(trigLeft, echoLeft) < 10) {
              Serial.print("Distance to the left is: ");
              Serial.println(getDistance(trigLeft, echoLeft));
              runRightWheel(200, forward);
              runLeftWheel(0.65 * 200, backward);
              delay(625);
              brakeRightWheel();
              brakeLeftWheel();
            } else if (getDistance(trigRight, echoRight) < 10) {
              Serial.print("Distance to the right is: ");
              Serial.println(getDistance(trigRight, echoRight));
              runRightWheel(200, forward);
              runLeftWheel(0.65 * 200, backward);
              delay(625);
              brakeRightWheel();
              brakeLeftWheel();
            }
            runWheels(200, forward);
            delay(625);
            brakeWheels();
          }

          Serial.print("Börjar vi om igen");
          delay(1000);
        }
      }
      break;


    /**
       Pick-up & Time To Yeet. Bollen plockas upp, ska byta state till Position och sedan kasta. Koden fungerar ifall
       man matar från 5V men då snurrar den även när den inte ska. Matning från någon annan pin ger ett för stort
       spänningsfall misstänker jag?
    */
    case 3:
      Serial.println("Case 3: Pick-up & Time To Yeet");
      digitalWrite(4, HIGH);
      runWheels(100, backward);
      delay(750);
      brakeWheels();

      Serial.println("Pick-up");
      servo.write(100);
      delay(300);

      // Stabilisering, wingat koden
      while (true) {
        servo.write(96);
        delay(150);
        servo.write(95);
        delay(25);
      }

      //state = 4; // Positionerar sig

      Serial.println("Yeet");
      servo.write(150);
      delay(300);
      servo.write(95);
      delay(2000);

      Serial.println("Let down");
      servo.write(93);
      delay(1760);
      servo.write(95);
      delay(500);

      //state = 2; // Gå tillbaka till Spin & Search
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

/* Slut på cases, metoder under */
/*--------------------------------------------------------------------------------------------------------------------------------------------------------------*/


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




/** Startar avståndsmätarna och räknar ut avstånden. Tar ungefär 2.5s, sjukt ineffektivt */
void getDistance() {

  for (int i = 0; i < 4; i++) {
    digitalWrite(trig[i], LOW); // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
    delayMicroseconds(5);
    digitalWrite(trig[i], HIGH); // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
    delayMicroseconds(10);
    digitalWrite(trig[i], LOW);
    pinMode(echo[i], INPUT); // Read the signal from the sensor: a HIGH pulse whose duration is the time (in microseconds) from the sending of the ping to the reception of its echo.
    duration = pulseIn(echo[i], HIGH);

    // Convert the time into a distance
    distvector[i] = (duration / 2) / 29.1;   // Divide by 29.1 or multiply by 0.0343
    delay(50);
  }

  distFront = distvector[0];
  distBall = distvector[1];
  distRight = distvector[2];
  distLeft = distvector[3];
  distRatio = (distvector[0] - distvector[1]) / distvector[1];
  distvector[4] = distRatio;

}

/** Räknar ut avståndet för en specifik avståndsmätare. Snabbare att göra 4st anrop?*/
int getDistance(int trig, int echo) {
  int dist = 0;

  digitalWrite(trig, LOW); // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  delayMicroseconds(5);
  digitalWrite(trig, HIGH); // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  pinMode(echo, INPUT); // Read the signal from the sensor: a HIGH pulse whose duration is the time (in microseconds) from the sending of the ping to the reception of its echo.
  duration = pulseIn(echo, HIGH);

  // Convert the time into a distance
  dist = (duration / 2) / 29.1;   // Divide by 29.1 or multiply by 0.0343
  if (trig == trigFront) {
    return distFront = dist;
  } else if (trig == trigBall) {
    return distBall = dist;
  } else if (trig == trigLeft) {
    return distLeft = dist;
  } else if (trig == trigRight) {
    return distRight = dist;
  }
  delay(50);
}


int getPosition() {
  char rtn = Lsm303d.initI2C();
  Lsm303d.getMag(mag);  // get the magnetometer values, store them in mag
  return Lsm303d.getHeading(mag);
}

void turnToCenter(int degrees) {
  Serial.println("turnToCenter()");
  int time;
  time =  2000; // Skapa en funktion av grader ifrån norr
  delay(time);
  brakeRightWheel();
  brakeLeftWheel();
  Serial.println("End");
}

/* Får asskeva värden?? Vill inte bromsa */
void turnDegrees(int degrees) {
  long time;
  long constant = 1420; // Tar ca 2.5s att snurra ett helt varv
  time = (abs(degrees) * constant) / 360; // 7 Blir skevt värde ??
  Serial.println(time);

  if (degrees > 0) {
    runRightWheel(200, true);
    runLeftWheel(200, false);
  } else {
    runRightWheel(200, false);
    runLeftWheel(200, true);
  }
  delay(time);
  brakeRightWheel();
  brakeLeftWheel();
  delay(1000);
}


/* Istället för att kalla på båda alltid */
void runWheels(byte speed, boolean direction) {
  runRightWheel(speed, direction);
  runLeftWheel(0.65 * speed, direction);
}

/* Istället för att kalla på båda alltid */
void brakeWheels() {
  brakeRightWheel();
  brakeLeftWheel();
}

/* Sätter hastighet och rikting för högerhjulet */
void runRightWheel(byte speed, boolean direction) {
  //Serial.print("runRightWheel(): ");
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

/* Bromsar högerhjulet och sätter hastigheten till 0*/
void brakeRightWheel() {
  //Serial.println("breakRightWheel()");
  digitalWrite(9, HIGH);
  analogWrite(3, 0);
}




/* Skräp under */

/**-----------------------------------------------------------------------------------------------------------------------------------------------------*/


/* Testcase 9: Samma som bnf med med metoderna. Fungerar bra! Bugga innan bara? */
//    case 9:
//      Serial.println("Fram 3s");
//      runRightWheel(200, true);
//      runLeftWheel(200, true);
//      delay(3000);
//      Serial.println("Paus efter fram 3s");
//      brakeRightWheel();
//      brakeLeftWheel();
//      delay(3000);
//      Serial.println("Bak 3s");
//      runRightWheel(200, false);
//      runLeftWheel(200, false);
//      delay(3000);
//      Serial.println("Paus efter bak 3s");
//      brakeRightWheel();
//      brakeLeftWheel();
//      delay(3000);
//      break;


/* Testcase 5: Back & forth. Fungerar bra! */

//    case 5:
//
//      Serial.println("Fram 3s");
//      digitalWrite(12, HIGH); //Establishes forward direction of Channel A
//      digitalWrite(9, LOW);   //Disengage the Brake for Channel A
//      analogWrite(3, 200);   //Spins the motor on Channel A at full speed
//
//      digitalWrite(13, HIGH); //Establishes forward direction of Channel B
//      digitalWrite(8, LOW);   //Disengage the Brake for Channel
//      analogWrite(11, 200);   //Spins the motor on Channel B at full speed
//
//      delay(3000);
//
//      Serial.println("Paus efter fram 3s");
//      digitalWrite(9, HIGH); //Eengage the Brake for Channel A
//      digitalWrite(8, HIGH); //Eengage the Brake for Channel B
//      analogWrite(3, 0);
//      analogWrite(11, 0);
//
//      delay(3000);
//
//      Serial.println("Bak 3s");
//      digitalWrite(12, LOW); //Establishes forward direction of Channel A
//      digitalWrite(9, LOW);   //Disengage the Brake for Channel A
//      analogWrite(3, 200);   //Spins the motor on Channel A at full speed
//
//      digitalWrite(13, LOW); //Establishes forward direction of Channel A
//      digitalWrite(8, LOW);   //Disengage the Brake for Channel A
//      analogWrite(11, 200);   //Spins the motor on Channel A at full speed
//
//      delay(3000);
//
//      Serial.println("Paus efter bak 3s");
//      digitalWrite(9, HIGH); //Eengage the Brake for Channel A
//      digitalWrite(8, HIGH); //Eengage the Brake for Channel B
//
//      delay(3000);
//      break;



void OGCompass() {
  // Setup kompass
  char rtn = 0;
  Serial.begin(9600);  // Serial is used for debugging
  Serial.println("\r\npower on");
  rtn = Lsm303d.initI2C();
  //rtn = Lsm303d.initSPI(SPI_CS);
  Serial.println("\r\Här");
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
