#include "mbed.h"
#include "mruby.h"
#include "mruby/proc.h"
#include "mruby/class.h"
#include "mruby/variable.h"
#include <stdio.h>
#include "em_malloc.h"

#include "mbedapi.h"

DigitalOut led1(LED1);

extern const void * __data_start__;
extern const void * __data_end__;

extern const void * __bss_start__;
extern const void * __bss_end__;

extern const void * __end__;
extern const void * __HeapLimit;

extern "C" {
  void trace(const char *msg)
  {
    int i;
    puts(msg);
    for (i=0; i<6; i++) {
      led1 = !led1;
      wait(0.5);
    }
  }

  /* puts(s) => Object */
  static mrb_value mrb_puts(mrb_state *mrb, mrb_value self)
  {
    char *s;
    mrb_get_args(mrb, "z", &s);
    puts(s);
    return self;
  }

  /* DigitalIO.new(pin=LED1) => DigitalIO */
  static mrb_value mrb_dio_init(mrb_state *mrb, mrb_value self)
  {
    mrb_int pin = MBED_LED1;
    mrb_get_args(mrb, "|i", &pin);
    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@pin"), mrb_fixnum_value(pin));
    return self;
  }

  /* DigitalIO#write(v) => nil */
  static mrb_value mrb_dio_write(mrb_state *mrb, mrb_value self)
  {
    mrb_int v;
    mrb_value vpin;
    mrb_get_args(mrb, "i", &v);
    vpin = mrb_iv_get(mrb, self, mrb_intern_lit(mrb, "@pin"));
    mbedDigitalWrite(mrb_fixnum(vpin), v);
    return mrb_nil_value();
  }

  /* delay(ms) => nil */
  static mrb_value mrb_delay(mrb_state *mrb, mrb_value self)
  {
    mrb_int t;
    mrb_get_args(mrb, "i", &t);
    mbedDelay(t);
    return mrb_nil_value();
  }
}

// main() runs in its own thread in the OS
int main()
{
  int i;
  mrb_value v;
  struct RClass *dio;
  extern const uint8_t appbin[];

  printf("mrb_open() ... ");
  mrb_state *mrb = mrb_open_allocf(em_mallocf, NULL);
  printf("done.\n");

  /* define puts and delay method */
  mrb_define_method(mrb, mrb->object_class, "puts", mrb_puts, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, mrb->object_class, "delay", mrb_delay, MRB_ARGS_REQ(1));

  /* define DigitalIO class */
  dio = mrb_define_class(mrb, "DigitalIO", mrb->object_class);
  mrb_define_method(mrb, dio, "initialize", mrb_dio_init,   MRB_ARGS_OPT(1));
  mrb_define_method(mrb, dio, "write",      mrb_dio_write,  MRB_ARGS_REQ(1));

  mrb_full_gc(mrb);

  /* Launch application */
  v = mrb_load_irep(mrb, appbin);
  if (mrb->exc) {
    mrb_print_error(mrb);
    mrb->exc = 0;
  }
  else {
    printf(" => ");
    mrb_p(mrb, v);
  }

  mrb_close(mrb);

  while (true) {
    led1 = !led1;
    mbedDelay(50);
  }
}
