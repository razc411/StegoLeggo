/*
*	Program:	Covert Data Integrator
*	Author: 	Ramzi Chennafi
*	Date: 		October 4 2015
*	Functions:
*       int main(int argc, char ** argv)	
*       void encode_data_basic(struct BITINFOHDR * infohdr, struct BMP_FHDR *fhdr, unsigned char * pixels, char * filename)
*       unsigned char * insert_encode_data(unsigned char * pixels, unsigned char * data)
*       void decode_data_basic(struct BITINFOHDR * infohdr, struct BMP_FHDR *fhdr, unsigned char * pixels, char * output)       
*       unsigned char * grab_decode_header(unsigned char * pixels, int maxsize, int headerflag)        
*       unsigned char * grab_bmpinfo_pixels(struct BITINFOHDR * infohdr, FILE * image)
*       int checkbmp_type(struct BITINFOHDR * infohdr, struct BMP_FHDR * fhdr)
*       void write_bmpi(struct BMP_FHDR * fhdr, struct BITINFOHDR * infohdr, unsigned char * pixels)
*       unsigned char move_bit(unsigned char c1, int from, unsigned char c2, int to)     
* 
*	Description
*       Stegonography program for hiding data into bitmap images. Uses the least significant bit of each byte of the
*       pixels of the image. Encodes a filename, file size and file to the pixel data.
*       
*       Use
*       covertdata encode "image filename" "data filename"
*       - will output the encoded data in an image called encoded_data.bmp
*       covertdata decode "image filename" "output filename"
*       - will output data to the "output filename" decoded from the "image filename"
*/
#include "img_manip.h"
/*
*	Function: 	int main(int argc, char ** argv)
*	Author: 	Ramzi Chennafi
*	Date:		October 4 2015
*	Returns:	void
*
*       Notes
*       The main area of the program. Checks if the file passed in is a proper bitmap and retrieves the pixel data,
*       the info header and the signature header. It then decodes or encodes that data based on the arguments.
*/
//position variable for bit setting
static int pos = 0;
char * pixels;

