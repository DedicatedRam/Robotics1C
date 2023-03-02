#include <Wire.h>
#include <Zumo32U4.h>
#include <Keyboard.h>
#include <StopWatch.h>
#include <Zumo32U4Buzzer.h>

StopWatch sw_secs(StopWatch::SECONDS);
StopWatch sw_mil(StopWatch::MILLIS);
Zumo32U4Motors motors;
Zumo32U4ProximitySensors proxSensors;
Zumo32U4ButtonA buttonA;
Zumo32U4ButtonB buttonB;
Zumo32U4ButtonC buttonC;
Zumo32U4LineSensors lineSensors;
Zumo32U4Buzzer buzzer;
Zumo32U4LCD display;

//Global variables I will be using throughout the program
const int turn_speed = 150;
const int turn_duration = 550;
unsigned int sensors[3];
int turnSpeed = 150;
int proxThreshhold = 6;
int blackThreshold = 600;
int personsSaved = 0;
int selectedMode = 0;

// Mick likes starwars so this is for him
const char march[] PROGMEM = 
"! O2 T100 MS"
"a8. r16 a8. r16 a8. r16 f8 r16 >c16" 
"ML"
"a8. r16 f8 r16" "MS" ">c16 a. r8"
"O3"
"e8. r16 e8. r16 e8. r16 f8 r16 <c16"
"O2 ML"
"a-8. r16" "MS" "f8 r16 >c16 a r"

"O3 ML"
"a8. r16 <a8 r16 <a16" "MS" "a8. r16 a-8 r16 g16"
"ML V10"
"g-16 f16 g-16 r16 r8 <b-16 r16" "MS" "e-8. r16 d8." "ML" "d-16"
"c16 <c-16 c16 r16 r8" "MS O2" "f16 r16 a-8. r16 f8. a-16"
"O3"
"c8. r16 <a8 r16 c16 e2. r8"

"O3 ML"
"a8. r16 <a8 r16 <a16" "MS" "a8. r16 a-8 r16 g16"
"ML V10"
"g-16 f16 g-16 r16 r8 <b-16 r16" "MS" "e-8. r16 d8." "ML" "d-16"
"c16 <c-16 c16 r16 r8" "MS O2" "f16 r16 a-8. r16 f8. >c16"
"ML"
"a8. r16 f8 r16 >c16 a2. r8"
;
void setup() {
  proxSensors.initFrontSensor();
  lineSensors.initThreeSensors();
  Serial.begin(9600);
}

// Various turn functions, these are only semi accurate as they are timed to milliseconds not the gyro
// This means if new batterys are put in the robot these will have to be reconfigured
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

