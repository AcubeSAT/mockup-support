// defines pins numbers
const int stepPin = 3; 
const int dirPin = 2; 
const int enable = 8;
const int pot = 4; 
int SPEED_OF_MOTOR;
int dir_speed;

void setup() {
  // Sets the 3 pins as Outputs
  pinMode(stepPin,OUTPUT); 
  pinMode(dirPin,OUTPUT);
  pinMode(enable, OUTPUT); // ON/OFF for the motor
  pinMode(pot, INPUT);
 // Serial.begin(9600);
}
void loop() {
//  digitalWrite(dirPin,HIGH); // Enables the motor to move in a particular direction
//  // Makes 200 pulses for making one full cycle rotation
//  for(int x = 0; x < 200; x++) {
//    digitalWrite(stepPin,HIGH); 
//    delayMicroseconds(500); 
//    digitalWrite(stepPin,LOW); 
//    delayMicroseconds(500); 
//  }
//  delay(1000); // One second delay
//  
//  digitalWrite(dirPin,LOW); //Changes the rotations direction
//  // Makes 400 pulses for making two full cycle rotation

dir_speed = analogRead(pot);
SPEED_OF_MOTOR = map(dir_speed, 0, 1023, 600, 60);
 
if(SPEED_OF_MOTOR > 450){
  digitalWrite(enable, HIGH);
}
else{
  digitalWrite(enable, LOW);


  for(int x = 0; x < 100; x++) {
    digitalWrite(stepPin,HIGH);
    delayMicroseconds(SPEED_OF_MOTOR);
    digitalWrite(stepPin,LOW);
    delayMicroseconds(SPEED_OF_MOTOR);
  }
}
  

  //Serial.println(SPEED_OF_MOTOR);

  
  
}
