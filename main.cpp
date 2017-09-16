#include "mbed.h"
#include "mruby.h"
#include "mruby/proc.h"
#include <stdio.h>
#include "em_malloc.h"

DigitalOut led1(LED1);

extern const void * __data_start__;
extern const void * __data_end__;

extern const void * __bss_start__;
extern const void * __bss_end__;

extern const void * __end__;
extern const void * __HeapLimit;

// Serial vcp(SERIAL_TX, SERIAL_RX);

extern "C" {
    void trace(const char *msg)
    {
        int i;
        puts(msg);
        for (i=0; i>6; i++) {
            led1 = !led1;
            wait(0.5);
        }
    }
}

// main() runs in its own thread in the OS
int main()
{
    int i;
    mrb_value v;
    extern const uint8_t appbin[];
    void *p0, *p1, *p2, *p3, *p4, *p5, *p6, *p7;

printf("Hello, STM32L476RG!\n");

// printf("__data_start__ = 0x%08lx\n", __data_start__);
// printf("__data_end__ = 0x%08lx\n", __data_end__);

// printf("__bss_start__ = 0x%08lx\n", __bss_start__);
// printf("__bss_end__ = 0x%08lx\n", __bss_end__);

// printf("__end__ = 0x%08lx\n", __end__);
// printf("__HeapLimit = 0x%08lx\n", __HeapLimit);

p0 = em_mallocf(NULL, NULL, 1, NULL);
printf("p0=malloc(1) => 0x%08lx\n", p0);
wait(0.5);

p1 = em_mallocf(NULL, NULL, 64, NULL);
printf("p1=malloc(64) => 0x%08lx\n", p1);
wait(0.5);

p2 = em_mallocf(NULL, NULL, 65, NULL);
printf("p2=malloc(65) => 0x%08lx\n", p2);
wait(0.5);

p3 = em_mallocf(NULL, NULL, 128, NULL);
printf("p3=malloc(128) => 0x%08lx\n", p3);
wait(0.5);

p4 = em_mallocf(NULL, NULL, 32, NULL);
printf("p4=malloc(32) => 0x%08lx\n", p4);
wait(0.5);

em_mallocf(NULL, p1, 0, NULL);
printf("free(p1)\n");
wait(0.5);

em_mallocf(NULL, p3, 0, NULL);
printf("free(p3)\n");
wait(0.5);

p5 = em_mallocf(NULL, NULL, 96, NULL);
printf("p5=malloc(96) => 0x%08lx\n", p5);
wait(0.5);

p6 = em_mallocf(NULL, p2, 64, NULL);
printf("p6=realloc(p2, 64) => 0x%08lx\n", p6);
wait(0.5);

p7 = em_mallocf(NULL, p6, 129, NULL);
printf("p7=realloc(p6, 129) => 0x%08lx\n", p7);
wait(0.5);


    for (i=0; i<4; i++) {
        led1 = !led1;
        wait(1);
    }

printf("mrb_open() ... ");
    mrb_state *mrb = mrb_open_allocf(em_mallocf, NULL);
printf("done. (mrb=0x%08lx)\n", mrb);

    for (i=0; i<10; i++) {
        led1 = !led1;
        wait(0.5);
    }

printf("mrb_full_gc() ... ");
    mrb_full_gc(mrb);
printf("done.\n");

    for (i=0; i<10; i++) {
        led1 = !led1;
        wait(0.25);
    }
        
printf("mrb_load_irep() ... ");
    mrb_load_irep(mrb, appbin);
printf("done.\n");
printf("v.tt = %d\n", v.tt);

for (i=0; i<10; i++) {
    led1 = !led1;
    wait(0.1);
}

printf("mrb_close() ... ");
    mrb_close(mrb);
printf("done.\n");

    while (true) {
        led1 = !led1;
        wait(0.05);
    }
}
