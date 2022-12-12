/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
//#include <iic_sensors.cpp>
#include <stdio.h>
#include <errno.h>
// Make sure you add sd card in mbed_app.json file or sd card won't work.
// If you don't have mbed_app_json, add this file.
// Blockdevice.
#include "mbed-os/storage/blockdevice/COMPONENT_SD/include/SD/SDBlockDevice.h"
// File system.
#include "FATFileSystem.h"
#include "BufferedSerial.h"

// Blinking rate in milliseconds
#define BLINKING_RATE   500ms
// Wait between print in milliseconds
//#define PRINTF_RATE     60000ms   // One minute.
//#define PRINTF_RATE     30000ms // 30 seconds.
//#define PRINTF_RATE     15000ms // 15 seconds.
//#define PRINTF_RATE     10000ms // 10 seconds.
#define PRINTF_RATE     5000ms // 5 seconds.
#define ONE_SEC         1000ms // 1 seconds.
#define TWO_SEC         2000ms // 2 seconds.
#define FIVE_SEC        5000ms // 1 seconds.

//BufferedSerial pc(USBTX, USBRX, 9600);

// Structure to hold shtc3 data.
struct Shtc3Outputs
{
	char data[19];  // Sensors data. Shtc3 actually only needs 6 bytes.
	//char *data;     // Sensors data array. 
	char done;		// State machine is done. 
};

/* Function prototype(s). **************************************************************************************/
struct Shtc3Outputs i2c_fsm_shtc3(void);        // Function returns the structure. 
struct Shtc3Outputs *i2c_fsm_shtc3_ptr(void);   // Function returns the pointer to structure. 
struct Shtc3Outputs *i2c_d6t_ptr(void);         // Function returns the pointer to structure. 
struct Shtc3Outputs *i2c_d6t_8l_ptr(void);      // Function returns the pointer to structure. 
char *i2c_d6t_8L_ptr(void);                     // Function returns the pointer to char. 
void ser_com (void);                            // Serial function to get data from gps.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Create an I2C object.
//I2C i2c(I2C_SDA , I2C_SCL);
//I2C i2c(ARDUINO_UNO_D14, ARDUINO_UNO_D15);  // i2c1_sda, i2c1_scl
    
// Set up the serial for the board.
//static BufferedSerial get_gps(D8, D7, 9600);
    
/***** Function begins *****************************************************************************************/
// This is for the Sensirion shtc3 sensor. Working.
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
    // Using arduino pins for sda and scl.
    I2C i2c(ARDUINO_UNO_D14, ARDUINO_UNO_D15);  // i2c1_sda, i2c1_scl
    
    /* To get the data from sensor Sensirion shtc3, do the next 6 steps. */
    // Step 1.
    i2c.start();
    // Send the wakeup command, msb and lsb.
    write_rtrn = i2c.write(shtc3_addr, i2c_buffer + 1, 2);
    ThisThread::sleep_for(5ms);
    // Step 2.
    i2c.start();
    // Send the measurement command, msb and lsb.
    i2c.write(shtc3_addr, i2c_buffer + 3, 2, 0);
    ThisThread::sleep_for(10ms);
    // Step 3.
    i2c.start();
    // Send i2c address with read.
    i2c.write(*(i2c_buffer + 5));
    ThisThread::sleep_for(5ms);
    // Step 4.
    // Read humidity msb, lsb, crc, temp msb, lsb and crc.
    i2c.read(shtc3_addr, ste_out.data, 6, 0);
    // Step 5.
    i2c.start();
    // Send the sleep command, msb and lsb.
    i2c.write(shtc3_addr, i2c_buffer + 6, 2, 1);
    i2c.stop();
    
    return ste_out;
}

// This is for the Sensirion shtc3 sensor. 
struct Shtc3Outputs *i2c_fsm_shtc3_ptr(void)
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
	struct Shtc3Outputs *ste_out_ptr;   // Pointer to sensor outputs.
    int write_rtrn = 0;

    // Create an I2C object.
    //I2C i2c(I2C_SDA , I2C_SCL);
    // Create an I2C object for pins, pf14 and pf15, scl4 and sda4.
    //I2C i2c(PF_15, PF_14);
    // Using arduino pins for sda and scl.
    I2C i2c(ARDUINO_UNO_D14, ARDUINO_UNO_D15);  // i2c1_sda, i2c1_scl
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
    i2c.read(shtc3_addr, ste_out_ptr->data, 6, 0);
    // Step 6.
    i2c.start();
    // Send the sleep command, msb and lsb.
    i2c.write(shtc3_addr, i2c_buffer + 6, 2, 1);
    //ThisThread::sleep_for(40ms);

    return ste_out_ptr; // Return pointer to sensor outputs.
}

