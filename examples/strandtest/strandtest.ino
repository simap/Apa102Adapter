
#include <Apa102Adapter.h>

#define NUM_PIXELS 30

Apa102Adapter strip;
//Apa102Adapter strip(APA102_RGB, 1000000L); // change color order to RGB and data speed to 1MHz

void setup() {
    strip.begin();
}

void loop() {
    for (int counter = 0; counter < NUM_PIXELS; counter++) {
        strip.show(NUM_PIXELS, [](uint16_t index, uint8_t rgbv[]) {
            //if color order is set right, this should show a bright red pixel on dim green background
            if (counter == index) {
                rgbv[0] = 255;
            } else {
                rgbv[1] = 1;
            }
        });
        delay(100);
    }
}