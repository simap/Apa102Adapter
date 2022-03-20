//
// Created by Ben Hencke on 2/10/17.
// Modified by Jon C. Thomason on 3/19/22.
//

#ifndef APA102ADAPTER_HPP
#define APA102ADAPTER_HPP

#include "SemiAsyncSPI.h"

//borrowed from Adafruit
// Color-order flag for LED pixels (optional extra parameter to constructor):
// Bits 0,1 = R index (0-2), bits 2,3 = G index, bits 4,5 = B index
#define APA102_RGB (0 | (1 << 2) | (2 << 4))
#define APA102_RBG (0 | (2 << 2) | (1 << 4))
#define APA102_GRB (1 | (0 << 2) | (2 << 4))
#define APA102_GBR (2 | (0 << 2) | (1 << 4))
#define APA102_BRG (1 | (2 << 2) | (0 << 4))
#define APA102_BGR (2 | (1 << 2) | (0 << 4))

typedef std::function<void(uint16_t index, uint8_t rgbv[])> ApaPixelFunction;

class Apa102Adapter {
public:
    Apa102Adapter(uint8_t o = APA102_BGR) {
        setColorOrder(o);
    }

    ~Apa102Adapter() {
        end();
    }

    void begin(uint32_t spiFrequency = 2000000L) {
        SPI.begin();
        SPI.setFrequency(spiFrequency);
        SPI.setBitOrder(MSBFIRST);
        SPI.setDataMode(SPI_MODE0);
        SPI.presetFrameSize(32);
    }

    void end() {
        SPI.end();
    }

    void setSpiFrequency(uint32_t spiFrequency) {
        SPI.setFrequency(spiFrequency);
    }

    void setColorOrder(uint8_t o) {
        rOffset = (uint8_t) (1+(o & 3));
        gOffset = (uint8_t) (1+((o >> 2) & 3));
        bOffset = (uint8_t) (1+((o >> 4) & 3));
    }

    void show(uint16_t numPixels, ApaPixelFunction cb) {
        int curPixel;
        union {
            uint32_t frame;
            uint8_t b[4];
        } buf, rgbv;

        //start frame
        SPI.semiAsyncWrite32(0);

        //pixels, sourced from callback
        for (curPixel = 0; curPixel < numPixels; curPixel++) {
            rgbv.frame = 0x1f000000; //default to brightest black
            cb(curPixel,rgbv.b);

            //pixel brightness (5 bits)
            buf.b[0] = (uint8_t) (rgbv.b[3] | 0xe0);

            //swap around rgb values based on mapping
            buf.b[rOffset] = rgbv.b[0];
            buf.b[gOffset] = rgbv.b[1];
            buf.b[bOffset] = rgbv.b[2];
            SPI.semiAsyncWrite32(buf.frame);
        }

        //end frame
        uint8_t extraShifts = (uint8_t) (1 + (numPixels >> 5));
        do {
            SPI.semiAsyncWrite32(0xffffffff);
        } while (--extraShifts > 0);
    }

private:
    uint8_t
            rOffset,                                // Index of red in 3-byte pixel
            gOffset,                                // Index of green byte
            bOffset;                                // Index of blue byte
};


#endif //APA102ADAPTER_HPP