#include <stdio.h>
#include<string.h>
#include "decode.h"
#include "types.h"
#include "common.h"
#define RED       "\033[31m"  //define a macro name for colors
#define RESET     "\033[0m"
#define GREEN     "\033[32m"
#define BBLUE     "\033[94m"
#define BCYAN     "\033[96m"
#define BMAGENTA  "\033[95m"
#define ORANGE    "\033[38;5;208m"

Statuss read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    //VALIDATE .bmp extension for source file.
    char *p=strstr(argv[2],".");
    if(p==NULL || (strcmp(p,".bmp") != 0))
    {
        printf(RED "Enter filename with '.bmp' extension only.\n" RESET);  //if validation failed, print error message and terminate.
        return d_failure;
    }
    else
    {
        decInfo->srce_image_fname = argv[2];  //if validation passed, store in structure member.
    }

    //VALIDATE the file name for output.
    if(argv[3] != NULL)     //check if destination file(.txt,.c,.sh) is given or not.
    {
        char temp[100];
        strcpy(temp,argv[3]);  //if given copy that to a temporary variable.
        char *token = strtok(temp,".");     //use strtok to remove extensions if given any.
        strcpy(decInfo->dest_name_backup,token);        //store it into the structure member.
    }
    else
    {
        strcpy(decInfo->dest_name_backup,"decoded");    //if not given then copy any default text into the structure member.
    } 
    return d_success;
}

Statuss open_files_for_decoding(DecodeInfo *decInfo)
{
    //OPEN FILE.
    decInfo->fptr_srce_image = fopen(decInfo->srce_image_fname,"r");
    //ERROR HANDLING.
    if (decInfo->fptr_srce_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->srce_image_fname);

        return d_failure;
    }
    return d_success;
}

Statuss skip_bmp_header(FILE *fptr_srce_image)
{
    fseek(fptr_srce_image,54,SEEK_SET);     //MOVE THE FILE POINTER TO THE 54TH BYTE. SKIP THE HEADER PART.
    long int src_file_pointer = ftell(fptr_srce_image); //CHECK IF FILE POINTER IS MOVED USING ftell AND RETURN SUCCESS OR FAILURE BASED ON THAT.
    if(src_file_pointer == 54)
    {
        return d_success;
    }
    else
        return d_failure;
}

Statuss decode_magic_string(DecodeInfo *decInfo)
{
    unsigned char buffer[8];        //DECLARE A BUFFER OF 8 BYTES.
    char magic[3];      //TO STORE MAGIC STRING
    for(int i=0;i<2;i++)    //RUN LOOP 2 TIMES AS ONLY 2 CHARACTERS PRESENT IN MAGIC STRING.
    {
        fread(buffer,8,1,decInfo->fptr_srce_image);     //READ 8 BYTES FROM SRC IMAGE AND STORE IN BUFFER.
        decode_byte_from_lsb(&magic[i],buffer);     //PASS THE ADDRESS OF MAGIC STRING INDEX AND BUFFER TO BYTE_FROM_LSB FUNCTION
    }
    magic[2] = '\0';        //ADD NULL CHARACTER AT LAST.
    if(strcmp(MAGIC_STRING,magic) == 0)     //COMPARE THE MACRO MAGIC_STRING AND THE DECODED MAGIC_STRING.
    {
        strcpy(decInfo->magic_string,magic);    //IF EQUAL STORE IN STRUCTURE MEMBER AND RETURN SUCCESS.
        return d_success;
    }
    else
    {
        return d_failure;       //RETURN FAILURE IF NOT EQUAL.
    }
}

Statuss decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    unsigned char buffer[32];       //DECLARE BUFFER OF 32 BYTES.
    fread(buffer,32,1,decInfo->fptr_srce_image);        //READ NEXT 32 BYTES FROM SRC IMAGE AND STORE INTO BUFFER.
    int size;       //DECLARE SIZE VARIABLE TO STORE SIZE.
    decode_size_from_lsb(&size,buffer);     //PASS ADDRESS OF SIZE AND BUFFER TO SIZE_FROM_LSB FUNCTION.
    decInfo->size_extn_size = size;     //NOW STORE THE SIZE IN STRUCTURE MEMBER.
    if(decInfo->size_extn_size > 0)     //CHECK IF PROPER SIZE IS STORED. AND RETURN SUCCESS OR FAILURE BASED ON THAT.
    {   
        return d_success;
    }
    else
        return d_failure;
}

