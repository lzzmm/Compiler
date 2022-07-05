#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <stdarg.h>
#include <string>
#include <vector>

using namespace std;

const size_t EXPRESSION_MAX_LENGTH = 128;
const size_t ERROR_MSG_MAX_LENGTH  = 128;
const string SEPARATE_CHARACTER    = "-"; // separate character in quaternary expression

//  first is analysis result, second is attribute
typedef pair<bool, string> Status;

class QuadrupleForm {
private:
    string op;  // operator
    string op1; // operand 1
    string op2; // operand 2
    string res; // result

public:
    QuadrupleForm() {}
    QuadrupleForm(string _op, string _op1, string _op2, string _res)
        : op(_op), op1(_op1), op2(_op2), res(_res) {}
    friend ostream &operator<<(ostream &os, const QuadrupleForm &q);
};

char str[EXPRESSION_MAX_LENGTH];
char sym;                // current input symbol
int  g_len_str;          // length of str
int  g_cur_char_idx = 0; // current sym index in str
int  g_cur_temp_idx = 0; // current index of temp variable

vector<QuadrupleForm> quadrupleFormList;          // list of quaternary
bool                  syntaxErrorFounded = false; // if syntax error founded

inline bool isLowerCase(char ch);
bool        advance();
void        throwSyntaxError(int indexOfSyntaxError, const char errorMsg[], ...);
void        error(const char errorMsg[]);
void        error(int indexOfSyntaxError, const char errorMsg[]);
string      newtemp();
void        emit(string op, string op1, string op2, string res);

// recursive descent subprogram
Status A();
Status E();
Status T();
Status F();
Status V();

int main() {
    FILE *fp;

    fp = fopen("test.txt", "r+");
    // scanf("%[^\n]", str);
    fscanf(fp, "%[^\n]", str);
    printf("Expression:\n  %s\n\n", str);
    fclose(fp);

    g_len_str  = strlen(str);
    sym        = str[0];

    Status res = A();

    printf("Quaternary:\n");
    for (vector<QuadrupleForm>::iterator it = quadrupleFormList.begin();
         it != quadrupleFormList.end();
         it++) {
        cout << *it << endl;
    }

    if (!res.first) {
        error("Syntax analysis terminated.");
    } else if (!advance()) {
        throwSyntaxError(g_cur_char_idx,
                         "Missing '#' at the end of the string",
                         str + g_cur_char_idx);
    } else if (sym != '#') {
        throwSyntaxError(g_cur_char_idx,
                         "Unexpected characters \"%s\" appears at the end of the string",
                         str + g_cur_char_idx);
    } else if (g_cur_char_idx != g_len_str) {
        throwSyntaxError(g_cur_char_idx,
                         "Unexpected characters \"%s\" appears at the end of the string",
                         str + g_cur_char_idx);
    }

    return 0;
}

// non-terminal A
Status A() {
    // A -> V=E
    Status statusV = V();
    if (sym == '=') {
        bool   advanceFlag = statusV.first && advance();
        Status statusE     = E();
        emit("=", statusE.second, SEPARATE_CHARACTER, statusV.second);
        return make_pair(statusV.first && advanceFlag && statusE.first, string());
    } else {
        throwSyntaxError(g_cur_char_idx, "Lack of '='");
        return make_pair(false, string("<Syntax Error>"));
    }
}

// non-terminal E
Status E() {
    Status statusT1 = T();      // E -> TE'
    Status statusE  = statusT1; // E.pla=T(1).pla

    while (sym == '+' || sym == '-') {
        char sym_temp   = sym;
        statusE.first   = statusE.first && advance();
        Status statusT2 = T();
        statusE.first   = statusE.first && statusT2.first;
        statusE.second  = newtemp(); // E.pla=newtemp
        if (sym_temp == '+')         // E' -> +TE'
        {
            emit("+", statusT1.second, statusT2.second, statusE.second);
        } else if (sym_temp == '-') // E' -> -TE'
        {
            emit("-", statusT1.second, statusT2.second, statusE.second);
        }
        statusT1.second = statusE.second; // T(1).pla=E.pla
    }
    // E' -> \epsilon

    return statusE;
}

