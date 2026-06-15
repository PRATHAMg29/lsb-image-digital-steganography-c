#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "types.h"
#include "common.h"
#define RED       "\033[31m"  //define a macro name for colors
#define RESET     "\033[0m"
#define GREEN     "\033[32m"
#define BBLUE     "\033[94m"
#define BCYAN     "\033[96m"
#define BMAGENTA  "\033[95m"
#define ORANGE    "\033[38;5;208m"
/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)  
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

uint get_file_size(FILE *fptr)
{
    // Find the size of secret file data
    fseek(fptr,0,SEEK_END);

    //Move the file pointer to the end and return the size.
    int secret_file_size = ftell(fptr);
    return secret_file_size;
}



Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    //VALIDATE .bmp extension for source file.
    char *p=strstr(argv[2],".");
    if(p==NULL || (strcmp(p,".bmp") != 0))
    {
        printf(RED "Enter filename with '.bmp' extension only.\n" RESET);  //if validation failed, print error message and terminate.
        return e_failure;
    }
    else
    {
        encInfo->src_image_fname = argv[2];  //if validation passed, store in structure member.
    }

    //VALIDATE .txt extension for secret text file
    p=strstr(argv[3],".");
    // printf("HI\n");
    if((p==NULL ) || ((strcmp(p,".txt") != 0) && (strcmp(p,".c") != 0)))
    {
        printf(RED "Enter filename with '.txt' or '.c' extension only.\n" RESET);  //if validation failed, print error message and terminate.
        return e_failure;
    }
    else
    {
        encInfo->secret_fname = argv[3];    //if validation passed, store in structure member.
        // int i=0;
        // char extn[5];
        // while(p[i])  //copy character by character from '.' to the end.
        // {
        //     extn[i] = p[i];
        //     i++;
        // }
        // extn[i] = '\0';  //add NULL character at the end.
        strcpy(encInfo->extn_secret_file,p);  //copy the extension to the structure member.
    }

    //VALIDATE .bmp extension for destination file if present
    if(argv[4] != NULL)
    {
        p=strstr(argv[4],".");
        if(p==NULL || (strcmp(p,".bmp") != 0))
        {
            printf(RED "Enter filename with '.bmp' extension only.\n" RESET);  //if validation failed, print error message and terminate.
            return e_failure;
        }
        else
        {
            encInfo->dest_image_fname = argv[4];    //if validation passed, store in structure member.
        }  
    }
    else
    {
        encInfo->dest_image_fname = "default.bmp";  //if destination file not present, then store name of our own in the structure member.
    }
    return e_success;  //return e_success if all validations are passed.
}

