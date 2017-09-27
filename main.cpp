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

  static mrb_value mrb_puts(mrb_state *mrb, mrb_value self)
  {
    char *s;
// printf("Object#puts() ... \n");
    mrb_get_args(mrb, "z", &s);
    puts(s);
  }
}

// main() runs in its own thread in the OS
int main()
{
  int i;
  mrb_value v;
  extern const uint8_t appbin[];

printf("Hello, STM32L476RG!\n");
#if 0
{
  static void *memp[_MAX_BLKS];
  int i;
printf("Allocate %d blocks\n", _MAX_BLKS);
wait(5);
  for (i=0; i<_MAX_BLKS; i++) {
    memp[i] = em_mallocf(NULL, NULL, _BLK_SIZE, NULL);
  }
  em_show_status();
wait(5);
  // Out of memory
printf("Check out of memory\n");
wait(3);
  em_mallocf(NULL, NULL, 1, NULL);
  em_show_status();
wait(5);
  // Free blocks
printf("Free %d blocks\n", _MAX_BLKS*3/4);
wait(1);
  for (i=0; i<_MAX_BLKS; i+=4) {
    em_mallocf(NULL, memp[i+1], 0, NULL);
    em_mallocf(NULL, memp[i+2], 0, NULL);
    em_mallocf(NULL, memp[i+3], 0, NULL);
  }
  em_show_status();
wait(5);
  // Expand blocks
printf("Expand blocks\n");
wait(3);
  for (i=0; i<_MAX_BLKS; i+=4) {
    em_mallocf(NULL, memp[i], _BLK_SIZE*4, NULL);
  }
  em_show_status();
wait(5);
  // Free all blocks
printf("Free all blocks\n");
wait(3);
  for (i=0; i<_MAX_BLKS; i+=4) {
    em_mallocf(NULL, memp[i], 0, NULL);
  }
  em_show_status();
wait(10);
}
#endif

#if 0
{
  void *p0, *p1, *p2, *p3, *p4, *p5, *p6, *p7;
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

  em_dump();

  em_mallocf(NULL, p3, 0, NULL);
  printf("free(p3)\n");
  wait(0.5);

  em_dump();

  p5 = em_mallocf(NULL, NULL, 96, NULL);
  printf("p5=malloc(96) => 0x%08lx\n", p5);
  wait(0.5);

  em_dump();

  p6 = em_mallocf(NULL, p2, 64, NULL);
  printf("p6=realloc(p2, 64) => 0x%08lx\n", p6);
  wait(0.5);

  em_dump();

  p7 = em_mallocf(NULL, p6, 129, NULL);
  printf("p7=realloc(p6, 129) => 0x%08lx\n", p7);
  wait(0.5);

  em_dump();
}
#endif

  for (i=0; i<4; i++) {
    led1 = !led1;
    wait(1);
  }

printf("mrb_open() ... ");
    mrb_state *mrb = mrb_open_allocf(em_mallocf, NULL);
printf("done. (mrb=0x%08lx)\n", mrb);
em_show_status();

  for (i=0; i<20; i++) {
    led1 = !led1;
    wait(0.5);
  }

printf("mrb_full_gc() ... ");
  mrb_full_gc(mrb);
printf("done.\n");
em_show_status();

printf("mrb_define_method Object#puts ... ");
mrb_define_method(mrb, mrb->object_class, "puts", mrb_puts, MRB_ARGS_REQ(1));
printf("done.\n");
em_show_status();

  for (i=0; i<40; i++) {
    led1 = !led1;
    wait(0.25);
  }

printf("mrb_load_irep() ... ");
  v = mrb_load_irep(mrb, appbin);
printf("done.\n");
printf("v.tt = %d\n", v.tt);
printf("v.i = %d\n", v.value.i);
em_show_status();

  for (i=0; i<100; i++) {
    led1 = !led1;
    wait(0.1);
  }

printf("mrb_close() ... ");
    mrb_close(mrb);
printf("done.\n");
em_show_status();

  while (true) {
    led1 = !led1;
    wait(0.05);
  }
}
