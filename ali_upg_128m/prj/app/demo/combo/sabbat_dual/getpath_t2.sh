#!/bin/bash

PATH_ROOT=../../../../../
cd $PATH_ROOT
ROOTPATH=$(pwd)

echo LOCAL_BASE = $ROOTPATH
echo LOCAL_BASE = $ROOTPATH > path.def

echo " " >> path.def
echo BASE_DIR = \$\{LOCAL_BASE\} >> path.def
echo DDK = ddkt2 >> path.def
echo INC_DIR = \$\{BASE_DIR\}/inc >> path.def
echo LIB_DIR = \$\{BASE_DIR\}/obj >> path.def
echo REL_DIR = \$\{BASE_DIR\}/rel >> path.def

echo DDK_DIR = \$\{BASE_DIR\}/\$\{DDK\} >> path.def
echo PLUGINDDK_DIR = \$\{BASE_DIR}/\$\{DDK\}/plugin >> path.def
echo PLUGINEXT_DIR = \$\{BASE_DIR}/\$\{DDK\}/plugin_ext >> path.def


cp path.def src/
rm -f path.def