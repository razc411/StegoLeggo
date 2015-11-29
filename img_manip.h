/* Header of img_manip.h
   Contains the structures and function headers for img_manip.c.   
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <mcrypt.h>
#include <unistd.h>
#include <fcntl.h>

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
#define HEADER 0
#define DATA 1
#define MAXFNAME 1000
#define MAXSIZE 100

int decrypt_data(char ** buffer, int buffer_len);
void genRandom(void * buffer, int buffer_len);
int encrypt_data(char * algorithm, char ** buffer, int buffer_len);
int writeKeyToFile(char * IV, int iv_len,  char * key, int key_len, char * algorithm);
void insert_encode_data(char * data);
char * grab_decode_header(int maxsize, int headerflag);
void write_bmpi(struct BMP_FHDR * fhdr, struct BITINFOHDR * infohdr);
void grab_bmpinfo_pixels(struct BITINFOHDR * infohdr, FILE * image);
int checkbmp_type(struct BITINFOHDR * infohdr, struct BMP_FHDR * fhdr);
void encode_data_basic(char * algorithm, struct BITINFOHDR * infohdr, struct BMP_FHDR *fhdr, char * filename);
void decode_data_basic(char * output);
char move_bit(char c1, int from, char c2, int to);

