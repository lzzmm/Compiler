// parser.cpp

// Copyright (c) 2022 CHEN Yuhan
// Date: 2022-05-07
//
// Parser in LL(1) and LR(0)
// Compile: g++ -o parser parser.cpp
// Run: ./parser [grammar file] [input file]

/*
 TODO:
 1. class LL1 tbd
    1. read one rule with "|" and spilt into two rules
    2. auto generate first, terminal and parsing table
    3. parsing with ll(1)
 2. class LR0 tbd
    1. 
 3. write report 0%
 4. make the code more clean and beautiful 0%
*/

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

#define RHS_OFFSET    2
#define MAX_RULE_NUM  32
#define MAX_RULE_SIZE 32
#define MAX_TOKEN_NUM 100

// print error message
void error(const char errorMsg[]) {
    std::cout << errorMsg << std::endl;
}

// print error message with index of syntax error
void error(int indexOfSyntaxError, const char errorMsg[], const char str[]) {
    std::cout << "At index [" << indexOfSyntaxError << "]: ";
    error(errorMsg);
    std::cout << str << std::endl;
    for (int i = 0; i < indexOfSyntaxError; i++) {
        std::cout << ' ';
    }
    std::cout << "\033[31m^\033[0m" << std::endl;
}
class LL1 {
public:
    LL1(){};
    ~LL1(){};
    // input grammer and generate LL(1) parser
    void readGrammarRules(FILE *file);
    void makeTable();
    // parse input tokens/strings
    void parse(FILE *fp);

private:
    void _getTermNTerm();
    void _getFirstSet();
    void _getFollowSet();
    void _makeTable();
    int  _getFirstNonTerm(char *str, char *non_term);

    unsigned int                     _rule_num = 0;
    char                             _rules[MAX_RULE_NUM][MAX_RULE_SIZE];
    char                             _str[MAX_TOKEN_NUM + 2];
    std::vector<std::vector<int>>    _analysisTable;
    std::vector<char>                _terminators;
    std::vector<char>                _nonTerminators;
    std::map<char, int>              _terminatorsToIdx;
    std::map<char, int>              _nonTerminatorsToIdx;
    std::map<int, std::vector<char>> _firstSet;
    std::map<int, std::vector<char>> _followSet;
};

int main(int argc, char *argv[]) {
    LL1   ll1;
    FILE *grammar_fp, *input_fp;
    char  grammar_path[32], input_path[32];

    if (argc != 3) {
        printf("Please input the path of the grammar file:\n");
        scanf("%s", grammar_path);
        grammar_fp = fopen(grammar_path, "r");
        printf("Please input the path of the file to parse:\n");
        scanf("%s", &input_path);
        input_fp = fopen(input_path, "r");
    } else {
        // grammar_fp = fopen("test.txt", "r");
        grammar_fp = fopen(argv[1], "r");
        // input_fp   = fopen("str.txt", "r");
        input_fp = fopen(argv[2], "r");
    }
    if (grammar_fp == NULL) {
        printf("Can't open the file! Terminated.\n");
        return -1;
    }
    if (input_fp == NULL) {
        printf("Can't open the file! Terminated.\n");
        return -1;
    }

    ll1.readGrammarRules(grammar_fp);
    fclose(grammar_fp);

    ll1.makeTable();

    ll1.parse(input_fp);
    fclose(input_fp);

    return 0;
}

