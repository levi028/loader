#include <sys_config.h>
#include <hld/nim/nim_dev.h>

#if(SYS_SDRAM_SIZE != 2) && defined(DISEQC_SUPPORT)

#include <types.h>
#include <retcode.h>
#include <osal/osal.h>
#include <osal/osal_timer.h>
#include <hld/hld_dev.h>
#include <hld/hld_dev.h>
#include <hld/nim/nim.h>
#include <api/libc/printf.h>
#include <api/libdiseqc/lib_diseqc.h>

#if(DISEQC_MOTOR_TYPE == DISEQC_MOTOR_USALS)
#include <math.h>

#if (USE_FIXED_MATH_FUNCS)
//#include "t2math.h"
#define word16  short         //16bit signed
#define word32  long          //32bit signed

word16 sin(word16 angle);
word16 cos(word16 angle);
word16 acos(word16 x);
word16 atan2(word16 x, word16 y);
word16 sqrt(word16 x);
#endif /*USE_FIXED_MATH_FUNCS==1*/

#define RADIUS_EARTH   6378.0   /* radius of earth 6378 Km */
#define RADIUS_ORBIT   42164.2   /* radius of geo-stationary orbit 42164.2 Km */
#define PI 3.141592654

// orbit:    0 ~ 3600   (0~1800: east  1801~3600: west)
// return:  -180 ~ 180  (0~180: east  -180~0: west)
double api_usals_get_sat_orbit(UINT32 orbit)
{
    double sat_orbit;

    sat_orbit = orbit&0xffff;

    if(sat_orbit > 1800)            //checking if the value exceeds east 180 degrees
        sat_orbit = sat_orbit - 3600;

    sat_orbit = sat_orbit /10.0;        //Longitude of the satellite

    return sat_orbit;
}

#if (USE_FIXED_MATH_FUNCS)
static double SQRT(double x)
{
    double xx,scale,tt;

    if(x>=0 && x<100) {
        tt = 10.0;
        scale = 100.0;
    } else if (x>=100 && x<10000) {
        tt = 100.0;
        scale = 10000.0;
    }else if (x>=10000 && x<1000000) {
        tt = 1000.0;
        scale = 1000000.0;
    }else if (x>=1000000 && x<100000000) {
        tt = 10000.0;
        scale = 100000000.0;
    }else if (x>=100000000 && x<10000000000) {
        tt = 100000.0;
        scale = 10000000000.0;
    } else {
        // Error ###
        return (1000000.0);
    }

    xx = x*0x7fff/scale;

    xx= (float)sqrt((word16)xx)/(float)(0x7fff);

    return (xx*tt);
}
#endif /*USE_FIXED_MATH_FUNCS==1*/

