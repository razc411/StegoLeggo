#include "img_manip.h"


/*
  for each bit


 */

int main(int argc, char ** argv)
{
    size_t i, n;
    int type;
    FILE *image;
    unsigned char * pixels;
    struct BMP_FHDR fhdr;
    struct BITINFOHDR infohdr;
    char filename[1024];
    char path[1024];
    size_t size;
    
    if(argc < 2)
    {
	exit(1);
    }
    
    image = fopen(argv[2], "rwb");
    if(!image)
    {
	printf("Failed to open file %s.\n", argv[1]);
	exit(1);
    }
    
    fread(&fhdr, sizeof(struct BMP_FHDR), 1, image);
    fread(&infohdr, sizeof(struct BITINFOHDR), 1, image);
    printf("%d num colors\n", infohdr.num_colors);
    printf("%d bits per pixel\n", infohdr.bits_per_pixel);

    type = checkbmp_type(&infohdr, &fhdr);

    if(infohdr.bits_per_pixel != 32)
    {
	printf("Must be a 32-bit image. Image is %d-bit\n", infohdr.bits_per_pixel);
	
    }
    
    if(type == BMPINFO)
    {
	if(strcmp(argv[1], "encode") == 0)
	{
	    pixels = grab_bmpinfo_pixels(&infohdr, image);
	    encode_data_basic(&infohdr, &fhdr, pixels, argv[3]);
	}
	else if(strcmp(argv[1], "decode") == 0)
	{
	    pixels = grab_bmpinfo_pixels(&infohdr, image);
	    decode_data_basic(&infohdr, &fhdr, pixels);
	    printf("%d size\n", size);
	}
    }
    else
    {
	printf("Format not supported.\n");
	exit(2);
    }

    fclose(image);
}

int encode_data_basic(struct BITINFOHDR * infohdr, struct BMP_FHDR *fhdr, unsigned char * pixels, char * filename)
{
    //bit = (number >> x) & 1; checking a bit
    //number ^= (-x ^ number) & (1 << n); setting the n bit to x
    uint32_t c, i, n = 0, bit, p;
    unsigned char echar;
    int esize;
    int pixelbit;
    
    FILE * encode_data;
    if(!(encode_data = fopen(filename, "rb")))
    {
	printf("Failed to open data to encode.\n");
	exit(1);
    }

    fseek(encode_data, 0, SEEK_END);
    esize = ftell(encode_data);
    fseek(encode_data, 0, SEEK_SET);

    if(infohdr->image_size/BYTESIZE < esize)
    {
	printf("File to encode is too large.\nEncoded file size: %d\nImage Capacity: %d\n", esize, infohdr->image_size/BYTESIZE);
	exit(1);
    }

    //filename
    for(c = 0; c < strlen(filename) + 1; c++)
    {
    	for(p = 0; p < BYTESIZE; p++)
    	{
    	    pixels[n] = move_bit(filename[c], p, pixels[n], 0);
    	    n++;
    	}
    }

    //set the file size
    p = 0;
    for(c = 0; c < sizeof(uint32_t); c++)
    {
    	for(; p < INTBITSIZE; p++)
    	{
    	    pixels[n] = move_bit(esize, p, pixels[n], 0);
	    n++;
    	}
    }
    
    //set the null terminator
    for(p = 0; p < BYTESIZE; p++)
    {
	CLEAR_BIT(pixels[n], 0);
	n++;
    }

    //set the encode data
    for(c = 0; c < esize; c++)
    {
    	echar = fgetc(encode_data);
	for(p = 0; p < BYTESIZE; p++)
    	{
    	    pixels[n] = move_bit(echar, p, pixels[n], 0);
    	    n++;
    	}
    }

    write_bmpi(fhdr, infohdr, pixels);
    free(pixels);
}

int decode_data_basic(struct BITINFOHDR * infohdr, struct BMP_FHDR *fhdr, unsigned char * pixels)
{
    //bit = (number >> x) & 1; checking a bit
    //number ^= (-x ^ number) & (1 << n); setting the n bit to x
    uint32_t p, c, i, n = 0, bit;
    char echar;
    int esize;
    char filename[1000];

    memset(filename, 0, 1000);
    
    for(c = 0; c < MAXFNAME; c++)
    {
	for(p = 0; p < BYTESIZE; p++)
	{
	    filename[c] = move_bit(pixels[n], 0, filename[c], p);
	    n++;
	}
	printf("%c char\n", filename[c]);
	if(filename[c] == '\0')
	{
	    break;
	}
    }

    printf("filename = %s", filename);

    p = 0;
    for(c = 0; c < sizeof(uint32_t); c++)
    {
	for(; p < BYTESIZE; p++)
	{
	    pixels[n] = move_bit(pixels[n], 0, esize, p);
	    n++;
	}
    }

    printf("File size = %d", esize);
           
    /* for(c = 0; c < size; c++) */
    /* { */
    /* 	for(p = 0; p < BYTESIZE; p++) */
    /* 	{ */
    /* 	    pixels[n] = move_bit(echar, p, pixels[n], 0); */
    /* 	    n++; */
    /* 	} */
    /* } */
    
    //write_bmpi(fhdr, infohdr, pixels);
    free(pixels);
}

unsigned char * grab_bmpinfo_pixels(struct BITINFOHDR * infohdr, FILE * image)
{
    size_t i;
    unsigned char * pixels = malloc(infohdr->image_size);
    for(i = 0; i < infohdr->image_size; i++)
    {
	fread(&pixels[i], sizeof(unsigned char), 1, image);
    }

    return pixels;
}

int write_bmpi(struct BMP_FHDR * fhdr, struct BITINFOHDR * infohdr, unsigned char * pixels)
{
    FILE *img_writer;
    img_writer = fopen("newimg.bmp", "wb");
    if(!img_writer)
    {
	printf("Failed to open file.");
	exit(1);
    }

    fwrite(fhdr, 1, sizeof(struct BMP_FHDR), img_writer);
    fwrite(infohdr, 1, sizeof(struct BITINFOHDR), img_writer);
    fwrite(pixels, 1, infohdr->image_size, img_writer);

    fclose(img_writer);
}

int checkbmp_type(struct BITINFOHDR * infohdr, struct BMP_FHDR * fhdr)
{
    //checking if the header is actually a bitmap
    if(fhdr->hdr[0] != 'B' && fhdr->hdr[1] != 'M')
    {
	printf("File must be a bitmap, aborting.\n");
	exit(1);
    }

    //checking the bitmap header type
    int remainder;
    if((remainder = (fhdr->size - sizeof(struct BMP_FHDR) - infohdr->hdr_size)) == infohdr->image_size)
    {
	 return BMPINFO;
    }

    printf("This bitmap format is not supported.\n");
    return 0;
}
