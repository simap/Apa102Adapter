//
// Created by Ben Hencke on 2/10/17.
//

#ifndef APA102ADAPTER_HPP
#define APA102ADAPTER_HPP

#include <Arduino.h>
#include <SPI.h>
#include <functional>

//borrowed from Adafruit
// Color-order flag for LED pixels (optional extra parameter to constructor):
// Bits 0,1 = R index (0-2), bits 2,3 = G index, bits 4,5 = B index
#define APA102_RGB (2 | (1 << 2) | (0 << 4))
#define APA102_RBG (2 | (2 << 2) | (1 << 4))
#define APA102_GRB (1 | (0 << 2) | (0 << 4))
#define APA102_GBR (0 | (0 << 2) | (1 << 4))
#define APA102_BRG (1 | (2 << 2) | (2 << 4))
#define APA102_BGR (0 | (1 << 2) | (2 << 4))

typedef std::function<void(uint16_t index, uint8_t rgbv[])> pixelCallback;

class Apa102Adapter {
public:
    Apa102Adapter(uint8_t o = APA102_BGR) {
        setColorOrder(o);
    }

    ~Apa102Adapter() {
        SPI.end();
    }

    void begin(uint32_t spiFrequency = 2000000L) {
        SPI.begin();
        SPI.setFrequency(spiFrequency);
        SPI.setBitOrder(MSBFIRST);
        SPI.setDataMode(SPI_MODE0);
    }

    void setSpiFrequency(uint32_t spiFrequency) {
        SPI.setFrequency(spiFrequency);
    }

    void setColorOrder(uint8_t o) {
        rOffset = (uint8_t) (o & 3);
        gOffset = (uint8_t) ((o >> 2) & 3);
        bOffset = (uint8_t) ((o >> 4) & 3);
    }

    void show(uint16_t numPixels, pixelCallback cb) {
        int i;
        union {
            uint32_t frame;
            uint8_t b[4];
        } buf, rgbv;

        //start frame
        SPI.write32(0);

        //pixels, sourced from callback
        for (i = 0; i < numPixels; i++) {
            rgbv.frame = 0x1f000000; //default to brightest black
            cb(i,rgbv.b);

            //pixel brightness (5 bits)
            buf.b[3] = (uint8_t) (rgbv.b[3] | 0xe0);

            //swap around rgb values based on mapping
            buf.b[rOffset] = rgbv.b[0];
            buf.b[gOffset] = rgbv.b[1];
            buf.b[bOffset] = rgbv.b[2];
            SPI.write32(buf.frame);
        }

        //end frame
        //reports differ as to how many end frame bits are required
        //since the strip is continuously updated in my app, I don't mind if some pixels lag slightly, e.g. are updated once the strip starts drawing the next update.
        //also instead of writing all ones, effectively a white pixel, draw a black pixel.
        SPI.write32(0xe0000000);
    }

private:
    uint8_t
            rOffset,                                // Index of red in 3-byte pixel
            gOffset,                                // Index of green byte
            bOffset;                                // Index of blue byte
};


#endif //APA102ADAPTER_HPP
