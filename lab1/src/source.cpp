#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>

#define STOPWHENERROR 0 // if 1, STOP analysis when error occurs

using namespace std;

static const unsigned int TOKEN_LEN = 32;
static const unsigned int CODE_LEN = 100000;

// 保留字表 C89 04 - 35
static char ReserveWords[32][20] = {
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

bool isDigit(char c) {
    /*
     check if the character is a digit
     @param c: the character to be checked
     @return: the result of the check, true if the character is a digit
     */
    return c >= '0' && c <= '9';
}

bool isLetter(char c) {
    /*
     check if the character is a letter
     @param c: the character to be checked
     @return: the result of the check, true if the character is a letter
     */
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool startWithOperatorOrDelimiter(char c) {
    /*
     check if the character is a operator or delimiter
     @param c: the character to be checked
     @return: the result of the check, true if the character is a operator or delimiter
     */
    for (int i = 0; i < 36; i++) {
        if (c == OperatorOrDelimiter[i][0]) {
            return true;
        }
    }
    return false;
}

bool isTheSecondOperatorOrDelimiter(char c) {
    /*
     check if the character is a operator or delimiter
     @param c: the character to be checked
     @return: the result of the check, true if the character is a operator or delimiter
     */
    for (int i = 0; i < 36; i++) {
        if (c == OperatorOrDelimiter[i][1] and c != '\0') {
            return true;
        }
    }
}

int isReservedWord(char *str) {
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

int isOperatorOrDelimiter(char *str) {
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

bool filter(char *str, int len) {
    /*
     preprocessing code, filter comments and some other stuff
     @param str: the original code to be filtered
     @param len: the length of the original code
     */
    char tmp[CODE_LEN];
    int p = 0;
    for (int i = 0; i < len; i++) {
        if ((str[i] == '/' and str[i + 1] == '/') or str[i] == '#') { // filter single line comments or preprocessor directives
            while (str[i] != '\n') i++;
        }
        if (str[i] == '/' and str[i + 1] == '*') { // filter multiline comments
            i += 2;
            while (str[i] != '*' and str[i + 1] != '/') {
                if (str[i] == '\0') {
                    cout << "error: unclosed comment" << endl;
                    return false;
                }
                i++;
            }
            i += 2;
        }
        if (str[i] != '\n' and str[i] != '\t' and str[i] != '\v' and str[i] != '\r') {
            tmp[p++] = str[i];
        }
    }
    tmp[p] = '\0';
    // cout << p << endl;
    strcpy(str, tmp);
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
    char c = str[p]; // ?
    while (c == ' ') c = str[++p];
    for (auto i = 0; i < TOKEN_LEN; i++) token[i] = '\0'; // clear token
    if (isLetter(str[p])) {
        // start with a letter must be a reserved word or an identifier
        token[token_index++] = str[p++];
        while (isLetter(str[p]) or isDigit(str[p]) or str[p] == '_') {
            token[token_index++] = str[p++];
        }
        token[token_index] = '\0';
        syn = isReservedWord(token);
        if (syn == -1) {
            syn = 0; // if not a reserved word, it is an identifier
        }
        return true;
    } else if (isDigit(str[p])) {
        // start with a digit must be a number
        token[token_index++] = str[p++];
        bool exist_e = false;
        bool exist_dot = false;
        while (isDigit(str[p]) or str[p] == '.' or str[p] == 'e' or str[p] == 'E' or str[p] == '-') {
            if (str[p] == 'e' or str[p] == 'E') { // scientific notation
                if (exist_e) {
                    cout << "error: invalid number" << endl;
                    return false;
                }
                exist_e = true;
                exist_dot = true; // scientific notation can't have dot after e
            }
            if (str[p] == '.') {
                if (exist_dot) {
                    cout << "error: invalid number" << endl;
                    return false;
                }
                exist_dot = true;
            }
            if (str[p] == '-') {
                if (!(str[p - 1] == 'e' or str[p - 1] == 'E')) {
                    cout << "error: invalid number" << endl;
                    return false;
                }
            }
            token[token_index++] = str[p++];
        }
        token[token_index] = '\0';
        syn = 3; // number constant
        return true;
    } else if (str[p] == '\0') {
        // end of the code
        // token[0] = '\0';
        syn = -1;
        return true;
    } else if (str[p] == '\'') {
        // start with a single quote must be a character constant
        if (str[p + 2] != '\'') {
            cout << "error: invalid character constant" << endl;
            return false;
        }
        for (auto i = 0; i < 3; i++) token[token_index++] = str[p++];
        token[token_index] = '\0';
        syn = 1; // character constant
        return true;
    } else if (str[p] == '\"') {
        // start with a double quote must be a string constant
        token[token_index++] = str[p++];
        while (str[p] != '\"') {
            token[token_index++] = str[p++];
            if (str[p] == '\0') {
                cout << "error: invalid string constant" << endl;
                return false;
            }
        }
        token[token_index++] = str[p++];
        token[token_index] = '\0';
        syn = 2; // string constant
        return true;
        // } else if (str[p] == '#') {
        //     // start with a # must be a preprocessor directive
        //     token[token_index++] = str[p++];
        //     while (str[p] != '\n') {
        //         token[token_index++] = str[p++];
        //     }
        //     token[token_index] = '\0';
        //     syn = 4;
        //     return;
    } else if (startWithOperatorOrDelimiter(str[p])) {
        // start with an operator or delimiter must be an operator or delimiter
        token[token_index++] = str[p++];
        if (isTheSecondOperatorOrDelimiter(str[p])) {
            token[token_index++] = str[p++];
            char tmp_s[3] = {token[0], token[1], '\0'};
            syn = isOperatorOrDelimiter(tmp_s);
        } else {
            char tmp_s[3] = {token[0], '\0', '\0'};
            syn = isOperatorOrDelimiter(tmp_s);
        }
        token[token_index] = '\0';
        return true;
    } else {
        return false;
    }
}

int main() {

    char code[CODE_LEN];
    char token[TOKEN_LEN];

    int syn = -2;
    int p = 0;

    ifstream in("demo.c");
    ofstream out("tokens.txt");

    // Read from file
    cout << "Reading source code from file..." << endl;

    if (!in.is_open()) {
        cout << "error: cannot open file" << endl;
        return -1;
    }

    while (!in.eof()) {
        in.get(code[p++]);
    }
    code[p++] = '\0';
    // cout << "input:" << endl
    //      << code << endl;
    in.close();

    cout << "Filtering code..." << endl;
    if (!filter(code, p)) return 0;
    // cout << "filtered:" << endl
    //      << code << endl;

    p = 0;
    cout << "Starting lexical syntax analysis..." << endl;
    while (syn != -1) {

#if STOPWHENERROR
        if (!scanner(code, token, syn, p)) {
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

    out.close();

    cout << "Lexical syntax analysis complete." << endl
         << "See tokens.txt" << endl;

    return 0;
}
