/*****************************************************************************
 *    Copyrights(C) 2010 ALI Corp. All Rights Reserved.
 *
 *    FILE NAME:        ci_srm.c
 *
 *    DESCRIPTION:     CI+ parse verify and apply SRM data
 *
 *    HISTORY:
 *        Date         Author           Version       Notes
 *        =========    =========      =========      =========
 *        2010/6/28   Elliott          0.1
 *
 *****************************************************************************/

//SRM
#include "ci_srm.h"

#ifdef CI_PLUS_SUPPORT

static unsigned char prime_modulus[128] =
{
    0xd3, 0xc3, 0xf5, 0xb2, 0xfd, 0x17, 0x61, 0xb7, 0x01, 0x8d, 0x75, 0xf7, 0x93, 0x43, 0x78, 0x6b,
    0x17, 0x39, 0x5b, 0x35, 0x5a, 0x52, 0xc7, 0xb8, 0xa1, 0xa2, 0x4f, 0xc3, 0x6a, 0x70, 0x58, 0xff,
    0x8e, 0x7f, 0xa1, 0x64, 0xf5, 0x00, 0xe0, 0xdc, 0xa0, 0xd2, 0x84, 0x82, 0x1d, 0x96, 0x9e, 0x4b,
    0x4f, 0x34, 0xdc, 0x0c, 0xae, 0x7c, 0x76, 0x67, 0xb8, 0x44, 0xc7, 0x47, 0xd4, 0xc6, 0xb9, 0x83,
    0xe5, 0x2b, 0xa7, 0x0e, 0x54, 0x47, 0xcf, 0x35, 0xf4, 0x04, 0xa0, 0xbc, 0xd1, 0x97, 0x4c, 0x3a,
    0x10, 0x71, 0x55, 0x09, 0xb3, 0x72, 0x15, 0x30, 0xa7, 0x3f, 0x32, 0x07, 0xb9, 0x98, 0x20, 0x49,
    0x5c, 0x7b, 0x9c, 0x14, 0x32, 0x75, 0x73, 0x3b, 0x02, 0x8a, 0x49, 0xfd, 0x96, 0x89, 0x19, 0x54,
    0x2a, 0x39, 0x95, 0x1c, 0x46, 0xed, 0xc2, 0x11, 0x8c, 0x59, 0x80, 0x2b, 0xf3, 0x28, 0x75, 0x27
};

static unsigned char prime_divisor[20] =
{
    0xee, 0x8a, 0xf2, 0xce, 0x5e, 0x6d, 0xb5, 0x6a, 0xcd, 0x6d, 0x14, 0xe2, 0x97, 0xef, 0x3f, 0x4d,
    0xf9, 0xc7, 0x08, 0xe7
};

static unsigned char generator[128] =
{
    0x92, 0xf8, 0x5d, 0x1b, 0x6a, 0x4d, 0x52, 0x13, 0x1a, 0xe4, 0x3e, 0x24, 0x45, 0xde, 0x1a, 0xb5,
    0x02, 0xaf, 0xde, 0xac, 0xa9, 0xbe, 0xd7, 0x31, 0x5d, 0x56, 0xd7, 0x66, 0xcd, 0x27, 0x86, 0x11,
    0x8f, 0x5d, 0xb1, 0x4a, 0xbd, 0xec, 0xa9, 0xd2, 0x51, 0x62, 0x97, 0x7d, 0xa8, 0x3e, 0xff, 0xa8,
    0x8e, 0xed, 0xc6, 0xbf, 0xeb, 0x37, 0xe1, 0xa9, 0x0e, 0x29, 0xcd, 0x0c, 0xa0, 0x3d, 0x79, 0x9e,
    0x92, 0xdd, 0x29, 0x45, 0xf7, 0x78, 0x58, 0x5f, 0xf7, 0xc8, 0x35, 0x64, 0x2c, 0x21, 0xba, 0x7f,
    0xb1, 0xa0, 0xb6, 0xbe, 0x81, 0xc8, 0xa5, 0xe3, 0xc8, 0xab, 0x69, 0xb2, 0x1d, 0xa5, 0x42, 0x42,
    0xc9, 0x8e, 0x9b, 0x8a, 0xab, 0x4a, 0x9d, 0xc2, 0x51, 0xfa, 0x7d, 0xac, 0x29, 0x21, 0x6f, 0xe8,
    0xb9, 0x3f, 0x18, 0x5b, 0x2f, 0x67, 0x40, 0x5b, 0x69, 0x46, 0x24, 0x42, 0xc2, 0xba, 0x0b, 0xd9
};

