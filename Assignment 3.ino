#include <Zumo32U4.h>
#include <Wire.h>
#include <Zumo32U4Buzzer.h>

Zumo32U4ProximitySensors proxSensors;
Zumo32U4LineSensors lineSensors;
Zumo32U4Motors motors;
Zumo32U4Buzzer buzzer;
Zumo32U4LCD display;
unsigned int sensors[5];
unsigned int pSensors[2];
const int blackThreshhold = 600;
const int turn_speed = 150;
const int turn_duration = 540;
const int forward_backward_speed = 100;
const int redThreshhold = 1000;
const int proxThreshhold = 6;
int countNone = 0;
int countLeft = 0;
int countRight = 0;
bool leftTurnMade = false;
bool cameFromLeft = false;
int leftTurn = 0;
int rightTurn = 0;
int goesForward = 0;


int toTurn = 0;
// class Direction{
//   char direction;
//   int duration;

//   Direction(char dir, int dur){
//     this.direction = dir;
//     this.duration = dur;
//   };
// }

void setup() {
  lineSensors.initFiveSensors();
  proxSensors.initFrontSensor();
}

void printDebugVals(){
    lineSensors.read(sensors);
  for (int i = 0; i < 5; i++) {
    Serial.print("Line sensor");
    Serial.print(i);
    Serial.print(" : ");
    Serial.print(sensors[i]);
    Serial.print("\t");
  }
  uint8_t leftValue = proxSensors.countsFrontWithLeftLeds();
  uint8_t rightValue = proxSensors.countsFrontWithRightLeds();
  Serial.print("Left prox: ");
  Serial.print(leftValue);
  Serial.print(" Right prox: ");
  Serial.print(rightValue);
  Serial.print("\t");
  Serial.println();
}

void pushCupFromMaze(){
  lineSensors.read(sensors);
  if(sensors[0] > blackThreshhold || sensors[2] > blackThreshhold || sensors[4] > blackThreshhold){
    turn90Clockwise();
  }
  else{
    motors.setSpeeds(forward_backward_speed, forward_backward_speed);
    pushCupFromMaze();
  }

}

void proxSensorsLoop(){
  proxSensors.read();
  float leftValue = proxSensors.countsFrontWithLeftLeds();
  float rightValue = proxSensors.countsFrontWithRightLeds();

  lineSensors.read(sensors);

  if (leftValue == proxThreshhold && rightValue == proxThreshhold){
    buzzer.playNote('C4', 20, 15);
    motors.setSpeeds(0,0);
    display.print(F("Person found"));
    pushCupFromMaze();
    display.clear();

    // This is where the program should start a new routine to get the robot not detecting the person found 
  }
  if (leftValue == proxThreshhold && rightValue != proxThreshhold){
    motors.setSpeeds(0, 0);
    motors.setSpeeds(-turn_speed, turn_speed);
    proxSensorsLoop();
  }
  if (rightValue == proxThreshhold && leftValue != proxThreshhold){
    motors.setSpeeds(0,0);
    motors.setSpeeds(turn_speed, -turn_speed);
    proxSensorsLoop();
  }
  
}

void turn90Clockwise(){
  motors.setSpeeds(turn_speed, -turn_speed);
  delay(turn_duration);
  motors.setSpeeds(0, 0);
  proxSensorsLoop();
}
void turn90AntiClockwise(){
  motors.setSpeeds(-turn_speed, turn_speed);
  delay(turn_duration);
  motors.setSpeeds(0, 0);
  proxSensorsLoop();
}
void turn45Clockwise(){
  motors.setSpeeds(turn_speed, -turn_speed);
  delay(turn_duration/2);
  motors.setSpeeds(0, 0);
}
void turn45AntiClockwise(){
  motors.setSpeeds(-turn_speed, turn_speed);
  delay(turn_duration/2);
  motors.setSpeeds(0, 0);
}