void LL1::parse(FILE *fp) {
    fgets(_str, MAX_TOKEN_NUM, fp);
    unsigned int str_size = strlen(_str);
    while (_str[str_size - 1] == '\n') str_size--;
    _str[str_size++] = '#';
    _str[str_size++] = '\0';
    // TODO:
    // parse the string with the table
    char x, w;
    bool get_next    = true;
    int  str_pointer = 0,
        cur_non_term_idx,
        cur_term_idx,
        cur_rule_no;
    std::vector<char> analysisStack;
    analysisStack.push_back('#');
    analysisStack.push_back(_nonTerminators[0]);

    printf("\nString Received: %s\n\n", _str);
    printf("Stack");
    printf("\t ");
    printf("CurSYM");
    printf("\t ");
    printf("Str");
    printf("\t ");
    printf("OP\n", w);

    while (true) {
        // for (auto &&i : analysisStack)
        //     printf("%c", i);
        // printf("\n");
        x = analysisStack.back();
        analysisStack.pop_back();
        // printf("x: %c, %d\n", x, x);
        if (get_next) {
            w        = _str[str_pointer++];
            // printf("w: %c\n", w);
            get_next = false;
        }
        if (_terminatorsToIdx.find(x) != _terminatorsToIdx.end()) {
            if (x == '#' && w == '#') {
                printf("\033[31m%c\033[0m", x);
                printf("\t ");
                printf("%c", w);
                printf("\t ");
                printf("\t ");
                printf("\033[33mSuccessfully parsed.\033[0m\n");
                break;
            } else if (x == w) {
                get_next = true;
                for (auto &&i : analysisStack)
                    printf("%c", i);
                printf("\033[31m%c\033[0m", x);
                printf("\t ");
                printf("%c", w);
                printf("\t ");
                for (int i = str_pointer; _str[i] != '\0'; i++)
                    printf("%c", _str[i]);
                printf("\t ");
                printf("\033[33mAccepted %c\033[0m\n", w);
                continue;
            } else if (x == 'e') {
                continue;
            } else {
                error(str_pointer - 1, "Error: Unexpected Terminal found.", _str);
                return;
            }
        } else if (_nonTerminatorsToIdx.find(x) != _nonTerminatorsToIdx.end()) {
            cur_term_idx     = _terminatorsToIdx[w];
            cur_non_term_idx = _nonTerminatorsToIdx[x];
            cur_rule_no      = _analysisTable[cur_non_term_idx][cur_term_idx];
            if (cur_rule_no == -1) {
                error(str_pointer - 1, "Error: Invalid Terminal found.", _str);
                return;
            } else {
                for (auto &&i : analysisStack)
                    printf("%c", i);
                printf("\033[31m%c\033[0m", x);
                printf("\t ");
                printf("%c", w);
                printf("\t ");
                for (int i = str_pointer; _str[i] != '\0'; i++)
                    printf("%c", _str[i]);
                printf("\t ");
                printf("Select %s \033[32m%d\033[0m\n", _rules[cur_rule_no], cur_rule_no);
                for (int i = strlen(_rules[cur_rule_no]); i > RHS_OFFSET; i--) {
                    if (_rules[cur_rule_no][i] != '\0') analysisStack.push_back(_rules[cur_rule_no][i]);
                }
                get_next = false;
                continue;
            }
        } else {
            error(str_pointer - 1, "Error: Unexpected Terminal found.", _str);
            break;
        }
    }
    // print the result
}

void LL1::readGrammarRules(FILE *fp) {
    // start with the line number of the grammar rules
    unsigned int rule_num = 0;
    char         cur      = getc(fp);
    while (cur != 10) { // LF 10
        rule_num = rule_num * 10 + cur - '0';
        cur      = getc(fp);
    }
    printf("Number of rules: %d \n", rule_num);

    // get rules from the file and store them in the array
    for (int rule_cnt = 0; rule_cnt < rule_num; rule_cnt++) {
        fgets(_rules[rule_cnt], MAX_RULE_SIZE, fp);
        _rules[rule_cnt][strlen(_rules[rule_cnt]) - 1] = '\0';
    }
    _rule_num = rule_num;
}

void LL1::makeTable() {
    _getTermNTerm();
    _getFirstSet();
    _getFollowSet();
    _makeTable();
}

