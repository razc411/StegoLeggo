#include "img_manip.h"

unsigned char BITMAP[2] = "BM";

int main(int argc, char ** argv)
{
    size_t i, n;
    FILE *image;

    if(argc < 3)
    {
	printf("Requires a file to imbed the data into and a file with data to imbed with.");
	exit(1);
    }
    
    image = fopen(argv[1], "rwb");
    if(!image)
    {
	printf("Failed to open file %s.\n", argv[1]);
	exit(1);
    }

    struct BMP_FHDR fhdr;
    struct BITINFOHDR infohdr;

    fread(&fhdr, sizeof(struct BMP_FHDR), 1, image);
    fread(&infohdr, sizeof(struct BITINFOHDR), 1, image);

    if(fhdr.hdr[0] != 'B' && fhdr.hdr[1] != 'M')
    {
	printf("File must be a bitmap, aborting.\n");
	exit(1);
    }
    
    uint32_t pixels[infohdr.bmheight_pixels][infohdr.bmwidth_pixels];
    for(i = 0; i < infohdr.bmheight_pixels; i++)
    {
	for(n = 0; n < infohdr.bmwidth_pixels; n++)
	{
	    fread(&pixels[i][n], sizeof(uint32_t), 1, image);
	}
    }

    write_bmpi(&fhdr, &infohdr, pixels);

    fclose(image);
}

int write_bmpi(struct BMP_FHDR * fhdr, struct BITINFOHDR * infohdr, uint32_t ** pixels)
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
    fwrite(pixels, 1, sizeof(uint32_t) * infohdr->bmheight_pixels  * (infohdr->bmwidth_pixels), img_writer);

    fclose(img_writer);
}



    
