/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File:     string.h
*
*    Description:This file contains all functions definition of string and memory operations.

*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef __LIB_C_STRING_H__
#define __LIB_C_STRING_H__
#include <types.h>

#ifdef MEMCPY
#undef MEMCPY
#endif
#define MEMCPY     memcpy

#ifdef MEMCMP
#undef MEMCMP
#endif
#define MEMCMP(buf1,buf2, len)          ali_memcmp(buf1,buf2, len)

#ifdef MEMSET
#undef MEMSET
#endif
#define MEMSET(dest, c, len)            ali_memset(dest, c, len)

#ifdef MEMMOVE
#undef MEMMOVE
#endif
#define MEMMOVE(dest, src, len)         ali_memmove(dest, src, len)

#ifdef STRCPY
#undef STRCPY
#endif
#define STRCPY(dest,src)                ali_strcpy(dest,src)

#ifdef STRCMP
#undef STRCMP
#endif
#define STRCMP(dest,src)                ali_strcmp(dest,src)

#ifdef STRLEN
#undef STRLEN
#endif
#define STRLEN(str)                     ali_strlen(str)

#ifdef ITOA
#undef ITOA
#endif
#define ITOA(str, val)               ali_itoa(str, val)

#ifdef ATOI
#undef ATOI
#endif
#define ATOI(str)                    ali_atoi(str)

#ifdef ATOI16
#undef ATOI16
#endif
#define ATOI16(str)                  ali_atoi16(str)

#ifdef RAND
#undef RAND
#endif
#define RAND(max_val)            ali_rand(max_val)

