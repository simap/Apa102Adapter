//
// Created by Jon C. Thomason on 3/19/22.
//

#ifndef NS108ADAPTER_HPP
#define NS108ADAPTER_HPP

#include <Arduino.h>

#include "SemiAsyncSPI.h"
#include <functional>


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

class NS108Adapter {
public:
    NS108Adapter(uint8_t order = APA102_RGB) {
        setColorOrder(order);
    }

    ~NS108Adapter() {
        end();
    }

    void begin(uint32_t spiFrequency = 16000000L) {
        SPIAsync.begin();
        SPIAsync.setFrequency(spiFrequency);
        SPIAsync.setBitOrder(MSBFIRST);
        SPIAsync.setDataMode(SPI_MODE0);
        SPIAsync.presetFrameSize(64);
    }

    void end() {
        SPIAsync.end();
    }

    void setSpiFrequency(uint32_t spiFrequency) {
        SPIAsync.setFrequency(spiFrequency);
    }

    void setColorOrder(uint8_t o) {
        rOffset = (uint8_t) (1+(o & 3)) * 2;
        gOffset = (uint8_t) (1+((o >> 2) & 3)) * 2;
        bOffset = (uint8_t) (1+((o >> 4) & 3)) * 2;
    }

    void show(uint16_t numPixels, ApaPixelFunction cb) {
        int pixelIndex;
        uint8_t gain;

        // 32-bit source pixel
        union {
            uint32_t frame;
            struct {
                uint8_t R;
                uint8_t G;
                uint8_t B;
                uint8_t V;
            } s;
            uint8_t b[4];
        } srcPixel;

        // 64-bit destination pixel
        union {
            uint64_t frame;
            uint32_t chunk[2];
            uint16_t w[4];
            uint8_t b[8];
        } outPixel;

        //start frame (128 bits low)
        SPIAsync.semiAsyncWrite64(0);
        SPIAsync.semiAsyncWrite64(0);

        //pixel sequence
        for (pixelIndex = 0; pixelIndex < numPixels; pixelIndex++) {

            // solicit from callback
            srcPixel.s = {0, 0, 0, 31}; //default to brightest black
            cb(pixelIndex, srcPixel.b);
            gain = srcPixel.s.V & 0x1f;

            //start bit, then three 5-bit component gain values, MSB-first
            outPixel.b[0] = outPixel.b[1] = 0xff; //start bit, and full gain for r, g, b

            //HACK: expand each 8-bit component and gain to 16-bit
            uint32_t element;
            element = srcPixel.s.R * gain * 256 / 31;
            outPixel.b[rOffset] = element>>8;
            outPixel.b[rOffset+1] = element;

            element = srcPixel.s.G * gain * 256 / 31;
            outPixel.b[gOffset] = element>>8;
            outPixel.b[gOffset+1] = element;

            element = srcPixel.s.B * gain * 256 / 31;
            outPixel.b[bOffset] = element>>8;
            outPixel.b[bOffset+1] = element;

            SPIAsync.semiAsyncWrite64(outPixel.frame);
        }

        //end frame (at least one additional bit per LED, high)
        for (uint8_t drain = (numPixels >> 6) + 1; drain > 0; drain--) {
            SPIAsync.semiAsyncWrite64(-1);
        }
    }

private:
    SemiAsyncSPIClass SPIAsync;
    uint8_t
            rOffset,                                // Index of red in 3-byte pixel
            gOffset,                                // Index of green byte
            bOffset;                                // Index of blue byte

};


#endif //NS108ADAPTER_HPP