// This is for the Omron D6T-1A-02 sensor. 
struct Shtc3Outputs *i2c_d6t_ptr(void)
{
	int d6t_addr = 0x14;                // Omron D6T address.
	//int d6t_addr = 0x0A;                // Omron D6T address.
	char i2c_buffer[10]= 	{
								0x14,	// Write address.       0	
								0x4C,	// Meas. command	    1
								0x00,	// 	                    2
								0x00,	// 	                    3 		0
								0x00,	//  	                4		1
								0x15,	// Send addr. read		5		2
								0x00,   // 	                    6		3
								0x00,	// 	                    7		4
								0x00,	// 		                8		5
								0x00 	// 		                9		6
							};	
    char sens_dat[6]= 	    {
                                0x00,	
	                            0x00,
                                0x00,	
		                        0x00,	
	                            0x00,	
	                            0x00,	
                            };
	struct Shtc3Outputs *ste_out_ptr;   // Pointer to sensor outputs.
    int write_rtrn = 0;

    // Create an I2C object.
    //I2C i2c(I2C_SDA , I2C_SCL);
    // Create an I2C object for pins, pf14 and pf15, scl4 and sda4.
    //I2C i2c(PF_15, PF_14);
    // Using arduino pins for sda and scl.
    I2C i2c(ARDUINO_UNO_D14, ARDUINO_UNO_D15);  // i2c1_sda, i2c1_scl
    // Create an I2C object for pins, pf14 and pf15, scl4 and sda4.
    //I2C i2c_ir_temp(PF_15, PF_14);

    /* To get the data from sensor Sensirion shtc3, do the next 7 steps. */
    // Step 1.
    i2c.start();
    // Step 2.
    // Send the write address and command with repeated start.
    //write_rtrn = i2c.write(d6t_addr, i2c_buffer + 1, 1, true);
    //write_rtrn = i2c.write(d6t_addr, i2c_buffer + 1, 1);
    i2c.write(*(i2c_buffer + 0));
    printf("i2c_buffer: %s ", (i2c_buffer+0));      // okay
    //printf("write_rtrn: %d ", write_rtrn);      // okay
    i2c.stop();
    ThisThread::sleep_for(10ms);
        // Step 3.
    // Send repeat start and read address.
    i2c.start();
    // Send i2c address with read.
    i2c.write(*(i2c_buffer + 5));
    // Step 4.
    // Read humidity msb, lsb, crc, temp msb, lsb and crc.
    //i2c.read(shtc3_addr, sens_dat, 6, 0);
    //i2c.read(d6t_addr, ste_out_ptr->data, 6, 0);
    // Step 5.
    i2c.stop();
    
    //printf("Data inside function = %s ", ste_out_ptr->data+1);    // okay
        
    return ste_out_ptr; // Return pointer to sensor outputs.
}

