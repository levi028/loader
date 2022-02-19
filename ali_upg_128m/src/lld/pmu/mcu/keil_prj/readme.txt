when use PMU_WIN7.bat
if your ic is 3821 , please modify it to 

bin2headfile.exe -T mcu.bin ali_pmu_bin_3821.h
cp ali_pmu_bin_3821.h ../../../../../prj/app/demo/combo/sabbat_dual/ali_pmu_bin.h -rf

if your ic is 3503 , please modify it to 

bin2headfile.exe -T mcu.bin ali_pmu_bin_3503.h
cp ali_pmu_bin_3503.h ../../../../../prj/app/demo/combo/sabbat_dual/ali_pmu_bin.h -rf

so you only need to modify ali_pmu_bin_xx.h according to differernt project

default is ali_pmu_bin_3503.h