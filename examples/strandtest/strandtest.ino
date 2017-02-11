
#include <Apa102Adapter.h>

#define NUM_PIXELS 30

Apa102Adapter strip;
//Apa102Adapter strip(APA102_RGB); // change color order to RGB

void setup() {
    strip.begin();
//    strip.begin(8000000L); // change the data speed to 8MHz
}

void loop() {
    for (int counter = 0; counter < NUM_PIXELS; counter++) {
        strip.show(NUM_PIXELS, [counter](uint16_t index, uint8_t rgbv[]) {
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