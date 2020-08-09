#ifndef Arduino
    #include "Arduino.h"
#endif
#ifndef Wire
    #include <Wire.h>
#endif

#include "INA260.h"

/*!
 *    @brief  Instantiates a new INA260 class
 */
INA260::INA260(void) {}

/*!
 *    @brief  Sets up the HW
 * 
 *    @return True if initialization was successful, otherwise false.
 */
bool INA260::begin() {
    static bool wireInitialized = false;
    if (! wireInitialized) {
        Wire.begin();
        wireInitialized = true;
    }
    reset();
    return wireInitialized;
}

/*!
 *  @brief Resets the hardware. All registers are set to default values,
 *  the same as a power-on reset.
 *
 *  @return True if reset was successfull, otherwise false.
*/
bool INA260::reset(void) {
    ConfigurationRegister reg{};
    reg.rst = 1;
    return writeConfigurationRegister(reg);
}

/*!
 *  @brief Sets a device address.
*/
void INA260::setAddress(uint8_t addr) {
    INA260::address = addr;
}

/*!
 *  @brief Gets the device address.
 *
 *  @return The currently set device address.
*/
uint8_t INA260::getAddress(void) {
    return INA260::address;
}

/*!
 *  @brief Reads the specified INA260 register.
 * 
 *  @param reg The register to read.
 * 
 *  @return The value of the register.
*/
uint16_t INA260::readRegister(uint8_t reg) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(address, 2u);
    if (Wire.available() == 2) {
        const uint16_t msb = Wire.read();
        const uint16_t lsb = Wire.read();
        return (msb << 8) | lsb;
    }
    return 0;
}

/*!
 *  @brief Reads the specified INA260 register.
 * 
 *  @param reg The register to read.
 *  @param value the value to write to the register.
 * 
 *  @return True if write was successfull, otherwise false.
*/
bool INA260::writeRegister(uint8_t reg, uint16_t value) {
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write((value >> 8) & 0xFF);
    Wire.write(value & 0xFF);
    return (Wire.endTransmission() == 0);
}

/*!
 *  @brief Reads the current configuration from the 
 *  ConfigurationRegister.
 * 
 *  @return The rawValue from the register.
 * 
 *  @note Reading from the ConfigurationRegister will not
 *  impact current conversions in progress.
*/
ConfigurationRegister INA260::readConfigurationRegister(void) {
    ConfigurationRegister reg = {};
    reg.rawValue = readRegister(INA260_CONFIG_REGISTER);
    return reg;
}

/*!
 *  @brief Writes a ConfigurationRegister to the register. 
 *  
 *  @return True if write was successfull, otherwise false.
 * 
 *  @note Writing to the ConfigurationRegister halts any 
 *  conversion in progress until the write sequence is completed
*/
bool INA260::writeConfigurationRegister(ConfigurationRegister value) {
    return writeRegister(INA260_CONFIG_REGISTER, value.rawValue);
}

/*!
 *  @brief Reads and scales the current value of the Current register.
 *  
 *  @return The current current measurement in mA.
*/
float INA260::readCurrent(void) {
    return readRegister(INA260_CURRENT_REGISTER) * 1.25;
}

/*!
 *  @brief Reads and scales the current value of the Bus Voltage register.
 *  
 *  @return The current bus voltage measurement in mV.
*/
float INA260::readBusVoltage(void) {
    return readRegister(INA260_VOLTAGE_REGISTER) * 1.25;
}

/*!
 *  @brief Reads and scales the current value of the Power register.
 *  
 *  @return The current Power calculation in mW.
*/
float INA260::readPower(void) {
    return readRegister(INA260_POWER_REGISTER) * 10;
}

/*!
 *  @brief Reads the current configuration from the 
 *  MaskEnableRegister.
 * 
 *  @return The rawValue from the register.
*/
MaskEnableRegister INA260::readMaskEnableRegister(void) {
    MaskEnableRegister reg{};
    reg.rawValue = readRegister(INA260_MASK_ENABLE_REGISTER);
    return reg;
}

