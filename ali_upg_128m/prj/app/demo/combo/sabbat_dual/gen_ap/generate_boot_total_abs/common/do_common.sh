#!/bin/bash

function message()
{
	echo -e "\033[47;30m$*\033[0m"
	echo ""
	return 0
}

function assert_file()
{
	local cnt=$#
	local i
	local ret="false"

	for((i=1;i<=cnt;i++));do
		if [ ! -f $1 ]; then
			message "Error: $1 not found"
			exit 1
		fi
		shift
	done

	return 0
}

# $1 : the source file to be inserted with specific UK
# $2 : the file of UK
# $3 : the index of the specific UK.
# Note:
#  1. Each UK occupies 0x80 bytes
#  2. The first UK is started from 0x3880 of boot_total_area.
#  3. What's the maximum number of UK to be inserted??
function insert_uk()
{
	local offset
	local template

	assert_file $1 $2

	((offset=0x3880 + $3*128))
	offset="$(printf "0x%x" $offset)"
	template="
[PARAMETER]        
src_file=$1
out_file=$1
sub_file=$2
dest_addr=$offset
insert_length=0x10
[CMD]
insert_data
	"
	echo "$template" > $TEMPDIR/insert_uk_$3.cmd
	assert_file $1 $2
	$ALICRYPTO $TEMPDIR/insert_uk_$3.cmd
	[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1

	return 0
}

# $1 : the source file to be inserted with specific rsa public key
# $2 : the file of rsa public key
# Note:
#  1. Each RSA public key occupies 0x200 bytes
function append_pk()
{
	local filename1="$(basename $1)"
	local filename2="$(basename $2)"

	assert_file $1

	local template="
[PARAMETER]
src_file=$1
out_file=$1
sub_file=$2
back_offset=0x00
insert_length=0x200
[CMD]
back_insert_data
	"
	echo "$template" > $TEMPDIR/append_$filename1.$filename2.cmd
	assert_file $1 $2
	$ALICRYPTO $TEMPDIR/append_$filename1.$filename2.cmd
	[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1

	return 0
}

# $1: input file
# $2: output file
# $3: compress type
function compress()
{
	local input=$1
	local output=$2
	local comp=$3
	local tmp=$TEMPDIR/tmpfile

	assert_file $input

	echo "No compress...!"
	cat $input > $tmp && cat $tmp > $output && rm -f $tmp
	[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1

	message "compressed file: $output"

	return 0
}

# $1: input file
# $2: output file
# $3: ukid
# $4: chipset (only required by signing via crypto server)
# $5: CA (only required by signing via crypto server)
# $6: algorithmn (optional)
# $7: residue_mode (optional)
# $8: chaining_mode (optional)
function encrypt()
{
	local argc=$#
	local input=$1
	local output=$2
	local ukid=$3
	local uk=""
	local chipset=""
	local ca=""
	local algo=""
	local residue=""
	local chaining=""
	local sign_by_ali=0
	local tmp=$TEMPDIR/tmpfile
	local filename="$(basename $input)"

	if [ "$ukid" = "ABL_AES_NONE" ]; then
		cat $input > $tmp && cat $tmp > $output && rm -f $tmp
		[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1
		return 0
	elif [ "$ukid" = "ABL_AES_UK_0" ]; then
		uk=$TOP/keys/abl_aes_uk_0.bin
	elif [ "$ukid" = "ABL_AES_UK_1" ]; then
		uk=$TOP/keys/abl_aes_uk_1.bin
	elif [ "$ukid" = "ABL_AES_UK_2" ]; then
		uk=$TOP/keys/abl_aes_uk_2.bin
	elif [ "$ukid" = "ABL_AES_UK_3" ]; then
		uk=$TOP/keys/abl_aes_uk_3.bin
	elif [ "$ukid" = "ABL_AES_UK_4" ]; then
		uk=$TOP/keys/abl_aes_uk_4.bin
	elif [ "$ukid" = "ABL_AES_UK_5" ]; then
		uk=$TOP/keys/abl_aes_uk_5.bin
	elif [ "$ukid" = "ABL_AES_OTP_KEY_7" ]; then
		uk=key7
		sign_by_ali=1
	elif [ "$ukid" = "ABL_AES_OTP_KEY_8" ]; then
		uk=key8
		sign_by_ali=1
	elif [ "$ukid" = "ABL_AES_OTP_KEY_9" ]; then
		uk=key9
		sign_by_ali=1
	elif [ "$ukid" = "ABL_AES_OTP_KEY_10" ]; then
		uk=key10
		sign_by_ali=1
	elif [ "$ukid" = "ABL_AES_IRD_IKEY" ]; then
		uk=$TOP/keys/ik.bin
	elif [ "$ukid" = "ABL_AES_IRD_BCKEY" ]; then
		uk=$TOP/keys/bck.bin
	else
		message "($BASH_SOURCE:$FUNCNAME:$LINENO) Not support UK <$ukid>"
		exit 1
	fi

	if [ "$sign_by_ali" = "1" ]; then
		assert_file $input
		if [ $argc -eq 8 ]; then
			chipset=$4
			ca=$5
			algo=$6
			residue=$7
			chaining=$8
			$ALICRYPTOCLIENT --type basic \
				--chipset-name $chipset \
				--ca $ca \
				--key-name $uk \
				--algo $algo \
				--residue $residue \
				--chaining $chaining \
				--object-in $input \
				--object-out $output
			[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1
		elif [ $argc -eq 7 ]; then
			chipset=$4
			ca=$5
			algo=$6
			residue=$7
			$ALICRYPTOCLIENT --type basic \
				--chipset-name $chipset \
				--ca $ca \
				--key-name $uk \
				--algo $algo \
				--residue $residue \
				--object-in $input \
				--object-out $output
			[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1
		elif [ $argc -eq 6 ]; then
			chipset=$4
			ca=$5
			algo=$6
			$ALICRYPTOCLIENT --type basic \
				--chipset-name $chipset \
				--ca $ca \
				--key-name $uk \
				--algo $algo \
				--object-in $input \
				--object-out $output
			[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1
		elif [ $argc -eq 5 ]; then
			chipset=$4
			ca=$5
			$ALICRYPTOCLIENT --type basic \
				--chipset-name $chipset \
				--ca $ca \
				--key-name $uk \
				--algo AES \
				--object-in $input \
				--object-out $output
			[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1
		else
			message "($BASH_SOURCE:$FUNCNAME:$LINENO) arguments error!"
			exit 1
		fi

		assert_file $output
	else
		assert_file $input $uk
		cat $input > $tmp
		template="
[PARAMETER]
encrypt_key=$uk
src_file=$tmp
[CMD]
sw_enc
"
		echo "$template" > $TEMPDIR/encrypt_$filename.cmd
		$ALICRYPTO $TEMPDIR/encrypt_$filename.cmd
		[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1
		cat $tmp.ecf > $output
		[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1
		rm -f $tmp
		rm -f $tmp.*
	fi

	message "encrypted file: $output"

	return 0
}

# $1: input file
# $2: output file of the only 64 bytes of ubo header
# $3: ukid
# $4: comp
# $5: name
# $6-: args (extra args of mkimage)
#   The ukid is used here to adjust name with ENC_ prefix or not
function gen_ubohdr()
{
	local argc=$#
	local input=$1
	local output=$2
	local ukid=$3
	local comp=$4
	local name=$5
	local tmp=$TEMPDIR/tmpfile
	if [ $argc -eq 5 ]; then
		local args=""
	else
		shift; shift; shift; shift; shift;
		local args="$*"
	fi

	assert_file $input

	case $ukid in
	ABL_AES_NONE )
		args="$args -n $name";;
	ABL_AES_UK_* | ABL_AES_OTP_KEY_* | ABL_AES_IRD_* )
		args="$args -n ENC_&*@";;
	* )
		message "($BASH_SOURCE:$FUNCNAME:$LINENO) Not support UK <$ukid>"
		exit 1 ;;
	esac

	if [ "$comp" = "none" ]; then
		args="$args -C none"
	elif [ "$comp" = "gzip" ]; then
		args="$args -C gzip"
	elif [ "$comp" = "lzma" ]; then
		args="$args -C lzma"
	else
		message "($BASH_SOURCE:$FUNCNAME:$LINENO) Not support compress <$comp>"
		exit 1
	fi

	rm -f $tmp

	echo "$MKIMAGE $args -d $input $tmp"

#	$MKIMAGE $args -d $input $tmp
	echo "$input"
	echo "$(pwd)"
	if [ "$name" = "seebl" ]; then
		$MKIMAGE $args -d ./../temporary/common/see_bl.abs.comp ./tmpfile
	elif [ "$name" = "see" ]; then
		$MKIMAGE $args -d ./../temporary/common/see_bin.abs.comp ./tmpfile
	fi
	[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1
	dd if=./tmpfile of=$output bs=64 count=1
	[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1
	rm -f $tmp

	message "generate ubo header: $output"

	return 0
}

# $1: 1st input file
# $2: output file of the signature
# $3: pkid
# $4: chipset (only required by signing via crypto server)
# $5: CA (only required by signing via crypto server)
function gen_sig()
{
	local argc=$#
	local input=$1
	local output=$2
	local pkid=$3
	local pk=""
	local chipset=""
	local ca=""
	local sign_by_ali=0
	local filename="$(basename $input)"
	local tmp=$TEMPDIR/tmpfile

	if [ "$pkid" = "ABL_RSA_PK_NONE" ]; then
		rm -f $output
		touch $output
		message "empty of signature: $output"
		return
	elif [ "$pkid" = "ABL_RSA_PK_0" ]; then
		pk=$TOP/keys/abl_rsa_pk_0.der
	elif [ "$pkid" = "ABL_RSA_PK_1" ]; then
		pk=$TOP/keys/abl_rsa_pk_1.der
	elif [ "$pkid" = "ABL_RSA_PK_2" ]; then
		pk=$TOP/keys/abl_rsa_pk_2.der
	elif [ "$pkid" = "ABL_RSA_PK_3" ]; then
		pk=$TOP/keys/abl_rsa_pk_3.der
	elif [ "$pkid" = "ABL_RSA_PK_4" ]; then
		pk=$TOP/keys/abl_rsa_pk_4.der
	elif [ "$pkid" = "ABL_RSA_PK_5" ]; then
		pk=$TOP/keys/abl_rsa_pk_5.der
	elif [ "$pkid" = "ABL_RSA_PK_SEESW" ]; then
		pk=see_sw_rsa
		sign_by_ali=1
	elif [ "$pkid" = "ABL_RSA_PK_TEESW" ]; then
		pk=tee_sw_rsa
		sign_by_ali=1
	elif [ "$pkid" = "ABL_RSA_PK_SEEROOT" ]; then
		pk=see_root_rsa
		sign_by_ali=1
	elif [ "$pkid" = "ABL_RSA_PK_TEEROOT" ]; then
		pk=tee_root_rsa
		sign_by_ali=1
	else
		message "($BASH_SOURCE:$FUNCNAME:$LINENO) Not support RSA Key <$pkid>"
		exit 1
	fi

	
	cat $input > $tmp

	if [ "$sign_by_ali" = "1" ]; then
		if [ $argc -eq 5 ]; then
			chipset=$4
			ca=$5
			assert_file $tmp
			$ALICRYPTOCLIENT --type basic \
				--chipset-name $chipset \
				--ca $ca \
				--key-name $pk \
				--algo RSA \
				--object-in $input \
				--object-out $output
			[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1

			assert_file $output
		else
			message "($BASH_SOURCE:$FUNCNAME:$LINENO) arguments error!"
			exit 1
		fi

	else
		assert_file $pk $input
		local template="
[PARAMETER]
rsa_key_format=DER
sign_rsa_key=$pk
src_file=$tmp
[CMD]
sw_sign
"
		echo "$template" > $TEMPDIR/sign_$filename.cmd
		$ALICRYPTO $TEMPDIR/sign_$filename.cmd
		[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1
		cat $tmp.sig > $output
		[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1
		rm -f $tmp
		rm -f $tmp.*
	fi

	message "generate signature: $output"

	return 0
}

# $1 : the source file need to calculate sha256 value
# Note:
#  1. Only support Hash256
#  2. Hash value will be appended to the end of source file
function gen_sha256()
{
	local filename1="$(basename $1)"
	assert_file $1

	local input=$1
	local output=$1.hash256

	local template="
[PARAMETER]
src_file=$input
out_file=$output
[CMD]
hash256
	"

	echo "$template" > $TEMPDIR/genhash256_$filename1.cmd

	$ALICRYPTO $TEMPDIR/genhash256_$filename1.cmd
	[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1

	cat $output >> $input
	rm -f $output
	return 0
}

# $1 : the source file need to generate ubohdr
# Note:
#  Ubohdr will add to the top of image
function add_outter_ubohdr()
{
	local filename1="$(basename $1)"
	assert_file $1

	local output=$1

	#generate outter ubo hdr for $1
	cp -f $output $output.backup
	echo ">>> gen outter ubohdr"
	gen_ubohdr $output \
		$output.ubohdr \
		ABL_AES_NONE \
		none \
		NONE

	[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1
	cat $output.ubohdr > $output
	cat $output.backup >> $output

	return 0
}

teeaux_align()
{
	local input=$1
	local output=$2
	local filename="$(basename $input)"
	local template="
[PARAMETER]
src_file=$input
out_file=$output
padding_data=0xFFFFFFFF
padding_len=0x1F6F0
[CMD]
padding_data
"
	assert_file $1
	echo "$template" > $TEMPDIR/teeaux_align_$filename.cmd
	$ALICRYPTO $TEMPDIR/teeaux_align_$filename.cmd
	[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1

	return 0
}

#
# Different CA may have different flow, for example:
#  * Is the ubo header generated by clear payload or encrypted payload?
#  * Is the ubo header generated with payload + ctrdata or only payload?
#  * Does the signature cover ubo header or only the payload?
#  * Does the signature cover clear payload or encrypted payload?
#  * Does the ubo header needs extra payload?
#
# $1 name
# $2 compress: none/gzip/lzma
# $3 hdr_pkid:
# $4 ukid:
# $5 pkid:
# $6 input image
# $7 output image
# $8 ctrdata
# $9 chipset
# $10 CA
# $11, $12, $13 ...(optional: extra args of mkimage)
#   Since the load addr and entry addr should be configured correctly for linux
#   kernel image, those extra args should be set according to DTS mapping.
#
function do_ubo_c2000a()
{
	local argc=$#
	local name=""
	local comp=""
	local hdr_pkid=""
	local ukid=""
	local pkid=""
	local input=""
	local output=""
	local ctrdata=""
	local chipset=""
	local ca=""
	local args=""
	local i=1
	local filename=""

	if [ $argc -lt 10 ]; then
		message "($BASH_SOURCE:$FUNCNAME:$LINENO) arguments error!"
		exit 1
	fi

	for((i=1;i<=10;i++));do
		if [ $i -eq 1 ]; then
			name=$1
			echo "name is $name"
			shift
		elif [ $i -eq 2 ]; then
			comp=$1
			echo "comp is $comp"
			shift
		elif [ $i -eq 3 ]; then
			hdr_pkid=$1
			echo "hdr_pkid is $hdr_pkid"
			shift
		elif [ $i -eq 4 ]; then
			ukid=$1
			echo "ukid is $ukid"
			shift
		elif [ $i -eq 5 ]; then
			pkid=$1
			echo "pkid is $pkid"
			shift
		elif [ $i -eq 6 ]; then
			input=$1
			echo "input is $input"
			shift
		elif [ $i -eq 7 ]; then
			output=$1
			echo "output is $output"
			shift
		elif [ $i -eq 8 ]; then
			ctrdata=$1
			echo "ctrdata is $ctrdata"
			shift
		elif [ $i -eq 9 ]; then
			chipset=$1
			echo "chipset is $chipset"
			shift
		elif [ $i -eq 10 ]; then
			ca=$1
			echo "ca is $ca"
			shift
		fi
	done

	if [ $argc -gt 10 ]; then
		args=$*
	else
		args=""
	fi

	echo "extra args is <$args>"

	filename="$(basename $input)"

	assert_file $input
	cp -vf $input $TEMPDIR/common/$filename

	compress $TEMPDIR/common/$filename \
		$TEMPDIR/common/$filename.comp \
		$comp
	[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1

	encrypt $TEMPDIR/common/$filename.comp \
		$TEMPDIR/common/$filename.comp.enc \
		$ukid \
		$chipset \
		$ca AES no_handle
	[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1

	gen_ubohdr $TEMPDIR/common/$filename.comp \
		$TEMPDIR/common/$filename.ubohdr \
		$ukid \
		$comp \
		$name $args
	[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1

	gen_sig $TEMPDIR/common/$filename.ubohdr \
		$TEMPDIR/common/$filename.ubohdr.sig \
		$hdr_pkid \
		$chipset \
		$ca
	[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1

	cat $TEMPDIR/common/$filename.comp \
		$ctrdata \
		> $TEMPDIR/common/$filename.for.signing
	[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1

	gen_sig $TEMPDIR/common/$filename.for.signing \
		$TEMPDIR/common/$filename.sig \
		$pkid \
		$chipset \
		$ca
	[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1

	cat $TEMPDIR/common/$filename.ubohdr.sig \
		$TEMPDIR/common/$filename.ubohdr \
		$TEMPDIR/common/$filename.comp.enc \
		$ctrdata \
		$TEMPDIR/common/$filename.sig \
		> $output/$filename.ok
	[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1

	message "Generate final ubo: $output"

	return 0
}

#
# Different CA may have different flow, for example:
#  * Is the ubo header generated by clear payload or encrypted payload?
#  * Is the ubo header generated with payload + ctrdata or only payload?
#  * Does the signature cover ubo header or only the payload?
#  * Does the signature cover clear payload or encrypted payload?
#  * Does the ubo header needs extra payload?
#
# $1 name
# $2 compress: none/gzip/lzma
# $3 hdr_pkid:
# $4 ukid:
# $5 pkid:
# $6 input image
# $7 output image
# $8 ctrdata
# $9 chipset
# $10 CA
# $11, $12, $13 ...(optional: extra args of mkimage)
#   Since the load addr and entry addr should be configured correctly for linux
#   kernel image, those extra args should be set according to DTS mapping.
#
function do_ubo_c1800a()
{
	local argc=$#
	local name=""
	local comp=""
	local hdr_pkid=""
	local ukid=""
	local pkid=""
	local input=""
	local output=""
	local ctrdata=""
	local chipset=""
	local ca=""
	local args=""
	local i=1
	local filename=""

	if [ $argc -lt 10 ]; then
		message "($BASH_SOURCE:$FUNCNAME:$LINENO) arguments error!"
		exit 1
	fi

	for((i=1;i<=10;i++));do
		if [ $i -eq 1 ]; then
			name=$1
			echo "name is $name"
			shift
		elif [ $i -eq 2 ]; then
			comp=$1
			echo "comp is $comp"
			shift
		elif [ $i -eq 3 ]; then
			hdr_pkid=$1
			echo "hdr_pkid is $hdr_pkid"
			shift
		elif [ $i -eq 4 ]; then
			ukid=$1
			echo "ukid is $ukid"
			shift
		elif [ $i -eq 5 ]; then
			pkid=$1
			echo "pkid is $pkid"
			shift
		elif [ $i -eq 6 ]; then
			input=$1
			echo "input is $input"
			shift
		elif [ $i -eq 7 ]; then
			output=$1
			echo "output is $output"
			shift
		elif [ $i -eq 8 ]; then
			ctrdata=$1
			echo "ctrdata is $ctrdata"
			shift
		elif [ $i -eq 9 ]; then
			chipset=$1
			echo "chipset is $chipset"
			shift
		elif [ $i -eq 10 ]; then
			ca=$1
			echo "ca is $ca"
			shift
		fi
	done

	if [ $argc -gt 10 ]; then
		args=$*
	else
		args=""
	fi

	echo "extra args is <$args>"

	filename="$(basename $input)"

	assert_file $input
	cp -vf $input $TEMPDIR/$filename

	compress $TEMPDIR/$filename \
		$TEMPDIR/$filename.comp \
		$comp
	[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1

	encrypt $TEMPDIR/$filename.comp \
		$TEMPDIR/$filename.comp.enc \
		$ukid \
		$chipset \
		$ca AES no_handle
	[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1

	gen_ubohdr $TEMPDIR/$filename.comp \
		$TEMPDIR/$filename.ubohdr \
		$ukid \
		$comp \
		$name $args
	[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1

	gen_sig $TEMPDIR/$filename.ubohdr \
		$TEMPDIR/$filename.ubohdr.sig \
		$hdr_pkid \
		$chipset \
		$ca
	[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1

	if [ $ctrdata = "NONE" ]; then
		mv -f $TEMPDIR/$filename.comp $TEMPDIR/$filename.for.signing
	else
	cat $TEMPDIR/$filename.comp \
		$ctrdata \
		> $TEMPDIR/$filename.for.signing
	[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1
	fi

	gen_sig $TEMPDIR/$filename.for.signing \
		$TEMPDIR/$filename.sig \
		$pkid \
		$chipset \
		$ca
	[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1
	
	if [ $ctrdata = "NONE" ]; then
	cat $TEMPDIR/$filename.ubohdr.sig \
		$TEMPDIR/$filename.ubohdr \
		$TEMPDIR/$filename.comp.enc \
		$TEMPDIR/$filename.sig \
		> $output
	[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1
	else
	cat $TEMPDIR/$filename.ubohdr.sig \
		$TEMPDIR/$filename.ubohdr \
		$TEMPDIR/$filename.comp.enc \
		$ctrdata \
		$TEMPDIR/$filename.sig \
		> $output
	[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1
	fi	

	message "Generate final ubo: $output"

	return 0
}

# $1 : rsa public key name
# $2 : output
# $3 : chipset
# $4 : ca
function get_rsa_pk()
{
	local pkname=$1
	local output=$2
	local chipset=$3
	local ca=$4

	if [ "$#" -eq 4 ]; then
		$ALICRYPTOCLIENT --type pub_key --chipset-name $chipset --ca $ca --key-name $pkname --object-out $output
	else
		message "($BASH_SOURCE:$FUNCNAME:$LINENO) arguments error!"
		exit 1;
	fi
}

# $1 byte value
# $2 count of bytes
# $3 output
function padding_byte()
{
	local bts=$1
	local cnt=$2
	local output=$3
	local i=1

	bts=$(printf "0x%02x" $bts)

	for ((i=1;i<=$cnt;i++));do
		echo -e -n "\x"${bts:2:2} >> $output
	done
}

# $1 word value (4 bytes)
# $2 output
function padding_word()
{
	local wd=$1
	local output=$2
	
	wd=$(printf "0x%08x" $wd)

	echo -e -n "\x"${wd:8:2} >> $output
	echo -e -n "\x"${wd:6:2} >> $output
	echo -e -n "\x"${wd:4:2} >> $output
	echo -e -n "\x"${wd:2:2} >> $output
}

#
# Different CA may have different flow, for example:
#  * Is the ubo header generated by clear payload or encrypted payload?
#  * Is the ubo header generated with payload + ctrdata or only payload?
#  * Does the signature cover ubo header or only the payload?
#  * Does the signature cover clear payload or encrypted payload?
#  * Does the ubo header needs extra payload?
#
# $1 name
# $2 compress: none/gzip/lzma
# $3 ukid:
# $4 pkid:
# $5 input image
# $6 output image
# $7 ctrdata
# $8 chipset
# $9 CA
# $11, $12, $13 ...(optional: extra args of mkimage)
#   Since the load addr and entry addr should be configured correctly for linux
#   kernel image, those extra args should be set according to DTS mapping.
#
function do_tds_ca()
{
	local argc=$#
	local name=""
	local comp=""
	local ukid=""
	local pkid=""
	local input=""
	local output=""
	local ctrdata=""
	local chipset=""
	local ca=""
	local args=""
	local i=1
	local filename=""

	if [ $argc -lt 9 ]; then
		message "($BASH_SOURCE:$FUNCNAME:$LINENO) arguments error!"
		exit 1
	fi

	for((i=1;i<=10;i++));do
		if [ $i -eq 1 ]; then
			name=$1
			echo "name is $name"
			shift
		elif [ $i -eq 2 ]; then
			comp=$1
			echo "comp is $comp"
			shift
		elif [ $i -eq 3 ]; then
			ukid=$1
			echo "ukid is $ukid"
			shift
		elif [ $i -eq 4 ]; then
			pkid=$1
			echo "pkid is $pkid"
			shift
		elif [ $i -eq 5 ]; then
			input=$1
			echo "input is $input"
			shift
		elif [ $i -eq 6 ]; then
			output=$1
			echo "output is $output"
			shift
		elif [ $i -eq 7 ]; then
			ctrdata=$1
			echo "ctrdata is $ctrdata"
			shift
		elif [ $i -eq 8 ]; then
			chipset=$1
			echo "chipset is $chipset"
			shift
		elif [ $i -eq 9 ]; then
			ca=$1
			echo "ca is $ca"
			shift
		fi
	done

	if [ $argc -gt 9 ]; then
		args=$*
	else
		args=""
	fi

	echo "extra args is <$args>"

	filename="$(basename $input)"

	assert_file $input
	cp -vf $input $TEMPDIR/common/$filename

	compress $TEMPDIR/common/$filename \
		$TEMPDIR/common/$filename.comp \
		$comp
	[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1

	encrypt $TEMPDIR/common/$filename.comp \
		$TEMPDIR/common/$filename.comp.enc \
		$ukid \
		$chipset \
		$ca AES no_handle
	[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1

	cat $TEMPDIR/common/$filename.comp \
		$ctrdata \
		> $TEMPDIR/common/$filename.for.signing
	[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1

	gen_sig $TEMPDIR/common/$filename.for.signing \
		$TEMPDIR/common/$filename.sig \
		$pkid \
		$chipset \
		$ca
	[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1

	cat $TEMPDIR/common/$filename.comp.enc \
		$ctrdata \
		$TEMPDIR/common/$filename.sig \
		> $output/$filename.ok
	[ $? -ne 0 ] && message "($BASH_SOURCE:$FUNCNAME:$LINENO) Error!" && exit 1

	message "Generate final ubo: $output"

	return 0
}