# 一. Overview

## 1.1 Description

The `sstar/i2c/ut` directory contains pseudocode and documentation for testing the MIIC (Media Interface Controller). During the test process, you can obtain the executable programs required for testing in this directory.

## 1.2 Files

- Makefile: Used for compilation
- readme.md: Usage instructions
- i2c_read_write.c: Test pseudocode, which can be used for flexible testing of specified slave device register and write values
- ut_i2cdev.c: Test pseudocode, which can automatically complete the reading and writing of the MIIC and perform data accuracy comparisons. It is used for testing the i2c eeprom device of model AT24C256.

# 二. Operation Steps

## 2.1 Compilation

Execute the `make` command in the `ut` directory to obtain the executable files `i2c_read_write` and `ut_i2cdev`.

## 2.2 Testing

a) Set up the Linux environment, refer to the wiki technical support reference for specific details.

b) Confirm the padmux of the corresponding i2c bus, connect the slave device to the corresponding PAD, with VCC/GND/SCL/SDA connected in turn.

c) Copy the compiled executable program to the Linux environment on the board, or mount its directory via NFS to the board.

d) Execute the test command:

    ./i2c_read_write -b 0 -a 0xa0 -f A16D8 -w "0x0000 0x23"

- `-b 0`: Test the i2c bus 0
- `-a 0xa0`: Slave device address
- `-f A16D8`: Slave device address format, where A16 represents the register address length of 16 bits, D8 represents the length of the slave device address of 8 bits; in addition, there are also options A8D8, A8D16, and A16D16
- `-w "0x0000 0x23"`: Write operation, write the value 0x23 to the register 0x0000 of the slave device; if it is a read operation of register 0x0000: `-r "0x0000"`

Or use the following command:

    ./ut_i2cdev 0 0xa0 0x00 0x12

`./ut_i2cdev [bus] [slave addr] [regaddr] [length] [speed]`

- `bus`: i2c bus
- `slave addr`: Slave device address
- `regaddr`: Starting register address in the slave device to be operated on
- `length`: Transmission data length, in bytes
- `speed`: CLK cycle

This command will perform a write-read-calibration sequence, and after the command is executed, you only need to wait for the final result.