/*!
 *  @brief Writes a MaskEnableRegister to the register. 
 * 
 *  @return True if write was successfull, otherwise false.
*/
bool INA260::writeMaskEnableRegister(MaskEnableRegister reg) {
    return writeRegister(INA260_MASK_ENABLE_REGISTER, reg.rawValue);
}

/*!
 *  @brief Reads the current value of the alert limit register. 
 * 
 *  @return a value based on which limit register is set.
*/
double INA260::readAlertLimitRegister(void) {
    MaskEnableRegister reg = readMaskEnableRegister();
    double value = 0.0;
    if (reg.pol == 1) {
        value = readRegister(INA260_ALERT_LIMIT_REGISTER) * 10;
    } else {
        value = readRegister(INA260_ALERT_LIMIT_REGISTER) * 1.25;        
    }
    return value;
}

/*!
 *  @brief Writes the Alert Limit Register value.
 * 
 *  @param value the raw value to write.
 *  @return True if write was successfull, otherwise false.
*/
bool INA260::writeAlertLimitRegister(uint16_t value) {
    return writeRegister(INA260_ALERT_LIMIT_REGISTER, value);
}

/*!
 *  @brief Configures the device to pull the ALERT pin low when the 
 *  shunt current exceeds the value given. Clears all other limit alerts.
 * 
 *  @param milliAmps value to set in milliAmps.
 *  @return True if write was successfull, otherwise false.
*/
bool INA260::enableOverCurrentLimitAlert(uint16_t milliAmps) {
    MaskEnableRegister reg = readMaskEnableRegister();
    reg.ocl = 1;
    reg.ucl = 0;
    reg.bol = 0;
    reg.bul = 0;
    reg.pol = 0;
    setCurrentLimit(milliAmps);
    return writeMaskEnableRegister(reg);
}

/*!
 *  @brief Configures the device to pull the ALERT pin low when the 
 *  shunt current drops below the value given. Clears all other limit alerts.
 * 
 *  @param milliAmps value to set in milliAmps.
 *  @return True if write was successfull, otherwise false.
*/
bool INA260::enableUnderCurrentLimitAlert(uint16_t milliAmps) {
    MaskEnableRegister reg = readMaskEnableRegister();
    reg.ocl = 0;
    reg.ucl = 1;
    reg.bol = 0;
    reg.bul = 0;
    reg.pol = 0;
    setCurrentLimit(milliAmps);
    return writeMaskEnableRegister(reg);
}

/*!
 *  @brief Configures the device to pull the ALERT pin low when the 
 *  bus voltage exceeds the value given. Clears all other limit alerts.
 * 
 *  @param milliVolts value to set in milliVolts.
 *  @return True if write was successfull, otherwise false.
*/
bool INA260::enableBusOvertLimitAlert(uint16_t milliVolts) {
    MaskEnableRegister reg = readMaskEnableRegister();
    reg.ocl = 0;
    reg.ucl = 0;
    reg.bol = 1;
    reg.bul = 0;
    reg.pol = 0;
    setBusVoltageLimit(milliVolts);
    return writeMaskEnableRegister(reg);
}

/*!
 *  @brief Configures the device to pull the ALERT pin low when the 
 *  bus voltage drops below the value given. Clears all other limit alerts.
 * 
 *  @param milliVolts value to set in milliVolts.
 *  @return True if write was successfull, otherwise false.
*/
bool INA260::enableBusUnderLimitAlert(uint16_t milliVolts) {
    MaskEnableRegister reg = readMaskEnableRegister();
    reg.ocl = 0;
    reg.ucl = 0;
    reg.bol = 0;
    reg.bul = 1;
    reg.pol = 0;
    setBusVoltageLimit(milliVolts);
    return writeMaskEnableRegister(reg);
}

/*!
 *  @brief Configures the device to pull the ALERT pin low when the 
 *  power excceeds the value given. Clears all other limit alerts.
 * 
 *  @param milliWatts value to set in milliWatts.
 *  @return True if write was successfull, otherwise false.
*/
bool INA260::enableOverPowerLimitAlert(uint16_t milliWatts) {
    MaskEnableRegister reg = readMaskEnableRegister();
    reg.ocl = 0;
    reg.ucl = 0;
    reg.bol = 0;
    reg.bul = 0;
    reg.pol = 1;
    setPowerLimit(milliWatts);
    return writeMaskEnableRegister(reg);
}

