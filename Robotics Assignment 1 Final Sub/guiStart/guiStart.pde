import g4p_controls.*;
import processing.serial.*;


Serial port;
int personsSaved = 0;


int bgcol = 15;

public void setup() {
  size(500, 360);
  createGUI();
  printArray(Serial.list());
  String name = Serial.list()[4];
  port = new Serial(this, name, 9600);
}

public void draw() {
  background(200, 200, 200);
  
  
  while (port.available() > 0) {
    String inByte = port.readString();
    println(inByte);
    if (inByte.equals("w")){
    String txt = textarea1.getText();
    textarea1.setText("\n Zumo going forward"+ txt);
    println(inByte);
    }
    if (inByte.equals("a")){
    String txt = textarea1.getText();
    textarea1.setText("\n Zumo going left"+ txt);
    println(inByte);
    }
    if (inByte.equals("d")){
    String txt = textarea1.getText();
    textarea1.setText("\n Zumo going right"+ txt);
    println(inByte);
    }
    if (inByte.equals("s")){
    String txt = textarea1.getText();
    textarea1.setText("\n Zumo going backward"+ txt);
    println(inByte);
    }
    if (inByte.equals("e")){
    String txt = textarea1.getText();
    textarea1.setText("\n Person Found"+ txt);
    println(inByte);
    }
    if(inByte.equals("personSaved")){
      println("o entered");
      personsSaved++;
      textfield2.setText("Persons found: " + personsSaved);
    }
  }
}

void keyPressed() {
  //println(key);
  if(key == 'w'){
    port.write('w');
    // Goes forward
    textfield1.setText("Last Command Sent - Forward");
  }
  if(key == 'd'){
    port.write('d');
    // Goes right
    textfield1.setText("Last Command Sent - Right");
  }
  if(key == 'a'){
    port.write('a');
    // Goes left
    textfield1.setText("Last Command Sent - Left");
  }
  if(key == 's'){
    port.write('s');
    //Goes backward
    textfield1.setText("Last Command Sent - Backward");
  }
  if(key == 'e'){
    port.write('e');
    textfield1.setText("Last Command Sent - Person Located");
}
}

void keyReleased() {
  port.write(0);
}
