rm -rf  mcu.bin 
HEX2BIN.EXE mcu.hex 
bin2headfile.exe -T mcu.bin ali_pmu_bin_3526.h