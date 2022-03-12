#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>

#define STOPWHENERROR false // if true, STOP analysis when error occurs
#define NOMULTICHAR   false // if true, multi-char char tokens are not allowed
#define INFILE        "wrong_demo.c"
#define OUTFILE       "tokens_wd.txt"
#define IDENTIFIER    00
#define CHARACTER     01
#define STRING        02
#define NUMBER        03
#define ENDOFCODE     99

/*
 TODO:
 1. error reporting (50%)
 2. test all occurences (tbd)
 3. write report (tbd)
 4. make the code more clean and beautiful (50%)
*/
using namespace std;

static const unsigned int TOKEN_LEN = 64;     // max length of a token
static const unsigned int CODE_LEN  = 100000; // max length of code

// reserve words table C89 04 - 35
static char ReserveWords[32][12]    = {
    "auto", "break", "case", "char", "const", "continue",
    "default", "do", "double", "else", "enum", "extern",
    "float", "for", "goto", "if", "int", "long",
    "register", "return", "short", "signed", "sizeof", "static",
    "struct", "switch", "typedef", "union", "unsigned", "void",
    "volatile", "while"};

// operator or delimiter table C89 36 - 71
static char OperatorOrDelimiter[36][3] = {
    "+", "-", "*", "/", "<", ">", "=", "!", ";",
    "(", ")", "^", ",", "\"", "\'", "#", "&", "|",
    "%", "~", "[", "]", "{", "}", "\\", ".", "\?", ":",
    "&&", "||", "<<", ">>", "==", "<=", "!=", ">="};

inline bool isDigit(const char c) {
    /*
     check if the character is a digit
     @param c: the character to be checked
     @return: true if the character is a digit
     */
    return c >= '0' && c <= '9';
}

