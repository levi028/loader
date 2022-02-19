/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: win_countryband_t.c
*
*    Description: The realize of country setting
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/
#include <types.h>
#include <basic_types.h>
#include <api/libc/string.h>
#include <api/libtsi/db_3l.h>
#include "sys_config.h"
#define MAX_COUNTRY 20
#define MAX_BAND_NUMBER  10
#define MAX_PARAM 4



//*************The definition of fourth element in every discontinuous band********************
//              Bit15~8                     Bit7                    Bit6~0
//      end channel no. of this group band     band type      start channel no. of this group band
//******************************************************************************
//const UINT32 dvbt_countryband[MAX_COUNTRY][MAX_BAND_NUMBER][MAX_PARAM] __attribute__((section(".flash")))=
static const band_param dvbt_countryband[MAX_COUNTRY][MAX_BAND_COUNT] __attribute__((section(".flash")))=
{///stanley wang 20050621
  //    changed by SN  for Alphabetically arranged 20050818
/*       "Argntina",*/{{ 174000, 216000 , 6000 ,0x701,0,0,0,0,0},
                       { 470000, 890000 , 6000 ,0x4D88,0,0,0,0,0},
                       { 0,       0,       0,     0,   0,0,0,0,0},
                       { 0,       0,       0,     0,   0,0,0,0,0},
                       { 0,       0,       0,     0,   0,0,0,0,0},
                       { 0,       0,       0,     0,   0,0,0,0,0},
                       { 0,       0,       0,     0,   0,0,0,0,0},
                       { 0,       0,       0,     0,   0,0,0,0,0},
                       { 0,       0,       0,     0,   0,0,0,0,0},
                       { 0,       0,       0,     0,   0,0,0,0,0}},

// Update the latest channel table from customer by Jie Wu, [2/17/2006]
/*        "Australia",*/{{ 174000, 202000, 7000 ,0x906,0,0,0,0,0 },
                         { 209000, 230000, 7000 ,0xC0A,0,0,0,0,0 },
                         { 520000, 526000, 6000 ,0x1B9B,0,0,0,0,0 },
                         { 526000, 582000, 7000 ,0x239C,0,0,0,0,0 },
                         { 582000, 820000, 7000 ,0x45A4,0,0,0,0,0 },
                         { 0,       0,       0,     0,   0,0,0,0,0},
                         { 0,       0,       0,     0,   0,0,0,0,0},
                         { 0,       0,       0,     0,   0,0,0,0,0},
                         { 0,       0,       0,     0,   0,0,0,0,0},
                         { 0,       0,       0,     0,   0,0,0,0,0}},

/*       "Brazil",*/{{ 174000, 254000 , 8000 ,0xD04,0,0,0,0,0},
					 { 470000, 862000 , 8000 ,0x4595,0,0,0,0,0},
					 {   0,       0,     0,    0,0,0,0,0,0},
					 {   0,       0,     0,    0,0,0,0,0,0},
					 {   0,       0,     0,    0,0,0,0,0,0},
					 {   0,       0,     0,    0,0,0,0,0,0},
					 {   0,       0,     0,    0,0,0,0,0,0},
					 {   0,       0,     0,    0,0,0,0,0,0},
					 {   0,       0,     0,    0,0,0,0,0,0},
					 {   0,       0,     0,    0,0,0,0,0,0}},                         
                    
/*        "China",*/{{ 48500, 72500, 8000, 0x301,0,0,0,0,0},
                     { 76000, 92000, 8000, 0x504,0,0,0,0,0},
                     {167000,223000, 8000, 0xC06,0,0,0,0,0},
                     {470000,862000, 8000,0x3D8D,0,0,0,0,0},
                     {    0,      0,     0,    0,0,0,0,0,0},
                     {   0,       0,     0,    0,0,0,0,0,0},
                     {   0,       0,     0,    0,0,0,0,0,0},
                     {   0,       0,     0,    0,0,0,0,0,0},
                     {   0,       0,     0,    0,0,0,0,0,0},
                     {   0,       0,     0,    0,0,0,0,0,0}},

/*"Denmark"*/       {{ 47000,  68000, 7000,0x402,0,0,0,0,0},
                     { 174000,230000, 7000,0xC05,0,0,0,0,0},
                     { 470000,862000, 8000,0x4595,0,0,0,0,0},
                     {   0,       0,     0,    0,0,0,0,0,0},
                     {   0,       0,     0,    0,0,0,0,0,0},
                     {   0,       0,     0,    0,0,0,0,0,0},
                     {   0,       0,     0,    0,0,0,0,0,0},
                     {   0,       0,     0,    0,0,0,0,0,0},
                     {   0,       0,     0,    0,0,0,0,0,0},
                     {   0,       0,     0,    0,0,0,0,0,0}
                     },

/*"Finland"*/       {{ 47000, 68000 , 7000 ,0x402,0,0,0,0,0},
                     { 174000, 230000 , 7000 ,0xC05,0,0,0,0,0},
                     {470000,862000,8000,0x4595,0,0,0,0,0},
                     {   0,       0,     0,    0,0,0,0,0,0},
                     {   0,       0,     0,    0,0,0,0,0,0},
                     {   0,       0,     0,    0,0,0,0,0,0},
                     {   0,       0,     0,    0,0,0,0,0,0},
                     {   0,       0,     0,    0,0,0,0,0,0},
                     {   0,       0,     0,    0,0,0,0,0,0},
                     {   0,       0,     0,    0,0,0,0,0,0}},

/*        "France",    */{{ 41000, 65000 , 8000 ,0x301,0,0,0,0,0},
                          { 174750, 222750 , 8000 ,0x904,0,0,0,0,0},
                          {470000,862000,8000,0x4595,0,0,0,0,0},
                          {   0,       0,     0,    0,0,0,0,0,0},
                          {   0,       0,     0,    0,0,0,0,0,0},
                          {   0,       0,     0,    0,0,0,0,0,0},
                          {   0,       0,     0,    0,0,0,0,0,0},
                          {   0,       0,     0,    0,0,0,0,0,0},
                          {   0,       0,     0,    0,0,0,0,0,0},
                          {   0,       0,     0,    0,0,0,0,0,0}},

/*"German"*/       {{ 47000, 68000 , 7000 ,0x402,0,0,0,0,0},
                    { 174000, 230000 , 7000 ,0xD05,0,0,0,0,0},
                    {470000,862000,8000,0x4595,0,0,0,0,0},
                    {   0,       0,     0,    0,0,0,0,0,0},
                    {   0,       0,     0,    0,0,0,0,0,0},
                    {   0,       0,     0,    0,0,0,0,0,0},
                    {   0,       0,     0,    0,0,0,0,0,0},
                    {   0,       0,     0,    0,0,0,0,0,0},
                    {   0,       0,     0,    0,0,0,0,0,0},
                    {   0,       0,     0,    0,0,0,0,0,0}},

/*"Hong-Kong"*/       {{ 47000,   68000, 7000,0x402,0,0,0,0,0},
                       { 174000, 230000, 7000,0xC05,0,0,0,0,0},
                       { 470000, 862000, 8000,0x4595,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0}},

/*"Italy"*/       {{ 52500, 59500 , 7000, 0x101,0,0,0,0,0},
                   { 61000, 68000 , 7000, 0x202,0,0,0,0,0},
                   { 81000,  88000, 7000, 0x303,0,0,0,0,0},
                   {174000, 181000, 7000, 0x404,0,0,0,0,0},
                   {182500, 189500, 7000, 0x505,0,0,0,0,0},
                   {191000, 198000, 7000, 0x606,0,0,0,0,0},
                   {200000, 207000, 7000, 0x707,0,0,0,0,0},
                   {209000, 230000, 7000, 0xA08,0,0,0,0,0},
                   {470000, 862000, 8000,0x4595,0,0,0,0,0},
                   {   0,        0,   0,      0,0,0,0,0,0}},
#ifdef POLAND_SPEC_SUPPORT
/* "Polish",*/  {{ 174000, 230000 , 7000 ,0xc05,0,0,0,0,0},
                    { 470000, 862000 , 8000 ,0x4595,0,0,0,0,0},
                    {   0,       0,     0,    0,0,0,0,0,0},
                            {   0,       0,     0,    0,0,0,0,0,0},
                        {   0,       0,     0,    0,0,0,0,0,0},
                        {   0,       0,     0,    0,0,0,0,0,0},
                        {   0,       0,     0,    0,0,0,0,0,0},
                    {   0,       0,     0,    0,0,0,0,0,0},
                    {   0,       0,     0,    0,0,0,0,0,0},
                    {   0,       0,     0,    0,0,0,0,0,0}},
#else
/*        "Polish",*/{{ 48500, 72500 , 8000 ,0x301,0,0,0,0,0},
                      { 76000, 92000 , 8000 ,0x504,0,0,0,0,0},
                      {167000,223000,8000,0xC06,0,0,0,0,0},
                      {470000,862000,8000,0x3D8D,0,0,0,0,0},
                      {   0,       0,     0,    0,0,0,0,0,0},
                      {   0,       0,     0,    0,0,0,0,0,0},
                      {   0,       0,     0,    0,0,0,0,0,0},
                      {   0,       0,     0,    0,0,0,0,0,0},
                      {   0,       0,     0,    0,0,0,0,0,0},
                      {   0,       0,     0,    0,0,0,0,0,0}},
#endif
/*        "Russia",*/{{ 47000,   68000, 7000, 0x402,0,0,0,0,0},
                      { 174000, 230000, 7000, 0xC05,0,0,0,0,0},
                      {470000,  862000, 8000,0x4595,0,0,0,0,0},
                      {   0,       0,     0,    0,0,0,0,0,0},
                      {   0,       0,     0,    0,0,0,0,0,0},
                      {   0,       0,     0,    0,0,0,0,0,0},
                      {   0,       0,     0,    0,0,0,0,0,0},
                      {   0,       0,     0,    0,0,0,0,0,0},
                      {   0,       0,     0,    0,0,0,0,0,0},
                      {   0,       0,     0,    0,0,0,0,0,0}},

/*        "Singapore",{{ 470000, 862000 , 8000 ,0x3181,0,0,0,0,0},0,0,0,0,0,0,0,0,0},*/
/*change on 2011-10-31 for BUG43315*/
/*        "Singapore",*/{{ 47000, 68000 , 7000 ,0x402,0,0,0,0,0},
                         { 174000, 230000 , 7000 ,0xC05,0,0,0,0,0},
                         { 470000, 862000 , 8000 ,0x3181,0,0,0,0,0},
                         {   0,       0,     0,    0,0,0,0,0,0},
                         {   0,       0,     0,    0,0,0,0,0,0},
                         {   0,       0,     0,    0,0,0,0,0,0},
                         {   0,       0,     0,    0,0,0,0,0,0},
                         {   0,       0,     0,    0,0,0,0,0,0},
                         {   0,       0,     0,    0,0,0,0,0,0},
                         {   0,       0,     0,    0,0,0,0,0,0}},

/*        "Spanish",*/{{ 47000, 68000 , 7000 ,0x402,0,0,0,0,0},
                       { 174000, 230000 , 7000 ,0xC05,0,0,0,0,0},
                       {470000,862000,8000,0x4595,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0}},

/*"Sweden"*/       {{  47000, 68000 , 7000, 0x402,0,0,0,0,0},
                    { 174000, 230000, 7000, 0xC05,0,0,0,0,0},
                    { 470000, 862000, 8000,0x4595,0,0,0,0,0},
                    {   0,       0,     0,    0,0,0,0,0,0},
                    {   0,       0,     0,    0,0,0,0,0,0},
                    {   0,       0,     0,    0,0,0,0,0,0},
                    {   0,       0,     0,    0,0,0,0,0,0},
                    {   0,       0,     0,    0,0,0,0,0,0},
                    {   0,       0,     0,    0,0,0,0,0,0},
                    {   0,       0,     0,    0,0,0,0,0,0}},

#ifdef ali_a00_m3330_model1
#if(ali_a00_m3330_model1_swver == ali_a00_m3330_chinese_mode)
{{470000, 806000,6000,0x458e,0,0,0,0,0},0,0,0,0,0,0,0,0,0},
#elif(ali_a00_m3330_model1_swver == ali_a00_m3330_english_mode)

{{ 54000,  72000, 6000, 0x402,0,0,0,0,0},
 { 76000,  88000, 6000, 0x605,0,0,0,0,0},
 {174000, 216000, 6000, 0xD07,0,0,0,0,0},
 {470000, 806000, 6000,0x458e,0,0,0,0,0},
 {   0,       0,     0,    0,0,0,0,0,0},
 {   0,       0,     0,    0,0,0,0,0,0},
 {   0,       0,     0,    0,0,0,0,0,0},
 {   0,       0,     0,    0,0,0,0,0,0},
 {   0,       0,     0,    0,0,0,0,0,0},
 {   0,       0,     0,    0,0,0,0,0,0}},
#endif
#else
/* "Taiwan"*/       {{ 54000, 72000, 6000, 0x402,0,0,0,0,0},
                     { 76000, 88000, 6000, 0x605,0,0,0,0,0},
                     {174000,216000, 6000, 0xD07,0,0,0,0,0},
                     {470000, 806000,6000,0x458e,0,0,0,0,0},
                     {   0,       0,     0,    0,0,0,0,0,0},
                     {   0,       0,     0,    0,0,0,0,0,0},
                     {   0,       0,     0,    0,0,0,0,0,0},
                     {   0,       0,     0,    0,0,0,0,0,0},
                     {   0,       0,     0,    0,0,0,0,0,0},
                     {   0,       0,     0,    0,0,0,0,0,0}},
#endif

/* "United Kingdom",{{ 470000, 862000 , 8000 ,0x4595,0,0,0,0,0},0,0,0,0,0,0,0,0,0},*/
/*change on 2011-10-31 update*/
/* "United Kingdom",*/{{ 174000, 254000 , 8000 ,0xD04,0,0,0,0,0},
                       { 470000, 862000 , 8000 ,0x4595,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0}},
/* "Iran",*/          {{ 174000, 230000, 7000, 0xc05,0,0,0,0,0},
                       { 470000, 862000, 8000,0x4595,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0}},

};