#ifdef __cplusplus
extern "C"
{
#endif

/**
* Name  :    ali_memcpy
* Description:  Copies len bytes of src to dest..
* Parameters : 
*		dest : IN, Pointer to destination
*		src  : IN, Buffer to copy from.
*		len  : IN, Number of bytes to copy.
* Return value: 
*   The value of dest.
*/
void* ali_memcpy(void *dest,const void *src, unsigned int len);

/**
* Name  :    ali_memset
* Description:  Sets the first len chars of dest to the character c.
* Parameters : 
*		dest : IN/OUT, Pointer to destination
*		c    : IN, Character to set.
*		len  : IN, Number of characters.
* Return value: 
*   The value of dest.
*/
void* ali_memset(void *dest, int  c,unsigned int  len);

/**
* Name  :    ali_memcmp
* Description:  Compares the first count bytes of buf1 and buf2 and
				returns a value indicating their relationship.
* Parameters : 
*		buf1 : IN, The first buffer.
*		buf2 : IN, The second buffer.
*		len  : IN, Number of bytes.
* Return value: 
*    	   =0: buf1 identical to buf2.
*   	   <0: buf1 less than buf2.
*		   >0: buf1 greater than buf2.
*/
int ali_memcmp(const void *buf1,const void *buf2,unsigned int  len);

/**
* Name  :    ali_memmove
* Description:  Copies count bytes of characters from src to dest.	
* Parameters : 
*		dest : IN/OUT, Destination object.
*		src  : IN, Source object.
*		n    : IN, Number of bytes of characters to copy.
* Return value: 
* 		The value of dest.
*/
void *ali_memmove(void *dest,const void *src, unsigned int  n);

/**
* Name  :    ali_strcpy
* Description:  This function copies src, including the terminating null character, 
			    to the location specified by dest.
* Parameters : 
*		dest : IN/OUT, Destination string.
*		src  : IN, Null-terminated source string.
* Return value: 
* 		Each of these functions returns the destination string. 
*		No return value is reserved to indicate an error.
*/
char* ali_strcpy(char *dest, const char *src);

/**
* Name  :    ali_strcmp
* Description:  This function compares dest and src lexicographically and 
				returns a value indicating their relationship.
* Parameters : 
*		dest : IN, Null-terminated strings to compare.
*		src  : IN, Null-terminated strings to compare.
* Return value: 
* 	<0: dest less than src.
*   =0: dest identical to src.
*   >0: dest greater than src.
*/
int ali_strcmp(const char *dest, const char *src);

/**
* Name  :    ali_strlen
* Description:  This function returns the number of characters in string, 
				not including the terminating null character.
* Parameters : 
*		src  : IN, Null-terminated source string.
* Return value: 
* 	Each of these functions returns the number of characters in string, 
*   excluding the terminal NULL. No return value is reserved to indicate an error.
*/
unsigned long ali_strlen(const char *src);
char *ali_strdup(char const *str);

/**
* Name  :    ali_strlen
* Description:  This function convert the digits of the given value argument to
				a null-terminated character string and stores the result in string. 
* Parameters : 
*		str  : OUT, String result.
*		val  : IN, Number to be converted.
* Return value: 
* 		Returns a pointer to string. There is no error return.
*/
int ali_itoa(char *str, unsigned long val);

/**
* Name  :    ali_strlen
* Description:  This function convert a character string to an integer value. 
* Parameters : 
*		str  : IN, String to be converted.  
* Return value: 
* 		The function returns the integer value produced by interpreting the input characters as a number.
*		The return value is 0 if the input cannot be converted to a value of that type.
*/
int ali_atoi(const char *str);

/**
* Name  :    ali_strlen
* Description:  This function convert a character string in the hex value format to an integer value.
* Parameters : 
*		str  : IN, String to be converted.  
* Return value: 
* 		The function returns the integer value produced by interpreting the input characters as a number.
*		The return value is 0 if the input cannot be converted to a value of that type.
*/
unsigned long ali_atoi16(const char *str);

/**
* Name  :    ali_strlen
* Description:  This function convert a character string in atoi/atoi16 to an integer value.
* Parameters : 
*		str  : IN, String to be converted.  
* Return value: 
* 		The function returns the integer value produced by interpreting the input characters as a number.
*		The return value is 0 if the input cannot be converted to a value of that type.
*/
unsigned long com_atoi(char *str);


//porting functions ali_memcpy
void* libc_memcpy(void *dest,const void *src, unsigned int len);
void* libc_memset(void *dest, int  c,unsigned int  len);
int libc_memcmp(const void *buf1,const void *buf2,unsigned int  len);
void * libc_memmove(void *dest,const void *src, unsigned int  n);


char* libc_strcpy(char *dest, const char *src);
int libc_strcmp(const char *dest, const char *src);
unsigned long libc_strlen(const char *src);

int libc_itoa(char *str, unsigned long val);
int libc_atoi(const char *str);
unsigned long libc_atoi16(const char *str);


int rand_r(unsigned int *seed);
void srand(unsigned int i);
int rand(void);
UINT32 ali_rand(UINT32 max_val);

UINT32 libc_rand(UINT32 max_val);

double strtod(const char *string, char **end_ptr);

double atof(const char *nptr);
size_t strspn(const char *s, const char *accept);
size_t strcspn(const char *s, const char *reject);
char*strtok_r(char *s,const char *delim,char **ptrptr);
char *strtok(char *s, const char *delim);

#ifndef __mips16
//static void MemSet4(void *dest, int c, UINT32 len);
//static void MemCopy4(void *dest, const void *src, UINT32 len);
//void *ali_memcpy(void *dest, const void *src, unsigned int len);
//void *ali_memset(void *dest, int c, unsigned int  len);
/**
* Name  :    memcpy
* Description:  Copies len bytes of src to dest..
* Parameters : 
*		dest : IN, Pointer to destination
*		src  : IN, Buffer to copy from.
*		len  : IN, Number of bytes to copy.
* Return value: 
*   The value of dest.
*/
void* memcpy(void *dest, const void *src, unsigned int len);// __attribute__((alias("ali_memcpy")));

/**
* Name  :    memset
* Description:  Sets the first len chars of dest to the character c.
* Parameters : 
*		dest : IN/OUT, Pointer to destination
*		c    : IN, Character to set.
*		len  : IN, Number of characters.
* Return value: 
*   The value of dest.
*/
void* memset(void *dest, int c, unsigned int  len);// __attribute__((alias("ali_memset")));
#else
//void *ali_memcpy(void *dest, const void *src, unsigned int len);
void* memcpy(void *dest, const void *src, unsigned int len);
//void* ali_memset(void *dest, int c, unsigned int  len);
void* memset(void *dest, int c, unsigned int  len);
#endif

#ifdef NEW_CLIB_FUNCTION
int my_tolower(char ch);
int memicmp(const void *buffer1,const void *buffer2,int count);
#endif

/**
* Name  :    memcmp
* Description:  Compares the first count bytes of buf1 and buf2 and
				returns a value indicating their relationship.
* Parameters : 
*		buf1 : IN, The first buffer.
*		buf2 : IN, The second buffer.
*		len  : IN, Number of bytes.
* Return value: 
*    	   =0: buf1 identical to buf2.
*   	   <0: buf1 less than buf2.
*		   >0: buf1 greater than buf2.
*/
int memcmp(const void *buf1, const void *buf2, unsigned int  len); // __attribute__((alias("ali_memcmp")));

/**
* Name  :    memmove
* Description:  Copies count bytes of characters from src to dest.	
* Parameters : 
*		dest : IN/OUT, Destination object.
*		src  : IN, Source object.
*		n    : IN, Number of bytes of characters to copy.
* Return value: 
* 		The value of dest.
*/
void * memmove(void *dest, const void *src, unsigned int n); // __attribute__((alias("ali_memmove")));

/**
* Name  :    strcpy
* Description:  This function copies src, including the terminating null character, 
			    to the location specified by dest.
* Parameters : 
*		dest : IN/OUT, Destination string.
*		src  : IN, Null-terminated source string.
* Return value: 
* 		Each of these functions returns the destination string. 
*		No return value is reserved to indicate an error.
*/
char *strcpy(char *dest, const char *src);// __attribute__((alias("ali_strcpy")));

/**
* Name  :    strlen
* Description:  This function returns the number of characters in string, 
				not including the terminating null character.
* Parameters : 
*		src  : IN, Null-terminated source string.
* Return value: 
* 	Each of these functions returns the number of characters in string, 
*   excluding the terminal NULL. No return value is reserved to indicate an error.
*/
unsigned long strlen(const char *src);// __attribute__((alias("ali_strlen")));

/**
* Name  :    atoi
* Description:  This function convert a character string to an integer value. 
* Parameters : 
*		str  : IN, String to be converted.  
* Return value: 
* 		The function returns the integer value produced by interpreting the input characters as a number.
*		The return value is 0 if the input cannot be converted to a value of that type.
*/
int atoi(const char *str);// __attribute__((alias("ali_atoi")));

/**
* Name  :    strcmp
* Description:  This function compares dest and src lexicographically and 
				returns a value indicating their relationship.
* Parameters : 
*		dest : IN, Null-terminated strings to compare.
*		src  : IN, Null-terminated strings to compare.
* Return value: 
* 	<0: dest less than src.
*   =0: dest identical to src.
*   >0: dest greater than src.
*/
int strcmp(const char *s1, const char *s2);// __attribute__((alias("ali_strcmp")));

///////////////////////////////////////////////
//declare standard C functions(libc.a)
long atol(const char *);

#define strdup(s)	ali_strdup(s)

/**
* Name  :    memchr
* Description:  This function find out the first ch from count bytes of specify buffer.
* Parameters : 
*		str  : IN, String to be converted.  
* Return value: 
* 		the pointer to the first ch if success.
*       else return NULL.
*/
void *memchr(const void *, int, unsigned long);

//declare standard C functions(libc.a)
char *strcat(char *, const char *);
//declare standard C functions(libc.a)
char *strchr(const char *, int);
//declare standard C functions(libc.a)
char *strncpy(char *, const char *, int);
//declare standard C functions(libc.a)
char *strncat(char *, const char *, unsigned long);
//declare standard C functions(libc.a)
unsigned long strlcat(char *, const char *, unsigned long);
//declare standard C functions(libc.a)
int strncmp(const char *, const char *, unsigned long);
//declare standard C functions(libc.a)
char *strrchr(const char *, int);
//declare standard C functions(libc.a)
char *strstr(const char *, const char *);
//declare standard C functions(libc.a)
int strcasecmp(const char *, const char *);
//declare standard C functions(libc.a)
int strncasecmp(const char *, const char *, unsigned long);
//declare standard C functions(libc.a)
long strtol(const char *, char **, int);
//declare standard C functions(libc.a)
unsigned long strtoul(const char *, char **, int);


#ifdef isinf
#undef isinf
#endif
//isinf
int isinf(double);

#ifdef isnan
#undef isnan
#endif
//isnan
int isnan(double);
//isnan
int tolower(int);
//toascii
int toascii(int);
//toupper
int toupper(int);
//alloca
void *alloca(unsigned long);
//labs
long labs(long);
//abs
int abs(int);
//isalpha
int isalpha(int);
///////////////////////////////////////////////


#ifdef __cplusplus
}
#endif

#endif /* __LIB_C_STRING_H__ */
