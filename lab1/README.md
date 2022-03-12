# Lab1 Lexical Syntax Analysis

实现 C 语言的词法分析器

## 功能

输入一个C语言源程序文件demo.c
输出一个文件tokens.txt，该文件包括每一个单词及其种类枚举值，每行一个单词

提交5个文件实验报告（所支持的单词范围，自动机设计，设计思路）
C语言词法分析源程序：source.c（源程序包）
C语言词法分析程序的可执行文件：source.exe
C语言源程序文件：demo.c（实验输入）
词法分析及结果文件： tokens.txt（实验输出）

## 文件树

```
lab1
│   README.md
│
└───src
        demo.c
        demo.exe
        source.cpp
        source.exe
        tokens.txt
        wrongDemo.c
```

## 设计词法分析器

### 设计种别码

标识符  00
字符    01
字符串  02
常数    03
关键字  04 - 35
界符    36 - 71

### 设计自动机

TBD

### 使用 C++ 代码实现

TBD

## 实验验证

TBD
