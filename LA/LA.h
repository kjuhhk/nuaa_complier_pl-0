#include<iostream>
#include<fstream>
#include<cstdio>
#include<cstdlib>
#include<cstring>

using namespace std;

/**  
* COP compare operator 比较符
* AOP assignment operator 赋值符
* OOP operation operator 操作符
* EOP end operator 结束符
* SOP separate operator 分隔符
*/

// 定义关键字，第一个是占位符，无实际意义
const string key[] = {
    "", "program", "const", "var", "procedure", "begin", "if", 
    "else", "end", "while", "call", "read", "write", "then", "odd", "do"
};

// 行，列
int row, col;

/**
 * 判断字符是否为空白字符
 * @param ch
 * @return true为真
 */
bool isBC(char ch);

/**
 * 将字符ch连接在strToken后面
 * @param strToken
 * @param ch
 */
void Concat(string& strToken, char ch);

/**
 * 判断字符是否为字母
 * @param ch
 * @return true为真
 */
bool IsLetter(char ch);

/**
 * 判断字符是否为数字
 * @param ch
 * @return true为真
 */
bool IsDigit(char ch);

/**
 * 查找关键字，并返回下标，若不是关键字则返回0
 * @param strToken
 * @return pos为关键字下标
 */
int Reserve(string strToken);

/**
 * 将搜索指示器回调一个字符
 * @param ch
 */
void Retract(char ch);