void stayWithinLines(){
    lineSensors.read(sensors);
    // If robot is in a corner turn clockwise
    if(sensors[0] > blackThreshhold && sensors[2] < blackThreshhold-250 && sensors[4] > blackThreshhold){
      //motors.setSpeeds(-forward_backward_speed, -forward_backward_speed);
      //delay(200);
      Serial.println("in corner");
      turn45Clockwise();
      countLeft = 0;
      countRight = 0;
      countNone = 0;
      leftTurn, rightTurn, goesForward = 0;
    }
     // If left black line is no longer there and the right black line is, turn left
    else if(sensors[0] < blackThreshhold && sensors[4] > blackThreshhold){
        if(rightTurn >=5 && leftTurn >=5 && goesForward >=5){
        turn90Clockwise();
        leftTurn, rightTurn = 0;
        display.write("Is stuck");
        }
        else{
          motors.setSpeeds(-150, 0);
          delay(200);
          motors.setSpeeds(-turn_speed, turn_speed);
          Serial.println("left turn");
          countLeft++;
          leftTurn++;
        }        
        
      } 
      // if right black line is no longer there but left is, turn right
    else if(sensors[0] > blackThreshhold && sensors[4] < blackThreshhold){
      if(rightTurn >=5 && leftTurn >=5 && goesForward >=5){
        turn90Clockwise();
        leftTurn, rightTurn, goesForward = 0;
        display.write("Is stuck");
      }
      else{
        motors.setSpeeds(0, -150);
        delay(200);
        motors.setSpeeds(turn_speed, -turn_speed);
        Serial.println("right turn");
        countRight++;
        rightTurn++;        
      }
    }
    
   // If black line is in front stop
    else if(sensors[0] > blackThreshhold &&  sensors[2] > (blackThreshhold-250)&&  sensors[4] > blackThreshhold){
      motors.setSpeeds(0, 0);
      motors.setSpeeds(-forward_backward_speed, -forward_backward_speed);
      delay(200);
      turn45Clockwise();
      leftTurnMade = false;
      Serial.println("black line in front");
      countLeft = 0;
      countRight = 0;
      countNone = 0;
      leftTurn, rightTurn, goesForward =0;
    }
    // Goes forward if no black lines are detected on either side
    else{
      motors.setSpeeds(forward_backward_speed, forward_backward_speed);
      Serial.println("goes forward");
      countNone++;
      //leftTurn, rightTurn = 0;
      goesForward++;
    }

    //if nothing been seen for 5 seconds
    if(countNone == 250)
    {
      motors.setSpeeds(-forward_backward_speed, -forward_backward_speed);
      delay(100);
      turn90AntiClockwise();
      turn90AntiClockwise();
      turn90AntiClockwise();
      countNone = 0;
      countLeft = 0;
      countRight = 0;
      display.clear();
      display.print("nowt");
      Serial.println("nothing seen");
}
    if(countLeft == 150)
    {
      motors.setSpeeds(-forward_backward_speed, -forward_backward_speed);
      delay(100);
      turn90Clockwise();
      display.clear();
      display.print("left seen");
      Serial.println("left seen");
      countNone = 0;
      countLeft = 0;
      countRight = 0;
    }
    if(countRight == 150)
    {
      motors.setSpeeds(-forward_backward_speed, -forward_backward_speed);
      delay(100);
      turn90AntiClockwise();
      turn90AntiClockwise();
      turn90AntiClockwise();
      // turn90AntiClockwise();
      display.clear();
      display.print("right seen");
      Serial.println("right seen");
      countNone = 0;
      countLeft = 0;
      countRight = 0;
    } 
}

void stayOnLine(){
  if (sensors[0] > blackThreshhold && sensors[2] < blackThreshhold && sensors[4] < blackThreshhold){
    motors.setSpeeds(forward_backward_speed, forward_backward_speed);
  }
  if (sensors[0] > blackThreshhold && sensors[2] > blackThreshhold && sensors[4] > blackThreshhold){
    motors.setSpeeds(-forward_backward_speed, -forward_backward_speed);
    delay(100);
    motors.setSpeeds(turn_speed, -turn_speed);
  }
  if (sensors[0] < blackThreshhold && sensors[2] < blackThreshhold && sensors[4] < blackThreshhold){
    turn90AntiClockwise();
    do{
      motors.setSpeeds(forward_backward_speed, forward_backward_speed);
      lineSensors.read(sensors);
    } while(sensors[0] < blackThreshhold && sensors[2] < blackThreshhold && sensors[4] < blackThreshhold);
  }
}

void loop() {

  // turn90AntiClockwise();
  //     turn90AntiClockwise();
  //     turn90AntiClockwise();
  //     turn90AntiClockwise();
  //   display.print("test");
  //     delay(20000);
  //stayOnLine();
  printDebugVals();
  stayWithinLines();
  proxSensorsLoop();
}