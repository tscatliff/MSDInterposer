# MSDInterposer
MSD Interposer : Use a USB MSD to connect to your devices.

## Components
1. Hardware capable of presenting as a USB MSD (mass storage device).
2. The hardware exposes a file system, containing at least one file in the root folder. The rest of the file system might be filled with other files and folders, if you like.
3. One (or more) files in the file system are sparsed (containing no actual data), and reside in a known location on the backing store and have a known size.
4. Any reads from the file are intercepted, and user data is dynamically generated to fill the file.
5. Any writes to the file are intercepted, and the data is dynamically parsed to perform a function.

## Example Software
Found in the "Example" folder.

Based on:
USB Composite library for Roger's Melbourne's STM32F1 core: https://github.com/rogerclarkmelbourne/Arduino_STM32/
arpruss's USB Composite Library: https://github.com/arpruss/USBComposite_stm32f1

The example implements a ROM-backed MSD, 32x512 bytes in size. A minimal FAT12 file system is presented, with a single "README.HTM" file in the root, occupying block 4 on the backing store.

The device is has an external serial port. Any characters received on the serial port are buffered, and then sent during a read of "README.HTM", encapsulated in a HTML document for parsing. Of course, it could also return raw data, JSON, etc..

A write to "README.HTM" can be parsed, and a stream of characters can be sent out on the external serial port.

This is a quick example of how on can use the technique to build a serial port, accessible by file reads and writes on any USB-capable platform.

The technique could also be used to communicate over non-standard ports for a desktop, such as CAN/SPI/I2C/etc..

## Example Hardware
This example uses a "Blue Pill" board, loaded with the STM32duino bootloader: https://github.com/rogerclarkmelbourne/STM32duino-bootloader

## License
This project is licensed under CC0 1.0 Universal.
