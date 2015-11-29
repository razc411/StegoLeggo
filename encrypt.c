#include "img_manip.h"
/*
*	Function: 	int encrypt_data(char * algorithm, void * buffer, int buffer_len)
*	Author: 	Ramzi Chennafi and Chris Hunter
*	Date:		October 4 2015
*	Returns:	int, 0 on success and a positive integer on failure
*
*	Notes
*       Turns the passed in buffer into encrypted text and generate a key for that
*       symmeteric encryption. Allows for a choice of algorithm, but uses cipher block
*       chaining with it.
*/
int encrypt_data(char * algorithm, char ** buffer, int buffer_len) {

    int remainder = 0, data_read = 0;
    MCRYPT td = mcrypt_module_open(algorithm, NULL, "cbc", NULL);
    int blocksize = mcrypt_enc_get_block_size(td);
    char * block_buffer = malloc(blocksize);

    //padding for block ciphers
    if((remainder = (buffer_len % blocksize)) != 0){
	printf("Padding final block, %d bytes short.\n", blocksize - remainder);
	buffer_len = buffer_len + (blocksize - remainder);
	*buffer = realloc(*buffer, buffer_len);

	int i;
	for(i = buffer_len - (blocksize - remainder); i < buffer_len; i++){
	    (*buffer)[i] = '\0';
	}
    }

    //generate the intialization vector
    int iv_len = mcrypt_enc_get_iv_size(td);
    char IV[iv_len];
    genRandom(IV, iv_len);
   
    //generate the actual symmetric key
    int key_len =  mcrypt_enc_get_key_size(td);
    char key[key_len];
    genRandom(key, key_len);
    
    //write the key to a file for later usage
    if(writeKeyToFile(IV, iv_len, key, key_len, algorithm)){
	return 0;
    }
       
    //encrypt and return
    mcrypt_generic_init(td, key, key_len, IV);

    while(data_read < buffer_len){

	memcpy(block_buffer, *buffer + data_read, blocksize);
	mcrypt_generic (td, block_buffer, blocksize);
	memcpy(*buffer + data_read, block_buffer, blocksize);

	data_read += blocksize;
    }
    
    mcrypt_generic_deinit(td);
    mcrypt_module_close(td);

    printf("Data successfully encrypted. Key produced in stglg.key\n");
    
    return buffer_len;
}

void genRandom(void * buffer, int buffer_len){

    int i, readLen = 0, res = 0;
    int devRand = open("/dev/random", O_RDONLY);
    
    while(readLen < buffer_len){
	res = read(devRand, buffer + readLen, (buffer_len - readLen));
	if(res < 0){
	    printf("Failed to read data from dev rand.");
	    exit(1);
	}
	readLen += res;
    }

    close(devRand);
}
/*
*	Function: 	int encrypt_data(char * algorithm, void * buffer, int buffer_len)
*	Author: 	Ramzi Chennafi and Chris Hunter
*	Date:		October 4 2015
*	Returns:	int, 0 on success and a positive integer on failure
*
*	Notes
*       Turns the passed in buffer into encrypted text and generate a key for that
*       symmeteric encryption. Allows for a choice of algorithm, but uses cipher block
*       chaining with it.
*/
int writeKeyToFile(char * IV, int iv_len,  char * key, int key_len, char * algorithm){
    
    FILE * keyFile;
    if(!(keyFile = fopen("stglg.key", "w+"))){
	printf("Failed to open file for key writing");
	return 1;
    }

    fprintf(keyFile, "%s\n", algorithm);
    fprintf(keyFile, "%d\n", iv_len);
    fwrite(IV, 1, iv_len, keyFile);
    fprintf(keyFile, "\n%d\n", key_len);
    fwrite(key, 1, key_len, keyFile);

    fclose(keyFile);
    
    return 0;
}
/*
*	Function: 	int encrypt_data(char * algorithm, void * buffer, int buffer_len)
*	Author: 	Ramzi Chennafi and Chris Hunter
*	Date:		October 4 2015
*	Returns:	int, 0 on success and a positive integer on failure
*
*	Notes
*       Turns the passed in buffer into encrypted text and generate a key for that
*       symmeteric encryption. Allows for a choice of algorithm, but uses cipher block
*       chaining with it.
*/
int decrypt_data(char ** buffer, int buffer_len) {

    int key_len = 0, iv_len = 0;
    size_t len = 0;
    char * key, * IV;
    char * algorithm = NULL;
  
    FILE * keyFile;
    if(!(keyFile = fopen("stglg.key", "r+"))){
	printf("Failed to open file for key writing");
	return 1;
    }

    //get alg
    getline(&algorithm, &len, keyFile);
    algorithm[strlen(algorithm) - 1] = '\0';
 
    //get iv
    fscanf(keyFile, "%d", &iv_len);
    fgetc(keyFile); // skips newline
    IV = malloc(iv_len);
    int res = 0;
    while((res += fread(IV, 1, iv_len, keyFile)) != iv_len){}

    //get key
    fscanf(keyFile, "%d", &key_len);
    fgetc(keyFile); // skips newline
    key = malloc(key_len);
    res = 0;
    while((res += fread(key, 1, key_len, keyFile)) != key_len){}
    
    MCRYPT td = mcrypt_module_open(algorithm, NULL, "cbc", NULL);
    int blocksize = mcrypt_enc_get_block_size(td);
    
    if(buffer_len % blocksize != 0){
    	printf("Incorrect data size for block ciphers.");
    	return 1;
    }
  
    mcrypt_generic_init(td, key, key_len, IV);

    char * block_buffer = calloc(1, blocksize);
    size_t data_read = 0;

    while(data_read < buffer_len){

	memcpy(block_buffer, *buffer + data_read, blocksize);
	mdecrypt_generic(td, block_buffer, blocksize);
	memcpy(*buffer + data_read, block_buffer, blocksize);

	data_read += blocksize;
    }


    mcrypt_generic_deinit(td);
    mcrypt_module_close(td);
  
    return 0;
}