//  sat_orbit:             -180 ~ 180
//  local_longitude:     -180 ~ 180
//  local_latitude:         -90  ~ 90
//  return:                     -65 ~ 65 otherwise error
double api_usals_target_formula_calcuate(double sat_orbit, double local_longitude, double local_latitude)
{
     double result_dsq13_value;
    double ocb, p_p1, c_p1, p1_h, ph, ch, bh, pb, op, ob, dummy, xx;
    INT32 i;
    UINT16 data_buf[] = { 0xE0, 0x31, 0x6E, 0xE0, 0x00 };
    UINT8 data __MAYBE_UNUSED__;

#if (USE_FIXED_MATH_FUNCS)
    float fixed_temp;
    double fixed_temp_d0,fixed_temp_d1;
#endif /*USE_FIXED_MATH_FUNCS==1*/

    if(fabs(sat_orbit) > 180.0 || fabs(local_longitude) > 180.0 || fabs(local_latitude) > 90.0)         //fabs ==> double absolute value
    {
        result_dsq13_value = 300.0;
        return result_dsq13_value;
    }

    ocb = fabs(sat_orbit - local_longitude);

    if(ocb > 180.0) ocb = -(360.0 - ocb);

    if(fabs(ocb) > 65.0)     //65.0    //The range DiSEqC can move was set at 37.5 degree.
    {
        result_dsq13_value = 300.0;
        return result_dsq13_value;
    }

#if (USE_FIXED_MATH_FUNCS)
    //p_p1 = RADIUS_EARTH * sin(PI * local_latitude / 180.0);
    fixed_temp =  local_latitude *(float)0x4000/90.0;
    p_p1 = RADIUS_EARTH * (double)(sin((word16)fixed_temp))/(double)(0x7fff);

    //c_p1 = sqrt(pow(RADIUS_EARTH, 2) - pow(p_p1, 2));
    fixed_temp_d0 = RADIUS_EARTH*RADIUS_EARTH;
    fixed_temp_d1 = p_p1*p_p1;
    c_p1 = SQRT(fixed_temp_d0-fixed_temp_d1);

    //p1_h = c_p1 * sin(PI * ocb / 180.0);
    fixed_temp =  ocb *(float)0x4000/90.0;
    p1_h = c_p1 * (double)(sin((word16)fixed_temp))/(double)(0x7fff);

    //ph = sqrt(pow(p_p1, 2) + pow(p1_h, 2));
    fixed_temp_d0 = p_p1*p_p1;
    fixed_temp_d1 = p1_h*p1_h;
    ph = SQRT(fixed_temp_d0-fixed_temp_d1);

    //ch = sqrt(pow(RADIUS_EARTH, 2) - pow(ph, 2));
    fixed_temp_d0 = RADIUS_EARTH*RADIUS_EARTH;
    fixed_temp_d1 = ph*ph;
    ch = SQRT(fixed_temp_d0-fixed_temp_d1);

    bh = RADIUS_ORBIT - ch;

    //pb = sqrt(pow(bh, 2) + pow(ph, 2));
    fixed_temp_d0 = bh*bh;
    fixed_temp_d1 = ph*ph;
    pb = SQRT(fixed_temp_d0-fixed_temp_d1);

    //op = sqrt(pow(RADIUS_ORBIT - c_p1, 2) + pow(p_p1, 2));
    fixed_temp_d0 = (RADIUS_ORBIT - c_p1)*(RADIUS_ORBIT - c_p1);
    fixed_temp_d1 = p_p1*p_p1;
    op = SQRT(fixed_temp_d0-fixed_temp_d1);

    //ob = sqrt((2 * pow(RADIUS_ORBIT, 2)) - (2 * pow(RADIUS_ORBIT, 2)) * cos(PI * ocb / 180.0));
    fixed_temp_d0 = 2*RADIUS_ORBIT*RADIUS_ORBIT;
    fixed_temp =  ocb *(float)0x4000/90.0;
    ob = SQRT(fixed_temp_d0-fixed_temp_d0*(double)(cos((word16)fixed_temp))/(double)(0x7fff));

    //dummy = acos((pow(pb, 2) + pow(op, 2) - pow(ob, 2)) / ( 2.0 * pb * op));
    fixed_temp_d0 = (pb*pb + op*op - ob*ob)/ ( 2.0 * pb * op);
    fixed_temp_d0 = fixed_temp_d0*0x7fff;
    dummy = (double)(acos((word16)fixed_temp_d0))*PI/((double)(0x4000))/2.0;

    xx = dummy * 180.0 / PI;

#else /*USE_FIXED_MATH_FUNCS==1*/
    p_p1 = RADIUS_EARTH * sin(PI * local_latitude / 180.0);
    c_p1 = sqrt(pow(RADIUS_EARTH, 2) - pow(p_p1, 2));
    p1_h = c_p1 * sin(PI * ocb / 180.0);
    ph = sqrt(pow(p_p1, 2) + pow(p1_h, 2));
    ch = sqrt(pow(RADIUS_EARTH, 2) - pow(ph, 2));
    bh = RADIUS_ORBIT - ch;
    pb = sqrt(pow(bh, 2) + pow(ph, 2));
    op = sqrt(pow(RADIUS_ORBIT - c_p1, 2) + pow(p_p1, 2));
    ob = sqrt((2 * pow(RADIUS_ORBIT, 2)) - (2 * pow(RADIUS_ORBIT, 2)) * cos(PI * ocb / 180.0));
    dummy = acos((pow(pb, 2) + pow(op, 2) - pow(ob, 2)) / ( 2.0 * pb * op));
    xx = dummy * 180.0 / PI;
#endif /*USE_FIXED_MATH_FUNCS==1*/
    if(fabs(xx) > 65.0)
    {
        for( i=0; i < 5; i++ )
        {
            data = data_buf[i];
        }
        result_dsq13_value = 300.0;
        return result_dsq13_value;
    }

    xx = (float)((int)((xx + 0.05) * 10)) / 10.0;
    if(((sat_orbit < local_longitude) && (local_latitude >= 0.)) ||  ((sat_orbit > local_longitude) && (local_latitude < 0.)))     ;
    else xx = -xx;

    if((sat_orbit < -89.9) && (local_longitude > 89.9)) xx = -xx;
    if((sat_orbit > 89.9) && (local_longitude < -89.9)) xx = -xx;

    result_dsq13_value = xx;

    return result_dsq13_value;
}

UINT16 api_usals_xx_to_wxyz(double degree)
{
    UINT16 degree_int;
    UINT8 d;
    float temp;
    UINT16 result;

    if (fabs(degree) >= 256)
    {
        result = 0xFFFF;
        return result;
    }
    if(degree<0)
        {
        degree = -degree;
        result = 0xe000;
        }
    else
        result = 0xd000;
    temp = (float)degree*10;
    degree_int = (UINT16)temp;

    d = (UINT8)(degree_int/160);
    result |= (d<<8);

    degree_int -= d*160;
    d = (UINT8)(degree_int/10);
    result |= (d<<4);

    degree_int -=d*10;

    switch(degree_int)
    {
        case 0:
            result |= 0x00;
            break;
        case 1:
            result |= 0x02;
            break;
        case 2:
            result |= 0x03;
            break;
        case 3:
            result |= 0x05;
            break;
        case 4:
            result |= 0x06;
            break;
        case 5:
            result |= 0x08;
            break;
        case 6:
            result |= 0x0a;
            break;
        case 7:
            result |= 0x0b;
            break;
        case 8:
            result |= 0x0d;
            break;
        case 9:
            result |= 0x0e;
            break;
        default:
            break;
    }
    return result;
}
#endif

#endif

