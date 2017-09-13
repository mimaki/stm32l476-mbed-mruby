#include "mbed.h"
#include "mruby.h"
#include <stdio.h>

DigitalOut led1(LED1);

// Serial vcp(SERIAL_TX, SERIAL_RX);

// main() runs in its own thread in the OS
int main() {
    int i;

printf("Hello, STM32L476RG!\n");

    for (i=0; i<4; i++) {
        led1 = !led1;
        wait(1);
    }

printf("mrb_open() ... ");
    mrb_state *mrb = mrb_open();
printf("done.\n");

    for (i=0; i<10; i++) {
        led1 = !led1;
        wait(0.5);
    }

printf("mrb_close() ... ");
    mrb_close(mrb);
printf("done.\n");

    while (true) {
        led1 = !led1;
        wait(0.1);
    }
}
