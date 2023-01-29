const float DepthGoal = 255.106;//37 psi - 50ft under water!
const float OffSet = 0.483 ; // See Calibration Note!
float V, P; // Voltage & Pressure
bool GoalReached = false;
byte MotorPin = 10;
void setup()
{
//motor output
pinMode(MotorPin, OUTPUT);
//sart motor..
digitalWrite(MotorPin, HIGH);
}
void loop()
{
//Connect sensor’s output (SIGNAL) to Analog 0
V = analogRead(0) * 5.00 / 1024; 
P = (V - OffSet) * 400;

if (P>=DepthGoal) //done, turn off motor, floats to surface we hope.. :)
{digitalWrite(MotorPin, LOW);}
//check depth every second..
delay(1000);
}