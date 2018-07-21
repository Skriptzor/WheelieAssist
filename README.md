# Wheelie Assist

Improve your wheelie skills and share your highest wheelies with friends.

## Arduino Libraries

### EEPROMAnything

To give each device a unique ID we will have to save a value to the EEPROM on the Arduino. This will ensure each device has a unique bluetooth name to prevent confusion when setting it up near other devices.

We are using the EEPROMAnything library that can be found here: http://playground.arduino.cc/Code/EEPROMWriteAnything

To add your own library, create a new directory in the libraries directory with the name of your library. The libraries directory on Mac OS is ~/Documents/Arduino/libraries/. On Windows, it would be My Documents\Arduino\libraries\. The folder should contain a C or C++ file with your code and a header file with your function and variable declarations. It will then appear in the Sketch | Import Library menu in the Arduino IDE.