//*************The definition of fourth element in every discontinuous band********************
//              Bit15~8                     Bit7                    Bit6~0
//      end channel no. of this group band     band type      start channel no. of this group band
//******************************************************************************
//const UINT32 isdbt_countryband[MAX_COUNTRY][MAX_BAND_NUMBER][MAX_PARAM] __attribute__((section(".flash")))=
static const band_param isdbt_countryband[MAX_COUNTRY][MAX_BAND_COUNT] __attribute__((section(".flash")))=
{///stanley wang 20050621
  //    changed by SN  for Alphabetically arranged 20050818
/*       "Argntina",*/{{ 174000, 216000 , 6000 ,0xd07,0,0,0,0,0},
                       { 470000, 806000 , 6000 ,0x458e,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0}},
                       
/*       "Argntina",*/{{ 174000, 216000 , 6000 ,0xd07,0,0,0,0,0},
					   { 470000, 806000 , 6000 ,0x458e,0,0,0,0,0},
					   {   0,       0,     0,    0,0,0,0,0,0},
					   {   0,       0,     0,    0,0,0,0,0,0},
					   {   0,       0,     0,    0,0,0,0,0,0},
					   {   0,       0,     0,    0,0,0,0,0,0},
					   {   0,       0,     0,    0,0,0,0,0,0},
					   {   0,       0,     0,    0,0,0,0,0,0},
					   {   0,       0,     0,    0,0,0,0,0,0},
					   {   0,       0,     0,    0,0,0,0,0,0}},                       

//for test
/*       "Argntina",*/{{ 174000, 216000 , 6000 , 0xd07,0,0,0,0,0},
                       { 470000, 806000 , 6000 ,0x458e,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0}},
/*       "Argntina",*/{{ 174000, 216000 , 6000 ,0xd07,0,0,0,0,0},
                       { 470000, 806000 , 6000 ,0x458e,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0}},
/*       "Argntina",*/{{ 174000, 216000 , 6000 ,0xd07,0,0,0,0,0},
                       { 470000, 806000 , 6000 ,0x458e,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0}},
/*       "Argntina",*/{{ 174000, 216000, 6000, 0xd07,0,0,0,0,0},
                       { 470000, 806000, 6000,0x458e,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0}},
/*       "Argntina",*/{{ 174000, 216000 , 6000 ,0xd07,0,0,0,0,0},
                       { 470000, 806000 , 6000 ,0x458e,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0}},
/*       "Argntina",*/{{ 174000, 216000 , 6000 ,0xd07,0,0,0,0,0},
                       { 470000, 806000 , 6000 ,0x458e,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0}},
/*       "Argntina",*/{{ 174000, 216000 , 6000 ,0xd07,0,0,0,0,0},
                       { 470000, 806000 , 6000 ,0x458e,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0}},
/*       "Argntina",*/{{ 174000, 216000 , 6000 ,0xd07,0,0,0,0,0},
                       { 470000, 806000 , 6000 ,0x458e,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0}},
/*       "Argntina",*/{{ 174000, 216000 , 6000 ,0xd07,0,0,0,0,0},
                       { 470000, 806000 , 6000 ,0x458e,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0}},
/*       "Argntina",*/{{ 174000, 216000 , 6000 ,0xd07,0,0,0,0,0},
                       { 470000, 806000 , 6000 ,0x458e,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0}},
/*       "Argntina",*/{{ 174000, 216000 , 6000 ,0xd07,0,0,0,0,0},
                       { 470000, 806000 , 6000 ,0x458e,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0}},
/*       "Argntina",*/{{ 174000, 216000 , 6000 ,0xd07,0,0,0,0,0},
                       { 470000, 806000 , 6000 ,0x458e,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0}},
/*       "Argntina",*/{{ 174000, 216000 , 6000 ,0xd07,0,0,0,0,0},
                       { 470000, 806000 , 6000 ,0x458e,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0}},
/*       "Argntina",*/{{ 174000, 216000 , 6000 ,0xd07,0,0,0,0,0},
                       { 470000, 806000 , 6000 ,0x458e,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0}},
/*       "Argntina",*/{{ 174000, 216000 , 6000 ,0xd07,0,0,0,0,0},
                       { 470000, 806000 , 6000 ,0x458e,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0},
                       {   0,       0,     0,    0,0,0,0,0,0}},
/*       "Argntina",*/{{ 174000, 216000 , 6000 ,0xd07,0,0,0,0,0},
					   { 470000, 806000 , 6000 ,0x458e,0,0,0,0,0},
					   {   0,       0,     0,    0,0,0,0,0,0},
					   {   0,       0,     0,    0,0,0,0,0,0},
					   {   0,       0,     0,    0,0,0,0,0,0},
					   {   0,       0,     0,    0,0,0,0,0,0},
					   {   0,       0,     0,    0,0,0,0,0,0},
					   {   0,       0,     0,    0,0,0,0,0,0},
					   {   0,       0,     0,    0,0,0,0,0,0},
					   {   0,       0,     0,    0,0,0,0,0,0}},                       

///*       "Brazil",*/{{ 174000, 216000 , 6000 ,0x701,0,0,0,0,0},
//                        {470000, 806000,6000,0x458e,0,0,0,0,0},0,0,0,0,0,0,0,0},
};



