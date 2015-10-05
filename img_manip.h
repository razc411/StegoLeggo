#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#pragma pack(push,1)
struct BMP_FHDR
{
    unsigned char hdr[2];
    uint32_t size;
    uint32_t reserv_1;
    uint32_t foffset;
};
#pragma pack(pop)

#pragma pack(push,1)
struct BITMAPV5HDR
{
    uint32_t hdr_size;
    uint32_t bmwidth_pixels;
    uint32_t bmheight_pixels;
    uint16_t num_cpanes;
    uint16_t bits_per_pixel;
    uint32_t compression_mthd;
    uint32_t image_size;
    uint32_t xpm; //x per meter
    uint32_t ypm; //y per meter
    uint32_t colortable_count;
    uint32_t iccount;
    uint32_t red_bitmask;
    uint32_t green_bitmask;
    uint32_t blue_bitmask;
    uint32_t alpha_bitmask;
    uint32_t cspace_type;
    uint32_t cs_endpoints;
    uint32_t red_gamma;
    uint32_t green_gamma;
    uint32_t blue_gamma;
    uint32_t intent;
    uint32_t icc_data_size;
    uint32_t icc_profile_size;
    uint32_t reserv_2;
};
#pragma pack(pop)

#pragma pack(push,1)
struct BITMAPV4HDR
{
    uint32_t hdr_size;
    uint32_t bmwidth_pixels;
    uint32_t bmheight_pixels;
    uint16_t num_cpanes;
    uint16_t bits_per_pixel;
    uint32_t compression_mthd;
    uint32_t image_size;
    uint32_t bmheight;
    uint32_t bmwidth;
    uint32_t num_colors;
    uint32_t num_colors_imp;
    uint32_t red_channel;
    uint32_t green_channel;
    uint32_t blue_channel;
    uint32_t alpha_channel;
    uint32_t type;
    uint32_t cs_endpoints;
    uint32_t red_gamma;
    uint32_t green_gamma;
    uint32_t blue_gamma;
};
#pragma pack(pop)

#pragma pack(push,1)
struct BITINFOHDR
{
    uint32_t hdr_size;
    uint32_t bmwidth_pixels;
    uint32_t bmheight_pixels;
    uint16_t num_cpanes;
    uint16_t bits_per_pixel;
    uint32_t compression_mthd;
    uint32_t image_size;
    uint32_t bmheight;
    uint32_t bmwidth;
    uint32_t num_colors;
    uint32_t num_colors_imp;
};
#pragma pack(pop)


#define BYTESIZE 8
#define BMPINFO 1
#define BMPV4   2
#define BMPV5   3
#define HEADER 0
#define DATA 1
#define MAXFNAME 1000
#define MAXSIZE 100
#define INTBITSIZE 32
#define CLEAR_BIT(number, x) (number &= ~(1 << x))

unsigned char * insert_encode_data(unsigned char * pixels, unsigned char * data);
unsigned char * grab_decode_header(unsigned char * pixels, int maxsize, int headerflag);
int write_bmpi(struct BMP_FHDR * fhdr, struct BITINFOHDR * infohdr, unsigned char *  pixels);
unsigned char * grab_bmpinfo_pixels(struct BITINFOHDR * infohdr, FILE * image);
int checkbmp_type(struct BITINFOHDR * infohdr, struct BMP_FHDR * fhdr);
void encode_data_basic(struct BITINFOHDR * infohdr, struct BMP_FHDR *fhdr, unsigned char * pixels, char * filename);
void decode_data_basic(struct BITINFOHDR * infohdr, struct BMP_FHDR *fhdr, unsigned char * pixels, char * output);

unsigned char move_bit(unsigned char c1, int from, unsigned char c2, int to) /*  */
{
    int bit;
    bit = (c1 >> from) & 1;            /* Get the source bit as 0/1 value */
    c2 &= ~(1 << to);                  /* clear destination bit */
    return (unsigned char)(c2 | (bit << to));  /* set destination bit */
}

