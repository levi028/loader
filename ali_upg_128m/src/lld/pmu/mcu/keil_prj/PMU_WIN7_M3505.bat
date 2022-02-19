cp ../../../../../prj/tools/HEX2BIN.EXE ./ -rf
cp ../../../../../prj/tools/bin2headfile.exe ./ -rf
rm -rf  mcu.bin 
HEX2BIN.EXE mcu.hex 
bin2headfile.exe -T mcu.bin ali_pmu_bin_3505.h
cp ali_pmu_bin_3505.h ../../../../../prj/app/demo/combo/sabbat_dual/ali_pmu_bin.h -rf
cp mcu.bin ../mcu_debug/ -rf