/*!
 *  @brief Sets the value of the Alert Limit Register to value given.
 * 
 *  @param milliAmps value to set in milliAmps.
 *  @return True if write was successfull, otherwise false.
*/
bool INA260::setCurrentLimit(uint16_t milliAmps) {
    uint16_t value = milliAmps / 1.25;
    return writeAlertLimitRegister(value);
}

/*!
 *  @brief Sets the value of the Alert Limit Register to value given.
 * 
 *  @param milliVolts value to set in milliVolts.
 *  @return True if write was successfull, otherwise false.
*/
bool INA260::setBusVoltageLimit(uint16_t milliVolts) {
    uint16_t value = milliVolts / 1.25;
    return writeAlertLimitRegister(value);
}

/*!
 *  @brief Sets the value of the Alert Limit Register to value given.
 * 
 *  @param milliWatts value to set in milliWatts.
 *  @return True if write was successfull, otherwise false.
*/
bool INA260::setPowerLimit(uint16_t milliWatts) {
    uint16_t value = milliWatts / 10;
    return writeAlertLimitRegister(value);
}

/*!
 *  @brief This bit is set to '1' if an arithmetic operation resulted 
 *  in an overflow error. It indicates that power data may have exceeded 
 *  the maximum reportable value of 419.43 W.
 * 
 *  @return True if flag is set, otherwise false.
*/
bool INA260::isMathOverFlow(void) {
    MaskEnableRegister reg = readMaskEnableRegister();
    return reg.ovf;
}

/*!
 *  @brief Is the Alert Function Flag currently set. The Alert Function Flag
 *  allows the user to determine if the Alert Function was the source of the
 *  alert.
 * 
 *  @return True if flag is set, otherwise false.
*/
bool INA260::isAlert(void) {
    MaskEnableRegister reg = readMaskEnableRegister();
    return reg.aff;
}

/*!
 *  @brief Clears the Alert Function Flag.
*/
void INA260::clearAlert(void) {
    readMaskEnableRegister();
}

/*!
 *  @brief Is the Alert Polarity Flag is currently set to inverted.
 * 
 *  @return True if flag is set, otherwise false.
*/
bool INA260::isAlertPolaritySet(void) {
    MaskEnableRegister reg = readMaskEnableRegister();
    return reg.apol;
}

/*!
 *  @brief Sets the Alert Polarity Flag.
 * 
 *  @param polarity 1 = Inverted, 0 = Normal (default).
 * 
 *  @return True if write was successfull, otherwise false.
*/
bool INA260::setAlertPolarity(bool polarity) {
    MaskEnableRegister reg = readMaskEnableRegister();
    reg.apol = polarity;
    return writeMaskEnableRegister(reg);
}

/*!
 *  @brief Is the Alert Latch Enable currently set to Latch enabled.
 * 
 *  @return True if latch is set, otherwise false.
*/
bool INA260::isAlertLatchSet(void) {
    MaskEnableRegister reg = readMaskEnableRegister();
    return reg.len;
}

/*!
 *  @brief Sets Alert Latch Enable.
 * 
 *  @param latch 1 = Latch enabled, 0 = Transparent (default).
 * 
 *  @return True if write was successfull, otherwise false.
*/
bool INA260::setAlertLatch(bool latch) {
    MaskEnableRegister reg = readMaskEnableRegister();
    reg.len = latch;
    return writeMaskEnableRegister(reg);
}

/*!
 *  @brief Returns the current measurement mode
 * 
 *  @return Mode that is currently set.
*/
Mode INA260::getMode(void) {
    ConfigurationRegister reg = readConfigurationRegister();
    return (Mode)reg.mode;
}

/*!
 *  @brief Sets the operating mode from the list given in enum type "Mode".
 * 
 *  @param mode the new mode to set.
 */