void LL1::_getTermNTerm() {
    bool new_non_term = true, new_term = true;
    int  i = 0, j = 0, k = 0;
    char code;

    for (int rule_cnt = 0; rule_cnt < _rule_num; rule_cnt++) {
        printf("%d. %s\n", rule_cnt, (_rules[rule_cnt]));

        for (i = 0; i < strlen(_rules[rule_cnt]); i++) {
            new_non_term = true;
            new_term     = true;
            code         = _rules[rule_cnt][i];

            if ((code >= 65) && (code <= 90)) { // CAPITAL LETTER signify non-terminals
                for (j = 0; j < _nonTerminators.size(); j++) {
                    if (_nonTerminators[j] == _rules[rule_cnt][i]) { // judge if the non-terminal is already in the vector
                        new_non_term = false;
                    }
                }
                if (new_non_term == true) { // add the non-terminal to the vector
                    _nonTerminatorsToIdx[_rules[rule_cnt][i]] = _nonTerminators.size();
                    _nonTerminators.push_back(_rules[rule_cnt][i]);
                }
            }

            if (i > RHS_OFFSET) {
                if (!(((code >= 65) && (code <= 90)) || (code == 101))) { // 101 'e' terminals
                    for (k = 0; k < _terminators.size(); k++) {
                        if (_terminators[k] == _rules[rule_cnt][i]) {
                            new_term = false;
                        }
                    }
                    if (new_term == true) {
                        _terminatorsToIdx[_rules[rule_cnt][i]] = _terminators.size();
                        _terminators.push_back(_rules[rule_cnt][i]);
                    }
                }
            }
        }
    }
    _terminatorsToIdx['e'] = -1;
    _terminatorsToIdx['#'] = _terminators.size();
    _terminators.push_back('#'); // '#'
    // _terminators.push_back('\0');
    // _nonTerminators.push_back('\0');
    printf("\nNumber of non-terminators: %d\nNon-terminators: ", _nonTerminators.size());
    for (i = 0; i < _nonTerminators.size(); i++) {
        printf("%c", _nonTerminators[i]);
        if (i != _nonTerminators.size() - 1) printf(", ");
    }
    printf("\nNumber of terminators: %d\nTerminators: ", _terminators.size());
    for (i = 0; i < _terminators.size(); i++) {
        printf("%c", _terminators[i]);
        if (i != _terminators.size() - 1) printf(", ");
    }
    printf("\n\n");
}

void LL1::_getFirstSet() {
    bool  changed = true;
    int   i = 0, j = 0, rule_cnt = 0;
    char  cur_term, cur_non_term, code;
    int   cur_term_idx, cur_non_term_idx;
    char *rule_rhs;

    while (changed) { // loop until find all, where there's no change in a whole loop
        changed = false;
        for (rule_cnt = 0; rule_cnt < _rule_num; rule_cnt++) {
            cur_non_term     = _rules[rule_cnt][0];
            cur_non_term_idx = _nonTerminatorsToIdx[cur_non_term];
            rule_rhs         = &(_rules[rule_cnt][RHS_OFFSET + 1]); // the right part of the rule
            int  prev_exist  = 0;
            bool does_exist  = false;
            for (i = 0; i < strlen(rule_rhs); i++) { // i in rhs, i = code in rule

                code = rule_rhs[i]; // current character

                if ((i == 0) && (code == 101)) { // start with e means empty
                    prev_exist = 0;
                    does_exist = false;
                    // check if code occurs in _firstSet[cur_non_term_idx]
                    for (prev_exist = 0; prev_exist < _firstSet[cur_non_term_idx].size(); prev_exist++) {
                        if (code == _firstSet[cur_non_term_idx][prev_exist]) {
                            does_exist = true;
                        }
                    }
                    if (does_exist == false) {
                        _firstSet[cur_non_term_idx].push_back('e'); // add 'e' to _firstSet[cur_non_term_idx]
                        changed = true;                             // changed _firstSet[cur_non_term_idx]
                    }
                    break;
                } else if ((i == 0) && (!((code >= 65) && (code <= 90)))) { // start with terminal
                    prev_exist = 0;
                    does_exist = false;
                    for (prev_exist = 0; prev_exist < _firstSet[cur_non_term_idx].size(); prev_exist++) {
                        if (code == _firstSet[cur_non_term_idx][prev_exist]) {
                            does_exist = true;
                        }
                    }
                    if (does_exist == 0) {
                        _firstSet[cur_non_term_idx].push_back(rule_rhs[i]); // add current character to _firstSet[cur_non_term_idx]
                        changed = true;
                    }
                    break;
                } else if ((code >= 65) && (code <= 90)) { // non-terminal
                    int  new_non_term_idx = _nonTerminatorsToIdx[rule_rhs[i]];
                    bool is_nullable      = false;                             // if includes 'e'
                    for (j = 0; j < _firstSet[new_non_term_idx].size(); j++) { // j in _firstSet[new_non_term_idx]
                        if (_firstSet[new_non_term_idx][j] != 101) {           // 101 'e'
                            prev_exist = 0;
                            does_exist = false;
                            for (prev_exist = 0; prev_exist < _firstSet[cur_non_term_idx].size(); prev_exist++) { // skip exist
                                if (_firstSet[new_non_term_idx][j] == _firstSet[cur_non_term_idx][prev_exist]) {
                                    does_exist = true;
                                }
                            }
                            if (does_exist == false) {
                                _firstSet[cur_non_term_idx].push_back(_firstSet[new_non_term_idx][j]);
                                changed = true;
                            }
                        } else if (_firstSet[new_non_term_idx][j] == 101) {
                            is_nullable = true;
                        }
                    }
                    if (is_nullable == false) { // without 'e'
                        break;
                    }
                }
            }
        }
    }

    for (i = 0; i < _nonTerminators.size(); i++) {
        printf("FIRST[%c] = { ", _nonTerminators[i]);
        for (j = 0; j < _firstSet[i].size(); j++) {
            printf("%c", _firstSet[i][j]);
            if (j != _firstSet[i].size() - 1) printf(", ");
        }
        printf(" }\n");
    }
}

