// Include the AccelStepper library:
#include <AccelStepper.h>

// Define the AccelStepper interface type:
#define MotorInterfaceType 4

#define PIR1 2
#define PIR2 3
#define SW_LOCK 4

int mov = 0;
int movAnt;
// Create a new instance of the AccelStepper class: step ,dir
AccelStepper stepper = AccelStepper(1, 10, 11);

//AccelStepper stepper = AccelStepper(MotorInterfaceType, 8, 9, 10, 11);
bool DoorOpened = false;

unsigned long openMilli = 0;
int closeDelay = 4000;

unsigned long lastSample = 0;
int intervalSample = 1000;

byte DoorState = 0;
bool Locked = false;


void setup() {
  // Set the maximum steps per second:
  stepper.setMaxSpeed(1000);
  // Set speed
  stepper.setSpeed(200);
  // Set the maximum acceleration in steps per second^2:
  stepper.setAcceleration(800);

  stepper.setCurrentPosition(0);

  pinMode(PIR1, INPUT);
  pinMode(PIR2, INPUT);
  pinMode(SW_LOCK, INPUT_PULLUP);
}

void loop() {

  int mov = 0;

  if (millis() - lastSample >= intervalSample ) {
    lastSample = millis();
    mov = digitalRead(PIR1);
    if (!mov) mov = digitalRead(PIR2);
    Locked = !digitalRead(SW_LOCK);
  }

  if (!Locked) {
    if (mov == HIGH) {
      OpenDoor();
    } else
    {
      CloseDoor();
    }

  }


}


void OpenDoor() {
  if (DoorState == 0) {
    openMilli = millis();
  }
  else {
    DoorOpened = false;
    stepper.stop();
  }
  if (!DoorOpened) {
    DoorOpened = true;
    DoorState = 0;
    // Set the maximum steps per second:
    stepper.setMaxSpeed(1000);
    // Set speed
    stepper.setSpeed(200);
    // Set the maximum acceleration in steps per second^2:
    stepper.setAcceleration(800);

    // Set target position:
    stepper.moveTo(1000);
    // Run to position with set speed and acceleration:
    stepper.runToPosition();
  }
}

void CloseDoor() {
  if (DoorOpened) {
    if (millis() - openMilli >= closeDelay)
    {
      switch (DoorState) {
        case 0:   // Move back to original position:
          // Set the maximum steps per second:
          stepper.setMaxSpeed(500);
          // Set speed
          stepper.setSpeed(100);
          // Set the maximum acceleration in steps per second^2:
          stepper.setAcceleration(400);
          stepper.moveTo(0);
          DoorState++;
          break;
        case 1:  stepper.run();
          if (stepper.currentPosition() == stepper.targetPosition()) {
            DoorOpened = false;
          }
          break;
      }

    }
  }
}