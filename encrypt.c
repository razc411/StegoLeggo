#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mcrypt.h>

char * IV;
char * key;
int key_len;
int iv_len;

int encrypt_data(void * buffer, int buffer_len, char * algorithm) {

    MCRYPT td = mcrypt_module_open(algorithm, NULL, "cbc", NULL);
    int blocksize = mcrypt_enc_get_block_size(td);
    if(buffer_len % blocksize != 0){
	return 1;
    }

    for (i=0; i< mcrypt_enc_get_iv_size( td); i++) {
	IV[i]=rand();
    }
    
    mcrypt_generic_init(td, key, key_len, IV);
    mcrypt_generic(td, buffer, buffer_len);
    mcrypt_generic_deinit (td);
    mcrypt_module_close(td);
  
    return 0;
}

int decrypt_data(void * buffer, int buffer_len) {

    MCRYPT td = mcrypt_module_open("rijndael-128", NULL, "cbc", NULL);
    int blocksize = mcrypt_enc_get_block_size(td);
    if(buffer_len % blocksize != 0){
	return 1;
    }
  
    mcrypt_generic_init(td, key, key_len, IV);
    mdecrypt_generic(td, buffer, buffer_len);
    mcrypt_generic_deinit (td);
    mcrypt_module_close(td);
  
    return 0;
}

int write_encryption_key(){

} 
