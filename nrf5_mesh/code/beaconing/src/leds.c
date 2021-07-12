#include "leds.h"
#include "neopixel.h"
#include "neopixel_SPI.h"
#include "log.h"

int nodeNumber = 2; // TODO: switch between 0 and N depending on radio
const int N = 6;    // Reading only supported on pipes 1-5
// Addresses through which N modules communicate.
// Note: Using other types for addressing did not work for reading from multiple pipes.  Why?  IDK but this works ;-)
long long unsigned addresses[6] = {0xF0F0F0F0F0, 0xF0F0F0F0AA, 0xF0F0F0F0BB, 0xF0F0F0F0CC, 0xF0F0F0F0DD, 0xF0F0F0F0EE};

// Note: The LED_BUILTIN is connected to tx/rx so it requires
// serial communication (monitor open) in order to work.
// Using other LED instead

#define lowPulse 10
#define highPulse 255

// The length of the full breathing period
#define period 2200 // ms

const float periodMidpoint = (float)period / 2.0;
float amplitudeSlope = (float)(highPulse - lowPulse) / periodMidpoint;

// Keep track of when last message was received from another bike
// This tracks whether this bike is currently ‘in sync’ (in phase) with another bike
// Note: variables storing millis() must be unsigned longs
unsigned long lastReceiveTime;
unsigned long lastTransmitTime;
unsigned long currentTime;
unsigned long loopTime;   // using for profiling/debugging
unsigned long loopLength; // used for calculated expected latency

int sendFrequency = period / 2;

bool inSync = false; // True when in sync with another bicycle

// Keep track of where this bike is in its pulsating interval time
// This variable is updated with time when the bike is in sync with other bikes
// Otherwise it stays constant at the default
// The current value of the interval time determines the brightness of the light
const int defaultPhase = 0;
int phase = defaultPhase; // initialize to default

int expectedLatency;                  // ms; This number is updated based on length of loop.
const int maxAllowedPhaseShift = 100; // ms

// A variable keeps track of when time was last checked in order to properly
// update the interval time with time
unsigned long lastTimeCheck = 0; // set to now() in setup

int updatePhase();
void pulseLightCurve(int phase);

static nrf_drv_pwm_t m_pwm0 = NRF_DRV_PWM_INSTANCE(0);
const nrf_drv_timer_t TIMER_LED = NRF_DRV_TIMER_INSTANCE(1);

// Declare variables holding PWM sequence values. In this example only one channel is used
nrf_pwm_values_individual_t seq_values[] = {0, 0, 0, 0};
nrf_pwm_sequence_t const seq =
    {
        .values.p_individual = seq_values,
        .length = NRF_PWM_VALUES_LENGTH(seq_values),
        .repeats = 0,
        .end_delay = 0};

// Event handler for timer interrupts
void timer_led_event_handler(nrf_timer_event_t event_type, void *p_context)
{
    switch (event_type)
    {
    case NRF_TIMER_EVENT_COMPARE0:
        updatePhase();
        pulseLightCurve(phase);
        break;

    default:
        //Do nothing.
        break;
    }
}

