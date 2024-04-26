# imp23ultrasonic-sensor

This is IMP project 2023. 

## 📚 Introduction

The device measures the distance using sound up to 4 meters. It displays four digits (meter, decimeter, centimeter, millimeter), where the meter is separated from the rest by a dot. MCU is MK60D10. I worked at Kinetic Design Studio.

## 💾 Hardware

The device consists of a FITkit3 board, an ultrasonic distance meter HY-SRF05 and a segment LED display module.

## 📑 The solution method

The board was connected to the computer using a USB-A cable. The components were connected to the connecting connector P1.

The ultrasonic meter was connected:
* `Vcc` to pin number 2 (V5)
* `Trig` on pin number 35 (`port A`, pin 27)
* `Echo` to terminal number 37 (`port A`, pin 26)
* `OUT` was not used
* `GND` to pin number 50 (`GND`)

The segment LED display was connected to `PTA6` to `PTA11`, `MCU TMS`, `MCU TDO` (pins 23 to 30) to check the individual parts of the digit of the segment LED display (a, b, c, d, e, f, g). And to select the position of the segment LED display `MCU SPI2 CLK`, `MCU SPI2 CS1`, `MCU SPI2 MOSI`, `MCU SPI2 MISO` pins (pins 19 to 22).
Furthermore, the button marked `SW6` on the board was used to start the measurement.

## 📇 Way of solving

For the basic skeleton of the code, I used the LED display demo support material from Studis.
In the first step, it was necessary to set the pins of `port A` for `GPIO` (by setting 1):

```
PORTA->PCR[26] = ( 0|PORT_PCR_MUX(0x01) );
PORTA->PCR[27] = ( 0|PORT_PCR_MUX(0x01) );
```

And by setting 1 to position 27 of port A. A signal is sent to Trig via this pin:

```
PTA->PDDR = GPIO_PDDR_PDD( 0x8000FCC ); // original 0xFCC + 0x8000000
```

I reworked the PORTE_IRQHandler function so that it starts measuring when pressed and stops measuring after a second press. The state in which it is not measured shows 0 in the first position on the LED display.

Everything repeats ad infinitum in the forum. The start of measurement begins by sending a 10 us long pulse to the Trig (by setting `PTA->PDOR`). Then we wait in the while until `PTA->PDIR`, at position 26, is 0. As soon as there is a 1, we go to the second while, where we count the time and as soon as 0 occurs again, we calculate the distance and write it on the display.

The formula for measuring distance in the air using sound:

```
distance = ((time / 2) * 331) / 1000
```

This repeats ad infinitum until the abort button is pressed. The maximum distance is 4 meters. When measuring a distance so close that the component touches the object, no measurement is performed (time from Echo must be from 100 us to 25 ms).

## 📃 Final summary

The project meets the requirements of the assignment. The accuracy of the measurement depends on the accuracy of the person measuring the device straight and not swinging the device. But it still measures distance well. I don't know about the problems in my project, so I can't write anything about it. But the problems were easy with the parts that were given to me. The LED display always had a problem after debugging that some parts of the digits were flashing. This was resolved by unplugging and plugging the cable.
