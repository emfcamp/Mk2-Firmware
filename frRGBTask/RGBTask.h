// (RGB, LED, TYPE, TIME, Period, PRIOITY)
enum RGBLeds {
    LED1,
    LED2,
    BOTH
};

enum RGBType {
    STATIC,
    FADE,
    FLASH,
    FLASH_ALT,
    TORCH,
    OFF
};

struct RGBRequest_t {
    uint8_t rgb[3];
    RGBLeds led;
    RGBType type;
    uint16_t time;
    uint16_t period;
    uint8_t prioity;
    TimerHandle_t timer;
    uint8_t stateRGB1[3];
    uint8_t stateRGB2[3];
};