inline bool isLetter(const char c) {
    /*
     check if the character is a letter
     @param c: the character to be checked
     @return: true if the character is a letter
     */
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

inline int isReservedWord(const char *str) {
    /*
     check if the string is a reserved word
     @param str: the string to be checked
     @return: the index of the reserved word in the array, -1 if not found
     */
    for (int i = 0; i < 32; i++) {
        if (strcmp(str, ReserveWords[i]) == 0) {
            return i + 4;
        }
    }
    return -1;
}

inline bool isTheFirstCharOfOperatorOrDelimiter(const char c) {
    /*
     check if the character is the first character of a operator or delimiter
     @param c: the character to be checked
     @return: true if the character is the first character of a operator or delimiter
     */
    for (int i = 0; i < 36; i++) {
        if (c == OperatorOrDelimiter[i][0]) {
            return true;
        }
    }
    return false;
}

inline bool isTheSecondCharOfOperatorOrDelimiter(const char c) {
    /*
     check if the character is the second character of a operator or delimiter
     @param c: the character to be checked
     @return: true if the character is the second character of a operator or delimiter
     */
    if (c == '\0') return false;
    for (int i = 28; i <= 35; i++) {
        // there is no 2-character operator or delimiter before 28
        if (c == OperatorOrDelimiter[i][1]) {
            return true;
        }
    }
    return false;
}

inline int isOperatorOrDelimiter(const char *str) {
    /*
     check if the string is an operator or delimiter
     @param str: the string to be checked
     @return: the index of the operator or delimiter in the array, -1 if not found
     */
    for (int i = 0; i < 36; i++) {
        if (strcmp(str, OperatorOrDelimiter[i]) == 0) {
            return i + 36;
        }
    }
    return -1;
}

inline bool tokenStartWithLetter(const char *str, char *token, int &syn, int &p,
                                 unsigned int &token_index) {
    /*
     generate a token start with a letter
     @return: always return true
     */
    token[token_index++] = str[p++];

    while (isLetter(str[p]) || isDigit(str[p]) || str[p] == '_') {
        token[token_index++] = str[p++];
    }

    token[token_index] = '\0';
    syn                = isReservedWord(token);
    if (syn == -1) {
        syn = IDENTIFIER; // if not a reserved word, it is an identifier
    }
    return true;
}

inline void handleInvalidSuffix(const char *str, char *token, int &p) {
    /*
     handle invalid suffix
     */
    static const int suffix_len             = 10;
    char             suffix[suffix_len + 4] = {'\0'};
    unsigned int     suffix_index           = 0;
    while (!isTheFirstCharOfOperatorOrDelimiter(str[p]) &&
               str[p] != ' ' && str[p] != '\0' ||
           str[p] == '.') {
        if (suffix_index < suffix_len - 1) {
            suffix[suffix_index++] = str[p++];
        } else if (suffix_index == suffix_len) {
            for (auto i = 0; i < 3; i++) suffix[suffix_index++] = '.';
            suffix[suffix_index] = '\0';
        } else {
            p++;
        }
    }
    cout << "Error: invalid suffix \"" << suffix << "\" on constant " << token << endl;
}

inline bool octalNumber(const char *str, char *token, int &syn, int &p,
                        unsigned int &token_index) {
    /*
     generate a token start with 0
     @return: always return true
     */
    token[token_index++] = str[p++];

    while (isDigit(str[p]) && str[p] <= '7') {
        token[token_index++] = str[p++];
    }

    if (!isTheFirstCharOfOperatorOrDelimiter(str[p]) &&
        str[p] != ' ' && str[p] != '\0') {
        handleInvalidSuffix(str, token, p);
        return false;
    }

    token[token_index] = '\0';
    // NOTE: should oct2dec done in lsa?
    syn                = NUMBER;
    return true;
}

inline bool hexadecimalNumber(const char *str, char *token, int &syn, int &p,
                              unsigned int &token_index) {
    /*
     generate a token start with 0x
     @return: always return true
     */
    token[token_index++] = str[p++];
    token[token_index++] = str[p++];

    while (isDigit(str[p]) || (str[p] >= 'a' && str[p] <= 'f') ||
           (str[p] >= 'A' && str[p] <= 'F')) {
        token[token_index++] = str[p++];
    }

    if (!isTheFirstCharOfOperatorOrDelimiter(str[p]) &&
        str[p] != ' ' && str[p] != '\0') {
        handleInvalidSuffix(str, token, p);
        return false;
    }

    token[token_index] = '\0';
    // NOTE: should hex2dec done in lsa?
    syn                = NUMBER;
    return true;
}

inline bool binaryNumber(const char *str, char *token, int &syn, int &p,
                         unsigned int &token_index) {
    /*
     generate a token start with 0b
     @return: always return true
     */
    token[token_index++] = str[p++];
    token[token_index++] = str[p++];

    while (str[p] == '0' || str[p] == '1') {
        token[token_index++] = str[p++];
    }

    if (!isTheFirstCharOfOperatorOrDelimiter(str[p]) &&
        str[p] != ' ' && str[p] != '\0') {
        handleInvalidSuffix(str, token, p);
        return false;
    }

    token[token_index] = '\0';
    syn                = NUMBER;
    return true;
}

inline void handleTypeSuffix(const char *str, char *token, int &p,
                             unsigned int &token_index, const bool is_double) {
    /*
     handle type suffix U L F
     */
    if (is_double) { // only f/F or l/L is valid suffix on double
        if (str[p] == 'F' || str[p] == 'f') {
            token[token_index++] = str[p++];
        } else if (str[p] == 'L' || str[p] == 'l') {
            token[token_index++] = str[p++];
        }
    } else { // u/U l/L ll/LL are valid suffix on int
        if (str[p] == 'L') {
            token[token_index++] = str[p++];
            if (str[p] == 'L') token[token_index++] = str[p++];                  // LL
            if (str[p] == 'U' || str[p] == 'u') token[token_index++] = str[p++]; // LLU/LLu/LU/Lu
        } else if (str[p] == 'l') {
            token[token_index++] = str[p++];
            if (str[p] == 'l') token[token_index++] = str[p++];                  // ll
            if (str[p] == 'U' || str[p] == 'u') token[token_index++] = str[p++]; // llU/llu/lU/lu
        } else if (str[p] == 'U' || str[p] == 'u') {
            token[token_index++] = str[p++];
            if (str[p] == 'L') {
                token[token_index++] = str[p++];                    // UL/uL
                if (str[p] == 'L') token[token_index++] = str[p++]; // ULL/uLL
            } else if (str[p] == 'l') {
                token[token_index++] = str[p++];                    // Ul/ul
                if (str[p] == 'l') token[token_index++] = str[p++]; // Ull/ull
            }
        }
    }
}

inline bool decimalNumber(const char *str, char *token, int &syn, int &p,
                          unsigned int &token_index) {
    /*
     generate a decimal token start with a digit
     @return: always return true
     */
    bool exist_e           = false;
    bool exist_dot         = false;
    bool is_invalid_number = false;
    if (str[p] != '.') token[token_index++] = str[p++]; // .01f is also a number

    while (isDigit(str[p]) || str[p] == '.' or
           str[p] == 'e' || str[p] == 'E' or
           ((str[p] == '-' || str[p] == '+') &&
            (str[p - 1] == 'e' || str[p - 1] == 'E'))) {
        if (str[p] == 'e' || str[p] == 'E') { // scientific notation
            if (exist_e) {
                handleInvalidSuffix(str, token, p);
                is_invalid_number = true;
            }
            exist_e   = true;
            exist_dot = true; // scientific notation can't have dot after e
        }
        if (str[p] == '.') {
            if (exist_dot) {
                handleInvalidSuffix(str, token, p);
                is_invalid_number = true;
            }
            exist_dot = true;
        }
        token[token_index++] = str[p++];
    }

    // type suffix
    if (str[p] == 'f' || str[p] == 'u' || str[p] == 'l' ||
        str[p] == 'F' || str[p] == 'U' || str[p] == 'L') {
        handleTypeSuffix(str, token, p, token_index, exist_dot || exist_e);
    }

    if (!isTheFirstCharOfOperatorOrDelimiter(str[p]) &&
        str[p] != ' ' && str[p] != '\0') {
        handleInvalidSuffix(str, token, p);
        return false;
    }

    token[token_index] = '\0';
    syn                = NUMBER;
    if (is_invalid_number) {
        // cout << "Error: invalid number " << token << endl;
        return false;
    }
    return true;
}

// inline void checkInvalidSuffixOnConstants(const char c) {}

inline bool tokenStartWithDigit(const char *str, char *token, int &syn, int &p,
                                unsigned int &token_index) {
    /*
     generate a token start with a digit
     @return: true if no error occurs
     */
    if (str[p] == '0' && (str[p + 1] == 'x' || str[p + 1] == 'X')) { // hexadecimal
        return hexadecimalNumber(str, token, syn, p, token_index);
    } else if (str[p] == '0' && (str[p + 1] == 'b' || str[p + 1] == 'B')) { // binary
        return binaryNumber(str, token, syn, p, token_index);
    } else if (str[p] == '0' && (isDigit(str[p + 1]))) { // octal
        return octalNumber(str, token, syn, p, token_index);
    } else { // decimal
        return decimalNumber(str, token, syn, p, token_index);
    }
}

inline bool tokenStartWithOperatorOrDelimiter(const char *str, char *token, int &syn, int &p,
                                              unsigned int &token_index) {
    /*
     generate a token start with an operator or delimiter
     @return: always return true
     */
    if (str[p] == '.' && isDigit(str[p + 1])) return decimalNumber(str, token, syn, p, token_index);
    token[token_index++] = str[p++];
    if (isTheSecondCharOfOperatorOrDelimiter(str[p])) {
        token[token_index++]   = str[p++];
        char three_char_str[3] = {token[0], token[1], '\0'};
        syn                    = isOperatorOrDelimiter(three_char_str);
    } else {
        char three_char_str[3] = {token[0], '\0', '\0'};
        syn                    = isOperatorOrDelimiter(three_char_str);
    }
    token[token_index] = '\0';
    return true;
}

inline bool tokenStartWithSingleQuote(const char *str, char *token, int &syn, int &p,
                                      unsigned int &token_index) {
    /*
     generate a token start with a single quote
     @return: true if no error occurs
     */
    do {
        token[token_index++] = str[p++];
    } while (str[p] != '\'');

    token[token_index++] = str[p++];
    token[token_index]   = '\0';
    syn                  = CHARACTER;

    if (!(token[1] == '\\' && token[3] == '\'') && token[2] != '\'') {
#if !NOMULTICHAR
        cout << "Warning: multi-character character constant " << token << endl;
#endif
#if NOMULTICHAR
        cout << "Error: multi-character character constant " << token << endl;
        return false;
#endif
    }

    return true;
}

inline bool tokenStartWithDoubleQuote(const char *str, char *token, int &syn, int &p,
                                      unsigned int &token_index) {
    /*
     generate a token start with a double quote
     @return: true if no error occurs
     */
    // token[token_index++] = str[p++];

    do {
        token[token_index++] = str[p++];
        if (str[p] == '\0') {
            cout << "Error: invalid string constant: unclosed string" << endl;
            return false;
        }
    } while (str[p] != '\"');

    token[token_index++] = str[p++];
    token[token_index]   = '\0';
    syn                  = STRING;
    return true;
}

bool filter(char *str, const int len) {
    /*
     preprocessing code, filter comments and some other stuff
     @param str: the original code to be filtered
     @param len: the length of the original code
     */
    char filteredCode[CODE_LEN];
    int  p = 0;

    for (int i = 0; i < len; i++) {
        if ((str[i] == '/' && str[i + 1] == '/') ||
            (str[i] == '#' && (i == 0 || str[i - 1] == '\n'))) {
            // filter single line comments or preprocessor directives
            // preprocessor directives should have other methods to deal with
            while (str[i] != '\n') i++;
        }
        if (str[i] == '/' && str[i + 1] == '*') { // filter multiline comments
            i += 2;
            while (str[i] != '*' || str[i + 1] != '/') {
                if (str[i] == '\0') {
                    cout << "Error: unclosed comment" << endl;
                    return false;
                }
                i++;
            }
            i += 2;
        }
        if (str[i] != '\n' && str[i] != '\t' &&
            str[i] != '\v' && str[i] != '\r') { // filter tabulator key
            filteredCode[p++] = str[i];
        }
    }

    filteredCode[p] = '\0';
    strcpy(str, filteredCode);
    return true;
}

bool scanner(char *str, char *token, int &syn, int &p) {
    /*
     scan the code and generate tokens
     @param str: the code to be scanned
     @param token: the token to be generated
     @param syn: the index of the token in the array
     @param p: the index of the current character in the code
     */

    unsigned int token_index = 0;

    // skip the spaces
    while (str[p] == ' ') p++;

    // clear token
    while (token[token_index] != '\0') token[token_index++] = '\0';
    token_index = 0;

    if (isLetter(str[p])) {
        // start with a letter must be a reserved word or an identifier
        return tokenStartWithLetter(str, token, syn, p, token_index);
    } else if (isDigit(str[p])) {
        // start with a digit must be a number
        return tokenStartWithDigit(str, token, syn, p, token_index);
    } else if (str[p] == '\'') {
        // start with a single quote must be a character constant
        return tokenStartWithSingleQuote(str, token, syn, p, token_index);
    } else if (str[p] == '\"') {
        // start with a double quote must be a string constant
        return tokenStartWithDoubleQuote(str, token, syn, p, token_index);
    } else if (isTheFirstCharOfOperatorOrDelimiter(str[p])) {
        // must after the judgement of charater and string, because \' and \" is also a delimiter
        // start with an operator or delimiter must be an operator or delimiter
        return tokenStartWithOperatorOrDelimiter(str, token, syn, p, token_index);
    } else if (str[p] == '\0') {
        // end of the code
        syn = ENDOFCODE;
        return true;
    } else {
        return false;
    }
}

int main(int argc, char *argv[]) {
    int      syn = -2;
    int      p   = 0;
    char     code[CODE_LEN];
    char     token[TOKEN_LEN];
    ifstream in(INFILE);
    ofstream out(OUTFILE);

    cout << "Reading source code from \"" << INFILE << "\"..." << endl;
    if (!in.is_open()) {
        cout << "Error: cannot open file" << endl;
        return -1;
    }
    while (!in.eof()) {
        in.get(code[p++]);
    }
    code[p++] = '\0';
    in.close();

    cout << "Filtering code..." << endl;
    if (!filter(code, p)) {
        // #if STOPWHENERROR // if not stopped, lsa will occur runtime error
        cout << "Filter terminated with error." << endl;
        return 0;
        // #endif
    }
    p = 0;

    cout << "Starting lexical syntax analysis..." << endl;
    while (syn != ENDOFCODE) {
#if STOPWHENERROR
        if (!scanner(code, token, syn, p)) {
            cout << "Scanner terminated." << endl;
            return 0;
        }
#endif
#if !STOPWHENERROR
        scanner(code, token, syn, p);
#endif
        if (syn >= 0 && syn <= 71) {
            out << "<" << token << ", " << syn << ">" << endl;
            // cout << "<" << token << ", " << syn << ">" << endl;
        }
    }
    cout << "Lexical syntax analysis complete." << endl
         << "See \"tokens.txt\"." << endl;
    out.close();
    return 0;
}
