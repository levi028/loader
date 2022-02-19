#!/bin/sh

current_dir=`pwd`
root_dir=${current_dir}/../../../../..
obj_dir=${root_dir}/obj

date_and_time=`date +%y%m%d%H%M%S`

rm ${obj_dir}/Sabbat_s3711c.*.out
#rm ${obj_dir}/see_sabbat_s3702.*.out

cp -vf ${obj_dir}/Sabbat_s3711c.out ${obj_dir}/Sabbat_s3711c.${date_and_time}.out
#cp -vf ${obj_dir}/see_sabbat_s3702.out ${obj_dir}/see_sabbat_s3702.${date_and_time}.out

eval sed -i 's/Sabbat_s3711c.*\.out/Sabbat_s3711c.${date_and_time}\.out/g' ${obj_dir}/*.ini
#eval sed -i 's/see_sabbat_s3702.*\.out/see_sabbat_s3702.${date_and_time}\.out/g' ${obj_dir}/*.ini