/*
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

        return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

        return e_failure;
    }

    // Stego Image file
    encInfo->fptr_dest_image = fopen(encInfo->dest_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_dest_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->dest_image_fname);

        return e_failure;
    }

    // No failure return e_success
    return e_success;
}

//CHECK IMAGE CAPACITY
Status check_capacity(EncodeInfo *encInfo)
{
    //call function to calculate image size and store the return value in the structure member.
    int image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->image_capacity = image_capacity;

    //call function to calculate secret file size and store the return value in structure member.
    int secret_file_size = get_file_size(encInfo->fptr_secret);
    encInfo->size_secret_file = secret_file_size;

    //check if image capacity is greater than required size to store the information.
    if(image_capacity > ((2+sizeof(int)+strlen(encInfo->extn_secret_file)+sizeof(int)+ encInfo->size_secret_file) * 8))
        return e_success;  //if greater, return success.
    else
        return e_failure;  //else return failure.
}

//COPY THE 54 BYTES HEADER FROM SOURCE TO DESTINATION
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{ 
    //seek both source and destination file pointers to the start.
    fseek(fptr_src_image,0,SEEK_SET);
    fseek(fptr_dest_image,0,SEEK_SET);

    //copy the 54 bytes header from source to destination as it is.
    char header[54];
    fread(header,54,1,fptr_src_image);
    fwrite(header,54,1,fptr_dest_image);

    //check if both file pointers are pointing to the same offset.
    long int src_file_pointer = ftell(fptr_src_image);
    long int dest_file_pointer = ftell(fptr_dest_image);
    if(src_file_pointer != dest_file_pointer)
    {
        return e_failure;
    }
    else
        return e_success;
}

//ENCODE THE MAGIC STRING INTO THE LSB OF EACH SOURCE BYTE.
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    //read 8 bytes from source file and store it in a buffer.
    //then call byte to lsb function which will store each bit of a character of the magic string into the LSBs of each byte.
    //write the updated 8 bytes into the destination file.
    //do the same using loop until NULL character is reached.
    unsigned char magic_string_buffer[8];
    for(int i=0;magic_string[i] != '\0';i++)
    {
        fread(magic_string_buffer,8,1,encInfo->fptr_src_image);
        encode_byte_to_lsb(magic_string[i],magic_string_buffer);
        fwrite(magic_string_buffer,8,1,encInfo->fptr_dest_image);
    }

    //check if both file pointers are pointing to the same offset.
    long int src_file_pointer = ftell(encInfo->fptr_src_image);
    long int dest_file_pointer = ftell(encInfo->fptr_dest_image);
    if(src_file_pointer != dest_file_pointer)
    {
        return e_failure;
    }
    else
        return e_success;
}

//ENCODE THE SECRET FILE EXTENSION SIZE INTO THE LSB OF EACH SOURCE BYTE.
Status encode_secret_file_extn_size(int size, EncodeInfo *encInfo)
{
    //read next 32 bytes from source file and store it in a buffer.
    //then call size to lsb function which will store each bit of the secret file extension size into the LSBs of each byte.
    //write the updated 32 bytes into the destination file.
    unsigned char secret_file_extn_size_buffer[32];
    fread(secret_file_extn_size_buffer,32,1,encInfo->fptr_src_image);
    encode_size_to_lsb(size,secret_file_extn_size_buffer);
    fwrite(secret_file_extn_size_buffer,32,1,encInfo->fptr_dest_image);

    //check if both file pointers are pointing to the same offset.
    long int src_file_pointer = ftell(encInfo->fptr_src_image);
    long int dest_file_pointer = ftell(encInfo->fptr_dest_image);
    if(src_file_pointer != dest_file_pointer)
    {
        return e_failure;
    }
    else
        return e_success;
}

//ENCODE THE SECRET FILE EXTENSION INTO THE LSB OF EACH SOURCE BYTE.
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    //read next 8 bytes from source file and store it in a buffer.
    //then call byte to lsb function which will store each bit of the secret file extension into the LSBs of each byte.
    //write the updated 8 bytes into the destination file.
    //do the same using loops until NULL character is reached.
    unsigned char secret_file_extn_buffer[8];
    for(int i=0;file_extn[i] != '\0';i++)
    {
        fread(secret_file_extn_buffer,8,1,encInfo->fptr_src_image);
        encode_byte_to_lsb(file_extn[i],secret_file_extn_buffer);
        fwrite(secret_file_extn_buffer,8,1,encInfo->fptr_dest_image);
    }

    //check if both file pointers are pointing in the same offset.
    long int src_file_pointer = ftell(encInfo->fptr_src_image);
    long int dest_file_pointer = ftell(encInfo->fptr_dest_image);
    if(src_file_pointer != dest_file_pointer)
    {
        return e_failure;
    }
    else
        return e_success;
}

//ENCODE THE SECRET FILE SIZE INTO THE LSB OF EACH SOURCE BYTE.
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    //read next 32 bytes from source file and store it in a buffer.
    //then call size to lsb function which will store each bit of the secret file size into the LSBs of each byte.
    //write the updated 32 bytes into the destination file.
    unsigned char secret_file_size_buffer[32];
    fread(secret_file_size_buffer,32,1,encInfo->fptr_src_image);
    encode_size_to_lsb(file_size,secret_file_size_buffer);
    fwrite(secret_file_size_buffer,32,1,encInfo->fptr_dest_image);

    //check if both file pointers are pointing in the same offset.
    long int src_file_pointer = ftell(encInfo->fptr_src_image);
    long int dest_file_pointer = ftell(encInfo->fptr_dest_image);
    if(src_file_pointer != dest_file_pointer)
    {
        return e_failure;
    }
    else
        return e_success;
}

//ENCODE THE SECRET FILE DATA INTO THE LSB OF EACH SOURCE BYTE.
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    //seek the secret file pointer to the start.
    fseek(encInfo->fptr_secret,0,SEEK_SET);

    //create a buffer which is of size of the secret file.
    char secret_file_data[encInfo->size_secret_file];

    //read the data from the secret file and store it into the buffer.
    fread(secret_file_data,encInfo->size_secret_file,1,encInfo->fptr_secret);

    //run a loop upto size of secret file times.
    //each time read 8 bytes from the source file.
    //then call byte to lsb function which will store each bit of the secret file data into the LSBs of each byte.
    //write the updated data into the destination file.
    unsigned char secret_file_data_buffer[8];
    for(int i=0;i<encInfo->size_secret_file;i++)
    {
        fread(secret_file_data_buffer,8,1,encInfo->fptr_src_image);
        encode_byte_to_lsb(secret_file_data[i],secret_file_data_buffer);
        fwrite(secret_file_data_buffer,8,1,encInfo->fptr_dest_image);
    }

    //check if both file pointers are pointing in the same offset.
    long int src_file_pointer = ftell(encInfo->fptr_src_image);
    long int dest_file_pointer = ftell(encInfo->fptr_dest_image);
    if(src_file_pointer != dest_file_pointer)
    {
        return e_failure;
    }
    else
        return e_success;

}

//COPY THE REMAINING DATA FROM SOURCE TO DESTINATION.
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest,EncodeInfo *encInfo)
{
    //copy byte by byte from source file and write into the destination file until END OF FILE is reached.
    unsigned char ch;
    while(fread(&ch,1,1,fptr_src) != 0)
    {
        fwrite(&ch,1,1,fptr_dest);
    }

    //check if both file pointers are pointing in the same offset.
    long int src_file_pointer = ftell(encInfo->fptr_src_image);
    long int dest_file_pointer = ftell(encInfo->fptr_dest_image);
    if(src_file_pointer != dest_file_pointer)
    {
        return e_failure;
    }
    else
        return e_success;
}

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for(int j=0;j<8;j++)   //RUN LOOP 8 TIMES
    {
        int bit = (data>>(7-j)) & 1;     //EXTRACT BIT POSITION AT j FROM DATA.(MSB TO LSB).
        image_buffer[j]= (image_buffer[j] & 0XFE);   //CLEAR THE LSB BIT OF IMAGE_BUFFER
        image_buffer[j] = image_buffer[j] | bit;    //SET THE LSB BIT OF DATA GOT INTO THE LSB BIT OF IMAGE_BUFFER
    }
}

Status encode_size_to_lsb(int size, char *image_buffer)
{
    for(int i=0;i<32;i++)    //RUN LOOP 32 TIMES.
    {
        int bit = (size>>(31-i)) & 1;   //EXTRACT BIT POSITION AT j FROM SIZE(MSB TO LSB).
        image_buffer[i] = (image_buffer[i] & 0XFE);    //CLEAR THE LSB BIT OF IMAGE_BUFFER
        image_buffer[i] = image_buffer[i] | bit;    //SET THE LSB BIT OF SIZE GOT INTO THE LSB BIT OF IMAGE_BUFFER
    }
}

Status do_encoding(EncodeInfo *encInfo)
{
    if(open_files(encInfo) == e_success)    //CALL OPEN FILES FUNCTION AND CHECK IF SUCCESS OR FAILURE.
    {
        printf("------------------------------------------\n");
        printf("Opening of all files : " GREEN "SUCCESSFULL\n" RESET);
        printf("------------------------------------------\n");
        if(check_capacity(encInfo) == e_success)    //CALL CHECK CAPACITY FUNCTION AND CHECK IF SUCCESS OR FAILURE.
        {
            if(copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_dest_image) == e_success)  //CALL COPY HEADER FUNCTION AND CHECK IF SUCCESS OR FAILURE.
            {
                printf("------------------------------------------\n");
                printf("Copying of Header    : " GREEN "SUCCESSFULL\n" RESET);
                if(encode_magic_string(MAGIC_STRING,encInfo) == e_success)  //CALL MAGIC STRING ENCODING FUNCTION AND CHECK IF SUCCESS OR FAILURE.
                {
                    printf("------------------------------------------\n");
                    printf("Encoding of Magic\nString \t\t     : " GREEN "SUCCESSFULL\n" RESET);
                    if(encode_secret_file_extn_size(strlen(encInfo->extn_secret_file),encInfo) == e_success)    //CALL SECRET FILE EXTN SIZE ENCODING FUNCTION AND CHECK IF SUCCESS OR FAILURE.
                    {
                        printf("------------------------------------------\n");
                        printf("Encoding of Secret\nFile Extension size  : " GREEN "SUCCESSFULL\n" RESET);
                        if(encode_secret_file_extn(encInfo->extn_secret_file,encInfo) == e_success)     //CALL SECRET FILE EXTN ENCODING FUNCTION AND CHECK IF SUCCESS OR FAILURE.
                        {
                            printf("------------------------------------------\n");
                            printf("Encoding of Secret\nFile Extension \t     : " GREEN "SUCCESSFULL\n" RESET);
                            if(encode_secret_file_size(encInfo->size_secret_file,encInfo) == e_success)     //CALL SECRET FILE SIZE ENCODING FUNCTION AND CHECK IF SUCCESS OR FAILURE.
                            {
                                printf("------------------------------------------\n");
                                printf("Encoding of Secret\nFile Size \t     : " GREEN "SUCCESSFULL\n" RESET);
                                if(encode_secret_file_data(encInfo) == e_success)       //CALL SECRET FILE DATA ENCODING FUNCTION AND CHECK IF SUCCESS OR FAILURE.
                                {
                                    printf("------------------------------------------\n");
                                    printf("Encoding of Secret\nFile Data \t     : " GREEN "SUCCESSFULL\n" RESET);
                                    if(copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_dest_image,encInfo) == e_success)      //CALL FUNCTION TO COPY REMIANING DATA AND CHECK IF SUCCESS OR FAILURE.
                                    {
                                        printf("------------------------------------------\n");
                                        printf("Copying of Remaining\nData \t\t     : " GREEN "SUCCESSFULL\n" RESET);
                                        return e_success;  //RETURN e_success TO MAIN FUNCTION
                                    }
                                    else
                                    {
                                        printf(RED "Error copying remaining data.\n" RESET);
                                        return e_failure;
                                    }
                                }
                                else
                                {
                                    printf(RED "Error encoding secret file data.\n" RESET);
                                    return e_failure;
                                }
                            }
                            else
                            {
                                printf(RED "Error encoding secret file size.\n" RESET);
                                return e_failure;
                            }
                        }
                        else
                        {
                            printf(RED "Error encoding secret file extension.\n" RESET);
                            return e_failure;
                        }
                    }
                    else
                    {
                        printf(RED "Error encoding secret file extension size.\n" RESET);
                        return e_failure;
                    }
                }
                else
                {
                    printf(RED "Error encoding the magic string.\n" RESET);
                    return e_failure;
                }
            }
            else
            {
                printf(RED "Copying header failed.\n" RESET);
                return e_failure;
            }
        }
        else
        {
            printf(RED "Image capacity not sufficient.\n" RESET);
            return e_failure;
        }
    }
    else
    {
        return e_failure;
    }
}