Statuss decode_secret_file_extn(DecodeInfo *decInfo)
{
    unsigned char buffer[8];        //DECLARE BUFFER OF 8 BYTES.
    char extension[5];      //DECLARE 5 BYTES TO STORE EXTENSION.
    int i;
    for(i=0;i<decInfo->size_extn_size;i++)  //RUN LOOP UPTO EXTN SIZE TIMES.
    {
        fread(buffer,8,1,decInfo->fptr_srce_image);     //READ NEXT 8 BYTES FROM SRC IMAGE AND STORE INTO BUFFER.
        decode_byte_from_lsb(&extension[i],buffer);     //PASS THE ADDRESS OF EXTN INDEX AND BUFFER TO BYTE_FROM_LSB FUNCTION.
    }
    extension[i] = '\0';        //ADD NULL CHARACTER AT THE END.
    strcat(decInfo->dest_name_backup,extension);      //CONCATENATE THE EXTENSION WITH THE DESTINATION FILE NAME STORED IN STRUCTURE MEMBER.
    // decInfo->dest_fname = backup;
    decInfo->dest_fname = decInfo->dest_name_backup;    //MAKE THE DESTINATION FILE NAME POINT TO THIS CONCATENATED STRING.
    // printf("%s\n",decInfo->dest_fname);

    //OPENING FILE.
    decInfo->fptr_desti_image = fopen(decInfo->dest_fname,"w");     //OPEN DESTINATION FILE IN WRITE MODE.
    //ERROR HANDLING.
    if (decInfo->fptr_desti_image == NULL)
    {
        perror("fopen");
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->dest_fname);

        return d_failure;
    }
    return d_success;
}

Statuss decode_secret_file_size(DecodeInfo *decInfo)
{
    unsigned char buffer[32];   //DECLARE BUFFER OF 32 BYTES.
    fread(buffer,32,1,decInfo->fptr_srce_image);    //READ NEXT 32 BYTES FROM SRC IMAGE AND STORE INTO BUFFER.
    int size;   //DECLARE SIZE VARIABLE TO STORE SIZE.
    decode_size_from_lsb(&size,buffer);     //PASS ADDRESS OF SIZE AND BUFFER TO SIZE_FROM_LSB FUNCTION.
    // printf("size is %d\n",size);
    decInfo->secret_size_file = size;       //NOW STORE THE SIZE IN STRUCTURE MEMBER.
    if(decInfo->secret_size_file > 0)       //CHECK IF PROPER SIZE IS STORED. AND RETURN SUCCESS OR FAILURE BASED ON THAT.
    {
        return d_success;
    }
    else
        return d_failure;
}

Statuss decode_secret_file_data(DecodeInfo *decInfo)
{
    unsigned char buffer[8];    //DECLARE A BUFFER OF 8 BYTES.
    char secret_data[200];      //VARIABLE STORE THE SECRET DATA.
    for(int i=0;i<decInfo->secret_size_file;i++)    //RUN LOOP UPTO SIZE TIMES.
    {
        fread(buffer,8,1,decInfo->fptr_srce_image);     //READ NEXT 8 BYTES FROM SRC IMAGE AND STORE INTO BUFFER.
        decode_byte_from_lsb(&secret_data[i],buffer);   //PASS THE ADDRESS OF SECRET_DATA INDEX AND BUFFER TO BYTE_FROM_LSB FUNCTION.
    }
    fwrite(secret_data,decInfo->secret_size_file,1,decInfo->fptr_desti_image);  //WRITE THE DATA INTO THE DESTINATION FILE.
    return d_success;
}

