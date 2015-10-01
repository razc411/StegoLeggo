#include "img_manip.h"


/*
  for each bit


 */

int main(int argc, char ** argv)
{
    size_t i, n;
    int type;
    FILE *image;
    uint32_t * pixels;
    struct BMP_FHDR fhdr;
    struct BITINFOHDR infohdr;
    char filename[1024];
    char path[1024];
    
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

    //grab filename
    sprintf(path, "/proc/self/fd/%d", image);
    memset(filename, 0, sizeof(filename));
    readlink(path, filename, sizeof(filename) - 1);
    
    fread(&fhdr, sizeof(struct BMP_FHDR), 1, image);
    fread(&infohdr, sizeof(struct BITINFOHDR), 1, image);

    printf("%d bits per pixel\n", infohdr.bits_per_pixel);

    type = checkbmp_type(&infohdr, &fhdr);

    if(infohdr.bits_per_pixel != 32)
    {
	printf("Must be a 32-bit image. Image is %d-bit\n", infohdr.bits_per_pixel);
	
    
    if(type == BMPINFO && (strcmp(argv[1], "encode") == 0))
    {
	pixels = grab_bmpinfo_pixels(&infohdr, image);
	//encode_data_basic(&infohdr, &fhdr, pixels, filename);
    }
    else
    {
	printf("Format not supported.\n");
	exit(2);
    }

    fclose(image);
}

int encode_data_basic(struct BITINFOHDR * infohdr, struct BMP_FHDR *fhdr, uint32_t * pixels, char * filename)
{
    //bit = (number >> x) & 1; checking a bit
    //number ^= (-x ^ number) & (1 << n); setting the n bit to x
    uint32_t c, i, n = 0, bit;
    char size[50];
    char echar;
    int esize;
    
    FILE * encode_data;
    if(!(encode_data = fopen(filename, "rb")))
    {
	printf("Failed to open data to encode.\n");
	exit(1);
    }

    fseek(encode_data, 0, SEEK_END);
    esize = ftell(encode_data);
    fseek(encode_data, 0, SEEK_SET);

    if(infohdr.image_size/BYTESIZE > esize)
    {
	printf("File to encode is too large.\nEncoded file size: %d\nImage Capacity: %d\n", esize, infohdr.image_size/BYTESIZE);
	exit(1);
    }

    //filename
    for(c = 0; c < strlen(filename) + 1; c++)
    {
	for(i = 0; i < BYTESIZE; i++)
	{
	    for(; i  < sizeof(uint32_t)/BYTESIZE; n++)
	    {
		bit = (filename[c] >> i) & 1;
		pixels[n] ^= (-bit ^ pixels[n]) & (1 << i);
	    }
	}
    }

    //encode size of file
    for(i = 0; i < BYTESIZE; i++)
    {
	for(; i  < sizeof(uint32_t)/BYTESIZE; n++)
	{
	    bit = (esize >> i) & 1;
	    pixels[n] ^= (-bit ^ pixels[n]) & (1 << i);
	}
    }
    pixels[++n] = NULL;

    //encode data
    while((echar = fgetc(encode_data)) != EOF)
    {
	for(i = 0; i < BYTESIZE; i++)
	{
	    for(; i  < sizeof(uint32_t)/BYTESIZE; n++)
	    {
		bit = (echar >> i) & 1;
		pixels[n] ^= (-bit ^ pixels[n]) & (1 << i);
	    }
	}
    }

    write_bmpi(fhdr, infohdr, pixels);
    free(pixels);
}

uint32_t * grab_bmpinfo_pixels(struct BITINFOHDR * infohdr, FILE * image)
{
    size_t i;
    size_t image_size = (infohdr->bmwidth_pixels * infohdr->bmheight_pixels);
    uint32_t * pixels = malloc(sizeof(uint32_t*) * image_size);
    for(i = 0; i < image_size; i++)
    {
	fread(&pixels[i], sizeof(uint32_t), 1, image);
    }

    return pixels;
}

int write_bmpi(struct BMP_FHDR * fhdr, struct BITINFOHDR * infohdr, uint32_t * pixels)
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
    fwrite(pixels, 1, sizeof(uint32_t) * infohdr->bmheight_pixels  * infohdr->bmwidth_pixels, img_writer);

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