// This is for the Omron D6T-8L sensor. 
struct Shtc3Outputs *i2c_d6t_8l_ptr(void)   // Function returns pointer to structure.
//char *i2c_d6t_8l_ptr(void)                   // Function returns pointer to character.
{
	int d6t_addr = 0x14;                // Omron D6T address.
	char i2c_buffer[10]= 	{
								0x14,	// Write address.       0	
								0x4C,	// Meas. command	    1
								0x02,	// 	                    2
								0x05,	// 	                    3 		0
								0x03,	//  	                4		1
								0x15,	// Send addr. read		5		2
								0x00,   // 	                    6		3
								0x00,	// 	                    7		4
								0x00,	// 		                8		5
								0x00 	// 		                9		6
							};	
    char bufr_1[4]= 	{
								0x02,	// 0	
								0x00,	// 1
								0x01,	// 2
								0xee	// 3----
						};	
    char bufr_2[4]= 	{
								0x05,	// 4
								0x90,	// 5
								0x3a,   // 6
								0xb8    // 7----		
						};	
    char bufr_3[4]= 	{
								0x03,	// 8
								0x00,   // 9
                                0x03,   // 10
                                0x8b    // 11----
                        };	
    char bufr_4[4]= 	{
								0x03,   // 12
                                0x00,   // 13
                                0x07,   // 14
                                0x97    // 15----
                        };	
    char bufr_5[4]= 	{
								0x02,   // 16
                                0x00,   // 17
                                0x00,   // 18
                                0xe9 	// 19----
						};	
    //char *sens_dat;
    
    char sens_dat[19]= 	    {
                                0x00,   // 0	
	                            0x00,   // 1
                                0x00,	// 2
		                        0x00,	// 3
	                            0x00,	// 4
	                            0x00,	// 5
                                0x00,   // 6	
	                            0x00,   // 7
                                0x00,	// 8
		                        0x00,	// 9
	                            0x00,	// 10
	                            0x00,	// 11
                                0x00,   // 12	
	                            0x00,   // 13
                                0x00,	// 14
		                        0x00,	// 15
	                            0x00,	// 16
	                            0x00,	// 17
                                0x00    // 18                            
                            };
	
    //char sens_dat[19];
    //char *sens_dat;
    // Need to initialize *ste_pout_ptr?
    struct Shtc3Outputs *ste_out_ptr;   // Pointer to sensor outputs.
    int write_rtrn = 0;
    
    //sens_dat = malloc(sizeof(char) * 19);

    // Assign memory to the pointer.
    //ste_out_ptr = malloc(sizeof(Shtc3Outputs));
    // Assign memory to data pointer.
    //ste_out_ptr->data = malloc(sizeof(char) * 19);

    /* Can't do this with because *ste_out_ptr is a pointer not a variable.
    Shtc3Outputs ste_out_ptr = {
        data = "0123456789012345678",
        done = "0"
    } */

    // Create an I2C object.
    // Using arduino pins for sda and scl.
    I2C i2c(ARDUINO_UNO_D14, ARDUINO_UNO_D15);  // i2c1_sda, i2c1_scl
    
    /* To get the data from sensor do next several steps. */
    // Step 1.
    i2c.start();
    i2c.write(d6t_addr, bufr_1 + 0, 4);
    i2c.stop();
    // Step 2.
    i2c.start();
    i2c.write(d6t_addr, bufr_2 + 0, 4);
    i2c.stop();
    // Step 3.
    i2c.start();
    i2c.write(d6t_addr, bufr_3 + 0, 4);
    i2c.stop();
    // Step 4.
    i2c.start();
    i2c.write(d6t_addr, bufr_4 + 0, 4);
    i2c.stop();
    // Step 5.
    i2c.start();
    i2c.write(d6t_addr, bufr_5 + 0, 4);
    i2c.stop();
    // Steps 6 through 8 are needed when the sensor is first turned on.
    // Step 6.
    //i2c.start();
    //i2c.write(d6t_addr, i2c_buffer + 2, 1);
    //i2c.read(d6t_addr, sens_dat, 2);
    //i2c.stop();
    // Step 7.
    //i2c.start();
    //i2c.write(d6t_addr, i2c_buffer + 3, 1);
    //i2c.read(d6t_addr, sens_dat, 2);
    //i2c.stop();
    // Step 8.
    //i2c.start();
    //i2c.write(d6t_addr, i2c_buffer + 4, 1);
    //i2c.read(d6t_addr, sens_dat, 2);
    //i2c.stop();
    ThisThread::sleep_for(250ms);
    // Step 9.
    i2c.start();
    i2c.write(d6t_addr, i2c_buffer + 1, 1, true);
    // Step 10.
    i2c.read(d6t_addr, sens_dat, 19);   // This works.
    //i2c.read(d6t_addr, ste_out_ptr->data, 19);  // Not working.
    i2c.stop();
    
    //printf("i2c_buffer: %s ", (i2c_buffer+0));      // okay
    printf("sens_dat(0): %s, (1)%s ", (sens_dat + 0), (sens_dat + 1));      // okay
    //printf("write_rtrn: %d ", write_rtrn);      // okay
    ThisThread::sleep_for(2ms);
    
    //ste_out_ptr->data = sens_dat; // Does not work.
    //printf("Data inside function = %s ", ste_out_ptr->data+1);    // okay
        
    return ste_out_ptr; // Return pointer to sensor outputs.
}