static unsigned char public_key[128] =
{
    0xc7, 0x06, 0x00, 0x52, 0x6b, 0xa0, 0xb0, 0x86, 0x3a, 0x80, 0xfb, 0xe0, 0xa3, 0xac, 0xff, 0x0d,
    0x4f, 0x0d, 0x76, 0x65, 0x8a, 0x17, 0x54, 0xa8, 0xe7, 0x65, 0x47, 0x55, 0xf1, 0x5b, 0xa7, 0x8d,
    0x56, 0x95, 0x0e, 0x48, 0x65, 0x4f, 0x0b, 0xbd, 0xe1, 0x68, 0x04, 0xde, 0x1b, 0x54, 0x18, 0x74,
    0xdb, 0x22, 0xe1, 0x4f, 0x03, 0x17, 0x04, 0xdb, 0x8d, 0x5c, 0xb2, 0xa4, 0x17, 0xc4, 0x56, 0x6c,
    0x27, 0xba, 0x97, 0x3c, 0x43, 0xd8, 0x4e, 0x0d, 0xa2, 0xa7, 0x08, 0x56, 0xfe, 0x9e, 0xa4, 0x8d,
    0x87, 0x25, 0x90, 0x38, 0xb1, 0x65, 0x53, 0xe6, 0x62, 0x43, 0x5f, 0xf7, 0xfd, 0x52, 0x06, 0xe2,
    0x7b, 0xb7, 0xff, 0xbd, 0x88, 0x6c, 0x24, 0x10, 0x95, 0xc8, 0xdc, 0x8d, 0x66, 0xf6, 0x62, 0xcb,
    0xd8, 0x8f, 0x9d, 0xf7, 0xe9, 0xb3, 0xfb, 0x83, 0x62, 0xa9, 0xf7, 0xfa, 0x36, 0xe5, 0x37, 0x99
};

INT32 dcp_llc_verification(UINT8 *dsa_data, UINT32 dsa_data_len, UINT8 *dsa_r, UINT8 *dsa_s)
{
    DSA *d;
    DSA_SIG *sign;
    SHA_CTX ctx;
    int ret_dsa, i, signlen, dgst_len, nid;
    unsigned char dgst[20], signret[50];
    //unsigned char prime_modulus[128], prime_divisor[20], generator[128], public_key[128];
    //unsigned char data[5076], sign_r[20], sign_s[20];

    d=dsa_new();
    d->p=bn_new();
    d->q=bn_new();
    d->g=bn_new();
    d->pub_key=bn_new();

    //DSA parameters: bin to bignumber
    bn_bin2bn(prime_modulus, 128, d->p);
    bn_bin2bn(prime_divisor, 20, d->q);
    bn_bin2bn(generator, 128, d->g);
    bn_bin2bn(public_key, 128, d->pub_key);

    //SHA-1 digest
    dgst_len=20;
    nid=nid_sha1;//not work
    sha1_init(&ctx);
    sha1_update(&ctx, dsa_data, dsa_data_len);
    sha1_final(dgst, &ctx);

    //DSA parameter: bin to bignumber, then to bin
    sign = dsa_sig_new();
    sign->r = bn_new();
    sign->s = bn_new();
    bn_bin2bn(dsa_r, 20, sign->r);
    bn_bin2bn(dsa_s, 20, sign->s);
    ret_dsa = dsa_sig_to_bin(signret, &signlen, sign);
    if(ret_dsa == -1)
    {
        APPL_PRINTF("DSA r and s data transform err.");
        dsa_free(d);
        dsa_sig_free(sign);
        return !SUCCESS;
    }

    //DSA verify
    ret_dsa=dsa_verify(nid, dgst, dgst_len, signret, signlen, d);
    if(ret_dsa!=1)
    {
        APPL_PRINTF("DSA verify err.");
        dsa_free(d);
        dsa_sig_free(sign);
        return !SUCCESS;
    }

    APPL_PRINTF("DSA verify success.");
    dsa_free(d);
    dsa_sig_free(sign);

    return SUCCESS;
}