Statuss decode_byte_from_lsb(char *data, char *buffer)
{
    char ch=0;  //VARIABLE TO STORE THE CHARACTER AND INITIALZED T0 0.
    for(int i=0;i<8;i++)    //RUN LOOP 8 TIMES SINCE 1 CHARACTER = 8 BITS.
    {
        ch = (ch<<1);       //LEFT SHIFT SO THAT SPACE IS CREATED FOR THE NEXT BIT.
        ch = ch | (buffer[i] & 1);      //EXTRACT LSB BIT FROM BUFFER AND STORE INTO ch.
    }
    *data=ch;       //STORE THE DECODED CHARACTER INTO THE VARIABLE POINTED BY DATA.
}

Statuss decode_size_from_lsb(int *size, char *buffer)
{
    int res=0;      //VARIABLE TO STORE THE SIZE AND INITIALZED T0 0.
    for(int i=0;i<32;i++)       //RUN LOOP 32 TIMES SINCE INTEGER SIZE = 8 BITS.
    {
        res = (res<<1);        //LEFT SHIFT SO THAT SPACE IS CREATED FOR THE NEXT BIT.
        res = res | (buffer[i] & 1);    //EXTRACT LSB BIT FROM BUFFER AND STORE INTO res.
    }
    *size=res;      //STORE THE DECODED CHARACTER INTO THE VARIABLE POINTED BY SIZE.
}

Statuss do_decoding(DecodeInfo *decInfo)
{
    if(open_files_for_decoding(decInfo) == d_success)   //CALL OPEN FILES FUNCTION AND CHECK IF SUCCESS OR FAILURE.
    {
        printf("------------------------------------------\n");
        printf("Opening of all files : " GREEN "SUCCESSFULL\n" RESET);
        // printf("------------------------------------------\n");
        if(skip_bmp_header(decInfo->fptr_srce_image) == d_success)  //CALL SKIP HEADER FUNCTION AND CHECK IF SUCCESS OR FAILURE.
        {
            if(decode_magic_string(decInfo) == d_success)   //CALL DECODE MAGIC STRING FUNCTION AND CHECK IF SUCCESS OR FAILURE. 
            {
                printf("------------------------------------------\n");
                printf("Decoding of Magic\nString \t\t     : " GREEN "SUCCESSFULL\n" RESET);
                if(decode_secret_file_extn_size(decInfo) == d_success)      //CALL DECODE SECRET FILE EXTN SIZE FUNCTION AND CHECK IF SUCCESS OR FAILURE.
                {
                    printf("------------------------------------------\n");
                    printf("Decoding of Secret\nFile Extension size  : " GREEN "SUCCESSFULL\n" RESET);
                    if(decode_secret_file_extn(decInfo) == d_success)       //CALL DECODE SECRET FILE EXTN FUNCTION AND CHECK IF SUCCESS OR FAILURE.
                    {
                        printf("------------------------------------------\n");
                        printf("Decoding of Secret\nFile Extension \t     : " GREEN "SUCCESSFULL\n" RESET);
                        if(decode_secret_file_size(decInfo) == d_success)   //CALL DECODE SECRET FILE SIZE FUNCTION AND CHECK IF SUCCESS OR FAILURE.
                        {
                            printf("------------------------------------------\n");
                            printf("Decoding of Secret\nFile Size \t     : " GREEN "SUCCESSFULL\n" RESET);
                        }
                        if(decode_secret_file_data(decInfo) == d_success)   //CALL DECODE SECRET FILE DATA FUNCTION AND CHECK IF SUCCESS OR FAILURE.
                        {
                            printf("------------------------------------------\n");
                            printf("Decoding of Secret\nFile Data \t     : " GREEN "SUCCESSFULL\n" RESET);
                            return d_success;
                        }
                        else
                        {
                            printf(RED "Error decoding file data.\n" RESET);
                            return d_failure;
                        }
                    }
                    else
                    {
                        printf(RED "Error decoding file extension.\n" RESET);
                        return d_failure;
                    }
                }
                else
                {
                    printf(RED "Error decoding file extension size.\n" RESET);
                    return d_failure;
                }
            }
            else
            {
                printf(RED "Error decoding Magic String.\n" RESET);
                return d_failure;
            }
        }
        else
        {
            printf(RED "Error skipping BMP header.\n" RESET);
            return d_failure;
        }
    }
    else
    {
        return d_failure;
    }
}
