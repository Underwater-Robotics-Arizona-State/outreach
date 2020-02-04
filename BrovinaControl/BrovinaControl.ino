/* Author       : Pratik Panda
 * Editor(s)    :
 * Last Edited  : 02/01/20
 * Description  : Maps Xbox 360 joystick values to motor values used to control 
 *                  Brovina
 */

#include <XBOXUSB.h>
#include <Servo.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

// -------------------- ALL NECESSARY VARIABLES -------------------- //
Servo UpDownServo, RightServo, LeftServo;                               // The 3 motors that are actually on the robot
USB Usb;                                                                // The USB connection
XBOXUSB Xbox(&Usb);                                                     // The XBox Controller connection via USB
int leftHatY, rightHatX, invrightHatX, rightHatY;                       // The values we send to the motors (STILL CONFUSED)
int UpDownServoPin = 4, RightServoPin = 3, LeftServoPin = 2;            // The pins that the motors are connected to on the Arduino Board
int defaultValues = 1500, minMotorValue = 1000, maxMotorValue = 2000;   // The default values of the motors
int minXBoxValue = -32768, maxXBoxValue = 32767;                        // The min/max values rom a joystick on XBox Controller
int joystickBuffer = 7500; //114;                                       // Do not rotate motors unless you are outside of the +/- 7500 range

// writeMicroseconds() -- use this to set the pulse duration in microseconds (range: 544 to 2400??)
// pwm of the sero is 50 Hz

void setup() {

    // Actually attach pins to the motors
    UpDownServo.attach(UpDownServoPin);
    RightServo.attach(RightServoPin);
    LeftServo.attach(LeftServoPin);

    // Write the default values to the motors (1500 is no motor rotation)
    UpDownServo.write(defaultValues);
    RightServo.write(defaultValues);
    LeftServo.write(defaultValues);

    // Not really sure what this code does ---------------------------------------------------------
    Serial.begin(115200);       //                                                                 |
    #if !defined(__MIPSEL__)    // Wait for serial port to connect - used on Leonardo, Teensy,     |
        while (!Serial);        // and other boards with built-in USB CDC serial connection        |
    #endif                      //                                                                 |
    // ---------------------------------------------------------------------------------------------

    // Checks if USB is connected and initiated.
    if (Usb.Init() == -1) {
        Serial.print(F("\r\nOSC did not start"));           // Putting the print in an F() makes sure the string stays in flash and does not eat up RAM
        while (1);                                          //Stops execution of code
    }
    else {
        Serial.print(F("\r\nXBOX USB Library Started"));
    }
}

void loop() {
    /* Task() polls connected usb devices for updates to their status.
     * NB : If there is no activity on a connected USB device, 
     * task() will block all other calls for 5 second intervals.
     */
    Usb.Task();
    
    if (Xbox.Xbox360Connected) {
          
        // Maps from the +/- 32,768 range (defaultrange of XBox Controllers) to the 1000-2000 range
        int leftJoystick_YDirection = map(Xbox.getAnalogHat(LeftHatY),minXBoxValue,maxXBoxValue,minMotorValue,maxMotorValue);       // Joystick values for rise and dive
        int rightJoystick_XDirection = map(Xbox.getAnalogHat(RightHatX),minXBoxValue,maxXBoxValue,minMotorValue,maxMotorValue);     // Joystick values for left and right
        int rightJoystick_YDirection = map(Xbox.getAnalogHat(RightHatY),minXBoxValue,maxXBoxValue,minMotorValue,maxMotorValue);     // Joystick values for forward and backward

        int motorBuffer = map(joystickBuffer, minXBoxValue, maxXBoxValue, minMotorValue, maxMotorValue) - defaultValues;

        int shouldBeGreaterThanThis = defaultValues + motorBuffer;                                                                  // Don't spin motor unless greater than this
        int shouldBeLessThanThis = defaultValues - motorBuffer;                                                                     // or less than this

        bool motorsRotated = false;                                                                                                 // Flag to see if motors have rotated this iteration or not

        if(leftJoystick_YDirection > shouldBeGreaterThanThis) {         // Robot should rise
            UpDownServo.writeMicroseconds(leftJoystick_YDirection);
            motorsRotated = true;
            Serial.print(F("Rise\n"));
        }
        else if(leftJoystick_YDirection < shouldBeLessThanThis) {       // Robot should dive
            UpDownServo.writeMicroseconds(leftJoystick_YDirection);
            motorsRotated = true;
            Serial.print(F("Dive\n"));
        }
        if(rightJoystick_XDirection > shouldBeGreaterThanThis) {        // Robot should turn right
            LeftServo.writeMicroseconds(rightJoystick_XDirection);
            RightServo.writeMicroseconds(rightJoystick_XDirection - (maxMotorValue - minMotorValue));
            motorsRotated = true;
            Serial.print(F("Right\n"));
        }
        else if(rightJoystick_XDirection < shouldBeLessThanThis) {      // Robot should turn left
            LeftServo.writeMicroseconds(rightJoystick_XDirection);
            RightServo.writeMicroseconds(rightJoystick_XDirection + (maxMotorValue - minMotorValue));
            motorsRotated = true;
            Serial.print(F("Left\n"));
        }
        if(rightJoystick_YDirection > shouldBeGreaterThanThis) {         // Robot should move forward
            LeftServo.writeMicroseconds(rightJoystick_YDirection);
            RightServo.writeMicroseconds(rightJoystick_YDirection);
            motorsRotated = true;
            Serial.print(F("Forward\n"));
        }
        else if(rightJoystick_YDirection < shouldBeLessThanThis) {       // Robot should move backward
            LeftServo.writeMicroseconds(rightJoystick_YDirection);
            RightServo.writeMicroseconds(rightJoystick_YDirection);
            motorsRotated = true;
            Serial.print(F("Backward\n"));
        }

        if(motorsRotated == false) {                                    // Default case for when no joystick values outside of buffer range
            RightServo.write(defaultValues);
            UpDownServo.write(defaultValues);
            LeftServo.write(defaultValues);
            Serial.print(F("No Movement\n"));                                    // Might be too much output to keep this in
        }
    }
    delay(1);           // Wait 10 milliseconds before looping through again 
}
