Apa102 Adapter
=========

This is a lightweight buffer-less driver to send pixel data out via SPI on the ESP8266 to drive apa102 and apa102c (aka DotStar) LED strips.

Instead of setting pixels in a buffer then sending that data out, this calls out to a function to generate each pixel. The pixel data could come from some other buffer, or be created on the fly.

This uses less memory and can theoretically drive a very long chain of LEDs, albeit at slow refresh rates.

The apa102 "global brightness" per-pixel value is exposed as the 4th element in the rgbv array. This is a lower speed 5-bit brightness PWM on top of the main RGB pwm. While it will make flickering slightly more noticable, it can allow for much dimmer colors than is possible otherwise.

SPI data is sent while the next pixel is being generated for better refresh rates.

