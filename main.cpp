/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
//#include <iic_sensors.cpp>
#include <stdio.h>
#include <errno.h>
// Blockdevice.
#include "mbed-os/storage/blockdevice/COMPONENT_SD/include/SD/SDBlockDevice.h"
// File system.
#include "FATFileSystem.h"

// Blinking rate in milliseconds
#define BLINKING_RATE   500ms
// Wait between print in milliseconds
#define PRINTF_RATE     60000ms

// Structure to hold shtc3 data.
struct Shtc3Outputs
{
	char data[7];   // Sensors data. 
	char done;		// State machine is done. 
};

/* Function prototype(s). **************************************************************************************/
struct Shtc3Outputs i2c_fsm_shtc3(void);    // Function returns the structure. 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Create an I2C object.
//I2C i2c(I2C_SDA , I2C_SCL);

/***** Function begins *****************************************************************************************/
// This is for the Sensirion shtc3 sensor. 
struct Shtc3Outputs i2c_fsm_shtc3(void)
{
	int shtc3_addr = 0xe0;              // Shtc3 address.
	char i2c_buffer[10]= 	{
								0xE0,	// Send addr. write. 	0	
								0x35,	// Wakeup command msb	1
								0x17,	// Wakeup command lsb	2
								0x5C,	// Meas. command msb	3 		0
								0x24,	// Meas. command lsb	4		1
								0xE1,	// Send addr. read		5		2
								0xB0,   // Sleep command msb	6		3
								0x98,	// Sleep command lsb	7		4
								0x00,	// read r.h. data.		8		5
								0x00 	// read r.h. crc		9		6
							};	
    char sens_dat[6]= 	    {
                                0x00,	
	                            0x00,
                                0x00,	
		                        0x00,	
	                            0x00,	
	                            0x00,	
                            };
	struct Shtc3Outputs ste_out;	    // Sensor outputs
    int write_rtrn = 0;

    // Create an I2C object.
    //I2C i2c(I2C_SDA , I2C_SCL);
    // Create an I2C object for pins, pf14 and pf15, scl4 and sda4.
    I2C i2c(PF_15, PF_14);
    // Create an I2C object for pins, pf14 and pf15, scl4 and sda4.
    //I2C i2c_ir_temp(PF_15, PF_14);

    /* To get the data from sensor Sensirion shtc3, do the next 6 steps. */
    // Step 1.
    i2c.start();
    // Send the wakeup command, msb and lsb.
    //i2c.write(0x00);
    write_rtrn = i2c.write(shtc3_addr, i2c_buffer + 1, 2);
    ThisThread::sleep_for(10ms);
    // Step 2.
    i2c.start();
    // Send the measurement command, msb and lsb.
    i2c.write(shtc3_addr, i2c_buffer + 3, 2, 0);
    ThisThread::sleep_for(40ms);
    // Step 3.
    i2c.start();
    // Send repeated i2c address with read.
    i2c.write(*(i2c_buffer + 5));
    i2c.stop();
    ThisThread::sleep_for(10ms);
    // Step 4.
    i2c.start();
    // Send i2c address with read.
    i2c.write(*(i2c_buffer + 5));
    // Step 5.
    // Read humidity msb, lsb, crc, temp msb, lsb and crc.
    //i2c.read(shtc3_addr, sens_dat, 6, 0);
    i2c.read(shtc3_addr, ste_out.data, 6, 0);
    // Step 6.
    i2c.start();
    // Send the sleep command, msb and lsb.
    i2c.write(shtc3_addr, i2c_buffer + 6, 2, 1);
    //ThisThread::sleep_for(40ms);

    //ste_out.data = sens_dat;

    return ste_out;
}