// Push from maze function that gets run in mode 1 where it increments the counter of people found and sends this to notification to the GUI.
void mode1PushFromMaze(){
  display.clear();
  display.print(String(personsSaved));
  display.print(" People");
  buzzer.playNote(NOTE_E(70), 200, 15);
  delay(200);
  buzzer.playNote(NOTE_G(10), 100, 15); 
  personsSaved++;
  Serial.write("personSaved");
}
// Push from maze for modes 2 and 3. This will push the object from the maze until the sensors detect a black line underneath and automatically turn as part of the autonomous features.
void pushFromMaze(){
  lineSensors.read(sensors);
  if(sensors[1] > blackThreshold-300 || sensors[2] > blackThreshold){
    display.clear();
    display.println(personsSaved + "People");
    buzzer.playNote(NOTE_E(70), 200, 15);
    delay(200);
    buzzer.playNote(NOTE_G(10), 100, 15); 
    motors.setSpeeds(-100, -100);
    delay(150);
    turn90Clockwise();
    personsSaved++;
    Serial.write("personSaved");
  }
  else{
    motors.setSpeeds(120, 120);
    pushFromMaze();
  }
}
// Prints debugs vals in the arduino console so that the user can see the line sensor values if they need to be changed for the thresholds
void printDebugVals(){
  proxSensors.read();
  lineSensors.read(sensors);

   for (int i = 0; i < 3; i++) 
  {
    Serial.print("Line sensor ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print(sensors[i]);
    Serial.print("\t");
  }
  int leftProxValue = proxSensors.countsFrontWithLeftLeds();
  int rightProxValue = proxSensors.countsFrontWithRightLeds();
  Serial.print("Left prox: ");
  Serial.print(leftProxValue);
  Serial.print(" Right prox: ");
  Serial.print(rightProxValue);
  Serial.print("\t");
  Serial.println();
}
// Mode 1 starts for fully autonomous control 
void mode1(){
  int incomingByte = 0; // For incoming serial data
  display.print("Mode 1");
  if (Serial.available() > 0) {
    sw_mil.reset();
    
    // Read the incoming byte
    incomingByte = Serial.read();

    if (incomingByte == 'w'){
      // Forward W
      motors.setSpeeds(200, 200);
      sw_mil.start();
      Serial.write('w');
      delay(20);
    }
    if (incomingByte == 'a') {
      // Left A
      motors.setSpeeds(-200, 200);
      sw_mil.start();
      Serial.write('a');
      delay(20);
    }
    if (incomingByte == 's'){
      // Back S
      motors.setSpeeds(-200, -200);
      sw_mil.start();
      Serial.write('s');
      delay(20);
    }
    if (incomingByte == 'd'){
      // Right D
      motors.setSpeeds(200, -200);
      sw_mil.start();
      Serial.write('d');
      delay(20);
    }
    if(incomingByte == 'e'){
      // This is when the user wishes to denote a person has been found, E is pushed
      Serial.write('e');
      mode1PushFromMaze();
      delay(20);
    }
    if(incomingByte == 0){
      motors.setSpeeds(0, 0);
      sw_mil.stop();
      int ms = sw_mil.elapsed();
    }

  }
}
// Mode 2 which is semi autonomous 
// Very similar to mode 1
void mode2(){
  int incomingByte = 0; // For incoming serial data
  display.print("Mode 2");
  lineSensors.read(sensors);
  if(sensors[0] > blackThreshold && sensors[1] > blackThreshold && sensors[2] > blackThreshold){
    turn90Clockwise();
  }
  if(sensors[0] > blackThreshold){
    turn45Clockwise();
  }
  if(sensors[2] > blackThreshold){
    turn45AntiClockwise();
  }
  // How ever if any of the line sensors detect a black line they stop and turn accordingly so the robot does not stray 

  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();

    if (incomingByte == 'w'){
      // Forward W
      Serial.write('w');
      motors.setSpeeds(200, 200);
    }
    if (incomingByte == 'a') {
      // Left A
      Serial.write('a');
      motors.setSpeeds(-200, 200);
    }
    if (incomingByte == 's'){
      // Back S
      Serial.write('s');
      motors.setSpeeds(-200, -200);
    }
    if (incomingByte == 'd'){
      // Right D
      Serial.write('d');
      motors.setSpeeds(200, -200);
    }
    if(incomingByte == 'e'){
      // This is when the user wishes to denote a person has been found, E is pushed and the semo auto mode makes the robot push the object out the maze until the 
      // line sensors detect it has gone over a black line
      Serial.write('e');
      pushFromMaze();
    }
    if(incomingByte ==0){
      motors.setSpeeds(0, 0);
    }

  }
}
// Full auto mode
void mode3(){
  // Some motivational music that plays while the robot is searching
  if (buzzer.isPlaying())
    {  
    }
    else
    {
      buzzer.playFromProgramSpace(march);
    }
  sw_secs.start();// Timer starting which is used later
  lineSensors.read(sensors); // Line sensors read as often as it can be to get up to date information
  // Simple code for following the left wall, should the middle sensor not detect a line that means the zumo has to go anticlockwise 
  if(sensors[1] < blackThreshold-300)
  {
    motors.setSpeeds(0,200);
  }
  // Similar thing with the left sensor, the robot sort of shuffles left and right to keep on the left line
  if(sensors[0] > blackThreshold)
  {
    motors.setSpeeds(200,0);
  }
  if(sensors[0] > blackThreshold && sensors[1] > blackThreshold-300)
  {
    // If a wall is detected robot must turn, this could be changed depending on the battery level and surface which the robot is moving on 
    motors.setSpeeds(100,-300);
    delay(350);
  }
  proxSensorsLoop();
  // Prox sensor loop run to detect if anything is seen
}
void loop() 
{
  // Main loop where values are checked as often as they can be to ensure accurate data
  proxSensors.read();
  lineSensors.read(sensors);
  // Here modes are selected by pressing the buttons which saves a mode value which is then continuously run
  if(buttonA.isPressed()){
      selectedMode = 1;
      Serial.println("Mode 1 selected");
    }
    if(buttonB.isPressed()){
      selectedMode = 2;
      Serial.println("Mode 2 selected");
    }
    if(buttonC.isPressed()){
      selectedMode = 3;
      Serial.println("Mode 3 selected");
    }
  if(selectedMode == 1){
    mode1();
  }
  if(selectedMode == 2){
    mode2();
  }
  if(selectedMode == 3){
    mode3();
  }
}
void proxSensorsLoop(){
  // Prox sensor loop run and sensors are checked to get up to date information
  proxSensors.read();
  int leftValue = proxSensors.countsFrontWithLeftLeds();
  int rightValue = proxSensors.countsFrontWithRightLeds();
  lineSensors.read(sensors);
  if(sw_secs.elapsed() > 2){  // This is important as it means the robot must wait 2 seconds before it can detect another person to stop it detecting the same one
    if (leftValue == proxThreshhold && rightValue == proxThreshhold){
      // This occurs when the robot is directly facing the object and will then push said object out of the maze using the function mentioned earlier
      buzzer.stopPlaying();
      sw_secs.reset();
      sw_secs.start();
      motors.setSpeeds(0,0);
      pushFromMaze();
      display.clear();
    }
    // These 2 if statements must be checked as it allows the zumo to hone in and directly face the object
    if (leftValue == proxThreshhold && rightValue != proxThreshhold){
      motors.setSpeeds(0, 0);
      motors.setSpeeds(-150, 150);
      proxSensorsLoop();
    }
    if (rightValue == proxThreshhold && leftValue != proxThreshhold){
      motors.setSpeeds(0,0);
      motors.setSpeeds(150, -150);
      proxSensorsLoop();
    }
  }
}
