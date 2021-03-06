all: main.c
	avr-gcc -mmcu=atmega328p -DF_CPU=12000000 -Wall -funsigned-char -Os -o main.elf -I. \
		main.c usbdrv/usbdrv.c usbdrv/usbdrvasm.S usbdrv/oddebug.c rotary.c 
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex

flash: all
	avrdude -v -p m328p -c usbasp -U main.hex

clean:
	-rm main.elf main.hex
