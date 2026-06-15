#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"
#define RED       "\033[31m"  //define a macro name for colors
#define RESET     "\033[0m"
#define GREEN     "\033[32m"
#define BBLUE     "\033[94m"
#define BCYAN     "\033[96m"
#define BMAGENTA  "\033[95m"
#define ORANGE    "\033[38;5;208m"
OperationType check_operation_type(char *);

int main(int argc, char *argv[])
{
    EncodeInfo encInfo;   //declare structure variable.
    DecodeInfo decInfo;
    if(argc == 1)
    {
        printf(BMAGENTA "--> Enter '--help' to know the number of arguments to be passed based on operation.\n" RESET);
        return e_failure;
    }
    if(argc<3) //check if argument count is correct.
    {
        if(argc == 2 && (strcmp(argv[1],"--help") == 0))
        {
            printf("HELP OPTION : \n");
            printf("Encoding ---> ./a.out -e <.bmp_file> <.text_file> [output file]\nDecoding ---> ./a.out -d <.bmp_file> [output file]\n");
            return e_failure;
        }
        else
        {
            printf(RED "Insufficient number of arguments.\nEnter <.bmp_file> and <.text_file>\n" RESET);
            return e_failure;
        }
    }
    if(check_operation_type(argv[1]) == e_encode) //call function and check what is the 2nd argument. 
    {
        if(argc == 4 || argc == 5)  //check valid number of argument count.
        {
            if(read_and_validate_encode_args(argv,&encInfo) == e_success)  //call function for doing the validations on the arguments. And check if return is success or failure.
            {
                printf("------------------------------------------\n");
                printf(GREEN "All validations passed successfully.\n" RESET);  //if validations passed, then print success message.
                if(do_encoding(&encInfo) == e_success)   //Perform the encoding process by passing the address of structure. And check if return is success or failure.
                {
                    printf("------------------------------------------\n");
                    printf(GREEN "ENCODING SUCCESSFULLY COMPLETED TO "ORANGE "%s "RESET GREEN"FILE.\n" RESET,encInfo.dest_image_fname);  //if encoding passed, print success message.
                    printf("------------------------------------------\n");
                }
                else
                {
                    return e_failure;  //return e_failure if encoding process failed.
                }
            }
            else
            {
                return e_failure;  //return e_failure if validations failed.
            }
        }
        else
        {
            printf(RED "Secret file name is missing.\n" RESET);
            return e_failure;
        }
    }
    else if(check_operation_type(argv[1]) == e_decode) //call function and check what is the 2nd argument.
    {
        if(argc==3 || argc==4)  //check valid number of argument count.
        {
            if(read_and_validate_decode_args(argv,&decInfo) == d_success)   //call function for doing the validations on the arguments. And check if return is success or failure.
            {
                printf("------------------------------------------\n");
                printf(GREEN "All validations passed successfully.\n" RESET);   //if validations passed, then print success message.
                if(do_decoding(&decInfo) == d_success)  //Perform the decoding process by passing the address of structure. And check if return is success or failure.
                {
                    printf("------------------------------------------\n");
                    // printf("%s\n",decInfo.dest_fname);
                    printf(GREEN "DECODING SUCCESSFULLY COMPLETED TO" ORANGE " %s " RESET GREEN "FILE.\n" RESET,decInfo.dest_fname);    //if decoding passed, print success message.
                    printf("------------------------------------------\n");
                }
                else
                {
                    return d_failure;   //return e_failure if encoding process failed.
                }
            }
            else
            {
                return d_failure;   //return e_failure if validations failed.
            }
        }
        else
        {
            return d_failure;  //return e_failure when argument count is not proper.
        }
    }
    else
    {
        printf(RED "Unsupported arguments.\n" RESET);
        return d_failure;
    }
}

OperationType check_operation_type(char *symbol)
{
    if(strcmp(symbol,"-e") == 0)   //check if 2nd argument is '-e' or not. if yes return e_encode.
    {
        return e_encode;
    }
    else if(strcmp(symbol,"-d") == 0)  //else check 2nd argument is '-d' or not. if yes return e_decode.
    {
        return e_decode;
    }
    else
        return e_unsupported;   //else return e_unsupported. 
}