void get_isdbt_bandparam(UINT8 index, band_param *buf)
{
    UINT32 addr = 0;

    if(index >= MAX_COUNTRY)
    {
        return;
    }
    addr = (UINT32)isdbt_countryband + index * (sizeof(band_param)*MAX_BAND_NUMBER);

    MEMCPY(buf, (void*)addr, (sizeof(band_param)*MAX_BAND_NUMBER));
}

void get_dvbt_bandparam(UINT8 index, band_param *buf)
{
    UINT32 addr = 0;

    if(index >= MAX_COUNTRY)
    {
        return;
    }
    addr = (UINT32)dvbt_countryband + index * (sizeof(band_param)*MAX_BAND_NUMBER);

    MEMCPY(buf, (void*)addr, (sizeof(band_param)*MAX_BAND_NUMBER));
}
#ifdef POLAND_SPEC_SUPPORT
void get_dvbt_bandparam_for_poland(band_param *buf)
{
    band_param buf1[MAX_BAND_COUNT];
    UINT16 i=0,
    UINT16 j=0;

    for(j=0;j<MAX_BAND_COUNT;j++)
    {
       if(0x0080==(buf[j].band_type & 0x0080))
       {
        buf1[i]=buf[j];
        i++;
       }
        else
        continue;
    }
    MEMSET(buf,0,(sizeof(band_param)*MAX_BAND_NUMBER));
    MEMCPY(buf, buf1, (sizeof(band_param)*MAX_BAND_NUMBER));

}
#endif
