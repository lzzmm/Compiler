#include <stdio.h>
// ----------------------------------------------------------------
// Function: main
// Purpose:  main function
// Parameters:
//     int argc - number of arguments
//     char *argv[] - array of arguments
// Returns:
//     int - 0 if successful, 1 if error
// ----------------------------------------------------------------
int main(int argc, char *argv[]) {
    /*
     Test multiline comment
     int a = 0;
     int b = 0;
     int c = a + b;
     printf("Hello, World!\n");
     */
    printf("Hello, World!\n");
    static const unsigned int a = 0;
    float b = 0.01;
    double c = -3.14159;
    double d = -1.414e-2;
    char AbCd_e = 'a';
    if(b*c <= d || b*c >= a) printf("b*c is less than or equal to d or greater than a\n");
    else printf("b*c is greater than d and less than a\n");

    return 0;
}
