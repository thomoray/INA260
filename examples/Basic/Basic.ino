/*
   This sketch shows the absolute basic usage of the INA260 library.
   Most likely you will not want to use it exactly like this.
*/
#include <INA260.h>

// define the INA260 instance.
static INA260 ina260 = INA260();

void setup() {
    // For this demo, the readings will be printed over serial, so the Serial
    // library is initialized here.
    Serial.begin(115200);
    
    // Call the begin() function to initialize the hardware instance. This will also
    // initialize the Wire/I2C library.
    if (!ina260.begin()) {
        Serial.println("Unable to initialize I2C.");
        while (1);
    }
    Serial.println("Hardware initialized.");
}

void loop() {
    // Set the address for the device you would like to get readings from. In
    // this case we are using the enum "Address" to set the address.
    // Alternatively you can set by providing a int8_t value e.g. 64 or hex 
    // value e.g. 0x40.
    // the enum "Address" provides the pin configuration for each address.
    ina260.setAddress(ADDREESS_0x40);

    // Get the current reading from the device, checking for errors. This will
    // use I2C to talk to the device, no values are cached in the library.
    Serial.print("Current: ");
    Serial.print(ina260.readCurrent());
    Serial.println("mA");

    // Get the bus voltage reading from the device. This will
    // use I2C to talk to the device, no values are cached in the library.
    Serial.print("Voltage: ");
    Serial.print(ina260.readBusVoltage());
    Serial.println("mV");

    // Get the power reading from the device. This will
    // use I2C to talk to the device, no values are cached in the library.
    Serial.print("Power: ");
    Serial.print(ina260.readPower());
    Serial.println("mW");

    // for the demo, wait a second between taking readings
    delay(1000);
}
