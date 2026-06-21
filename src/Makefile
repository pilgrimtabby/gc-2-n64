CC=avr-gcc
AS=$(CC)
LD=$(CC)

VERSION=2.1-mod
CPU=atmega328p
CFLAGS=-Wall -mmcu=$(CPU) -DF_CPU=16000000L -Os -DVISUAL_BUZZER -DVERSION_STR=\"$(VERSION)\"
LDFLAGS=-mmcu=$(CPU) -Wl,-Map=gc_to_n64.map -Wl,--section-start=.endmarker=0x7ffc
HEXFILE=gc_to_n64b.hex
AVRDUDE=avrdude
AVRDUDE_CPU=m328p

OBJS=main.o gamecube.o n64_isr.o mapper.o gamecube_mapping.o n64_mapping.o \
     buzzer.o timer0.o eeprom.o sync.o lut.o gcn64_protocol.o menu.o \
     notch_cal.o retrospy.o

all: $(HEXFILE)

clean:
	rm -f gc_to_n64.elf gc_to_n64_tmp.hex $(HEXFILE) gc_to_n64.map $(OBJS)

gc_to_n64.elf: $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o gc_to_n64.elf

# For the Arduino Nano we flash via the on-board Arduino bootloader, so we
# do NOT need to append the raphnet siboot.  The hex is produced directly.
$(HEXFILE): gc_to_n64.elf
	avr-objcopy -j .data -j .text -j .endmarker -O ihex gc_to_n64.elf $(HEXFILE)
	avr-size gc_to_n64.elf

#
# ATmega328p fuse bytes (16 MHz crystal, BOD at 2.7 V, SPIEN, no CKDIV8)
#
# Extended: BODLEVEL = 101 (2.7 V)
EFUSE=0xFD

# Low: external crystal 8–16 MHz, slow rising power, no CKDIV8
LFUSE=0xFF

# High: SPIEN enabled, EESAVE, no RSTDISBL, no DWEN, no WDTON
HFUSE=0xD1

#
# Lock bits – no lock
#
LOCK=0xFF

fuse:
	$(AVRDUDE) -p $(AVRDUDE_CPU) -P usb -c avrispmkII \
	  -Uefuse:w:$(EFUSE):m -Uhfuse:w:$(HFUSE):m -Ulfuse:w:$(LFUSE):m -B 20.0 -F
	$(AVRDUDE) -p $(AVRDUDE_CPU) -P usb -c avrispmkII -Ulock:w:$(LOCK):m

#
# Flash via Arduino bootloader (Nano CH340/FTDI on /dev/ttyUSB0).
# Adjust -P and -b to match your programmer.
# For genuine FTDI Nano use -b 57600 instead.
#
flash: $(HEXFILE)
	$(AVRDUDE) -v -p $(AVRDUDE_CPU) -c arduino -P /dev/ttyUSB0 -b 115200 \
	  -D -Uflash:w:$(HEXFILE):i

reset:
	$(AVRDUDE) -p $(AVRDUDE_CPU) -P usb -c avrispmkII -B 1.0 -F

%.o: %.c
	$(CC) $(CFLAGS) -c $<

%.o: %.S
	$(CC) $(CFLAGS) -c $<