void INA260::setMode(Mode mode) {
    ConfigurationRegister reg = readConfigurationRegister();
    reg.mode = mode;
    writeConfigurationRegister(reg);
};

/*!
 *  @brief The Conversion Ready Flag bit is provided to help coordinate 
 *  one-shot or triggered conversions. 
 * 
 *  @return True if conversion is ready, otherwise false.
*/
bool INA260::isConversionRready() {
    MaskEnableRegister reg = readMaskEnableRegister();
    return reg.cvrf;
}

/*!
 *  @brief Configures the device to pull the ALERT pin low when a 
 *  conversion is complete.
 * 
 *  @param state sets the alert state true or false (default).
 *  @return True if write was successfull, otherwise false.
*/
bool INA260::setConversionReadyAlert(bool state) {
    MaskEnableRegister reg = readMaskEnableRegister();
    reg.cnvr = state;
    return writeMaskEnableRegister(reg);
}

/*!
 *  @brief Reads the current current conversion time.
 * 
 *  @return The current current conversion time.
*/
ConversionTime INA260::getCurrentConversionTime(void) {
    ConfigurationRegister reg = readConfigurationRegister();
    return (ConversionTime)reg.ishct;
}

/*!
 *  @brief Sets the current conversion time.
 * 
 *  @param time Sets the current conversion time.
 *  @return True if write was successfull, otherwise false.
*/
bool INA260::setCurrentConversionTime(ConversionTime time) {
    ConfigurationRegister reg = readConfigurationRegister();
    reg.ishct = time;
    return writeConfigurationRegister(reg);
}

/*!
 *  @brief Reads the current bus voltage conversion time.
 * 
 *  @return The current bus voltage conversion time.
*/
ConversionTime INA260::getVoltageConversionTime(void) {
    ConfigurationRegister reg = readConfigurationRegister();
    return (ConversionTime)reg.vbusct;
}

/*!
 *  @brief Sets the bus voltage conversion time.
 * 
 *  @param time Sets the bus voltage conversion time.
 *  @return True if write was successfull, otherwise false.
*/
bool INA260::setVoltageConversionTime(ConversionTime time) {
    ConfigurationRegister reg = readConfigurationRegister();
    reg.vbusct = time;
    return writeConfigurationRegister(reg);
}

/*!
 *  @brief Reads the current number of averaging samples.
 * 
 *  @return The current number of averaging samples.
*/
AveragingCount INA260::getAveragingCount(void) {
    ConfigurationRegister reg = readConfigurationRegister();
    return (AveragingCount)reg.avg;
}

/*!
 *  @brief Sets the number of averaging samples.
 * 
 *  @param count The number of samples to be averaged.
 *  @return True if write was successfull, otherwise false.
*/
bool INA260::setAveragingCount(AveragingCount count) {
    ConfigurationRegister reg = readConfigurationRegister();
    reg.avg = count;
    return writeConfigurationRegister(reg);
}

/*!
 *  @brief Reads the Manufacturer ID Register
 * 
 *  @return The manufacturers name
*/
String INA260::readManufactuerId(void) {
    uint16_t value = readRegister(INA260_MANUFACTURER_ID_REGISTER);
    char mfgStr[3];
    mfgStr[0] = static_cast<char>((value >> 8) & 0xFF);
    mfgStr[1] = static_cast<char>(value & 0xFF);
    mfgStr[2] = '\0';
    return mfgStr;
}

/*!
 *  @brief Reads the Die ID Register
 * 
 *  @return DieIdRegister with a unique id number and revision id for the die
*/
DieIdRegister INA260::readDieId(void) {
    DieIdRegister reg = {};
    reg.rawValue = readRegister(INA260_DIE_ID_REGISTER);
    return reg;
}

/*!
 *  @brief Scan for connected devices. The address of any
 *  devices found are stored in devices[] and deviceCount
 *  is incremented.
*/
void INA260::findDevices() {
    for (byte address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        // Use the return value of the Write.endTransmission
        // to see if a device did acknowledge the address
        // then add it to devices[]
        if (Wire.endTransmission() == 0) {
            devices[deviceCount] = address;
            deviceCount++;
        }
    }    
}