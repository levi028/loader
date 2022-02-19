
#ifndef _UTF_CONVERT_H_
#define _UTF_CONVERT_H_


/* Some fundamental constants */
#define UNI_REPLACEMENT_CHAR (UINT32)0x0000FFFD
#define UNI_MAX_BMP (UINT32)0x0000FFFF
#define UNI_MAX_UTF16 (UINT32)0x0010FFFF
#define UNI_MAX_UTF32 (UINT32)0x7FFFFFFF
#define UNI_MAX_LEGAL_UTF32 (UINT32)0x0010FFFF

typedef enum
{
    conversionOK,
    /* conversion successful */
    sourceExhausted,
    /* partial character in source, but hit end */
    targetExhausted,
    /* insuff. room in target for conversion */
    sourceIllegal           /* source sequence is illegal/malformed */
} ConversionResult;

typedef enum
{
    strictConversion    = 0,
    lenientConversion
} ConversionFlags;

/* This is for C++ and does no harm in C */
#ifdef __cplusplus
extern "C"
{
#endif

    ConversionResult ConvertUTF8toUTF16(const UINT8** sourceStart,
        const UINT8* sourceEnd, UINT16** targetStart, UINT16* targetEnd,
        ConversionFlags flags);

    ConversionResult ConvertUTF16toUTF8(const UINT16** sourceStart,
        const UINT16* sourceEnd, UINT8** targetStart, UINT8* targetEnd,
        ConversionFlags flags);

    ConversionResult ConvertUTF8toUTF32(const UINT8** sourceStart,
        const UINT8* sourceEnd, UINT32** targetStart, UINT32* targetEnd,
        ConversionFlags flags);

    ConversionResult ConvertUTF32toUTF8(const UINT32** sourceStart,
        const UINT32* sourceEnd, UINT8** targetStart, UINT8* targetEnd,
        ConversionFlags flags);

    ConversionResult ConvertUTF16toUTF32(const UINT16** sourceStart,
        const UINT16* sourceEnd, UINT32** targetStart, UINT32* targetEnd,
        ConversionFlags flags);

    ConversionResult ConvertUTF32toUTF16(const UINT32** sourceStart,
        const UINT32* sourceEnd, UINT16** targetStart, UINT16* targetEnd,
        ConversionFlags flags);

    BOOL isLegalUTF8Sequence(const UINT8* source, const UINT8* sourceEnd);

#ifdef __cplusplus
}
#endif

#endif
