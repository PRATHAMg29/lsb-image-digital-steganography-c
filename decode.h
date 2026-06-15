#ifndef DECODE_H
#define DECODE_H
#include <stdio.h>

#include "types.h" // Contains user defined types


typedef struct _DecodeInfo
{
    char *srce_image_fname;     //TO STORE THE SRC IMAGE NAME
    FILE *fptr_srce_image;      //FILE POINTER FOR THE SRC IMAGE

    char magic_string[3];       //TO STORE MAGIC STRING DECODED FROM SRC IMAGE.

    long size_extn_size;        //TO STORE SECRET FILE EXTENSION SIZE DECODED FROM SRC IMAGE.
    char exten_secret_file[5];  //TO STORE THE SECRET FILE EXTENSION DECODED FROM SRC IMAGE.
    long secret_size_file;      //TO STORE THE SECRET FILE SIZE DECODED FROM SRC IMAGE.

    char *dest_fname;           //TO STORE THE DESTINATION FILE NAME(.txt,.c,.sh)
    char dest_name_backup[40];  
    FILE *fptr_desti_image;     //FILE POINTER FOR WRITING INTO THE DESTINATION FILE.

}DecodeInfo;

/* Read and validate Decode args from argv */
Statuss read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Perform the decoding */
Statuss do_decoding(DecodeInfo *decInfo);

/* Get File pointers for i/p and o/p files */
Statuss open_files_for_decoding(DecodeInfo *decInfo);

/* Skip bmp image header */
Statuss skip_bmp_header(FILE *fptr_srce_image);

// /* Decode Magic String */
Statuss decode_magic_string(DecodeInfo *decInfo);

// /*Decode extension size*/
Statuss decode_secret_file_extn_size(DecodeInfo *decInfo);

// /* Decode secret file extenstion */
Statuss decode_secret_file_extn(DecodeInfo *decInfo);

// /* Decode secret file size */
Statuss decode_secret_file_size(DecodeInfo *decInfo);

// /* Decode secret file data*/
Statuss decode_secret_file_data(DecodeInfo *decInfo);

// /* Decode a byte from LSB of image data array */
Statuss decode_byte_from_lsb(char *data, char *buffer);

// // Decode a size from lsb
Statuss decode_size_from_lsb(int *size, char *buffer);

#endif