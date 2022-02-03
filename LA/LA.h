#include<iostream>
#include<fstream>
#include<cstdio>
#include<cstdlib>
#include<cstring>

using namespace std;

/**  
* COP compare operator �ȽϷ�
* AOP assignment operator ��ֵ��
* OOP operation operator ������
* EOP end operator ������
* SOP separate operator �ָ���
*/

// ����ؼ��֣���һ����ռλ������ʵ������
const string key[] = {
    "", "program", "const", "var", "procedure", "begin", "if", 
    "else", "end", "while", "call", "read", "write", "then", "odd", "do"
};

// �У���
int row, col;

/**
 * �ж��ַ��Ƿ�Ϊ�հ��ַ�
 * @param ch
 * @return trueΪ��
 */
bool isBC(char ch);

/**
 * ���ַ�ch������strToken����
 * @param strToken
 * @param ch
 */
void Concat(string& strToken, char ch);

/**
 * �ж��ַ��Ƿ�Ϊ��ĸ
 * @param ch
 * @return trueΪ��
 */
bool IsLetter(char ch);

/**
 * �ж��ַ��Ƿ�Ϊ����
 * @param ch
 * @return trueΪ��
 */
bool IsDigit(char ch);

/**
 * ���ҹؼ��֣��������±꣬�����ǹؼ����򷵻�0
 * @param strToken
 * @return posΪ�ؼ����±�
 */
int Reserve(string strToken);

/**
 * ������ָʾ���ص�һ���ַ�
 * @param ch
 */
void Retract(char ch);