char *i2c_d6t_8L_ptr(void)   // Function returns pointer to char.
{
	int d6t_addr = 0x14;                // Omron D6T address.
	char i2c_buffer[10]= 	{
								0x14,	// Write address.       0	
								0x4C,	// Meas. command	    1
								0x02,	// 	                    2
								0x05,	// 	                    3 		0
								0x03,	//  	                4		1
								0x15,	// Send addr. read		5		2
								0x00,   // 	                    6		3
								0x00,	// 	                    7		4
								0x00,	// 		                8		5
								0x00 	// 		                9		6
							};	
    char bufr_1[4]= 	{
								0x02,	// 0	
								0x00,	// 1
								0x01,	// 2
								0xee	// 3----
						};	
    char bufr_2[4]= 	{
								0x05,	// 4
								0x90,	// 5
								0x3a,   // 6
								0xb8    // 7----		
						};	
    char bufr_3[4]= 	{
								0x03,	// 8
								0x00,   // 9
                                0x03,   // 10
                                0x8b    // 11----
                        };	
    char bufr_4[4]= 	{
								0x03,   // 12
                                0x00,   // 13
                                0x07,   // 14
                                0x97    // 15----
                        };	
    char bufr_5[4]= 	{
								0x02,   // 16
                                0x00,   // 17
                                0x00,   // 18
                                0xe9 	// 19----
						};	
    char sens_dat[19]= 	    {
                                0x00,   // 0	
	                            0x00,   // 1
                                0x00,	// 2
		                        0x00,	// 3
	                            0x00,	// 4
	                            0x00,	// 5
                                0x00,   // 6	
	                            0x00,   // 7
                                0x00,	// 8
		                        0x00,	// 9
	                            0x00,	// 10
	                            0x00,	// 11
                                0x00,   // 12	
	                            0x00,   // 13
                                0x00,	// 14
		                        0x00,	// 15
	                            0x00,	// 16
	                            0x00,	// 17
                                0x00    // 18                            
                            };
	
    // Create an I2C object.
    // Using arduino pins for sda and scl.
    I2C i2c(ARDUINO_UNO_D14, ARDUINO_UNO_D15);  // i2c1_sda, i2c1_scl
    
    /* To get the data from sensor do next several steps. */
    // Step 1.
    i2c.start();
    i2c.write(d6t_addr, bufr_1 + 0, 4);
    i2c.stop();
    // Step 2.
    i2c.start();
    i2c.write(d6t_addr, bufr_2 + 0, 4);
    i2c.stop();
    // Step 3.
    i2c.start();
    i2c.write(d6t_addr, bufr_3 + 0, 4);
    i2c.stop();
    // Step 4.
    i2c.start();
    i2c.write(d6t_addr, bufr_4 + 0, 4);
    i2c.stop();
    // Step 5.
    i2c.start();
    i2c.write(d6t_addr, bufr_5 + 0, 4);
    i2c.stop();
    ThisThread::sleep_for(250ms);
    // Step 6.
    i2c.start();
    i2c.write(d6t_addr, i2c_buffer + 1, 1, true);
    // Step 7.
    i2c.read(d6t_addr, sens_dat, 19);   // This works.
    //i2c.read(d6t_addr, ste_out_ptr->data, 19);  // Not working.
    i2c.stop();
    
    ThisThread::sleep_for(2ms);

    // This works but you will get a warning.
    return sens_dat; // Return pointer to sensor outputs.
}

// Serial function to get data from gps.
void ser_com (void)
{
    static BufferedSerial get_gps(PD_8, PD_9);  // working
    char *gps_dat = new char[1];

    if (get_gps.readable()) {
        get_gps.read(gps_dat, sizeof(gps_dat));
    }

    printf("Gps char = %c", *(gps_dat+0));
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
    // Pointer to get d6t_8L sensor data.
    char *d6t_8L_dat;

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
        ThisThread::sleep_for(TWO_SEC);
        printf("Wait every 2 seconds... ");
        
        // This works.
        //sen_dat = i2c_fsm_shtc3();
        //printf("Sht3c data: = %s %s %s %s %s %s ", sen_dat.data + 0, sen_dat.data + 1, sen_dat.data + 2, 
        //                                            sen_dat.data + 3, sen_dat.data + 4, sen_dat.data + 5);
        
        //printf("First data element = %s ", sen_dat.data + 0);
        // For Shtc3 sensor.
        //sen_dat_ptr = i2c_fsm_shtc3_ptr();
        
        // For D6t sensor.
        //sen_dat_ptr = i2c_d6t_ptr();
        
        // For D6t-8L sensor.
        //sen_dat_ptr = i2c_d6t_8l_ptr();
        //i2c_d6t_8l_ptr();
        d6t_8L_dat = i2c_d6t_8L_ptr();
        printf("8L_data = %c,%c %c,%c %c,%c %c,%c %c,%c %c,%c %c,%c %c,%c %c,%c",  
                                                            *(d6t_8L_dat + 0), *(d6t_8L_dat + 1),
                                                            *(d6t_8L_dat + 2), *(d6t_8L_dat + 3),
                                                            *(d6t_8L_dat + 4), *(d6t_8L_dat + 5),
                                                            *(d6t_8L_dat + 6), *(d6t_8L_dat + 7),
                                                            *(d6t_8L_dat + 8), *(d6t_8L_dat + 9),
                                                            *(d6t_8L_dat + 10), *(d6t_8L_dat + 11),
                                                            *(d6t_8L_dat + 12), *(d6t_8L_dat + 13),
                                                            *(d6t_8L_dat + 14), *(d6t_8L_dat + 15),
                                                            *(d6t_8L_dat + 16), *(d6t_8L_dat + 17)
                                                            );
        //printf("First data element = %s ", (sen_dat_ptr->data)+1);    // okay
        //printf("First data element = %s ", sen_dat_ptr->data+1);      // okay
        //printf("Third char = %s ", (sen_dat_ptr->data)+1);              // okay
        
        ThisThread::sleep_for(10ms);
        
        ser_com();

    }
}
