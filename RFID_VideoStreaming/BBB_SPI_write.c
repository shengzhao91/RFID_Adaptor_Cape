/*
 * SPI testing utility (using spidev driver)
 *
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include "SimpleGPIO.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static void pabort(const char *s)
{
	perror(s);
	abort();
}

static const char *device = "/dev/spidev1.0";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 3000000;
static uint16_t delay;
unsigned int timeout = 1000*1000;

static void transfer(int fd, uint8_t *tx, uint8_t *rx, uint8_t size, uint8_t printflag)
{
	int ret;
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = size,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message");

	if (printflag) {
		for (ret = 0; ret < size; ret++) {
			if (!(ret % 6))
				puts("");
			printf("%.2X ", rx[ret]);
		}
		puts("");
	}
}

static void print_usage(const char *prog)
{
	printf("Usage: %s [-DsbdlHOLC3]\n", prog);
	puts("  -D --device   device to use (default /dev/spidev1.1)\n"
	     "  -s --speed    max speed (Hz)\n"
	     "  -d --delay    delay (usec)\n"
	     "  -b --bpw      bits per word \n"
	     "  -l --loop     loopback\n"
	     "  -H --cpha     clock phase\n"
	     "  -O --cpol     clock polarity\n"
	     "  -L --lsb      least significant bit first\n"
	     "  -C --cs-high  chip select active high\n"
	     "  -3 --3wire    SI/SO signals shared\n");
	exit(1);
}

static void parse_opts(int argc, char *argv[])
{
	while (1) {
		static const struct option lopts[] = {
			{ "device",  1, 0, 'D' },
			{ "speed",   1, 0, 's' },
			{ "delay",   1, 0, 'd' },
			{ "bpw",     1, 0, 'b' },
			{ "loop",    0, 0, 'l' },
			{ "cpha",    0, 0, 'H' },
			{ "cpol",    0, 0, 'O' },
			{ "lsb",     0, 0, 'L' },
			{ "cs-high", 0, 0, 'C' },
			{ "3wire",   0, 0, '3' },
			{ "no-cs",   0, 0, 'N' },
			{ "ready",   0, 0, 'R' },
			{ NULL, 0, 0, 0 },
		};
		int c;

		c = getopt_long(argc, argv, "D:s:d:b:lHOLC3NR", lopts, NULL);

		if (c == -1)
			break;

		switch (c) {
		case 'D':
			device = optarg;
			break;
		case 's':
			speed = atoi(optarg);
			break;
		case 'd':
			delay = atoi(optarg);
			break;
		case 'b':
			bits = atoi(optarg);
			break;
		case 'l':
			mode |= SPI_LOOP;
			break;
		case 'H':
			mode |= SPI_CPHA;
			break;
		case 'O':
			mode |= SPI_CPOL;
			break;
		case 'L':
			mode |= SPI_LSB_FIRST;
			break;
		case 'C':
			mode |= SPI_CS_HIGH;
			break;
		case '3':
			mode |= SPI_3WIRE;
			break;
		case 'N':
			mode |= SPI_NO_CS;
			break;
		case 'R':
			mode |= SPI_READY;
			break;
		default:
			print_usage(argv[0]);
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	int ret = 0;
	int fd;

	parse_opts(argc, argv);

	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	/*
	 * spi mode
	 */
	mode |= SPI_CPHA;
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	//printf("spi mode: %d\n", mode);
	//printf("bits per word: %d\n", bits);
	//printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

	// SPI_ISO15693_Single_Slot
	unsigned int EN_GPIO = 26;   // GPIO0_26 = (0x32) + 26 = 26
	gpio_export(EN_GPIO);
    gpio_set_dir(EN_GPIO, OUTPUT_PIN);
	gpio_set_value(EN_GPIO, HIGH);
	
	unsigned int irq_status = 0;
	unsigned int IRQ_GPIO = 45;   // GPIO1_13 = (32x1) + 13 = 45
	gpio_export(IRQ_GPIO);
    gpio_set_dir(IRQ_GPIO, INPUT_PIN);
	gpio_set_edge(IRQ_GPIO, "rising");
	/*
	 * 5438_TRF7960_SPI_ISO15693_Single_Slot 
	 */
	while(1)
	{
		timeout = 1000;
		
		uint8_t tx000[] = {0x83}; // Software Initialization
		uint8_t rx000[ARRAY_SIZE(tx000)] = {0, };
		transfer(fd, tx000, rx000, ARRAY_SIZE(tx000),0);
		
		uint8_t tx001[] = {0x80}; // Idle
		uint8_t rx001[ARRAY_SIZE(tx001)] = {0, };
		transfer(fd, tx001, rx001, ARRAY_SIZE(tx001),0);
		 
		uint8_t tx0[] = {0x20,0x21,0x02,0x00,0x00,0xC1,0xBB}; // Cont write 0x21 to Chip Status Control (0x00), 
		uint8_t rx0[ARRAY_SIZE(tx0)] = {0, }; // 0x02  to ISO Control (0x01)
		transfer(fd, tx0, rx0, ARRAY_SIZE(tx0),0);
		
		usleep(1000); // Sleep 1ms
		
		uint8_t tx[] = {0x09, 0x21}; //Write to 0x09 (Modulator and SYS_CLK control) 0x21. 
		uint8_t rx[ARRAY_SIZE(tx)] = {0, }; //Set SYSCLK to 6.78MHz
		transfer(fd, tx, rx, ARRAY_SIZE(tx),0);
		
		uint8_t tx2[] = {0x07, 0x13}; //Write to 0x07 (RX No Response Wait Time Register) value 0x13
		uint8_t rx2[ARRAY_SIZE(tx2)] = {0, };
		transfer(fd, tx2, rx2, ARRAY_SIZE(tx2),0);
		/*
		uint8_t tx01[] = {0x60,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; // Read Chip Status Control and ISO Control
		uint8_t rx01[ARRAY_SIZE(tx01)] = {0, };
		transfer(fd, tx01, rx01, ARRAY_SIZE(tx01),0);
		*/
		uint8_t tx3[] = {0x6C, 0x00, 0x00}; // Cont read from 0x0C (IRQ Status)
		uint8_t rx3[ARRAY_SIZE(tx3)] = {0, };
		transfer(fd, tx3, rx3, ARRAY_SIZE(tx3),0);
		
		uint8_t tx4[] = {0x8F,0x91,0x3D,0x00,0x30,0x26,0x01,0x00}; //Reset, Transmit w/ CRC
		uint8_t rx4[ARRAY_SIZE(tx4)] = {0, }; // Cont write from 0x1D, TX Length 3 bytes. Data: 0x26,0x01,0x00
		transfer(fd, tx4, rx4, ARRAY_SIZE(tx4),0); //Reset to Ready, Inventory, Idle
		
		while (irq_status != 1)
		{
			gpio_get_value(IRQ_GPIO, &irq_status);
		}
		//printf("irq_status: %d\n", irq_status);

		uint8_t tx5[] = {0x6C, 0x00,0x00}; // Cont read from 0x0C (IRQ Status)
		uint8_t rx5[ARRAY_SIZE(tx5)] = {0, };
		transfer(fd, tx5, rx5, ARRAY_SIZE(tx5),0);

		if (rx5[1] == 0x80)
		{
			uint8_t tx6[] = {0x8F};
			uint8_t rx6[ARRAY_SIZE(tx6)] = {0, };
			transfer(fd, tx6, rx6, ARRAY_SIZE(tx6),0);
		
			irq_status = 0;
			
			while ((irq_status != 1) && (timeout))
			{
				gpio_get_value(IRQ_GPIO, &irq_status);
				timeout--;
			}
			
			if (timeout == 0)
			{
				printf("timed out\n");
			}
			else {
				uint8_t tx7[] = {0x6C, 0x00,0x00};  // Cont read from 0x0C (IRQ Status)
				uint8_t rx7[ARRAY_SIZE(tx7)] = {0, };
				transfer(fd, tx7, rx7, ARRAY_SIZE(tx7),0);
				if (rx7[1] != 0x80)
				{
					printf("irq error: 0x%X\n",rx7[1]);
				}
				
				uint8_t tx8[] = {0x5C,0x00,}; //Read 0x1C (FIFO Status)
				uint8_t rx8[ARRAY_SIZE(tx8)] = {0, };
				transfer(fd, tx8, rx8, ARRAY_SIZE(tx8),0);
				//printf("bytes to read: %d\n", rx8[1]);
				
				if (irq_status) {
					uint8_t tx9[] = {0x7F,0x00,0x00,0x00,0x00, 
									 0x00,0x00,0x00,0x00,0x00,0x00}; //Read FIFO register
					uint8_t rx9[ARRAY_SIZE(tx9)] = {0, };
					transfer(fd, tx9, rx9, ARRAY_SIZE(tx9),1);
				}
				
				uint8_t tx15[] = {0x8F};
				uint8_t rx15[ARRAY_SIZE(tx15)] = {0, };
				transfer(fd, tx15, rx15, ARRAY_SIZE(tx15),0);
				
				uint8_t tx16[] = {0x4F, 0x00}; //Read RSSI Level
				uint8_t rx16[ARRAY_SIZE(tx16)] = {0, };
				transfer(fd, tx16, rx16, ARRAY_SIZE(tx16),0);
				printf("rssi: %d\n", rx16[1]);
				
				uint8_t tx17[] = {0x8F}; // Reset FIFO
				uint8_t rx17[ARRAY_SIZE(tx17)] = {0, };
				transfer(fd, tx17, rx17, ARRAY_SIZE(tx17),0);
				
				uint8_t tx18[] = {0x96}; // Block Receiver
				uint8_t rx18[ARRAY_SIZE(tx18)] = {0, };
				transfer(fd, tx18, rx18, ARRAY_SIZE(tx18),0);
				
				uint8_t tx19[] = {0x4C, 0x00}; // Read IRQ status
				uint8_t rx19[ARRAY_SIZE(tx19)] = {0, };
				transfer(fd, tx19, rx19, ARRAY_SIZE(tx19),0);
				
				uint8_t tx20[] = {0x00,0x01}; // Turn off transmitter
				uint8_t rx20[ARRAY_SIZE(tx20)] = {0, }; // 0x02  to ISO Control (0x01)
				transfer(fd, tx20, rx20, ARRAY_SIZE(tx20),0);
			}
		}
		else
		{
			printf(" ERROR: 0x%X. \n", rx5[1]);
		}
		
		//sleep(1);
		usleep(100000);
	}
	//

	
	/*
	uint8_t tx10[] = {0x6C, 0x00,0x00};  // Cont read from 0x0C (IRQ Status)
	uint8_t rx10[ARRAY_SIZE(tx10)] = {0, };
	transfer(fd, tx10, rx10, ARRAY_SIZE(tx10));
	
	irq_status = 0;
	while (irq_status != 1)
	{
		gpio_get_value(IRQ_GPIO, &irq_status);
	}
	printf("irq_status: %d\n", irq_status);
	uint8_t tx11[] = {0x6C, 0x00,0x00};  // Cont read from 0x0C (IRQ Status)
	uint8_t rx11[ARRAY_SIZE(tx11)] = {0, };
	transfer(fd, tx11, rx11, ARRAY_SIZE(tx11));
	
	uint8_t tx15[] = {0x8F};
	uint8_t rx15[ARRAY_SIZE(tx15)] = {0, };
	transfer(fd, tx15, rx15, ARRAY_SIZE(tx15));
	
	uint8_t tx16[] = {0x4F, 0x00}; //Read RSSI Level
	uint8_t rx16[ARRAY_SIZE(tx16)] = {0, };
	transfer(fd, tx16, rx16, ARRAY_SIZE(tx16));
	printf("rssi: %d\n", rx16[1]);
	
	
	// Last block
	uint8_t tx17[] = {0x8F}; // Reset FIFO
	uint8_t rx17[ARRAY_SIZE(tx17)] = {0, };
	transfer(fd, tx17, rx17, ARRAY_SIZE(tx17));
	
	uint8_t tx18[] = {0x96}; // Block Receiver
	uint8_t rx18[ARRAY_SIZE(tx18)] = {0, };
	transfer(fd, tx18, rx18, ARRAY_SIZE(tx18));
	
	uint8_t tx19[] = {0x4C, 0x00}; // Read IRQ status
	uint8_t rx19[ARRAY_SIZE(tx19)] = {0, };
	transfer(fd, tx19, rx19, ARRAY_SIZE(tx19));
	*/
	
	/*
	// IRQ dummy read
	uint8_t irq_tx[] = {0x6C, 0x00, 0x00};
	uint8_t irq_rx[ARRAY_SIZE(irq_tx)] = {0, };
	transfer(fd, irq_tx, irq_rx, ARRAY_SIZE(irq_tx));
	
	// RSSI read
	uint8_t rssi_tx[] = {0x4F, 0x00};
	uint8_t rssi_rx[ARRAY_SIZE(rssi_tx)] = {0, };
	transfer(fd, rssi_tx, rssi_rx, ARRAY_SIZE(rssi_tx));
	
	// Set SYS_CLK to 13.5 MHz
	uint8_t sysclk_tx[] = {0x09, 0xB1};
	uint8_t sysclk_rx[ARRAY_SIZE(sysclk_tx)] = {0, };
	transfer(fd, sysclk_tx, sysclk_rx, ARRAY_SIZE(sysclk_tx));
	
	// FIFO Status
	uint8_t fifo_tx[] = {0x5C, 0x00};
	uint8_t fifo_rx[ARRAY_SIZE(fifo_tx)] = {0, };
	transfer(fd, fifo_tx, fifo_rx, ARRAY_SIZE(fifo_tx));
	
	*/
	
	printf("Complete\n");
	
	
	close(fd);

	return ret;
}
