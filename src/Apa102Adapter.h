//
// Created by Ben Hencke on 2/10/17.
//

#ifndef APA102ADAPTER_HPP
#define APA102ADAPTER_HPP

#include <Arduino.h>
#include <SPI.h>
#include <functional>

#ifdef ESP32
#include "soc/spi_struct.h"
constexpr spi_dev_t * spiDev = &SPI3;
#endif


typedef std::function<void(uint16_t index, uint8_t rgbv[])> ApaPixelFunction;

class Apa102Adapter {
public:
    Apa102Adapter() {
        setColorOrder(2, 1, 0);
    }

    ~Apa102Adapter() {
        end();
    }

    void begin(uint32_t spiFrequency = 2000000L) {
        SPI.begin();
        SPI.setFrequency(spiFrequency);
        SPI.setBitOrder(MSBFIRST);
        SPI.setDataMode(SPI_MODE0);

#ifdef ESP8266
        //borrowed from SPI.cpp, set registers for a 32bit transfer buffer
        uint16_t bits = 32;
        const uint32_t mask = ~((SPIMMOSI << SPILMOSI) | (SPIMMISO << SPILMISO));
        bits--;
        SPI1U1 = ((SPI1U1 & mask) | ((bits << SPILMOSI) | (bits << SPILMISO)));
#endif

#ifdef ESP32
        spiDev->user.usr_miso = 0; //disable input
        spiDev->user.doutdin = 0; //half duplex

        //config for 32 bit xfers
        spiDev->mosi_dlen.usr_mosi_dbitlen = 31;
        spiDev->miso_dlen.usr_miso_dbitlen = 31;
#endif
    }

    void end() {
        SPI.end();
    }

    void setSpiFrequency(uint32_t spiFrequency) {
        SPI.setFrequency(spiFrequency);
    }

    void setColorOrder(uint8_t ri, uint8_t gi, uint8_t bi) {
        rOffset = ri + 1;
        gOffset = gi + 1;
        bOffset = bi + 1;
    }

    void show(uint16_t numPixels, ApaPixelFunction cb) {
        int curPixel;
        union {
            uint32_t frame;
            uint8_t b[4];
        } buf, rgbv;

        //start frame
        write32(0);

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
            write32(buf.frame);
        }

        //end frame
        uint8_t extraShifts = (uint8_t) (1 + (numPixels >> 5));
        do {
            write32(0xffffffff);
        } while (--extraShifts > 0);
    }

private:
    inline void write32(uint32_t v) {
#ifdef ESP8266
        while(SPI1CMD & SPIBUSY) {}
        SPI1W0 = v;
        SPI1CMD |= SPIBUSY;
#endif
#ifdef ESP32

        //as usual, default transfer blocks for sending, and has a lot of redundancies
//        SPI.transfer32(v);

        while(spiDev->cmd.usr);
        spiDev->data_buf[0] = v;
        spiDev->cmd.usr = 1;
        //don't do this since I turned off MISO and full duplex
        //data = spi->dev->data_buf[0];
#endif

    }
    uint8_t
            rOffset,                                // Index of red in 3-byte pixel
            gOffset,                                // Index of green byte
            bOffset;                                // Index of blue byte
};


#endif //APA102ADAPTER_HPP