// Initializes timer to update the phase every 100ms for smooth LED dimming
void timer_initalize(void)
{
    uint32_t time_ms = 16; //Time(in miliseconds) between consecutive compare events.
    uint32_t time_ticks;
    uint32_t err_code = NRF_SUCCESS;
    //Configure TIMER_LED for generating simple light effect - leds on board will invert his state one after the other.
    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    timer_cfg.frequency = NRF_TIMER_FREQ_31250Hz;
    err_code = nrf_drv_timer_init(&TIMER_LED, &timer_cfg, timer_led_event_handler);
    // printf("errcode: %d",err_code);
    APP_ERROR_CHECK(err_code);

    time_ticks = nrf_drv_timer_ms_to_ticks(&TIMER_LED, time_ms);
    nrf_drv_timer_extended_compare(&TIMER_LED, NRF_TIMER_CC_CHANNEL0, time_ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
    nrf_drv_timer_enable(&TIMER_LED);
}

// Initialize PWM for 3 channels of LEDS
void pwm_init(void)
{
    nrf_drv_pwm_config_t const config0 =
        {
            .output_pins =
                {
                    LED_1, // channel 0
                    // LED_2, // channel 3
                    LED_3,    // channel 1
                    LED_4,    // channel 2
                },
            .irq_priority = APP_IRQ_PRIORITY_LOWEST,
            .base_clock = NRF_PWM_CLK_1MHz,
            .count_mode = NRF_PWM_MODE_UP,
            .top_value = 100,
            .load_mode = NRF_PWM_LOAD_INDIVIDUAL,
            .step_mode = NRF_PWM_STEP_AUTO};
    // Init PWM without error handler
    APP_ERROR_CHECK(nrf_drv_pwm_init(&m_pwm0, &config0, NULL));
}

// Sets pwm for 3 channels similar to arduino AnalogWrite function.
void analogWrite(int pulse)
{

    int duty_cycle = pulse / 2.55;
    // __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Duty cycle: %d (%d)\n", duty_cycle, pulse);

    // Check if value is outside of range. If so, set to 100%
    if (duty_cycle >= 100)
    {
        seq_values->channel_0 = 100;
        seq_values->channel_1 = 100;
        seq_values->channel_2 = 100;
        // seq_values->channel_3 = 100;
    }
    else
    {
        seq_values->channel_0 = duty_cycle;
        seq_values->channel_1 = duty_cycle;
        seq_values->channel_2 = duty_cycle;
        // seq_values->channel_3 = duty_cycle;
    }

    nrf_drv_pwm_simple_playback(&m_pwm0, &seq, 1, NRF_DRV_PWM_FLAG_LOOP);
}

int min(int x, int y)
{
    return (x < y) ? x : y;
}

void swarm_leds_init(void)
{
    ret_code_t err_code;

    const led_sb_init_params_t led_sb_init_param = LED_SB_INIT_DEFAULT_PARAMS(BSP_LED_1_MASK | PIN_MASK(NRF_GPIO_PIN_MAP(0, 22)) | PIN_MASK(NRF_GPIO_PIN_MAP(0, 19)) | PIN_MASK(NRF_GPIO_PIN_MAP(0, 25)));

    err_code = led_softblink_init(&led_sb_init_param);
    APP_ERROR_CHECK(err_code);

    err_code = led_softblink_start(BSP_LED_1_MASK | PIN_MASK(NRF_GPIO_PIN_MAP(0, 22)) | PIN_MASK(NRF_GPIO_PIN_MAP(0, 19)) | PIN_MASK(NRF_GPIO_PIN_MAP(0, 25)));
    APP_ERROR_CHECK(err_code);
}

void swarm_leds_loop(void)
{
}

void swarm_leds_restart(void)
{
    ret_code_t err_code;

    err_code = led_softblink_uninit();
    APP_ERROR_CHECK(err_code);

    swarm_leds_init();
}

// Returns time since boot
unsigned long millis()
{
    unsigned long mills = 0;
    static unsigned long lastmills = 0;
    static unsigned int overflows = 0;

    mills = app_timer_cnt_get() * ((APP_TIMER_CONFIG_RTC_FREQUENCY + 1) * 1000) / APP_TIMER_CLOCK_FREQ;

    //if (mills<lastmills){
    //  overflows++;
    //}

    //mills= mills+(512000*overflows);

    //lastmills=mills;

    return mills;
}

void setup()
{
    // Starts serial communication, so that the Arduino can send out commands through the USB connection.
    // 9600 is the 'baud rate'
    // Serial.begin(9600);

    lastTimeCheck = millis();         // NOW
    lastReceiveTime = (-10) * period; // initialize at a long time ago
    lastTransmitTime = 0;

    // setupRadio();
    // pinMode(LED_PIN, OUTPUT);
    // nrf_gpio_cfg_output(LED_PIN);
}

int computePhaseShift(int phase1, int phase2)
{
    // Use phase1 as lower value, phase2 as higher value
    if (phase1 > phase2)
    {
        int temp = phase1;
        phase1 = phase2;
        phase2 = temp;
    }
    int a = phase2 - phase1;
    int b = period - phase2 + phase1;
    int phaseShift = min(a, b);
    printf("phaseShift %d \n", phaseShift);
    return phaseShift;
}

int updatePhase()
{
    // Update phase when in sync with another bike
    // Otherwise set phase to default and stay there
    currentTime = millis();
    if (inSync)
    {
        int timeDelta = (currentTime - lastTimeCheck);
        phase = (phase + timeDelta) % period;
    }
    else
    {
        phase = defaultPhase;
    }
    lastTimeCheck = currentTime;
    return phase;
}

void pulseLightCurve(int phase)
{
    // The light pulses on a sinusoidal corve
    // It starts HI and decreases in amplitude until the period midpoint: LO
    // And then increases in amplitude
    // 0:HI mid:LO
    // A = [cos(phase*2*pi/period) + 1]((HI - LO)/2) + LO
    float theta = phase * (2 * PI / (float)period);
    int amplitude = (cos(theta) + 1) * ((highPulse - lowPulse) / 2) + lowPulse;
    //    printf("amplitude %d  phase: %d  highPulse %d  lowPulse %d \n", amplitude, phase, highPulse, lowPulse);
    light(amplitude);
}

void pulseLightLinear(int phase)
{
    // The light pulses on a linear path
    // It starts HI and decreases in amplitude until the period midpoint: LO
    // And then increases in amplitude
    // Phase picture:
    //
    // |\           /\
// |  \       /    \
// |   \    /        \
// |     \/            \
//0:HI mid:LO
    int amplitude;
    if (phase < periodMidpoint)
    {
        amplitude = highPulse - (amplitudeSlope * phase);
    }
    else
    {
        amplitude = lowPulse + (amplitudeSlope * (phase - periodMidpoint));
    }
    light(amplitude);
}

void light(int amplitude)
{
    analogWrite(amplitude); // nrf52-DK
    
    neopixel_SPI(amplitude);

    __NOP();

    neopixel(amplitude); //(RJ)
}

void printTime(int num)
{
    unsigned long newTime = millis();
    printf("%d", num);
    printf(": Time: %d", newTime);

    printf("%d;  Delta: %d", newTime, newTime - currentTime);

    currentTime = newTime;
}

void ledloop()
{
    // Print how long a loop takes
    // Serial print statements add significant time
    // loop length 20-30ms without Serial print statements
    unsigned long newLoopTime = millis();
    loopLength = newLoopTime - loopTime;
    // printf("\n Total loop time length: %d \n", loopLength);
    // Serial.println(loopLength);
    loopTime = newLoopTime;
    // update current interval
    currentTime = millis();
    // check/update whether in sync with another bike
    // being in sync with another bike times out after given amount of time
    // without hearing from other bikes
    // inSync = false;
    // if ((currentTime - lastReceiveTime) < 2*period)  {
    inSync = true;
    //}
    // updatePhase();
    // pulseLightCurve(phase);
    // updatePhase();
    // listen for messages from other bikes
    bool changedPhase = false;
    // int otherPhase = radioReceive();

    //  int otherPhase = -1;
    // //needs to receive phase here
    //  if (otherPhase >= 0) {  // -1 indicates no message was received
    //    // another bike sent their current interval time
    //    lastReceiveTime = millis();  // update for later checking whether in sync
    //    // change to other bike’s interval iff it is GREATER than our current interval
    //    // allow a phase shift to determine whose interval is sooner to account for latency
    //    updatePhase();
    //    int allowedPhaseShift = min(maxAllowedPhaseShift, loopLength);
    //    if ((otherPhase > phase) && computePhaseShift(phase, otherPhase) > allowedPhaseShift)  {
    //      // change to other phase
    //      expectedLatency = loopLength/2;
    //      phase = otherPhase + expectedLatency;
    //      lastTimeCheck = lastReceiveTime;
    //      changedPhase = true;
    //    }
    //  }
    //  // send to other bikes at limited frequency or if phase recently changed
    //  if (changedPhase || (millis() - lastTransmitTime) > period) {
    //  //  radioBroadcast(phase);
    //  //needs to broadcast phase here
    //    lastTransmitTime = millis();
    //  }
}

int current_phase(void)
{
    return phase;
}

void set_phase(int phasecnt)
{

    phase = phasecnt;
    return;
}
