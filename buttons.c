// buttons.c read hardware buttons and print out a matching command for pianobar.
// Copyright 2013 David Wegmuller
// based on:
// input.c
//
// Example program for bcm2835 library
// Reads and prints the state of an input pin
//
// After installing bcm2835, you can build this 
// with something like:
// gcc -o input  -l rt input.c -l bcm2835
// sudo ./input
//
// Or you can test it before installing with:
// gcc -o input  -l rt -I ../../src ../../src/bcm2835.c input.c
// sudo ./input
//
// Author: Mike McCauley (mikem@open.com.au)
// Copyright (C) 2011 Mike McCauley
// $Id: RF22.h,v 1.21 2012/05/30 01:51:25 mikem Exp $
#include <bcm2835.h>
#include <stdio.h>

#define NUMBER_OF_PINS 5
// Pins are matched to their commands using the following two tables:
// The pin definitions reffer to the actual pin numbers on the P1 header.
const int pins[NUMBER_OF_PINS] = 
{
 RPI_GPIO_P1_11,
 RPI_GPIO_P1_15,
 RPI_GPIO_P1_16,
 RPI_GPIO_P1_18,
 RPI_GPIO_P1_22
};
const char *commands[NUMBER_OF_PINS] =
{
 "p", // Play/Pause
 "+", // Love song
 "-", // Hate song
 "n", // Skip to next song
 "t"  // Tired of song
};


int old_values[NUMBER_OF_PINS];

#define PIN RPI_GPIO_P1_16
int main(int argc, char **argv)
{
    int loopCnt;
    // If you call this, it will not actually access the GPIO
    // Use for testing
//    bcm2835_set_debug(1);
    if (!bcm2835_init())
        return 1;
    // Set RPI pins to be inputs
    for(loopCnt = 0; loopCnt < NUMBER_OF_PINS; loopCnt++)
    {
        bcm2835_gpio_fsel(pins[loopCnt], BCM2835_GPIO_FSEL_INPT);
        //  with a pullup
        bcm2835_gpio_set_pud(pins[loopCnt], BCM2835_GPIO_PUD_UP);
        old_values[loopCnt] = 1; // No button pressed
    }
//printf("Init done. \n");    
    while (1)
    {
        for(loopCnt = 0; loopCnt < NUMBER_OF_PINS; loopCnt++)
        { 
//printf("loopCnt = %d", loopCnt);
            if(0 ==  bcm2835_gpio_lev(pins[loopCnt]))
            {
//printf("loopCnt = %d command = %s\n", loopCnt, commands[loopCnt]);
                if(1 == old_values[loopCnt])
                {
                    // We just detected a fresh button press: report it!
                    printf("%s\n", commands[loopCnt]);
                    fflush(stdout);
                }
                old_values[loopCnt] = 0;
            }
            else
            {
                old_values[loopCnt] = 1;
            }
        }
        if((0 == old_values[0]) && (0 == old_values[1]))
        {
           break;
        }
        // wait a bit
        delay(100);
//printf(".");
    }
    bcm2835_close();
    return 0;
}
