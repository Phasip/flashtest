# flashtest
Check if sd-cards and usb devices are fake.

## WARNING: This program completely erases a disk without any prompts!

Trivial program that tests sd-cards and usb sticks by first completely filling
a device with random data and then attempting to read back that data.

This is ment to identify fake devices that report an incorrect size.

A faster test would probably be to write to the end of the device and see that the data can be retrieved.

## Usage
```./flashtest <device>```

