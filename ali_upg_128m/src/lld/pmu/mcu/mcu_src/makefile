SDCC_DIR = /zhsa122/usrhome/eks.yang/test/sdcc/bin
OBJCOPY = objcopy
SDCCCFLAGS =  -mmcs51 --model-large -D__SDCC__
ASLINKFLAGS = --code-loc 0x0 --xram-loc 0x3600 --xram-size 0x800
#--code-loc must = 0x0,  --xram-loc must > code size
#ASLINKFLAGS = --code-loc 0x0 --xram-loc 0x1000 --xram-size 0x4000 --xstack-loc 0x08
all: mcu.hex bin2array
	$(OBJCOPY) -I ihex -O binary mcu.hex mcu.bin
	gcc bin2array.c -o bin2array
	./bin2array mcu.bin

3505: all
	cp ali_pmu_bin.h ../../ali_pmu_bin_3505.h

3702: all
	cp ali_pmu_bin.h ../../ali_pmu_bin_3702.h

3922: all
	cp ali_pmu_bin.h ../../ali_pmu_bin_3922.h

mcu.hex: main.c sys.rel rtc.rel  panel.rel uart.rel ir.rel
	$(SDCC_DIR)/sdcc $(SDCCCFLAGS) $(ASLINKFLAGS) $^
	$(SDCC_DIR)/packihx main.ihx > mcu.hex

%.rel : %.c %.h
	$(SDCC_DIR)/sdcc $(SDCCCFLAGS) -c $<

bin2array:
	gcc bin2array.c -o bin2array

.PHONY : clean
clean:
	rm -f *.asm *.hex *.ihx *.lnk *.lst *.map *.rel *.rst *.sym *.mem *.bin ali_pmu_bin.h bin2array


