#include "img_manip.h"

int main(int argc, char ** argv)
{
    size_t i, n;
    int type;
    FILE *image;
    uint32_t ** pixels = 0;
    struct BMP_FHDR fhdr;
    struct BITINFOHDR infohdr;
    
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

    type = checkbmp_type(&infohdr, &fhdr);

    if(type == BMPINFO && (strcmp(argv[1], "encode") == 0))
    {
	grab_bmpinfo_pixels(&infohdr, image, &pixels);
	//encode_data_basic(&infohdr, &fhdr, pixels);
    }
    else
    {
	printf("Format not supported.\n");
	exit(2);
    }

    write_bmpi(&fhdr, &infohdr, &pixels);
    fclose(image);
}

int encode_data_basic(struct BITINFOHDR * infohdr, struct BMP_FHDR *fhdr, uint32_t *** pixels)
{
    //bit = (number >> x) & 1; checking a bit
    //number ^= (-x ^ number) & (1 << n); setting the n bit to x
    uint32_t c, r, i;
    uint32_t bit;
    uint32_t  tempbyte = 0;
    for(c = 0; c < infohdr->bmheight_pixels; c++)
    {
	for(r = 0; r < infohdr->bmwidth_pixels; r++)
	{
	    for(i = 0; i < BYTESIZE; i++)
	    {
		bit = (*pixels[c][r] >> i) & 1;
	        *pixels[c][r] ^= (-bit ^ *pixels[c][r]) & (1 << i);
	    }
	}
    }

    write_bmpi(fhdr, infohdr, pixels);
}

void grab_bmpinfo_pixels(struct BITINFOHDR * infohdr, FILE * image, uint32_t *** pixels)
{
    uint32_t i, n;
    *pixels = malloc(sizeof(uint32_t*) * infohdr->bmheight_pixels);
    for(i = 0; i < infohdr->bmheight_pixels; i++)
    {
	(*pixels)[i] = malloc(sizeof(uint32_t) * infohdr->bmwidth_pixels);
	for(n = 0; n < infohdr->bmwidth_pixels; n++)
	{
	    fread(pixels[i][n], sizeof(uint32_t), 1, image);
	}
    }
}

int write_bmpi(struct BMP_FHDR * fhdr, struct BITINFOHDR * infohdr, uint32_t *** pixels)
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
    fwrite(*pixels, 1, sizeof(uint32_t) * infohdr->bmheight_pixels  * (infohdr->bmwidth_pixels), img_writer);

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
