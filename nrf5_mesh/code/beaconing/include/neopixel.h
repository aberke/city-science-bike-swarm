 #ifndef NEOPIXEL_H
 #define NEOPIXEL_H

#define OVERFLOW ((uint32_t)(0xFFFFFFFF / 32.768))

uint32_t millis(void);
uint32_t compareMillis(uint32_t previousMillis, uint32_t currentMillis);

void neopixel(int phase);

#endif
