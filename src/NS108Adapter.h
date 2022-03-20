//
// Created by Jon C. Thomason on 3/19/22.
//

#ifndef NS108ADAPTER_HPP
#define NS108ADAPTER_HPP

#include "SemiAsyncSPI.h"

// LED component order
enum { rComponent, gComponent, bComponent };
enum outputOrder {
    NS108_RGB = ((rComponent << 0) | (gComponent << 2) | (bComponent << 4)),
    NS108_RBG = ((rComponent << 0) | (bComponent << 2) | (gComponent << 4)),
    NS108_GRB = ((gComponent << 0) | (rComponent << 2) | (bComponent << 4)),
    NS108_GBR = ((gComponent << 0) | (bComponent << 2) | (rComponent << 4)),
    NS108_BRG = ((bComponent << 0) | (rComponent << 2) | (gComponent << 4)),
    NS108_BGR = ((bComponent << 0) | (gComponent << 2) | (rComponent << 4)),
};

typedef std::function<void(uint16_t index, uint8_t rgbv[])> ApaPixelFunction;

class NS108Adapter {
public:
    NS108Adapter(outputOrder order = NS108_RGB) {
        setColorOrder(order);
    }

    ~NS108Adapter() {
        end();
    }

    void begin(uint32_t spiFrequency = 16000000L) {
        SPI.begin();
        SPI.setFrequency(spiFrequency);
        SPI.setBitOrder(MSBFIRST);
        SPI.setDataMode(SPI_MODE0);
        SPI.presetFrameSize(64);
    }

    void end() {
        SPI.end();
    }

    void setSpiFrequency(uint32_t spiFrequency) {
        SPI.setFrequency(spiFrequency);
    }

    void setColorOrder(outputOrder order) {
        //first, second, third output component offsets from within source pixel
        //e.g. NS108_RBG -> { rComponent=0, bComponent=2, gComponent=1 }
        srcComponentOffset[0] = ((order >> 0) & 3);
        srcComponentOffset[1] = ((order >> 2) & 3);
        srcComponentOffset[2] = ((order >> 4) & 3);
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

        //start frame (64 bits low)
        SPI.semiAsyncWrite64(0);

        //pixel sequence
        for (pixelIndex = 0; pixelIndex < numPixels; pixelIndex++) {

            // solicit from callback
            srcPixel.s = {0, 0, 0, 31}; //default to brightest black
            cb(pixelIndex, srcPixel.b);
            gain = srcPixel.s.V & 0x1f;

            //start bit, then three 5-bit component gain values, MSB-first
            uint8_t *p = outPixel.b;
            *p++ = 0x80 | (gain << 2) | (gain >> 3);
            *p++ = (gain << 5) | gain;

            //expand each 8-bit component to 16-bit by stuttering
            *p++ = *p++ = srcPixel.b[srcComponentOffset[0]];
            *p++ = *p++ = srcPixel.b[srcComponentOffset[1]];
            *p++ = *p++ = srcPixel.b[srcComponentOffset[2]];

            SPI.semiAsyncWrite64(outPixel.frame);
        }

        //end frame (at least one additional bit per LED, high)
        for (uint8_t drain = (numPixels >> 6) + 1; drain > 0; drain--) {
            SPI.semiAsyncWrite64(-1);
        }
    }

private:
    uint8_t srcComponentOffset[3];   //see setColorOrder(outputOrder)
};


#endif //NS108ADAPTER_HPP
