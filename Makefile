# This is a prototype Makefile. Modify it according to your needs.
# You should at least check the settings for
# DEVICE ....... The AVR device you compile for
# CLOCK ........ Target AVR clock rate in Hertz
# OBJECTS ...... The object files created from your source files. This list is
#                usually the same as the list of source files with suffix ".o".
# PROGRAMMER ... Options to avrdude which define the hardware you use for
#                uploading to the AVR and the interface where this hardware
#                is connected.
# FUSES ........ Parameters for avrdude to flash the fuses appropriately.

DEVICE     = atmega328p
CLOCK      = 16000000
PORT 	   = /dev/ttyUSB0
PROGRAMMER = -c stk500v1 -b57600 -P$(PORT)
FUSES      = -U hfuse:w:0xd9:m -U lfuse:w:0x24:m
SRCS       = main.c lib/crc.c lib/spi.c lib/w5100.c lib/uart.c lib/socket.c lib/udp.c lib/dhcp.c lib/util.c lib/timer.c lib/ir.c lib/tcp.c
HDRS       = lib/crc.h lib/spi.h lib/w5100.h lib/uart.h lib/socket.h lib/udp.h lib/dhcp.h lib/util.h lib/timer.h lib/ir.h lib/tcp.h

MAIN=main
OBJDIR=obj
BINDIR=bin
DEPDIR=dep

INCLUDE=lib

PREFIX=avr-
CC=$(PREFIX)gcc
AS=$(PREFIX)gcc
LD=$(PREFIX)gcc
AVRDUDE=avrdude

cwarnings=-pedantic -Wall -Wextra -Wfloat-equal -Wwrite-strings -Wpointer-arith -Wcast-qual -Wcast-align  -Wshadow -Wredundant-decls -Wdouble-promotion -Winit-self -Wswitch-default -Wswitch-enum -Wundef -Wlogical-op -Winline
# -Wconversion
copti=-ffunction-sections -fdata-sections -O3
ldopti=-Wl,--gc-sections
CFLAGS= $(CWARNINGS) -Werror -std=c99 -DF_CPU=$(CLOCK) -mmcu=$(DEVICE) $(addprefix -I,$(INCLUDE)) $(cwarnings) $(copti)
AFLAGS=$(CFLAGS) -x assembler-with-cpp
LFLAGS=$(CFLAGS) $(ldopti)
AVRFLAGS=$(PROGRAMMER) -p $(DEVICE)

OBJS=$(addprefix $(OBJDIR)/,$(addsuffix .o,$(SRCS)))
DEPS=$(addprefix $(DEPDIR)/,$(addsuffix .d,$(SRCS)))
HEX=$(BINDIR)/$(MAIN).hex
ELF=$(BINDIR)/$(MAIN).elf

# symbolic targets:
all: $(HEX) $(TAGS)

-include $(DEPS)

$(OBJDIR)/%.c.o: %.c
	@test -d $(dir $@) || mkdir $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

	@test -d $(dir $(DEPDIR)/$<.d) || mkdir $(dir $(DEPDIR)/$<.d)
	$(CC) -MM $(CFLAGS) $< >  $(DEPDIR)/$<.d
	@mv -f $(DEPDIR)/$<.d $(DEPDIR)/$<.d.tmp
	@sed -e 's|.*:|$(OBJDIR)/$<.o:|' < $(DEPDIR)/$<.d.tmp > $(DEPDIR)/$<.d
	@sed -e 's/.*://' -e 's/\\$$//' < $(DEPDIR)/$<.d.tmp | fmt -1 | \
	sed -e 's/^ *//' -e 's/$$/:/' >> $(DEPDIR)/$<.d
	@rm -f $(DEPDIR)/$<.d.tmp

$(OBJDIR)/%.S.o: %.S
	@test -d $(dir $@) || mkdir $(dir $@)
	$(AS) $(AFLAGS) -c -o $@ $<

# file targets:
$(ELF): $(OBJS)
	@test -d $(dir $@) || mkdir $(dir $@)
	$(LD) $(LFLAGS) -o $@ $^

$(HEX): $(ELF)
	rm -f main.hex
	avr-objcopy -j .text -j .data -O ihex $< $@

.PHONY: all flash fuse install clean

flash: $(HEX)
	$(AVRDUDE) $(AVRFLAGS) -U flash:w:$<:i

fuse:
	$(AVRDUDE) $(AVRFLAGS) $(FUSES)

install: flash fuse

clean:
	rm -f $(HEX) $(ELF) $(OBJS) $(DEPS) $(TAGS)