// Physical block device, can be any device that supports the BlockDevice API
SDBlockDevice   blockDevice(ARDUINO_UNO_D11, ARDUINO_UNO_D12, ARDUINO_UNO_D13, ARDUINO_UNO_D4);  // mosi, miso, sck, cs
//SDBlockDevice   blockDevice(ARDUINO_UNO_SPI_MOSI, ARDUINO_UNO_SPI_MISO, ARDUINO_UNO_SPI_SCK, ARDUINO_UNO_SPI_CS);  // mosi, miso, sck, cs
//SDBlockDevice   blockDevice(SPI_MOSI, SPI_MISO, SPI_SCK, SPI_CS);  // mosi, miso, sck, cs
//SDBlockDevice blockDevice(MBED_CONF_SD_SPI_MOSI, MBED_CONF_SD_SPI_MISO, MBED_CONF_SD_SPI_CLK, MBED_CONF_SD_SPI_CS); // mosi, miso, sck, cs

// File system declaration
FATFileSystem   fileSystem("fs");

int main()
{
    // Initialise the digital pin LED1 as an output.
    DigitalOut led(LED3);
    // Variable to get shtc3 sensor data.
    struct Shtc3Outputs sen_dat;
    // Pointer to get shtc3 sensor data.
    struct Shtc3Outputs *sen_dat_ptr;
    int err = 0;
    
    set_time(1256729737);  // Set RTC time to Wed, 28 Oct 2009 11:35:37

    // Try to mount the filesystem.
    printf("Mounting the filesystem... ");
    fflush(stdout);
    err = fileSystem.mount(&blockDevice);
    
    printf("%s\n", (err ? "Fail :(" : "OK"));
    if (err) {
        // Reformat if we can't mount the filesystem,
        // this should only happen on the first boot.
        printf("No filesystem found, formatting... ");
        fflush(stdout);
        err = fileSystem.reformat(&blockDevice);
        printf("%s\n", (err ? "Fail :(" : "OK"));
        if (err) {
            error("error: %s (%d)\n", strerror(-err), err);
        }
    }

    // Open the numbers file.
    printf("Opening \"/fs/numbers.txt\"... ");
    fflush(stdout);
 
    FILE*   f = fopen("/fs/numbers.txt", "r+");
    printf("%s\n", (!f ? "Fail :(" : "OK"));
    if (!f) {
        // Create the numbers file if it doesn't exist
        printf("No file found, creating a new file... ");
        fflush(stdout);
        f = fopen("/fs/numbers.txt", "w+");
        printf("%s\n", (!f ? "Fail :(" : "OK"));
        if (!f) {
            error("error: %s (%d)\n", strerror(errno), -errno);
        }
 
        for (int i = 0; i < 10; i++) {
            printf("\rWriting numbers (%d/%d)... ", i, 10);
            fflush(stdout);
            err = fprintf(f, "    %d\n", i);
            if (err < 0) {
                printf("Fail :(\n");
                error("error: %s (%d)\n", strerror(errno), -errno);
            }
        }
 
        printf("\rWriting numbers (%d/%d)... OK\n", 10, 10);
 
        printf("Seeking file... ");
        fflush(stdout);
        err = fseek(f, 0, SEEK_SET);
        printf("%s\n", (err < 0 ? "Fail :(" : "OK"));
        if (err < 0) {
            error("error: %s (%d)\n", strerror(errno), -errno);
        }
    }
    
    while (true) {

        time_t seconds = time(NULL);

        //printf("Time as seconds since January 1, 1970 = %u\n", (unsigned int)seconds);

        printf("Time as a basic string = %s", ctime(&seconds));

        //char buffer[32];
        //strftime(buffer, 32, "%I:%M %p\n", localtime(&seconds));
        //printf("Time as a custom formatted string = %s", buffer);

        led = !led;
        //ThisThread::sleep_for(BLINKING_RATE);
        ThisThread::sleep_for(PRINTF_RATE);
        printf("Wait every 1 minute... ");
    
        sen_dat = i2c_fsm_shtc3();
        //*sen_dat_ptr = i2c_fsm_shtc3();
    }
}
