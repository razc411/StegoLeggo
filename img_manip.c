#include "img_manip.h"


/*
  for each bit


 */

//position variable for bit setting
int pos = 0;

int main(int argc, char ** argv)
{
    int type;
    FILE *image;
    unsigned char * pixels;
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
	    decode_data_basic(&infohdr, &fhdr, pixels, argv[3]);
	}
    }
    else
    {
	printf("Format not supported.\n");
	exit(2);
    }

    fclose(image);
}

void encode_data_basic(struct BITINFOHDR * infohdr, struct BMP_FHDR *fhdr, unsigned char * pixels, char * filename)
{
    unsigned char strsize[100];
    int esize;
    pos = 0;
    
    memset(strsize, 0, 100);
    
    FILE * encode_data;
    if(!(encode_data = fopen(filename, "rb")))
    {
	printf("Failed to open data to encode.\n");
	exit(1);
    }

    fseek(encode_data, 0, SEEK_END);
    esize = ftell(encode_data);
    fseek(encode_data, 0, SEEK_SET);
    sprintf(strsize, "%d", esize);
    
    if(infohdr->image_size/BYTESIZE < esize)
    {
	printf("File to encode is too large.\nEncoded file size: %d\nImage Capacity: %d\n", esize, infohdr->image_size/BYTESIZE);
	exit(1);
    }

    printf("Encoding data from file %s into the image.\n", filename);
    printf("Data Size: %d\n", esize);
    printf("Image Max Encoding Size: %d\n", infohdr->image_size/BYTESIZE);
    
    unsigned char data[esize];
    fread(data, sizeof(unsigned char), esize, encode_data);

    //filename
    pixels = insert_encode_data(pixels, filename);
    //set the file size
    pixels = insert_encode_data(pixels, strsize);
    //set the encode data
    pixels = insert_encode_data(pixels, data);

    write_bmpi(fhdr, infohdr, pixels);

    printf("Data written to image successfully!\n");
    
    free(pixels);
}

unsigned char * insert_encode_data(unsigned char * pixels, unsigned char * data)
{
    int c, p;
    for(c = 0; c < strlen(data) + 1; c++)
    {
    	for(p = 0; p < BYTESIZE; p++)
    	{
    	    pixels[pos++] = move_bit(data[c], p, pixels[pos], 0);
    	}
    }

    return pixels;
}

void decode_data_basic(struct BITINFOHDR * infohdr, struct BMP_FHDR *fhdr, unsigned char * pixels, char * output)
{
    int esize = 0;
    unsigned char * filename;
    unsigned char * fsize;
    FILE * decode_writer;
    pos = 0;

    filename = grab_decode_header(pixels, MAXFNAME, HEADER);
    fsize = grab_decode_header(pixels, MAXSIZE, HEADER); 
    sscanf(fsize, "%d", &esize);
   
    unsigned char * encoded_data;
    encoded_data = grab_decode_header(pixels, esize, DATA);
    
    printf("File to be decoded: %s\n", filename);
    printf("File Size:  %d\n", esize);
    printf("Saving file as... %s\n", output);

    if(!(decode_writer = fopen(output, "wb")))
    {
	printf("Failed to decode file.\n");
	exit(1);
    }
    fwrite(encoded_data, 1, esize, decode_writer);
    
    free(pixels);
    free(filename);
    free(fsize);
}

unsigned char * grab_decode_header(unsigned char * pixels, int maxsize, int headerflag)
{
    int c, p;
    unsigned char * temparray = malloc(maxsize);
    memset(temparray, 0, maxsize);

    for(c = 0; c < maxsize; c++)
    {
	for(p = 0; p < BYTESIZE; p++)
	{
	    temparray[c] = move_bit(pixels[pos++], 0, temparray[c], p);
	}
	if(temparray[c] == '\0' && !headerflag)
	{
	    break;
	}
    }

    return temparray;
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
    img_writer = fopen("encoded_image.bmp", "wb");
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
