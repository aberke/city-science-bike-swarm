#include "leds.h"
#include "neopixel.h"
#include "neopixel_SPI.h"
#include "log.h"
#include "buttons.h"
#include "app_timer.h"

// LED boundaries, 0x00-0xFF
#define lowPulse 10
#define highPulse 255
#define phaseUpdatePeriod 16

// Limiting pulse anytime more than a single led channel is lit due to battery discharge capacity
#define limitedHighPulse 55

const float periodMidpoint = (float)PHASE_DURATION / 2.0;
float amplitudeSlope = (float)(highPulse - lowPulse) / periodMidpoint;

// Keep track of when last message was received from another bike
// This tracks whether this bike is currently ‘in sync’ (in phase) with another bike
// Note: variables storing millis() must be unsigned longs
unsigned long lastReceiveTime;
unsigned long lastTransmitTime;
unsigned long currentTime;
unsigned long loopTime;   // using for profiling/debugging
unsigned long loopLength; // used for calculated expected latency

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

APP_TIMER_DEF(timer_phasetimer);

static void phasetimer_handler(void *p_context)
{
         updatePhase();
         pulseLightCurve(phase);
}

void phasetimer_init() {
    ret_code_t err_code = app_timer_create(&timer_phasetimer, APP_TIMER_MODE_REPEATED, phasetimer_handler);
    APP_ERROR_CHECK(err_code);
    err_code = app_timer_start(timer_phasetimer, APP_TIMER_TICKS(phaseUpdatePeriod), NULL);
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


void setup()
{
    // Starts serial communication, so that the Arduino can send out commands through the USB connection.
    // 9600 is the 'baud rate'
    // Serial.begin(9600);

 //   lastTimeCheck = millis();         // NOW
    lastReceiveTime = (-10) * PHASE_DURATION; // initialize at a long time ago
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
    int b = PHASE_DURATION - phase2 + phase1;
    int phaseShift = min(a, b);
    printf("phaseShift %d \n", phaseShift);
    return phaseShift;
}

int updatePhase()
{
    // Update phase when in sync with another bike
    // Otherwise set phase to default and stay there
  //  currentTime = millis();
    if (inSync)
    {
    //    int timeDelta = (currentTime - lastTimeCheck);
    phase = (phase + phaseUpdatePeriod) % PHASE_DURATION;
    }
    else
    {
        phase = defaultPhase;
    }
    lastTimeCheck = currentTime;
  //   printf("phase %d \n", phase);
    return phase;
}

void pulseLightCurve(int phase)
{
    // The light pulses on a sinusoidal corve
    // It starts HI and decreases in amplitude until the period midpoint: LO
    // And then increases in amplitude
    // 0:HI mid:LO
    // A = [cos(phase*2*pi/period) + 1]((HI - LO)/2) + LO

    float theta = phase * (2 * PI / (float)PHASE_DURATION);
    int relativeHighPulse = highPulse;
    btn_color_t current_color = btn_current_color();
    if (current_color.r + current_color.g + current_color.b > 0xFF) {
        relativeHighPulse = limitedHighPulse;
    }
    int amplitude = (cos(theta + PI) + 1) * ((highPulse - lowPulse) / 2) + lowPulse;
    int limitedAmplitude = (cos(theta + PI) + 1) * ((relativeHighPulse - lowPulse) / 2) + lowPulse;
    //    printf("amplitude %d  phase: %d  highPulse %d  lowPulse %d \n", amplitude, phase, highPulse, lowPulse);
    light(amplitude, limitedAmplitude);
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
    light(amplitude, amplitude);
}

void light(int amplitude, int limitedAmplitude)
{
    analogWrite(amplitude); // nrf52-DK
    
    neopixel_SPI(amplitude);

    __NOP();

    neopixel(limitedAmplitude); //(RJ)
}

void ledloop()
{
    // Print how long a loop takes
    // Serial print statements add significant time
    // loop length 20-30ms without Serial print statements
    //unsigned long newLoopTime = millis();
    //loopLength = newLoopTime - loopTime;
    //// printf("\n Total loop time length: %d \n", loopLength);
    //// Serial.println(loopLength);
    //loopTime = newLoopTime;
    //// update current interval
    //currentTime = millis();
    // check/update whether in sync with another bike
    // being in sync with another bike times out after given amount of time
    // without hearing from other bikes

    inSync = true;

    // listen for messages from other bikes
    bool changedPhase = false;

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
