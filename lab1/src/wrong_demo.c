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
     Error: invalid suffix "abc" on constant 0
     Error: invalid suffix ".123" on constant 1e-2
     Warning: multi-character character constant 'gg'
     Error: invalid suffix "_0011" on constant 0b1110
     Error: invalid suffix "u" on constant 1e1
     Error: invalid suffix "e1" on constant 1e1
     */
    printf("#Hello, World!\n");
    int 0abc;                         // error
    long long   a      = 0llu;        // pass
    float       b      = .01f;        // pass
    double      c      = -3.14159;    // pass
    float       d      = -1.414E-2f;  // pass
    char        AbCd_e = 'a';         // pass
    double      f      = 1e-2.123;    // error
    double      g      = 1e-1;        // pass
    double      h      = 1.1e+1 - 1;  // pass
    char        i      = 'gg';        // warning
    char        j      = '\n';        // pass
    int         k      = 0x11;        // pass
    int         l      = 011;         // pass
    int         m      = 0b11;        // pass
    int         n      = 0b1110_0011; // error
    long long   o      = 11LLU;       // pass
    long double p      = 1e1l;        // pass
    long double q      = 1e1u;        // error
    long double r      = 1e1e1;       // error
    if (b * c <= d || b * c >= a)
        printf("b*c is less than or \\
        equal to d or greater than a\n"); // warning
    else
        printf("b*c is greater than d
         and less than a\n");   //error

    return 0;
}
