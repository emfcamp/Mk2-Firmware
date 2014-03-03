// (RGB, LED, TYPE, TIME, Period, PRIOITY)
enum RGBLeds {
    LED1,
    LED2,
    BOTH
};

enum RGBType {
    STATIC,
    FLASH,
    FLASH_ALT,
    TORCH,
    OFF
};

struct RGBRequest_t {
    uint8_t rgb[3];
    RGBLeds led;
    RGBType type;
    uint32_t time;
    uint32_t period;
    uint8_t prioity;
    TimerHandle_t timer;
};