void LL1::_getFollowSet() {
    int   i, j, rule_cnt, rule_length_cnt;
    char *rule_prod, *rule_rhs, *rest_rhs;
    char  current_t, cur_non_term, first_non_term;
    int   current_t_index, cur_non_term_idx, first_non_term_idx, first_non_term_tmp_idx;
    int   rest_rhs_length_count;
    int   code, first_length_cnt, code_index;
    int   prev_exist, follow_cnt;
    char  left_non_term;
    int   left_non_term_idx;
    bool  does_exist, changed = true, is_nullable = false;

    _followSet[0].push_back('#');

    while (changed) {
        changed = false;
        for (rule_cnt = 0; rule_cnt < _rule_num; rule_cnt++) {
            rule_prod       = &(_rules[rule_cnt][0]);
            rule_rhs        = &(rule_prod[RHS_OFFSET + 1]);
            rule_length_cnt = 0;
            while (rule_length_cnt < strlen(rule_rhs)) {
                rest_rhs               = rule_rhs + rule_length_cnt; // cur rhs idx
                first_non_term_tmp_idx = _getFirstNonTerm(rest_rhs, &first_non_term);
                if (first_non_term_tmp_idx == -1) {
                    break;
                }
                first_non_term_idx = rule_length_cnt + first_non_term_tmp_idx; // idx in rhs
                cur_non_term       = first_non_term;
                cur_non_term_idx   = _nonTerminatorsToIdx[cur_non_term]; // idx in non-terminal table

                if (first_non_term_idx == (strlen(rule_rhs) - 1)) { // last one in rhs
                    left_non_term     = _rules[rule_cnt][0];        // left non-terminal
                    left_non_term_idx = _nonTerminatorsToIdx[left_non_term];
                    for (follow_cnt = 0; follow_cnt < _followSet[left_non_term_idx].size(); follow_cnt++) {
                        if (_followSet[left_non_term_idx][follow_cnt] != 101) { // 101 'e'
                            prev_exist = 0;
                            does_exist = 0;
                            for (prev_exist = 0; prev_exist < _followSet[cur_non_term_idx].size(); prev_exist++) {
                                if (_followSet[left_non_term_idx][follow_cnt] == _followSet[cur_non_term_idx][prev_exist]) {
                                    does_exist = true;
                                }
                            }
                            if (does_exist == 0) {
                                _followSet[cur_non_term_idx].push_back(_followSet[left_non_term_idx][follow_cnt]);
                                changed = true;
                            }
                        }
                    }
                }

                for (rest_rhs_length_count = first_non_term_tmp_idx + 1; rest_rhs_length_count < strlen(rest_rhs); rest_rhs_length_count++) {
                    code = (int)rest_rhs[rest_rhs_length_count];

                    if (!((code >= 65) && (code <= 90)) && !(code == 101)) {
                        prev_exist = 0;
                        does_exist = 0;
                        for (prev_exist = 0; prev_exist < _followSet[cur_non_term_idx].size(); prev_exist++) {
                            if (code == _followSet[cur_non_term_idx][prev_exist]) {
                                does_exist = true;
                            }
                        }
                        if (does_exist == false) {
                            _followSet[cur_non_term_idx].push_back(rest_rhs[rest_rhs_length_count]);
                            changed = true;
                        }
                        break;
                    } else if ((code >= 65) && (code <= 90)) {
                        code_index = _nonTerminatorsToIdx[rest_rhs[rest_rhs_length_count]];
                        for (first_length_cnt = 0; first_length_cnt < _firstSet[code_index].size(); first_length_cnt++) {
                            if (_firstSet[code_index][first_length_cnt] != 101) {
                                prev_exist = 0;
                                does_exist = false;
                                for (prev_exist = 0; prev_exist < _followSet[cur_non_term_idx].size(); prev_exist++) {
                                    if (_firstSet[code_index][first_length_cnt] == _followSet[cur_non_term_idx][prev_exist]) {
                                        does_exist = true;
                                    }
                                }
                                if (does_exist == false) {
                                    _followSet[cur_non_term_idx].push_back(_firstSet[code_index][first_length_cnt]);
                                    changed = true;
                                }
                            } else if (_firstSet[code_index][first_length_cnt] == 101) {
                                is_nullable = true;
                                if (rest_rhs_length_count == (strlen(rule_rhs) - 1)) {
                                    left_non_term     = _rules[rule_cnt][0];
                                    left_non_term_idx = _nonTerminatorsToIdx[left_non_term];
                                    for (follow_cnt = 0; follow_cnt < _followSet[left_non_term_idx].size(); follow_cnt++) {
                                        if (_followSet[left_non_term_idx][follow_cnt] != 101) {
                                            prev_exist = 0;
                                            does_exist = false;
                                            for (prev_exist = 0; prev_exist < _followSet[cur_non_term_idx].size(); prev_exist++) {
                                                if (_followSet[left_non_term_idx][follow_cnt] == _followSet[cur_non_term_idx][prev_exist]) {
                                                    does_exist = true;
                                                }
                                            }
                                            if (does_exist == false) {
                                                _followSet[cur_non_term_idx].push_back(_followSet[left_non_term_idx][follow_cnt]);
                                                changed = true;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        if (is_nullable == false) {
                            break;
                        }
                    }
                }
                rule_length_cnt = first_non_term_idx + 1;
            }
        }
    }

    printf("\n");
    for (i = 0; i < _nonTerminators.size(); i++) {
        printf("FOLLOW[%c] = { ", _nonTerminators[i]);
        for (j = 0; j < _followSet[i].size(); j++) {
            printf("%c", _followSet[i][j]);
            if (j != _followSet[i].size() - 1) printf(", ");
        }
        printf(" }\n");
    }
    printf("\n");
}

// This function returns the index of the first given non-terminal in given string.
// If not found, it returns -1.
int LL1::_getFirstNonTerm(char *str, char *non_term) {
    for (int i = 0; i < strlen(str); i++) {
        if ((str[i] >= 65) && (str[i] <= 90)) {
            *non_term = str[i];
            return i;
        }
    }
    return -1;
}

void LL1::_makeTable() {
    char              left_non_term; // 左边的非终止符
    int               rule_cnt, rule_length_cnt, t_cnt_idx;
    char              first_t, t_cnt;
    int               first_t_index, left_non_term_idx, first_cnt;
    char *            rule_rhs;
    std::vector<char> select;
    int               prev_exist, does_exist, is_nullable;
    int               code_index, nt_cnt;
    char              code;

    select.resize(_nonTerminators.size());
    _analysisTable.resize(_nonTerminators.size());
    for (int i = 0; i < _nonTerminators.size(); i++) {
        _analysisTable[i].resize(_terminators.size(), -1);
    }
    for (rule_cnt = 0; rule_cnt < _rule_num; rule_cnt++) {
        select.clear();
        left_non_term     = _rules[rule_cnt][0];
        left_non_term_idx = _nonTerminatorsToIdx[left_non_term];
        rule_rhs          = &(_rules[rule_cnt][RHS_OFFSET + 1]); // 产生式右部
        for (rule_length_cnt = 0; rule_length_cnt < strlen(rule_rhs); rule_length_cnt++) {
            code = rule_rhs[rule_length_cnt]; // 取一个符号
            if (code == 101) {                //printf("epsilon\n");
                                              // 若产生式右侧为空则选取 follow 集
                for (auto &&item : _followSet[left_non_term_idx])
                    select.emplace_back(item);
                //printf("%s: %s \n",_rules[rule_cnt],select);
                break;
            } else if (!((code >= 65) && (code <= 90))) {
                // 终止符若没有记录过则加入
                prev_exist = 0;
                does_exist = false;
                for (prev_exist = 0; prev_exist < select.size(); prev_exist++) {
                    if (code == select[prev_exist]) {
                        does_exist = true;
                    }
                }
                if (does_exist == false) {
                    select.push_back(rule_rhs[rule_length_cnt]);
                    //printf("%s: %s \n",_rules[rule_cnt],select);
                    break;
                }
            } else if ((code >= 65) && (code <= 90)) {
                // 查找非终止符的 select
                code_index  = _nonTerminatorsToIdx[rule_rhs[rule_length_cnt]];
                is_nullable = false;
                for (first_cnt = 0; first_cnt < _firstSet[code_index].size(); first_cnt++) {
                    first_t = _firstSet[code_index][first_cnt];
                    if (first_t != 101) {
                        prev_exist = 0;
                        does_exist = false;
                        for (prev_exist = 0; prev_exist < select.size(); prev_exist++) {
                            if (first_t == select[prev_exist]) {
                                does_exist = true;
                            }
                        }
                        if (does_exist == false) {
                            select.push_back(first_t);
                            //							printf("%s: %s \n",_rules[rule_cnt],select);
                        }
                    } else if (first_t == 101) {
                        is_nullable = true;
                        if (rule_length_cnt == strlen(rule_rhs) - 1) {
                            for (t_cnt = 0; t_cnt < _followSet[left_non_term_idx].size(); t_cnt++) {
                                prev_exist = 0;
                                does_exist = false;
                                for (prev_exist = 0; prev_exist < select.size(); prev_exist++) {
                                    if (first_t == select[prev_exist]) {
                                        does_exist = true;
                                    }
                                }
                                if (does_exist == false) {
                                    select.push_back(_followSet[left_non_term_idx][t_cnt]);
                                    //printf("%s: %s \n",_rules[rule_cnt],select);
                                }
                            }
                        }
                    }
                }
                if (is_nullable == false) {
                    break;
                }
            }
        }

        printf("SELECT(%s) = { ", _rules[rule_cnt]);
        for (first_cnt = 0; first_cnt < select.size(); first_cnt++) {
            first_t       = select[first_cnt];
            first_t_index = _terminatorsToIdx[first_t];
            if (_analysisTable[left_non_term_idx][first_t_index] == -1 || _analysisTable[left_non_term_idx][first_t_index] == rule_cnt) {
                _analysisTable[left_non_term_idx][first_t_index] = rule_cnt;
            } else {
                error("Error: duplicate analysis table entry");
            }
            // if (first_t == '#') printf("%c, %d, %d\n", first_t, first_t_index, rule_cnt);
            printf("%c", first_t);
            if (first_cnt != select.size() - 1) printf(", ");
        }
        printf(" }\n");
    }
    printf("\n\n   Analysis Table\n\n   ");
    for (t_cnt = 0; t_cnt < _terminators.size(); t_cnt++) {
        printf("%c    ", _terminators[t_cnt]);
    }
    printf("\n");
    for (nt_cnt = 0; nt_cnt < _nonTerminators.size(); nt_cnt++) {
        printf("%c  ", _nonTerminators[nt_cnt]);
        for (t_cnt = 0; t_cnt < _terminators.size(); t_cnt++) {
            if (_analysisTable[nt_cnt][t_cnt] == -1) {
                printf("     ", _analysisTable[nt_cnt][t_cnt]);
            } else {
                printf("%d    ", _analysisTable[nt_cnt][t_cnt]);
            }
        }
        printf("\n");
    }
    printf("\n");
}
