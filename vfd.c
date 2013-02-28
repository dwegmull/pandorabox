/* VFD control program. (c) 2013 David Wegmuller
 *
 * This is part of Pandora Pi-box: a minimalist hardware Pandora client.
 *
 * Reads text from a file and displays it on the upper line
 * of the VFD. If the resulting text is longer than 20 characters, 
 * it is scrolled.
 * The bottom line displays static text that describes the button functions.
 *
 * Based on:
 */
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
/*
 *  VFD specific code is based on: https://github.com/adafruit/SPI_VFD
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
#include "vfd.h"
#include <time.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))


static void pabort(const char *s)
{
  perror(s);
  abort();
}

static const char *device = "/dev/spidev0.0";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 500000;
static uint16_t delay;

void transfer(int fd, char *data, int size)
{
  int ret;
  char *rx, *tx;
  rx = (char*)malloc(size + 1);
  if(NULL == rx)
  {
    return;
  }
  tx = (char*)malloc(size + 1);
  if(NULL == tx)
  {
    free(rx);
    return;
  }

  int cnt = 0;
  for(cnt= 0; cnt < size; cnt++)
  {
    tx[cnt + 1] = data[cnt];
  }
  tx[0] = VFD_SPIDATA;
  struct spi_ioc_transfer tr = 
  {
    .tx_buf = (unsigned long)tx,
    .rx_buf = (unsigned long)rx,
    .len = size + 1,
    .delay_usecs = delay,
    .speed_hz = speed,
    .bits_per_word = bits,
  };

  ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
  if (ret < 1)
  {
    free(rx);
    pabort("can't send spi data");
  }

  free(rx);
}

void send_command(int fd, uint8_t value) 
{
  uint8_t tx[2];
  uint8_t rx[2];
   
  struct spi_ioc_transfer tr = 
  {
    .tx_buf = (unsigned long)tx,
    .rx_buf = (unsigned long)rx,
    .len = ARRAY_SIZE(tx),
    .delay_usecs = delay,
    .speed_hz = speed,
    .bits_per_word = bits,
  };
  
  tx[0] = VFD_SPICOMMAND;
  tx[1] = value; 
  if(ioctl(fd, SPI_IOC_MESSAGE(1), &tr) < 1)
  {
    pabort("can't send spi command");
  }
}

void update_time(int fd)
{
  static int oldSeconds = 0;
  static int oldMinutes = 0;
  static int oldHours = 0;
  int update = 0;
  char timeString[20];
  time_t t = time(NULL);
  struct tm lt = *localtime(&t);
  if(lt.tm_sec != oldSeconds)
  {
    update = 1;
    oldSeconds = lt.tm_sec;
  }
  if(lt.tm_min != oldMinutes)
  {
    update = 1;
    oldMinutes = lt.tm_min;
  }
  if(lt.tm_hour != oldHours)
  {
    update = 1;
    oldHours = lt.tm_hour;
  }
  if(1 == update)
  {
    int size;
    size = sprintf(&timeString, "%d : %d : %d", lt.tm_hour, lt.tm_min, lt.tm_sec);
    int cnt = 19;
    int shift = (20 - size) >> 1;
    while(cnt)
    {
      if(cnt >= (size + shift))
      {
        timeString[cnt] = ' ';
      }
      else
      {
        timeString[cnt] = timeString[cnt - shift];
        if(cnt < shift)
        {
          timeString[cnt] = ' ';
        }
      }
      cnt--;
    }
    timeString[0] = ' ';
    send_command(fd, VFD_SETDDRAMADDR | 0x40);
    transfer(fd, timeString, 20);    
  }
}

void init_VFD(int fd)
{
  // Our display has two lines + set brightness to 100%
  send_command(fd, VFD_FUNCTIONSET | VFD_2LINE | VFD_BRIGHTNESS100);
  // Left to right display
  send_command(fd, VFD_ENTRYMODESET | VFD_ENTRYLEFT | VFD_ENTRYSHIFTDECREMENT);
  send_command(fd, VFD_SETDDRAMADDR);
  // Turn on the display
  send_command(fd, VFD_DISPLAYCONTROL | VFD_DISPLAYON);
  // Clear the display: this function takes a while to execute
  send_command(fd, VFD_CLEARDISPLAY);
//  sleep(2);
  // Set the cursor to the home position: this function takes a while to execute
  send_command(fd, VFD_RETURNHOME);
//  sleep(2);
  // Write the key functions to the lower line of the display
  send_command(fd, VFD_SETDDRAMADDR | 0x40);
  transfer(fd, "                    ", 20);
}

void  scroll(int fd, char* displayBuff, int size)
{
  if(size < 1)
  {
    return;
  }
  // Set the cursor to the start of the screen (adress 0)
  send_command(fd, VFD_SETDDRAMADDR);
  if(size < 21)
  {
    // the message fits on the screen: no need to scroll it
    transfer(fd, displayBuff, size);
    update_time(fd);
  }
  else
  {
    int cnt;
    // message is longer than the screen: let's scroll it!
    // First, pad the existing buffer with a screenful of spaces.
    displayBuff = realloc(displayBuff, size + 20);
    // Fill the addition with spaces
    for(cnt = size - 1; cnt < size + 20; cnt++)
    {
      displayBuff[cnt] = ' ';
    }
    // scroll by walking through the string
    for(cnt = 0; cnt < size; cnt++)
    {
      send_command(fd, VFD_SETDDRAMADDR);
      transfer(fd, &displayBuff[cnt], 20);
      if(0 == cnt)
      {
        usleep(1000000);
        update_time(fd);
      }
      else
      {
        usleep(200000);
        update_time(fd);
      }
    }
  }
}

int read_file(int fd)
{
  FILE* rfp;
  rfp = fopen("/root/.config/pianobar/nowplaying", "r");
  if(rfp)
  {
    // Read the line (the file only has one) and send it to be scrolled 
    char *line0;
    int initSize = 10;
    line0 = (char*)malloc(initSize);
    if(NULL == line0)
    {
       fclose(rfp);
       return -2;
    }
    int line0Size = getline(&line0,&initSize, rfp);
    fclose(rfp);
    if(1 > line0Size)
    {
      free(line0);
      return -3;
    }

    scroll(fd, line0, line0Size);

    free(line0);
  }
  else
  {
    return 1;
  }
  return 0;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	int fd;

	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	/*
	 * spi mode
	 */
	mode |= SPI_CPHA;
	mode |= SPI_CPOL;
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

	init_VFD(fd);
        // Main loop (don't blink or you'll miss it!)
        while(-2 != read_file(fd));
	return -2;
}
