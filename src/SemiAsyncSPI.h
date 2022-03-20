//
// Created by Jon C. Thomason on 3/19/22.
// Modified by Ben Hencke on 3/20/22.
//

#ifndef SEMI_ASYNC_SPI_HPP
#define SEMI_ASYNC_SPI_HPP

#include <SPI.h>

#ifdef ESP32
#include "soc/spi_struct.h"
//NOTE: it is also possible to get this via SPIClass::bus() and dereferencing the dev field
constexpr spi_dev_t * spiDev = &SPI3;
#endif


class SemiAsyncSPIClass: public SPIClass {
public:
  SemiAsyncSPIClass() : SPIClass(VSPI) {
    presetFrameSize(32);
  };

  // prepare for semiAsyncWrites of this size;
  // must reset whenever frame size changes
  // or if any regular sync writes are used
  //
  inline void presetFrameSize(uint8_t bitCount);

  // spinlock once upon entry to wait out queued bits,
  // then exit without waiting for completion or data
  //
  // all bytes are shifted out using in-memory order
  //
  inline void semiAsyncWrite32(uint32_t data);
  inline void semiAsyncWrite64(uint64_t data);
};


#ifdef ESP8266
  //synchronous implementation reference:
  //<https://github.com/esp8266/Arduino/blob/master/libraries/SPI/SPI.cpp>

  inline void SemiAsyncSPIClass::presetFrameSize(uint8_t bitCount) {
    const uint32_t mask = ~((SPIMMOSI << SPILMOSI) | (SPIMMISO << SPILMISO));
    bitCount -= 1;
    SPI1U1 = ((SPI1U1 & mask) | ((bitCount << SPILMOSI) | (bitCount << SPILMISO)));
  }

  inline void SemiAsyncSPIClass::semiAsyncWrite32(uint32_t data) {
    while(SPI1CMD & SPIBUSY) {}
    SPI1W0 = data;
    SPI1CMD |= SPIBUSY;
  }

  inline void SemiAsyncSPIClass::semiAsyncWrite64(uint64_t data) {
    uint32_t *chunk = (uint32_t*)&data;
    while(SPI1CMD & SPIBUSY) {}
    SPI1W0 = chunk[0];
    SPI1W1 = chunk[1];
    SPI1CMD |= SPIBUSY;
  }
#endif


#ifdef ESP32
  //synchronous implementation reference:
  //<https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/esp32-hal-spi.c>

  inline void SemiAsyncSPIClass::presetFrameSize(uint8_t bitCount) {
    bitCount -= 1;
    spiDev->user.usr_miso = 0; //disable input
    spiDev->user.doutdin = 0; //half duplex
    spiDev->mosi_dlen.usr_mosi_dbitlen = bitCount;
    spiDev->miso_dlen.usr_miso_dbitlen = bitCount;
  }

  inline void SemiAsyncSPIClass::semiAsyncWrite32(uint32_t data) {
    while(spiDev->cmd.usr);
    spiDev->data_buf[0] = data;
    spiDev->cmd.usr = 1;
  }

  inline void SemiAsyncSPIClass::semiAsyncWrite64(uint64_t data) {
    uint32_t *chunk = (uint32_t*)&data;
    while(spiDev->cmd.usr);
    spiDev->data_buf[0] = chunk[0];
    spiDev->data_buf[1] = chunk[1];
    spiDev->cmd.usr = 1;
  }
#endif

#endif // SEMI_ASYNC_SPI_HPP
