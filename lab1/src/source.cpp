#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>

#define STOPWHENERROR false // if true, STOP analysis when error occurs
#define INFILE        "wrong_demo.c"
#define OUTFILE       "tokens1.txt"
#define IDENTIFIER    00
#define CHARACTER     01
#define STRING        02
#define NUMBER        03
#define ENDOFCODE     99

/*
 TODO:
 1. error reporting
 2. test all occurences
 3. write report
 4. make the code more clean and beautiful
*/
using namespace std;

static const unsigned int TOKEN_LEN = 32;     // max length of token
static const unsigned int CODE_LEN  = 100000; // max length of code

// 保留字表 C89 04 - 35
static char ReserveWords[32][20]    = {
    "auto", "break", "case", "char", "const", "continue",
    "default", "do", "double", "else", "enum", "extern",
    "float", "for", "goto", "if", "int", "long",
    "register", "return", "short", "signed", "sizeof", "static",
    "struct", "switch", "typedef", "union", "unsigned", "void",
    "volatile", "while"};

// 界符运算符 C89 36 - 71
static char OperatorOrDelimiter[36][3] = {
    "+", "-", "*", "/", "<", "<=", ">", ">=", "=", "==",
    "!=", ";", "(", ")", "^", ",", "\"", "\'", "#", "&",
    "&&", "|", "||", "%", "~", "<<", ">>", "[", "]", "{",
    "}", "\\", ".", "\?", ":", "!"};

inline bool isDigit(const char c) {
    /*
     check if the character is a digit
     @param c: the character to be checked
     @return: the result of the check, true if the character is a digit
     */
    return c >= '0' and c <= '9';
}

inline bool isLetter(const char c) {
    /*
     check if the character is a letter
     @param c: the character to be checked
     @return: the result of the check, true if the character is a letter
     */
    return (c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z');
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
     @return: the result of the check, true if the character is the first character of a operator or delimiter
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
     @return: the result of the check, true if the character is the second character of a operator or delimiter
     */
    if (c == '\0') return false;
    for (int i = 5; i < 27; i++) { // there is no 2-character operator or delimiter before 5 or after 26
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

inline bool tokenStartWithLetter(const char *str, char *token, int &syn, int &p, unsigned int &token_index) {
    /*
     generate a token start with letter
     */
    token[token_index++] = str[p++];

    while (isLetter(str[p]) or isDigit(str[p]) or str[p] == '_') {
        token[token_index++] = str[p++];
    }

    token[token_index] = '\0';
    syn                = isReservedWord(token);
    if (syn == -1) {
        syn = IDENTIFIER; // if not a reserved word, it is an identifier
    }
    return true;
}

inline bool tokenStartWithDigit(const char *str, char *token, int &syn, int &p, unsigned int &token_index) {
    token[token_index++]   = str[p++];
    bool exist_e           = false;
    bool exist_dot         = false;
    bool is_invalid_number = false;

    while (isDigit(str[p]) or str[p] == '.' or str[p] == 'e' or str[p] == 'E' or (str[p] == '-' and str[p - 1] == 'e' or str[p - 1] == 'E')) {
        if (str[p] == 'e' or str[p] == 'E') { // scientific notation
            if (exist_e) {
                is_invalid_number = true;
            }
            exist_e   = true;
            exist_dot = true; // scientific notation can't have dot after e
        }
        if (str[p] == '.') {
            if (exist_dot) {
                is_invalid_number = true;
            }
            exist_dot = true;
        }
        // if (str[p] == '-') {
        //     if (!(str[p - 1] == 'e' or str[p - 1] == 'E')) {
        //         is_invalid_number = true;
        //     }
        // }
        token[token_index++] = str[p++];
    }

    token[token_index] = '\0';
    syn                = NUMBER;
    if (is_invalid_number) {
        cout << "Error: invalid number " << token << endl;
        return false;
    }
}

inline bool tokenStartWithOperatorOrDelimiter(const char *str, char *token, int &syn, int &p, unsigned int &token_index) {
    token[token_index++] = str[p++];
    if (isTheSecondCharOfOperatorOrDelimiter(str[p])) {
        token[token_index++] = str[p++];
        char tokenToStr[3]   = {token[0], token[1], '\0'};
        syn                  = isOperatorOrDelimiter(tokenToStr);
    } else {
        char tokenToStr[3] = {token[0], '\0', '\0'};
        syn                = isOperatorOrDelimiter(tokenToStr);
    }
    token[token_index] = '\0';
    return true;
}

inline bool tokenStartWithSingleQuote(const char *str, char *token, int &syn, int &p, unsigned int &token_index) {
    if ((str[p + 1] == '\\' and str[p + 3] != '\'') or str[p + 2] != '\'') {
        cout << "Error: invalid character constant" << endl;
        return false;
    }
    unsigned int token_len = str[p + 1] == '\\' ? 4 : 3;

    for (auto i = 0; i < token_len; i++) token[token_index++] = str[p++];

    token[token_index] = '\0';
    syn                = CHARACTER;
    return true;
}

inline bool tokenStartWithDoubleQuote(const char *str, char *token, int &syn, int &p, unsigned int &token_index) {

    token[token_index++] = str[p++];

    while (str[p] != '\"') {
        token[token_index++] = str[p++];
        if (str[p] == '\0') {
            cout << "Error: invalid string constant" << endl;
            return false;
        }
    }

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
        if ((str[i] == '/' and str[i + 1] == '/') or str[i] == '#') { // filter single line comments or preprocessor directives
            while (str[i] != '\n') i++;
        }
        if (str[i] == '/' and str[i + 1] == '*') { // filter multiline comments
            i += 2;
            while (str[i] != '*' or str[i + 1] != '/') {
                if (str[i] == '\0') {
                    cout << "Error: unclosed comment" << endl;
                    return false;
                }
                i++;
            }
            i += 2;
        }
        if (str[i] != '\n' and str[i] != '\t' and str[i] != '\v' and str[i] != '\r') { // filter tabulator key
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
    // for (auto i = 0; i < TOKEN_LEN; i++) token[i] = '\0';
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

int main() {

    char code[CODE_LEN];
    char token[TOKEN_LEN];
    int  syn = -2;
    int  p   = 0;

    ifstream in(INFILE);
    ofstream out(OUTFILE);

    // Read from file
    cout << "Reading source code from file..." << endl;

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
            cout << "Scanner terminated with error." << endl;
            return 0;
        }
#endif
#if !STOPWHENERROR
        scanner(code, token, syn, p);
#endif
        if (syn >= 0 and syn <= 71) {
            out << "<" << token << ", " << syn << ">" << endl;
            // cout << "<" << token << ", " << syn << ">" << endl;
        }
    }

    cout << "Lexical syntax analysis complete." << endl
         << "See \"tokens.txt\"." << endl;

    out.close();

    return 0;
}
