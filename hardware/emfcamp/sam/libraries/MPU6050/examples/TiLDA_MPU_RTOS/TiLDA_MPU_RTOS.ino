#include <Debounce.h>
#include <FreeRTOS_ARM.h>
#include <Wire.h>
#include <MPU6050.h>


void tildaButtonInterruptPriority() {
    // reset pin interrupt handler IRQn priority levels to allow use of FreeRTOS API calls
    NVIC_DisableIRQ(PIOA_IRQn);
    NVIC_ClearPendingIRQ(PIOA_IRQn);
    NVIC_SetPriority(PIOA_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(PIOA_IRQn);
    
    NVIC_DisableIRQ(PIOB_IRQn);
    NVIC_ClearPendingIRQ(PIOB_IRQn);
    NVIC_SetPriority(PIOB_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(PIOB_IRQn);
    
    NVIC_DisableIRQ(PIOC_IRQn);
    NVIC_ClearPendingIRQ(PIOC_IRQn);
    NVIC_SetPriority(PIOC_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(PIOC_IRQn);
    
    NVIC_DisableIRQ(PIOD_IRQn);
    NVIC_ClearPendingIRQ(PIOD_IRQn);
    NVIC_SetPriority(PIOD_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(PIOD_IRQn);
}


void setup() {
    pinMode(SRF_SLEEP, OUTPUT);
    digitalWrite(SRF_SLEEP, LOW);
    
    pinMode(PIN_LED_RXL, OUTPUT);
    digitalWrite(PIN_LED_RXL, HIGH);
    
    tildaButtonSetup();
    tildaButtonAttachInterrupts();
    tildaButtonInterruptPriority();
    // setup  Serial
    SerialUSB.begin(115200);
    while(!SerialUSB){};
    SerialUSB.println("TiLDA Mk2 IMU task tester");
    

    
    // holder for task handle
    BaseType_t t1,t2;
    
    t1 = xTaskCreate(vIMUTask,
                     NULL,
                     configMINIMAL_STACK_SIZE*5,
                     NULL,
                     2,
                     NULL);
                    
    t2 = xTaskCreate(vBlinkTask,
                     NULL,
                     configMINIMAL_STACK_SIZE,
                     NULL,
                     1,
                     NULL);
                     
    if (t1 != pdPASS || t2 != pdPASS) {
        // tasked didn't get created
        SerialUSB.println("Failed to create task");
        while(1);
    }
    // start scheduler
    SerialUSB.println("Start Scheduler");
    vTaskStartScheduler();
    SerialUSB.println("Insufficient RAM");
    while(1);

}

void loop() {
  // put your main code here, to run repeatedly:

}

EventGroupHandle_t eventGroup;
TickType_t sampleRate = 500;
uint8_t dmp_state;

// eventGroup Bits
#define IMU_SAMPLE_RATE_BIT (1 << 0)
#define IMU_INT_BIT (1 << 1)
#define IMU_DMP_BIT (1 << 2)

#define IMU_DMP_ON  1
#define IMU_DMP_OFF 0

/* Starting sampling rate. */
#define IMU_DEFAULT_MPU_HZ    (1)

// TODO: work out orientation for badge
static signed char gyro_orientation[9] = { 1, 0, 0,
                                           0, 1, 0,
                                           0, 0, 1};


// Callbacks
static void IMU_interrupt(void) {
  if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
    static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    
    // set the Charge state bit to wake the PMIC Task
    xEventGroupSetBitsFromISR(eventGroup,
                              IMU_INT_BIT,
                              &xHigherPriorityTaskWoken);
    
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
  }
}

static void IMU_tap_cb(unsigned char direction, unsigned char count)
{
    SerialUSB.println("IMUTask: tap callback(" + String(direction) + ", " +String(count) + ")");
}

static void IMU_android_orient_cb(unsigned char orientation)
{
    SerialUSB.println("IMUTask: orient callback(" + String(orientation) + ")");
}


void vIMUTask(void *pvParameters)
{
    SerialUSB.println("IMUTask: Starting");
    // eventGroup for notifsctions    
    eventGroup = xEventGroupCreate();
    if (eventGroup == NULL) {
        SerialUSB.println("IMUTask: Failed to create eventGroup");
    }
    
    EventBits_t uxBits;
    
    IMUInit();

    // everthing setup start the dmp generating interrupts
    SerialUSB.println("IMUTask: pre enable");
    //enable_dmp();
    SerialUSB.println("IMUTask: post enable");
    
    while(true) {

        uxBits = xEventGroupWaitBits(eventGroup,
                                     IMU_SAMPLE_RATE_BIT | IMU_INT_BIT | IMU_DMP_BIT,
                                     pdFALSE,
                                     pdFALSE,
                                     (sampleRate/portTICK_PERIOD_MS) );
        
        if( ( uxBits & IMU_SAMPLE_RATE_BIT ) != 0 ) {
            // new sample rate notting todo but clear the bit and re enter the wait
            xEventGroupClearBits(eventGroup,
                                 IMU_SAMPLE_RATE_BIT);
            
        } else if( ( uxBits & IMU_INT_BIT ) != 0 ) {
            // interupt has fired
            if (dmp_state == IMU_DMP_ON) {
                // interrupt as a result of DMP
                unsigned long sensor_timestamp;
                short gyro[3], accel[3], sensors;
                unsigned char more = 0;
                long quat[4];
                
                /* This function gets new data from the FIFO when the DMP is in
                 * use. The FIFO can contain any combination of gyro, accel,
                 * quaternion, and gesture data. The sensors parameter tells the
                 * caller which data fields were actually populated with new data.
                 * For example, if sensors == (INV_XYZ_GYRO | INV_WXYZ_QUAT), then
                 * the FIFO isn't being filled with accel data.
                 * The driver parses the gesture data to determine if a gesture
                 * event has occurred; on an event, the application will be notified
                 * via a callback (assuming that a callback function was properly
                 * registered). The more parameter is non-zero if there are
                 * leftover packets in the FIFO.
                 */
                int success = dmp_read_fifo(gyro, accel, quat, &sensor_timestamp, &sensors, &more);
                SerialUSB.println("IMUTask: DMP read fifo returned: "+ String(success));
                if (!more) {
                    //hal.new_gyro = 0;
                    // TODO: more data to pull need a way to fetch it
                }
                
            } else {
                // TODO: interrupt not from DMP
                
            }
            xEventGroupClearBits(eventGroup,
                                 IMU_INT_BIT);
        }  else if( ( uxBits & IMU_DMP_BIT ) != 0 ) {
            SerialUSB.println("IMUTask: DMP enable bit set");
            enable_dmp();
            xEventGroupClearBits(eventGroup,
                                 IMU_DMP_BIT);
        } else {
            // wait timed out
           SerialUSB.println("IMUTask: Wait time out");
        }
        
    }
}

int8_t IMUInit() 
{
  
     // reset I2C bus
    // This ensures that if there was a reset, but the devices
    // on the I2C bus was not, that any transfer in progress do
    // not hang the device/bus.  Since the MPU-6050 has a 1024-byte
    // fifo, and there are 8 bits/byte, 10,000 clock edges
    // are generated to ensure the fifo is completely cleared
    // in the worst case.
    
    pinMode(PIN_WIRE_SDA, INPUT);
    pinMode(PIN_WIRE_SCL, OUTPUT);
    
    // Clock through up to 1000 bits
    int x = 0;
    for ( int i = 0; i < 10000; i++ ) {
     
      digitalWrite(PIN_WIRE_SCL, HIGH);
      digitalWrite(PIN_WIRE_SCL, LOW);
      digitalWrite(PIN_WIRE_SCL, HIGH);
      
      x++;
      if ( x == 8 ) {
        x = 0;
        // send a I2C stop signal
        digitalWrite(PIN_WIRE_SDA, HIGH);
        digitalWrite(PIN_WIRE_SDA, LOW);
      }
    }
    
    // send a I2C stop signal
    digitalWrite(PIN_WIRE_SDA, HIGH);
    digitalWrite(PIN_WIRE_SDA, LOW);
    Wire.begin();
     // MPU setup
     //SerialUSB.println("IMUTask: Initializing MPU");

    int count = 0;
    do {
      SerialUSB.println("IMUTask: Initializing MPU");
      if (count != 0) {
        SerialUSB.println("IMUTask: Forcing reset attempt " + String(count));
        mpu_force_reset();
        vTaskDelay((100/portTICK_PERIOD_MS));
      }
      count++;
    } while (MPUSetup() != 0);
  
   return 0;
   
}

int8_t MPUSetup()
{
    // TODO: should check here to see if IMU is allready setup and just init the lib
  
    int8_t result = 0;
    struct int_param_s int_param;
    int_param.cb = IMU_interrupt;
    int_param.pin = MPU_INT;
    int_param.arg = FALLING;
    
    unsigned char accel_fsr;
    unsigned short gyro_rate, gyro_fsr;
    unsigned long timestamp;
    
    result = mpu_init(&int_param);
    SerialUSB.println("IMUTask: mpu_init returned " + String(result));
    if (result != 0) {
        return -1;
    }

    /* Get/set hardware configuration. Start gyro. */
    /* Wake up all sensors. */
    result = mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL);
    SerialUSB.println("IMUTask: mpu_set_sensors returned " + String(result));
    
    /* Push both gyro and accel data into the FIFO. */
  //  result = mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);
  //  SerialUSB.println("IMUTask: mpu_configure_fifo returned " + String(result));
    
    result = mpu_set_sample_rate(IMU_DEFAULT_MPU_HZ);
    SerialUSB.println("IMUTask: mpu_set_sample_rate returned " + String(result));
    SerialUSB.flush();
    
//    vTaskDelay(10);
    SerialUSB.println("pre load");
    SerialUSB.flush();
    // vTaskDelay(100);
    
    result = dmp_load_motion_driver_firmware();
    SerialUSB.println("IMUTask: dmp_load_motion_driver_firmware returned " + String(result));
    if (result != 0) {
       return -2;
    }
    result = dmp_set_orientation( inv_orientation_matrix_to_scalar(gyro_orientation) );
    SerialUSB.println("IMUTask: dmp_set_orientation returned " + String(result));
    if (result != 0) {
        return -3;
    }
    
    /*
     * Known Bug -
     * DMP when enabled will sample sensor data at 200Hz and output to FIFO at the rate
     * specified in the dmp_set_fifo_rate API. The DMP will then sent an interrupt once
     * a sample has been put into the FIFO. Therefore if the dmp_set_fifo_rate is at 25Hz
     * there will be a 25Hz interrupt from the MPU device.
     *
     * There is a known issue in which if you do not enable DMP_FEATURE_TAP
     * then the interrupts will be at 200Hz even if fifo rate
     * is set at a different rate. To avoid this issue include the DMP_FEATURE_TAP
     */
    unsigned short dmp_features = DMP_FEATURE_TAP |                 // Dont remove this, see above
                                  DMP_FEATURE_ANDROID_ORIENT |      // we use this
                                  DMP_FEATURE_SEND_RAW_ACCEL |
                                  DMP_FEATURE_SEND_RAW_GYRO;
    
    result = dmp_enable_feature(dmp_features);
    if (result != 0) {
        SerialUSB.println("IMUTask: mp_enable_feature returned " + String(result));
        return -4;
    }
    
    
    result = dmp_set_fifo_rate(IMU_DEFAULT_MPU_HZ);
    if (result != 0) {
        SerialUSB.println("IMUTask: dmp_set_fifo_rate returned " + String(result));
        return -5;
    }
    
    dmp_register_tap_cb(IMU_tap_cb);
    dmp_register_android_orient_cb(IMU_android_orient_cb);
    
    
    return 0;
}
/* These next two functions converts the orientation matrix (see
 * gyro_orientation) to a scalar representation for use by the DMP.
 * NOTE: These functions are borrowed from Invensense's MPL.
 */
unsigned short inv_row_2_scale(const signed char *row) {
    
    unsigned short b;
    
    if (row[0] > 0)
    b = 0;
    else if (row[0] < 0)
    b = 4;
    else if (row[1] > 0)
    b = 1;
    else if (row[1] < 0)
    b = 5;
    else if (row[2] > 0)
    b = 2;
    else if (row[2] < 0)
    b = 6;
    else
    b = 7;      // error
    return b;
}

inline unsigned short inv_orientation_matrix_to_scalar(const signed char *mtx)
{
    unsigned short scalar;
    
    /*
     XYZ  010_001_000 Identity Matrix
     XZY  001_010_000
     YXZ  010_000_001
     YZX  000_010_001
     ZXY  001_000_010
     ZYX  000_001_010
     */
    
    scalar = inv_row_2_scale(mtx);
    scalar |= inv_row_2_scale(mtx + 3) << 3;
    scalar |= inv_row_2_scale(mtx + 6) << 6;
    
    
    return scalar;
}


int8_t disable_dmp()
{
    int8_t result = 0;
    result = mpu_set_dmp_state(0);
    
    if (result != 0) {
        SerialUSB.println("IMUTask: mpu_set_dmp_state(0) returned " + String(result));
        return -1;
    }
    dmp_state = IMU_DMP_OFF;
    return 0;
}

int8_t enable_dmp()
{
    int8_t result = 0;
    result = mpu_set_dmp_state(1);
    SerialUSB.println("IMUTask: mpu_set_dmp_state(1) returned " + String(result));
        
    if (result != 0) {
        return -1;
    }
    dmp_state = IMU_DMP_ON;
    return 0;
}


// stupid attach interupt hack
uint8_t firstLightFire = 1;

void buttonLightPress(){

    if (firstLightFire) {
      firstLightFire = 0;
      return;
    }
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
        static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        
        // set the Charge state bit to wake the PMIC Task
        xEventGroupSetBitsFromISR(eventGroup,
                                  IMU_DMP_BIT,
                                  &xHigherPriorityTaskWoken);
        
        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }

}




/******************************************************************************/


void vBlinkTask(void *pvParameters) {
    SerialUSB.println("Blink Task start");
    // int to hold led state
    uint8_t state = 0;
    // enabled pin 13 led
    pinMode(PIN_LED_TXL, OUTPUT);
    pinMode(PIN_LED_RXL, OUTPUT);
    while(1) {
        digitalWrite(PIN_LED_TXL, state);
        digitalWrite(PIN_LED_RXL, !state);
        state = !state;
        
        vTaskDelay((100/portTICK_PERIOD_MS));
    }
    
}