static UINT8 revocation_list[5000];
static UINT8 dsa_data[5076];
void api_ciplus_parse_apply_srm(UINT8 *srm_file, UINT32 srm_file_len)
{
    // Load HDCP SRM
    UINT8* psrm_head            = (UINT8*) srm_file;
    UINT32 srm_total_length     = srm_file_len;
    UINT32 srm_id, srm_resrvd, srm_version, srm_generation_number, srm_vrls_length;
    UINT32 vrls_buf_length;
    UINT8 *vrls_buf, *dcp_llc;

    UINT32 vrl_index = 0, i;
    UINT8 num_of_dev = 0;
    UINT32 hdcp_revocation_dev_num = 0;

    UINT8 dsa_r[20], dsa_s[20];
    UINT32 dsa_data_len;

    if(srm_total_length < 48 || srm_total_length > 5116)
        APPL_PRINTF("Invalid SRM Length: %d\n", srm_total_length);

    // Parsing SRM Header
    srm_id                  = (psrm_head[0]&0xF0)>>4;
    srm_resrvd              = (psrm_head[0]&0x0F)<<8 | psrm_head[1];
    srm_version             =  psrm_head[2]<<8 | psrm_head[3];
    srm_generation_number   =  psrm_head[4];
    srm_vrls_length         =  psrm_head[5]<<16 | psrm_head[6]<<8 | psrm_head[7];
    vrls_buf                =  (UINT8*)&psrm_head[8];
    vrls_buf_length         =  srm_vrls_length - (24+ 320)/8; // not include VRL Length, (24 bit), and DCP LLC Signature (320bit)
    dcp_llc                 =  (UINT8*)&vrls_buf[vrls_buf_length];

    APPL_PRINTF("SRM ID: 0x%.1x\n", srm_id);
    APPL_PRINTF("SRM Reserved: 0x%.3x\n", srm_resrvd);
    APPL_PRINTF("SRM Version: 0x%.4x\n", srm_version);
    APPL_PRINTF("SRM Generation Num: 0x%.2x\n", srm_generation_number);
    APPL_PRINTF("SRM VRL Length: 0x%.6x Bytes (Max 5068 Bytes)\n", srm_vrls_length);
    APPL_PRINTF("DCP LLC Signature: 0x");

    for( i=0; i<320/8; i++)
        APPL_PRINTF("%.2x", dcp_llc[i]);
    APPL_PRINTF("\n");

    //   TODO: Verify DCP LLC Signature Here
    dsa_data_len = srm_vrls_length + 40/8 - 320/8;
    MEMCPY(dsa_data, psrm_head, dsa_data_len);
    MEMCPY(dsa_r, dcp_llc, 20);
    MEMCPY(dsa_s, &dcp_llc[20], 20);
    dcp_llc_verification(dsa_data, dsa_data_len, dsa_r, dsa_s);

    num_of_dev = vrls_buf[0];

    // Parsing VRLs
    while( num_of_dev != 0 && vrls_buf_length!= 0)
    {
        APPL_PRINTF("Number of Devices = %d\n", num_of_dev);
        for( i=0; i<num_of_dev; i++ )
        {
            APPL_PRINTF("Dev %d KSV: 0x%.2x%.2x%.2x%.2x%.2x\n", i,
                        vrls_buf[i*5+1],vrls_buf[i*5+2],vrls_buf[i*5+3],vrls_buf[i*5+4],vrls_buf[i*5+5]);
            // SRM is Big Endian, ALi Revocation List is Little Endian
            revocation_list[hdcp_revocation_dev_num*5] = vrls_buf[i*5+5];
            revocation_list[hdcp_revocation_dev_num*5+1] = vrls_buf[i*5+4];
            revocation_list[hdcp_revocation_dev_num*5+2] = vrls_buf[i*5+3];
            revocation_list[hdcp_revocation_dev_num*5+3] = vrls_buf[i*5+2];
            revocation_list[hdcp_revocation_dev_num*5+4] = vrls_buf[i*5+1];
            hdcp_revocation_dev_num++;
        }
        vrls_buf_length -= (1+ num_of_dev*5);
        if(vrls_buf_length !=0)
        {
            vrls_buf = &vrls_buf[num_of_dev*5+1];
            num_of_dev = vrls_buf[0];
        }
        else
            num_of_dev = 0;
    }

    if(srm_total_length > srm_vrls_length + 5 )
    {
        //   TODO: Parsing Next Generation VRLs if exist
    }

    struct hdmi_device *hdmi_dev = (struct hdmi_device *)dev_get_by_type(NULL,HLD_DEV_TYPE_HDMI);
    hdmi_dev->io_control(hdmi_dev,HDMI_CMD_ADD_SRM_LIST, (UINT32)revocation_list, hdcp_revocation_dev_num);
}

#endif