// non-terminal T
Status T() {
    Status statusF1 = F();      // T -> FT'
    Status statusT  = statusF1; // T.pla=F(1).pla

    while (sym == '*' || sym == '/') {
        char sym_temp   = sym;
        statusT.first   = statusT.first && advance();
        Status statusF2 = F();
        statusT.first   = statusT.first && statusF2.first;
        statusT.second  = newtemp(); // T.pla:=newtemp
        if (sym_temp == '*')         // T' -> *FT'
        {
            emit("*", statusF1.second, statusF2.second, statusT.second);
        } else if (sym_temp == '/') // T' -> /FT'
        {
            emit("/", statusF1.second, statusF2.second, statusT.second);
        }
        statusF1.second = statusT.second; // F(1).pla=T.pla
    }
    // T' -> \epsilon

    return statusT;
}

// non-terminal F
Status F() {
    if (sym == '(') {
        // F -> (E)
        bool   advanceFlag = advance();
        Status statusE     = E();
        if (sym == ')') {
            advanceFlag = advanceFlag && advance();
            // F.pla=E.pla
            return make_pair(advanceFlag && statusE.first,
                             statusE.second);
        } else {
            throwSyntaxError(g_cur_char_idx, "Lack of ')'");
            return make_pair(false, string("<Syntax Error>"));
        }
    } else {
        // F -> V
        // F.pla=V.pla
        return V();
    }
}

// non-terminal V
Status V() {
    if (isLowerCase(sym)) {
        // V -> var
        // V.pla= lower case character
        char sym_temp    = sym;
        bool advanceFlag = advance();
        return make_pair(advanceFlag, string(1, sym_temp));
    } else if (sym == '#') {
        throwSyntaxError(g_cur_char_idx,
                         "The parser reaches the end of the string. More characters are needed");
        return make_pair(false, string("<Syntax Error>"));
    } else {
        throwSyntaxError(g_cur_char_idx,
                         "The appearance of '%c' is a syntactic error",
                         sym);
        return make_pair(false, string("<Syntax Error>"));
    }
}

ostream &operator<<(ostream &os, const QuadrupleForm &q) {
    os << setw(4);
    if (q.op2 == SEPARATE_CHARACTER) {
        os << q.res << q.op << q.op1;
    } else {
        os << q.res << "=" << q.op1 << q.op << q.op2;
    }
    os << setw(4) << "\t ";
    os << '('
       << q.op << ','
       << q.op1 << ','
       << q.op2 << ','
       << q.res
       << ')';
    return os;
}

// true if ch is lower case
inline bool isLowerCase(char ch) {
    return (ch >= 'a' && ch <= 'z');
}

// read next symbol
bool advance() {
    g_cur_char_idx++;
    if (g_cur_char_idx < g_len_str) {
        sym = str[g_cur_char_idx];
        return true;
    } else if (g_cur_char_idx == g_len_str) {
        sym = '#';
        return true;
    } else {
        throwSyntaxError(g_cur_char_idx, "End of the string");
        return false;
    }
}

// throw a syntax error
void throwSyntaxError(int indexOfSyntaxError, const char errorMsg[], ...) {
    char errorPerfectInfo[ERROR_MSG_MAX_LENGTH + 1] = "Syntax error: ";

    va_list argsPtr;
    va_start(argsPtr, errorMsg);
    vsprintf(errorPerfectInfo + strlen(errorPerfectInfo), errorMsg, argsPtr);
    va_end(argsPtr);

    error(indexOfSyntaxError, errorPerfectInfo);
    syntaxErrorFounded = true;
}

// print error message
void error(const char errorMsg[]) {
    cout << errorMsg << endl;
}

// print error message with index of syntax error
void error(int indexOfSyntaxError, const char errorMsg[]) {
    cout << "At index [" << indexOfSyntaxError << "]: ";
    error(errorMsg);
    cout << str << endl;
    for (int i = 0; i < indexOfSyntaxError; i++) {
        cout << ' ';
    }
    cout << '^' << endl;
}

// generate a new temp variable name
string newtemp() {
    char temp[12];
    sprintf(temp, "t%d", g_cur_temp_idx++);
    return string(temp);
}

// generate a quadruple form
void emit(string op, string op1, string op2, string res) {
    if (!syntaxErrorFounded) {
        quadrupleFormList.push_back(QuadrupleForm(op, op1, op2, res));
    }
}
