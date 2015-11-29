#!/bin/bash
#Script for running each of the available algorithms. Writes all outputs to their respective decrypt/encrypt times files.

RUNS=50
algorithms=('blowfish' 'des' 'tripledes' 'gost' 'cast-128' 'xtea' 'rijndael-256' 'rijndael-128' 'twofish' 'rc2' 'cast-256' 'loki97')
fnames=('blowf_etimes' 'des_etimes' 'tdes_etimes' 'gost_etimes' 'c128_etimes' 'xtea_etimes' 'rj256_etimes' 'rj128_etimes' 'tf_etimes' 'rc2_etimes' 'c256_etimes' 'l97_etimes');
dfnames=('blowf_dtimes' 'des_dtimes' 'tdes_dtimes' 'gost_dtimes' 'c128_dtimes' 'xtea_dtimes' 'rj256_dtimes' 'rj128_dtimes' 'tf_dtimes' 'rc2_dtimes' 'c256_dtimes' 'l97_dtimes');

fp=0
for p in ${algorithms[@]}; do
    
    for i in `seq 1 $RUNS`; do
	./a.out encode butterfly.bmp img_manip.c $p > stderr
	./a.out decode encoded_image.bmp test > stderr
    done

    echo $p finished!
    mv encrypt_times Data/${fnames[$fp]}
    mv decrypt_times Data/${dfnames[$fp]}

    ((fp+=1))
done

echo "Operation complete!"
