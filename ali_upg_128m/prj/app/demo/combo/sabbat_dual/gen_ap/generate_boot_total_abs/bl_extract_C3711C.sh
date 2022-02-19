#!/bin/bash
# Illustration:
# This script extract images defined bellow from IN_FILE
# That is to extract data from correspoding area of IN_FILE to form OUT_FILEx images
# The OFFSET to extract and extract size please reference image information. 


IN_FILE=$1
DIR_EXTRACT=$2
# Judge whether IN_FILE exits
if [[ ! -e $IN_FILE ]]
then
    printf "\nError!!!: File $IN_FILE is not existed!\n"
    exit 1
fi
# Calculate & print IN_FILE size
IN_FILE_SIZE=`du -b $IN_FILE | awk '{print $1}'`

printf "\nBOOT_TOTAL_AREA_SIZE:0x%x as size get from $IN_FILE\n" $IN_FILE_SIZE

# Use tmp file
IN_TMP=./boot_total_area_tmp.abs
echo "Use tmp file: cp $IN_FILE $IN_TMP"
cp $IN_FILE $IN_TMP -rf

# Information of the image files to be extracted 
# Basic info & variable
DIR_ORIGIN=.
MAX_IMAGE_NUMBER=9
SIZE_SIG=$((16#100))
CONTROL_DATA_SIZE=$((16#10))
printf "ImageInfo>> DIR:%s/ ImageNum:%d SignatureSize:0x%x\n" $DIR_ORIGIN $MAX_IMAGE_NUMBER $SIZE_SIG
# Images
OUT_FILE0=$DIR_ORIGIN/ext_rsa_pubkey_area.bin           OFFSET0=$((16#0))                     SIZE0=$((16#150))
OUT_FILE1=$DIR_ORIGIN/ext_rsa_pubkey_area_sig.bin       OFFSET1=$((16#150))                   SIZE1=$SIZE_SIG
OUT_FILE2=$DIR_ORIGIN/boot_params_area.bin              OFFSET2=$((16#250))                   SIZE2=$((16#4B0))
OUT_FILE3=$DIR_ORIGIN/boot_params_area_sig.bin          OFFSET3=$((16#700))                   SIZE3=$SIZE_SIG
OUT_FILE4=$DIR_ORIGIN/boot_aux_code_area.bin            OFFSET4=$((16#800))                   SIZE4=$((16#7700))
OUT_FILE5=$DIR_ORIGIN/boot_aux_code_area_sig.bin        OFFSET5=$((16#7F00))                  SIZE5=$SIZE_SIG
OUT_FILE6=$DIR_ORIGIN/boot_unchecked_area.bin           OFFSET6=$((16#8000))                  SIZE6=$((16#2000))
OUT_FILE7=$DIR_ORIGIN/boot_code_area.bin                OFFSET7=$((16#A000))                  SIZE7=$(($IN_FILE_SIZE-$OFFSET7-$SIZE_SIG))
OUT_FILE8=$DIR_ORIGIN/boot_code_area_sig.bin            OFFSET8=$(($IN_FILE_SIZE-$SIZE_SIG))  SIZE8=$SIZE_SIG

# Begin to extract files in a circulation
echo "Begin to extract image data from $IN_TMP ..."
for ((i=0; i<$MAX_IMAGE_NUMBER; ++i))
do
    # use variable to institude image file information
    eval OUTFILE=\$OUT_FILE${i}
    eval OFFSET=\$OFFSET${i}
    eval SIZE=\$SIZE${i}

    printf "\nStep$i. Extract $OUTFILE from $IN_TMP OFFSET:0x%x SIZE:0x%x ...\n" $OFFSET $SIZE

    # Judge whether image file exist.
    if [[ -e $OUTFILE ]]
    then
        printf "Warning!: File $OUTFILE exists, and would be coverd! Ignore in this case.\n"
        EXIST_OUTFILE_SIZE=`du -b $OUTFILE | awk '{print $1}'`
        if [[ $EXIST_OUTFILE_SIZE -gt $SIZE ]]
        then
            printf "Warning!!!: The $OUTFILE exists and its size 0x%x is bigger than 0x%x which is to be extracted! It would be deleted and re-extracted. Ignore in this case.\n" $EXIST_OUTFILE_SIZE $SIZE
            printf "Suggestion: Check the size of it before use! Better empty $DIR_ORIGIN before extracting!\n"
            printf "rm -rf $OUTFILE!\n"
            rm -rf $OUTFILE 
        fi
    fi

    # Use tool util.sh function "extract" to extract image files, and judge result.
    ./util.sh extract --output=$OUTFILE --input_offset=$OFFSET $IN_TMP $SIZE
    if [[ $? -ne 0 ]]
    then
        echo "Extract $OUTFILE from $IN_TMP failed!!! Please check it!"
        exit 1
    fi
    printf "move $OUTFILE to $DIR_EXTRACT\n"
    mv -f $OUTFILE $DIR_EXTRACT
done

printf "\n$IN_FILE Extraction Finished!\n\n"

