MRUBY = ./mruby
LD = mbed-os/targets/TARGET_STM/TARGET_STM32L4/TARGET_STM32L476xG/device/TOOLCHAIN_GCC_ARM/STM32L476XX.ld

all:
	mbed deploy
	ruby patch-mruby.rb
	MRUBY_CONFIG=../mbed_build_config.rb make -C $(MRUBY)
	$(MRUBY)/bin/mrbc -Bappbin -o app.c app.rb
	mbed compile -m NUCLEO_L476RG -t GCC_ARM

first:
	mbed deploy
	patch -u --forward $(LD) < STM32L476XX.ld.patch

clean:
	make -C $(MRUBY) clean
	rm -rf BUILD

