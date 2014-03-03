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
    uint8_t rgb[3];        // 3 bytes
    RGBLeds led;           // 4 bytes
    RGBType type;          // 4 bytes
    uint32_t time;         // 4 bytes
    uint32_t period;       // 4 bytes 
    uint8_t prioity;       // 1 byte
    TimerHandle_t timer;   // 4 byte
};