int main(int argc, char ** argv)
{
    int type;
    FILE *image;
    struct BMP_FHDR fhdr;
    struct BITINFOHDR infohdr;
    char * algorithm = "rijndael-128";
    
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
	exit(1);
    }
    
    if(type == BMPINFO)
    {
	if(strcmp(argv[1], "encode") == 0)
	{
	    grab_bmpinfo_pixels(&infohdr, image);
	    encode_data_basic(algorithm, &infohdr, &fhdr, argv[3]);
	}
	else if(strcmp(argv[1], "decode") == 0)
	{
	    grab_bmpinfo_pixels(&infohdr, image);
	    decode_data_basic(argv[3]);
	}
    }
    else
    {
	printf("Format not supported.\n");
	exit(2);
    }

    fclose(image);

    return 0;
}
/*
*	Function: 	void encode_data_basic(struct BITINFOHDR * infohdr, struct BMP_FHDR *fhdr, unsigned char * pixels, char * filename)
*	Author: 	Ramzi Chennafi
*	Date:		October 4 2015
*	Returns:	void
*
*	Notes
*       Encodes the data into the data from the bitmap file.
*         struct BITINFOHDR * infohdr - info header of the bitmap file
*         struct BMP_FHDR * fhdr - the signature header of the bitmap file
*         unsigned char * pixels - the pixel array of the bitmap file
*         char * filename - the filename of the data to encode into the image
*/
void encode_data_basic(char * algorithm, struct BITINFOHDR * infohdr, struct BMP_FHDR *fhdr, char * filename)
{
    char strsize[100];
    int esize;
    pos = 0;
    
    memset(strsize, 0, 100);
    
    FILE * encode_data;
    if(!(encode_data = fopen(filename, "rb+")))
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

    printf("Encoding data from file %s into the image.\n", filename);
    printf("Image Max Encoding Size: %d\n", infohdr->image_size/BYTESIZE);
     
    char * data = malloc(esize);
    fread(data, 1, esize, encode_data);

    esize = encrypt_data(algorithm, &data, esize);
    printf("Data Size: %d\n", esize);
    sprintf(strsize, "%d", esize);
    
    //filename
    insert_encode_data(filename);
    //set the file size
    insert_encode_data(strsize);
    //set the encode data
    insert_encode_data(data);

    write_bmpi(fhdr, infohdr);

    printf("Data written to image successfully!\n");
    
    free(pixels);
    free(data);
}
/*
*	Function: 	unsigned char * insert_encode_data(unsigned char * pixels, unsigned char * data)
*	Author: 	Ramzi Chennafi
*	Date:		October 4 2015
*	Returns:	unsigned char * - the modified pixel array
*
*	Notes
*	Inserts data from the data array into the pixel array.
*           unsigned char * pixels - the pixel array
*           unsigned char * data - the data to put into the pixel array
*/
void insert_encode_data(char * data)
{
    size_t c, p;
    for(c = 0; c < strlen(data) + 1; c++)
    {
    	for(p = 0; p < BYTESIZE; p++)
    	{
    	    pixels[pos] = move_bit(data[c], p, pixels[pos], 0);
	    pos++;
    	}
    }
}
/*
*	Function: 	void decode_data_basic(unsigned char * pixels, char * output)
*	Author: 	Ramzi Chennafi
*	Date:		October 4 2015
*	Returns:	void
*
*	Notes
*       Decodes data hidden insidei of the image file data from the pixels.
*          unsigned char * pixels - the pixel array to retrieve data from
*          char * output - the name of the file to output retrieved data to
*/
void decode_data_basic(char * output)
{
    int esize = 0;
    char * filename;
    char * fsize;
    FILE * decode_writer;
    pos = 0;

    filename = grab_decode_header(MAXFNAME, HEADER);
    fsize = grab_decode_header(MAXSIZE, HEADER); 
    sscanf(fsize, "%d", &esize);

    char * encoded_data = malloc(esize);
    encoded_data = grab_decode_header(esize, DATA);

    decrypt_data(&encoded_data, esize);
    
    printf("File to be decoded: %s\n", filename);
    printf("File Size:  %d\n", esize);
    printf("Saving file as... %s\n", output);

    if(!(decode_writer = fopen(output, "wb+")))
    {
	printf("Failed to decode file.\n");
	exit(1);
    }
    fwrite(encoded_data, 1, esize, decode_writer);
    
    free(filename);
    free(fsize);
}
/*
*	Function: 	unsigned char * grab_decode_header(unsigned char * pixels, int maxsize, int headerflag)
*	Author: 	Raiimzi Chennafi
*	Date:		October 4 2015
*	Returns:	unsigned char * - an array containing the data retrieved
*
*	Notes
*       Grabs the encoded data from the pixel data based on the maxsize. If header flag is set to header, 
*       will check for null termination, if set to DATA, will not.
*            unsigned char * pixels - the pixel array to get data out of
*            int maxsize - the size of the array to retrieve from the pixels
*            int headerflag - can be set to DATA or HEADER based on the data you want
*/
char * grab_decode_header(int maxsize, int headerflag)
{
    int c, p;
    char * temparray = calloc(1, maxsize);

    for(c = 0; c < maxsize; c++)
    {
	for(p = 0; p < BYTESIZE; p++)
	{
	    temparray[c] = move_bit(pixels[pos++], 0, temparray[c], p);
	}
	if(temparray[c] == '\0' && headerflag != DATA)
	{
	    break;
	}
    }

    return temparray;
}
/*
*	Function:       unsigned char * grab_bmpinfo_pixels(struct BITINFOHDR * infohdr, FILE * image)
*	Author: 	Ramzi Chennafi
*	Date:		October 4 2015
*	Returns:	unsigned char array containing the pixel data of the image
*
*	Notes
*	Grabs the pixel data from a bitmap file.
*           struct BITINFOHDR * infohdr - the info header of the bitmap file
*           FILE * image - a file pointer to the image data to retrieve pixels from
*/			      
void grab_bmpinfo_pixels(struct BITINFOHDR * infohdr, FILE * image)
{
    size_t i;
    pixels = malloc(infohdr->image_size);
    for(i = 0; i < infohdr->image_size; i++)
    {
	fread(&pixels[i], 1, 1, image);
    }
}
/*
*	Function: 	void write_bmpi(struct BMP_FHDR * fhdr, struct BITINFOHDR * infohdr, unsigned char * pixels)
*	Author: 	Ramzi Chennafi
*	Date:		October 4 2015
*	Returns:	void
*
*	Notes
*	Writes a bmp file using the supplied headers and pixels.
*             struct BMP_FHDR * fhdr -  the signature header of the bitmap
*             struct BITINFOHDR * infohdr - the info header of the bitmap
*             unsigned char * pixels - the pixel array to write
*/
void write_bmpi(struct BMP_FHDR * fhdr, struct BITINFOHDR * infohdr)
{
    FILE *img_writer;
    img_writer = fopen("encoded_image.bmp", "wb+");
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
/*
*	Function: 	int checkbmp_type(struct BITINFOHDR * infohdr, struct BMP_FHDR * fhdr)
*	Author: 	Ramzi Chennafi
*	Date:		October 4 2015
*	Returns:	int
*                       - 0 if not a bitmap, BMPINFO if a bitmap
*
*	Notes
*	Checks if the headers passed in belong to a bitmapinfo header.
*               struct BITINFOHDR * infohdr - the info header of a bitmap file
*               struct BMP_FHDR * fhdr - the bmp signature header of a file
*/
int checkbmp_type(struct BITINFOHDR * infohdr, struct BMP_FHDR * fhdr)
{
    //checking if the header is actually a bitmap
    if(fhdr->hdr[0] != 'B' && fhdr->hdr[1] != 'M')
    {
	printf("File must be a bitmap, aborting.\n");
	exit(1);
    }

    //checking the bitmap header type
    if(infohdr->hdr_size == 40)
    {
	return BMPINFO;
    }

    return 0;
}
/*
*	Function:       unsigned char move_bit(unsigned char c1, int from, unsigned char c2, int to) 	
*	Author: 	Ramzi Chennafi
*	Date:		October 4 2015
*	Returns:	unsigned char - the character modified by c1 in c2
*
*	Notes
*       Changes the "to" bit of c2 to the bit of c1 at "from". Returns the modified character.
*/
char move_bit(char c1, int from, char c2, int to)
{
    int bit;
    bit = (c1 >> from) & 1;            /* Get the source bit as 0/1 value */
    c2 &= ~(1 << to);                  /* clear destination bit */
    return (char)(c2 | (bit << to));  /* set destination bit */
}
