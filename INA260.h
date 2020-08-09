#ifndef H
#define H

#include <stdint.h>

#define INA260_I2CADDR_DEFAULT          0x40 // Default I2C address
#define INA260_CONFIG_REGISTER          0x00 // Configuration Register
#define INA260_CURRENT_REGISTER         0x01 // Current measurement register (signed) in mA
#define INA260_VOLTAGE_REGISTER         0x02 // Bus voltage measurement register in mV
#define INA260_POWER_REGISTER           0x03 // Power calculation register in mW
#define INA260_MASK_ENABLE_REGISTER     0x06 // Interrupt/Alert setting and checking register
#define INA260_ALERT_LIMIT_REGISTER     0x07 // Alert limit value register
#define INA260_MANUFACTURER_ID_REGISTER 0xFE // Manufacturer ID register
#define INA260_DIE_ID_REGISTER          0xFF // Die ID and revision register

typedef enum _address {
    ADDREESS_0x40 = 0x40, // A1 = GND, A0 = GND
    ADDREESS_0x41 = 0x41, // A1 = GND, A0 = VS
    ADDREESS_0x42 = 0x42, // A1 = GND, A0 = SDA
    ADDREESS_0x43 = 0x43, // A1 = GND, A0 = SCL
    ADDREESS_0x44 = 0x44, // A1 = VS, A0 = GND
    ADDREESS_0x45 = 0x45, // A1 = VS, A0 = VS
    ADDREESS_0x46 = 0x46, // A1 = VS, A0 = SDA
    ADDREESS_0x47 = 0x47, // A1 = VS, A0 = SCL
    ADDREESS_0x48 = 0x48, // A1 = SDA, A0 = GND
    ADDREESS_0x49 = 0x49, // A1 = SDA, A0 = VS
    ADDREESS_0x4A = 0x4A, // A1 = SDA, A0 = SDA
    ADDREESS_0x4B = 0x4B, // A1 = SDA, A0 = SCL
    ADDREESS_0x4C = 0x4C, // A1 = SCL, A0 = GND
    ADDREESS_0x4D = 0x4D, // A1 = SCL, A0 = VS
    ADDREESS_0x4E = 0x4E, // A1 = SCL, A0 = SDA
    ADDREESS_0x4F = 0x4F, // A1 = SCL, A0 = SCL
} Address;

typedef enum _mode {
    MODE_TRIG_POWER_DOWN = 0b000, // Power-Down (or Shutdown) Triggered
    MODE_TRIG_ISH        = 0b001, // Shunt Current, Triggered
    MODE_TRIG_VBUS       = 0b010, // Bus Voltage, Triggered
    MODE_TRIG_ISH_VBUS   = 0b011, // Shunt Current and Bus Voltage, Triggered
    MODE_CONT_POWER_DOWN = 0b100, // Power-Down (or Shutdown) Continuous
    MODE_CONT_ISH        = 0b101, // Shunt Current, Continuous
    MODE_CONT_VBUS       = 0b110, // Bus Voltage, Continuous
    MODE_CONT_ISH_VBUS   = 0b111, // Shunt Current and Bus Voltage, Continuous
} Mode;

typedef enum _averaging {
    AVG_1       = 0b000, // Window size - 1 sample (Default)
    AVG_4       = 0b001, // Window size - 4 samples
    AVG_16      = 0b010, // Window size - 16 samples
    AVG_64      = 0b011, // Window size - 64 samples
    AVG_128     = 0b100, // Window size - 128 samples
    AVG_256     = 0b101, // Window size - 256 samples
    AVG_512     = 0b110, // Window size - 512 samples
    AVG_1024    = 0b111, // Window size - 1024 samples
} AveragingCount;

typedef enum _conversion {
    TIME_140_us     = 0b000, // Measurement time: 140us
    TIME_204_us     = 0b001, // Measurement time: 204us
    TIME_332_us     = 0b010, // Measurement time: 332us
    TIME_558_us     = 0b011, // Measurement time: 558us
    TIME_1_1_ms     = 0b100, // Measurement time: 1.1ms (Default)
    TIME_2_116_ms   = 0b101, // Measurement time: 2.116ms
    TIME_4_156_ms   = 0b110, // Measurement time: 4.156ms
    TIME_8_244_ms   = 0b111, // Measurement time: 8.224ms
} ConversionTime;

union ConfigurationRegister {
    struct __attribute__((packed)) {
        uint16_t mode : 3;
        uint16_t ishct : 3;
        uint16_t vbusct : 3;
        uint16_t avg : 3;
        uint16_t : 3;
        uint16_t rst : 1;
    };
    uint16_t rawValue;
};

union MaskEnableRegister {
    struct __attribute__((packed)) {
        uint16_t len : 1;
        uint16_t apol : 1;
        uint16_t ovf : 1;
        uint16_t cvrf : 1;
        uint16_t aff : 1;
        uint16_t : 5;
        uint16_t cnvr : 1;
        uint16_t pol : 1;
        uint16_t bul : 1;
        uint16_t bol : 1;
        uint16_t ucl : 1;
        uint16_t ocl : 1;
    };
    uint16_t rawValue;
};

union DieIdRegister {
    struct __attribute__((packed)) {
        uint16_t rid : 4;
        uint16_t did : 12;
    };
    uint16_t rawValue;
};

class INA260 {
    private:
        uint8_t address;

    public:
        byte devices[16];
        int deviceCount;

        INA260(void);

        bool begin(void);
        bool reset(void);

        void findDevices(void);

        void setAddress(uint8_t addr);
        uint8_t getAddress(void);

        uint16_t readRegister(uint8_t reg);
        bool writeRegister(uint8_t reg, uint16_t value);

        ConfigurationRegister readConfigurationRegister(void);
        bool writeConfigurationRegister(ConfigurationRegister value);

        float readCurrent(void);
        float readBusVoltage(void);
        float readPower(void);

        MaskEnableRegister readMaskEnableRegister(void);
        bool writeMaskEnableRegister(MaskEnableRegister reg);

        double readAlertLimitRegister(void);
        bool writeAlertLimitRegister(uint16_t value);

        bool enableOverCurrentLimitAlert(uint16_t milliAmps);
        bool enableUnderCurrentLimitAlert(uint16_t milliAmps);
        bool enableBusOvertLimitAlert(uint16_t milliVolts);
        bool enableBusUnderLimitAlert(uint16_t milliVolts);
        bool enableOverPowerLimitAlert(uint16_t milliWatts);

        bool setCurrentLimit(uint16_t milliAmps);
        bool setBusVoltageLimit(uint16_t milliVolts);
        bool setPowerLimit(uint16_t milliWatts);

        bool isMathOverFlow(void);
        bool isAlert(void);
        void clearAlert(void);

        bool isAlertPolaritySet(void);
        bool setAlertPolarity(bool polarity);

        bool isAlertLatchSet(void);
        bool setAlertLatch(bool latch);

        Mode getMode(void);
        void setMode(Mode mode);

        bool isConversionRready(void);
        bool setConversionReadyAlert(bool state);

        ConversionTime getCurrentConversionTime(void);
        bool setCurrentConversionTime(ConversionTime time);
        
        ConversionTime getVoltageConversionTime(void);
        bool setVoltageConversionTime(ConversionTime time);

        AveragingCount getAveragingCount(void);
        bool setAveragingCount(AveragingCount count);

        String readManufactuerId(void);
        DieIdRegister readDieId(void);
};

#endif // INA260.H

