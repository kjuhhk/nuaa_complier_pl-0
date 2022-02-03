#include<iostream>
#include<fstream>
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<sstream>
#include<stack>
#include<bits/stdc++.h>
#include"LA.h"

using namespace std;

const int MAX_N = 1e4;
string order[] = { "LIT","OPR","LOD","STO","CAL","INT","JMP","JPC","RED","WRT" };

#define CONST 0
#define VAR 1
#define PROCEDURE 2

#define LIT 0 
#define OPR 1
#define LOD 2
#define STO 3
#define CAL 4
#define INT 5
#define JMP 6
#define JPC 7
#define RED 8
#define WRT 9

class Unit{
public:
    string val;
    string key;
    int row;
    int col;

    void print()
    {
        cout << "-------------" << endl;
        cout << "Value: " << val << endl;
        cout << "Key: " << key << endl;
        cout << "Position: [" << row << ',' << col << "]" << endl;
        cout << "-------------" << endl;
    }
};

Unit unit, last_unit;//当前扫描和上一次扫描
stack<int> errorStack;//存放错误
bool error = false;//错误标志

fstream in;//输入文件
fstream out;//输出文件
string line;//读入的一行数据

struct Pcode {  //目标代码
	int f;//功能码
	int l;//层次差
	int a;//根据不同的指令有所区别
}Pcode[MAX_N];

struct SymTable {  //符号表
	string name;
	int type;//const=0, var=1, procedure=2
	int value;
	int level;
	int adr;
	int size;
	int num;    //变量在程序出现的次数，方便内存优化
}SymTable[MAX_N];

int tx = 0;// table的下标指针,符号表地址从1开始
int dx = 0;// 计算每个变量在运行栈中相对本过程基地址的偏移量
int cx = 0;// 指令下一个地址
int lev = 0;
int dataStack[MAX_N]; //数据栈
int T;//栈顶寄存器
int B;//栈基址寄存器
int P;//下条指令
int I;//指令寄存器
int INT_pos;// 记录指令INT的位置


//-----------函数声明----------------//
void test(int type);// 找同步
void addError(int type, string name = "");
void ReadLine();
void Factor();
void Term();
void Exp();
void Lexp();
void Statement();
void Body();
void para_func();
void Proc();
void Block(); 
void Var();
void Vardecl();
void Const();
void Condecl();
void Prog();
void OpenFile();
void CloseFile();
void PrintErrorStack();

void gen(int f, int l, int a);   //产生p代码
void addVar(string name, int level, int adr);    //将变量登入到符号表
void addConst(string name, int level, int val);   //将常量登入到符号表
void addProcedure(string name, int level, int adr);  //将过程登入到符号表
int position(string name);  //在符号表查找名字为name的符号
bool is_the_same_level(string name, int lev);   //是否在同层
bool is_pre_level(string name, int lev); // 
int stringtoint(string str);
int findproc();
int getBase(int nowBp, int lev);
void printPcode();   //输出中间代码
void printTable();  //输出符号表
void interpreter();


//-----------函数定义----------------//

void gen(int f, int l, int a)
{
	Pcode[cx].f = f;
	Pcode[cx].l = l;
	Pcode[cx].a = a;
	cx++;
}

void addVar(string name, int level, int adr)
{
    tx++;
	SymTable[tx].type = VAR;
	SymTable[tx].name = name;
	SymTable[tx].level = level;
	SymTable[tx].adr = adr;
	SymTable[tx].num = 1;
}

void addConst(string name, int level, int val)
{
    tx++;
	SymTable[tx].type = CONST;
	SymTable[tx].name = name;
	SymTable[tx].level = level;
	SymTable[tx].value = val;
	SymTable[tx].num = 1;
}

void addProcedure(string name, int level, int adr)
{
    tx++;
	SymTable[tx].type = PROCEDURE;
	SymTable[tx].name = name;
	SymTable[tx].level = level;
	SymTable[tx].adr = adr;
	SymTable[tx].num = 1;
}

int position(string name)
{
    for(int i = tx; i > 0; i--)
	{
		if (SymTable[i].name == name && SymTable[i].level <= lev)
			return i;
	}
	for(int i = tx; i > 0; i--)
	{
		if (SymTable[i].name == name)
			return i;
	}
	return -1;
}

bool is_the_same_level(string name, int lev)//是否在同层
{    

	for (int i = 1; i <= tx; i++)
	{
		if (SymTable[i].name == name && SymTable[i].level == lev)
		{
			return true;
		}
	}
	return false;
}

bool is_pre_level(string name, int lev)
{
    for(int i = 1; i <= tx; i++)
    {
        // cout << SymTable[i].name << "   " << SymTable[i].value << "   " << SymTable[i].level << endl;
        if(SymTable[i].name == name && SymTable[i].level <= lev)
            return true;
    }
    return false;
}

int stringtoint(string str)
{
    int res = 0;
    for(int i = 0; i < str.size(); i++)
    {
        res *= 10;
        res += str[i] - '0';
    }
    return res;
}

int findproc() // 寻找过程
{
    for (int i = tx; i >= 1; i--)
	{
		if (SymTable[i].type == PROCEDURE)
		{
			return i;
		}
	}
	return -1;
}

int getBase(int nowBp, int lev)    //根据层差得到基地址
{ 
	int oldBp = nowBp;
	while(lev > 0)//当存在层差时寻找非局部变量
	{
		oldBp = dataStack[oldBp + 1];//直接外层的活动记录首地址
		lev--;
	}
	return oldBp;
}

void printPcode()   //输出中间代码
{
    for (int i = 0; i < cx; i++)
    {    
    	cout << i << " ";
		printf("%s %d %d\n", order[Pcode[i].f].c_str(), Pcode[i].l, Pcode[i].a);//调用.c_str()打印string字符串 
	}
}

void printTable()  //输出符号表
{
    int i = 1;
    while(SymTable[i].num)
    {
		printf("名称:%s   类型:%d   数值:%d   层次:%d   相对地址:%d    出现次数:%d\n",
        SymTable[i].name.c_str(), SymTable[i].type, SymTable[i].value,SymTable[i].level, SymTable[i].adr, SymTable[i].num);
        i++;    
    }
}

void interpreter()
{
    //初始化
    P = 0;//程序地址寄存器
	B = 0;//基址寄存器
	T = 0;//栈顶寄存器
    int t;
    do
    {
        I = P;
        P++;
        switch (Pcode[I].f)//获取伪操作码
        {
        case 0: //LIT 0 a，取常量a放入数据栈栈顶
			dataStack[T] = Pcode[I].a;
			T++;
			break;
		case 1: //OPR 0 a，执行运算，a表示执行某种运算
			switch (Pcode[I].a)
			{
			case 0:						//opr,0,0 调用过程结束后，返回调用点并退栈
				T = B;                  //恢复调用前栈顶
				P = dataStack[B + 2];	//返回地址
				B = dataStack[B];		//静态链
				break;
			case 1:                 //opr 0,1取反指令
				dataStack[T - 1] = -dataStack[T - 1];
				break;
			case 2:                 //opr 0,2相加，将原来的两个元素退去，将结果置于栈顶
				dataStack[T - 2] = dataStack[T - 1] + dataStack[T - 2];
				T--;
				break;
			case 3:					//OPR 0,3 次栈顶减去栈顶，退两个栈元素，结果值进栈
				dataStack[T - 2] = dataStack[T - 2] - dataStack[T - 1];
				T--;
				break;
			case 4:    				//OPR 0,4次栈顶乘以栈顶，退两个栈元素，结果值进栈
				dataStack[T - 2] = dataStack[T - 2] * dataStack[T - 1];
				T--;
				break;
			case 5:					//OPR 0,5次栈顶除以栈顶，退两个栈元素，结果值进栈
				dataStack[T - 2] = dataStack[T - 2] / dataStack[T - 1];
				T--;
				break;
			case 6:                 //栈顶元素值奇偶判断，结果值进栈,奇数为1
				dataStack[T - 1] = dataStack[T - 1] % 2;
				break;
			case 7:
				break;
			case 8:					//次栈顶与栈项是否相等，退两个栈元素，结果值进栈
				if (dataStack[T - 1] == dataStack[T - 2])
				{
					dataStack[T - 2] = 1;
					T--;
				}
                else
				{
                    dataStack[T - 2] = 0;
				    T--;
                }
				break;
			case 9:					//次栈顶与栈项是否不等，退两个栈元素，结果值进栈
				if (dataStack[T - 1] != dataStack[T - 2])
				{
					dataStack[T - 2] = 1;
					T--;
				}
				else
				{
                    dataStack[T - 2] = 0;
				    T--;
                }
				break;
			case 10:				//次栈顶是否小于栈顶，退两个栈元素，结果值进栈
				if (dataStack[T - 2] < dataStack[T - 1])
				{
					dataStack[T - 2] = 1;
					T--;
				}
				else
				{
                    dataStack[T - 2] = 0;
				    T--;
                }
				break;
			case 11:				//次栈顶是否大于等于栈顶，退两个栈元素，结果值进栈
				if (dataStack[T - 2] >= dataStack[T - 1])
				{
					dataStack[T - 2] = 1;
					T--;
				}
				else
				{
                    dataStack[T - 2] = 0;
				    T--;
                }
				break;
			case 12:				//次栈顶是否大于栈顶，退两个栈元素，结果值进栈
				if (dataStack[T - 2] > dataStack[T - 1])
				{
					dataStack[T - 2] = 1;
					T--;
				}
                else
				{
                    dataStack[T - 2] = 0;
				    T--;
                }
				break;
			case 13:				//次栈顶是否小于等于栈顶，退两个栈元素，结果值进栈
				if (dataStack[T - 2] <= dataStack[T - 1])
				{
					dataStack[T - 2] = 1;
					T--;
				}
				else
				{
                    dataStack[T - 2] = 0;
				    T--;
                }
				break;
			case 15:				//屏幕输出换行
				cout << endl;
				break;
			}
			break;
        case 2: //LOD L ，a 取变量（相对地址为a，层差为L）放到数据栈的栈顶
			dataStack[T] = dataStack[Pcode[I].a + getBase(B, Pcode[I].l)];
//			cout << I << "------" << dataStack[T] << endl;
			T++;
			break;
        case 3: //STO L ，a 将数据栈栈顶的内容存入变量（相对地址为a，层次差为L）
            dataStack[Pcode[I].a + getBase(B, Pcode[I].l)] = dataStack[T - 1];
			T--;
			break;
        case 4: //CAL L ，a 调用过程（转子指令）（入口地址为a，层次差为L）
			dataStack[T] = B;		//静态链，直接外层过程
			dataStack[T + 1] = getBase(B, Pcode[I].l);	//动态链，调用前运行过程
			dataStack[T + 2] = P;		//返回地址，下一条要执行的
			B = T;
			P = Pcode[I].a;
			break;
        case 5: //INT 0 ，a 数据栈栈顶指针增加a
			T = B + Pcode[I].a;
			break;
		case 6: //JMP 0 ，a无条件转移到地址为a的指令
			P = Pcode[I].a;
			break;
		case 7: //JPC 0 ，a 条件转移指令，转移到地址为a的指令
			if (dataStack[T - 1] == 0)
				P = Pcode[I].a;
			break;
		case 8: //RED L ，a 从命令行读入一个数据并存入变量
            cout << "请在命令行中输入一个变量值的数据:";
			cin >> t;
			dataStack[Pcode[I].a + getBase(B, Pcode[I].l)] = t;
			break;
		case 9: //栈顶值输出至屏幕
			cout << dataStack[T - 1] << " ";
			break;
		}
	} while (P != 0);
}

void test(int type)
{
    if(type == 1)//statement的first集
    {
        while(unit.key != "id" && unit.val != "if" && unit.val != "while" && unit.val != "call" && unit.val != "begin" && unit.val != "read" && unit.val != "write")
        {
//            addError(17);//此单词多余，superfluous
            ReadLine();
        }
    }
    else if(type == 2)//block的first集
    {
        if(unit.val == "const" && unit.val == "var" && unit.val == "procedure")
            return;
        
        while(unit.val != "const" && unit.val != "var" && unit.val != "procedure" && unit.val != "begin" && unit.key != "id" && unit.val != "if" && unit.val != "while" && unit.val != "call" && unit.val != "read" && unit.val != "write")
        {
//            addError(17);//此单词多余，superfluous
            ReadLine();
        }

    }
    else
    {

    }
}

/**
 * add an error.
 * @param type Error type.
 */

void addError(int type, string name)//缺省函数
{
    errorStack.push(type);
    error = true;
    switch (type)
    {
    case 0://拼写错误
        printf("[Grammar error][%d,%d] Spell error \"program\"\n", unit.row, unit.col);
        break;
    case 1://program后缺少标识符
        printf("[Grammar error][%d,%d] Missing identifier after \"program\"\n", unit.row, unit.col);
        break;
    case 2://缺少分号
        printf("[Grammar error][%d,%d] Missing \";\" \n", last_unit.row, last_unit.col);
        break;
    case 3://缺少右括号 
        printf("[Grammar error][%d,%d] Missing \")\" \n", unit.row, unit.col);
        break;
    case 4://缺少左括号 
        printf("[Grammar error][%d,%d] Missing \"(\" \n", unit.row, unit.col);
        break;
    case 5://缺少逗号 
        printf("[Grammar error][%d,%d] Missing \",\" \n", unit.row, unit.col);
        break;
    case 6://缺少id
        printf("[Grammar error][%d,%d] Missing identifier \n", unit.row, unit.col);
        break;
    case 7://缺少比较符
        printf("[Grammar error][%d,%d] Missing compare operator \n", unit.row, unit.col);
        break;
    case 8://缺少赋值号 
        printf("[Grammar error][%d,%d] Missing assignment operator \n", unit.row, unit.col);
        break;
    case 9://缺少then
        printf("[Grammar error][%d,%d] Missing \"then\" \n", unit.row, unit.col);
        break;
    case 10://缺少do
        printf("[Grammar error][%d,%d] Missing \"do\" \n", unit.row, unit.col);
        break;
    case 11://缺少begin
        printf("[Grammar error][%d,%d] Missing \"begin\" \n", unit.row, unit.col);
        break;
    case 12://缺少end
        printf("[Grammar error][%d,%d] Missing \"end\" \n", unit.row, unit.col);
        break;
    case 13://常量赋值号后面应为数字
        printf("[Grammar error][%d,%d] after\":=\" should be a number  \n", unit.row, unit.col);
        break;
    case 14://缺少:
        printf("[Grammar error][%d,%d] Missing \":\" \n", unit.row, unit.col);
        break; 
    case 15://缺少:=
        printf("[Grammar error][%d,%d] Missing \":=\" \n", unit.row, unit.col);
        break;
    case 16://缺少program
        printf("[Grammar error][%d,%d] Missing \"program\" \n", unit.row, unit.col);
        break;
    case 17://存在多余的单词
        printf("[Grammar error][%d,%d] the word is superfluous \n", unit.row, unit.col);
        break;
    case 18://该变量未定义
        if(name != "")
        	printf("[error][%d,%d] not exist %s\n", unit.row, unit.col, name.c_str());
        else
        	printf("[error][%d,%d] not exist %s\n", unit.row, unit.col, unit.val.c_str());
        break;
    case 19://不是变量
    	if(name != "")
        	printf("[error][%d,%d] %s is not a variable \n", unit.row, unit.col, name.c_str());
        else
        	printf("[error][%d,%d] %s is not a variable \n", unit.row, unit.col, unit.val.c_str());
        break;
    case 20://不是常量
        if(name != "")
        	printf("[error][%d,%d] %s is not a const \n", unit.row, unit.col, name.c_str());
        else
        	printf("[error][%d,%d] %s is not a const \n", unit.row, unit.col, unit.val.c_str());
        break;
    case 21://不是过程
        if(name != "")
        	printf("[error][%d,%d] %s is not a procedure \n", unit.row, unit.col, name.c_str());
        else
        	printf("[error][%d,%d] %s is not a procedure \n", unit.row, unit.col, unit.val.c_str());
        break;
    case 22://参数个数不匹配
        printf("[error][%d,%d] The number of parameters does not match \n", unit.row, unit.col);
        break;
    case 23://多重定义
        if(name != "")
        	printf("[error][%d,%d] Duplicate definition %s\n", unit.row, unit.col, name.c_str());
        else
        	printf("[error][%d,%d] Duplicate definition %s\n", unit.row, unit.col, unit.val.c_str());
        break;

    default:
        printf("[error][%d,%d] Unknown error\n", unit.row, unit.col);
        break;
    }
}

/**
 * Read file line by line.
 * @return Global variables: unit.val & unit.key.
 */
void ReadLine()
{
    getline(in, line);
    last_unit = unit; 
    istringstream iss(line);//分割成单个元素
    iss >> unit.val;//单词的内容
    iss >> unit.key;//单词的种类
    iss >> unit.row;
    iss >> unit.col;
}

/**
 * <factor>→<id>|<integer>|(<exp>)
 */
void Factor()//-------应该ok-----------
{
    if(unit.key == "id")
    {
        //判断id是否已经定义
        if(is_pre_level(unit.val, lev) == false)
            addError(18);
        else
        {
            int i = position(unit.val);
            SymTable[i].num++;

            if (SymTable[i].type == VAR)
			{
				gen(LOD, lev - SymTable[i].level, SymTable[i].adr);
			}
			else if (SymTable[i].type == CONST)
			{
				gen(LIT, 0, SymTable[i].value);
			}
			else
			{
				addError(19);//不是变量和常量
				return;
			}
        }
        ReadLine();
    }
    else if(unit.key == "INT")
    {
        gen(LIT, 0, stringtoint(unit.val));
        ReadLine();
    }
    else if(unit.val == "(")
    {
        ReadLine();
        Exp();
        if(unit.val == ")")
            ReadLine();
        else
            addError(5);//缺少右括号
    }
    else
    {
        addError(6);//缺少标识符
    }
}

/**
 * <term> → <factor>{<mop><factor>}
 */
void Term()//-------应该ok-----------
{
    Factor();
    while(unit.val == "*" || unit.val == "/")
    {
        string name = unit.val;
        ReadLine();
        Factor();

        if(name == "*")
            gen(OPR, 0, 4);
        else if(name == "/")
            gen(OPR, 0, 5);
    }
}

/**
 * <exp> → [+|-]<term>{<aop><term>}
 */
void Exp() //-------应该ok-----------
{
    string name;
    if(unit.val == "+" || unit.val == "-")
    {
        name = unit.val;//先记录操作符，等读完两个操作数后再gen
        ReadLine();
    }    
    Term();
    if (name == "-")
	{
		gen(OPR, 0, 1);
	}
    while(unit.val == "+" || unit.val == "-")
    {
        name = unit.val;
        ReadLine();
        Term();
        if(name == "+")
            gen(OPR, 0, 2);
        else if(name == "-")
            gen(OPR, 0, 3);
        else
            exit(-1);//不可能到达这个地方
    }
}

/**
 * <lexp> → <exp> <lop> <exp>|odd <exp>
 */
void Lexp()//-------应该ok-----------
{
    if(unit.val == "odd")
    {
        ReadLine();
        Exp();
        gen(OPR, 0, 6);
    }
    else
    {
        Exp();
        string key = unit.val;
        if(unit.key != "COP")
        {
		    addError(7);//缺少比较符
		    Exp();
		}
        else
        {
        	ReadLine();
            Exp();
        }

        if(key == "=")
            gen(OPR, 0, 8);
        else if(key == "<>")
            gen(OPR, 0, 9);
        else if(key == "<")
            gen(OPR, 0, 10);
        else if(key == "<=")
            gen(OPR, 0, 13);
        else if(key == ">")
            gen(OPR, 0, 12);
        else if(key == ">=")
            gen(OPR, 0, 11);
    }
}

/**
 * <statement> → <id> := <exp>
               |if <lexp> then <statement>[else <statement>]
               |while <lexp> do <statement>
               |call <id>[（<exp>{,<exp>}）]
               |<body>
               |read (<id>{，<id>})
               |write (<exp>{,<exp>})
 */
void Statement()
{
    if(unit.key == "id")
    {
        string name = unit.val;
        ReadLine();
        if(unit.val == ":=")
        {
            ReadLine();
        }
        else
        {
            addError(8);//缺少赋值号
        }
        int i = position(name);
        SymTable[i].num++;
        Exp();

        if(is_pre_level(name, lev) == false)
            addError(18, name);//未定义
        else
        {
            if(SymTable[i].type == VAR)
                gen(STO, lev - SymTable[i].level, SymTable[i].adr);
            else
                addError(19,name);//不是变量
        }
    }

    else if(unit.val == "if")
    {
        ReadLine();
        Lexp();
        int cx1;
        if(unit.val == "then")
        {
            cx1 = cx;
            gen(JPC, 0, 0);
            ReadLine();
        }    
        else
            addError(9);//缺少then
        
        Statement();
        int cx2 = cx;
        gen(JMP, 0, 0);//是jmp不是jpc！！！！

        //回填
        Pcode[cx2].a = cx;
        Pcode[cx1].a = cx;
        if(unit.val == "else")
        {
            ReadLine();
            Statement();
            Pcode[cx2].a = cx;//回填
        }        
    }
    else if(unit.val == "while")
    {
		int cx1 = cx;
        ReadLine();
        Lexp();

        int cx2 = cx;
		gen(JPC, 0, 0);
        if(unit.val == "do")
            ReadLine();
        else
            addError(10);//缺少do
            
        Statement();
        gen(JMP, 0, cx1);
		Pcode[cx2].a = cx;
    }
    else if(unit.val == "call")
    {
        ReadLine();
        int i;
        int count = 0;//参数的个数
        if(unit.key == "id")
        {
            if(is_pre_level(unit.val, lev) == false)
            {
                i = position(unit.val);
                SymTable[i].num++;
                addError(18);//未定义 
            }
            else
            {
                i = position(unit.val);
                SymTable[i].num++;
                if(SymTable[i].type != PROCEDURE)
                    addError(21);//不是过程
            }
            ReadLine();
        }
        else
            addError(6);//缺少id

        if(unit.val == "(")
            ReadLine();
        else
            addError(4);//缺少(
        
        if (unit.val == ")")//说明无参数
		{
			gen(CAL, lev - SymTable[i].level, SymTable[i].value);//SymTable[i].value 要跳转的过程指令地址
		}

        if(unit.val == "+" || unit.val == "-" || unit.key == "id" || unit.key == "INT" || unit.val == "(")
        {
            Exp();
            count++;
            while(unit.val == ",")
            {
                ReadLine();
                Exp();
                count++;
            }
            if (count != SymTable[i].size)
                addError(22);//参数个数不匹配
            gen(CAL, lev - SymTable[i].level, SymTable[i].value);
        }
        if(unit.val == ")")
            ReadLine();
        else
            addError(3);//缺少)
    }
    else if(unit.val == "begin")
    {
        Body();
    }
    else if(unit.val == "read")
    {
        ReadLine();
        if(unit.val == "(")
            ReadLine();
        else
            addError(4);//缺少(

        if(unit.key == "id")
        {
            if(is_pre_level(unit.val, lev) == false)
            {
                addError(18);//未定义
            }
            else
            {
                int i = position(unit.val);
                SymTable[i].num++;
                if(SymTable[i].type == VAR)
                {
                    gen(RED, lev - SymTable[i].level, SymTable[i].adr);
                }
                else
                    addError(19);//不是变量
            }
            ReadLine();
        }
        else
            addError(6);//缺少id
        
        while(unit.val == ",")
        {
            ReadLine();
            if(unit.key != "id")
            {
                addError(6);//缺少id
                if(unit.val != ")" && unit.val != ";")
                	ReadLine();
                break;
            }

            if(is_pre_level(unit.val, lev) == false)
                addError(18);//未定义
            else
            {
                int i = position(unit.val);
                SymTable[i].num++;
                
                if(SymTable[i].type == VAR)
                    gen(RED, lev - SymTable[i].level, SymTable[i].adr);
                else addError(19);//不是变量
            }
            ReadLine();
        }

        if(unit.val == ")")
            ReadLine();
        else
            addError(3);//缺少)
    }
    else if(unit.val == "write")
    {
        ReadLine();
        if(unit.val == "(")
            ReadLine();
        else
            addError(4);//缺少(
        
        Exp();
        gen(WRT, 0, 0);
        while(unit.val == ",")
        {
            ReadLine();
            Exp();
            gen(WRT, 0, 0);
        }
        gen(OPR, 0, 15);//输出换行
        if(unit.val == ")")
            ReadLine();
        else
            addError(3);//缺少)
    }
    else
    {
        cout << "不可能跳到statement的这个地方" << endl;
    }
}

/**
 * <body> → begin <statement>{;<statement>}end
 */
void Body()//-------应该ok-----------
{
    if(unit.val == "begin")
        ReadLine();
    else
        addError(11);//缺少begin
    
    //同步
    test(1);
    Statement();

    while(unit.val == ";")
    {
        ReadLine();
        test(1);

        Statement();
    }
    if(unit.val == "end")
        ReadLine();
    else
        addError(12);//缺少end

}

void para_func()//-------应该ok-----------
{
    if(unit.key == "id")
    {
        ReadLine();
        while(unit.val == "," || unit.key == "id")
        {
            if(unit.val == ",")
                ReadLine();
            else
                addError(5);//缺少,
            
            if(unit.key == "id")
            {
                ReadLine();
            }
            else
            {
                addError(6);//缺少id
                while(unit.val != ")" && unit.key != "id" && unit.val != ";" && unit.val != "const" && unit.val != "var" && unit.val != "procedure")
                    ReadLine();
                if(unit.key == "id")
                    ReadLine();
            }
        }
        if(unit.val != ")")
        {   
            addError(3);//缺少)
        }
        else
            ReadLine();
    }
    else if(unit.val == ")")
        ReadLine();
    else
    {
        addError(3);//缺少)
        ReadLine();
        while(unit.val != ")" && unit.val != ";" && unit.val != "const" && unit.val != "var" && unit.val != "procedure")
            ReadLine();
    }
}

/**
 * <proc> → procedure <id>（[<id>{,<id>}]）;<block>{;<proc>}
 */
void Proc()//-------应该ok-----------
{
    //进入该函数的前提是有procedure，故无需判断procedure
    int count = 0;
    int tx0;
    ReadLine();
    if(unit.key == "id")
    {
        if(is_the_same_level(unit.val, lev))
        {
            addError(23);//多重定义
        }
        tx0 = tx + 1;
        addProcedure(unit.val, lev, dx);
        lev++;
        ReadLine();

        if(unit.val != "(")
        {
            addError(4);//缺少(
        }
        else
        {
            ReadLine();
        }
        if(unit.key == "id")
        {
            string name = unit.val;
            addVar(name, lev, count + 3);
            count++;
            SymTable[tx0].size = count;

            ReadLine();
            while(unit.val == "," || unit.key == "id")
            {
                if(unit.val == ",")
                    ReadLine();
                else
                    addError(5);//缺少,
                
                if(unit.key == "id")
                {
                    string name = unit.val;
                    addVar(name, lev, count + 3);
                    count++;
                    SymTable[tx0].size = count;
                    ReadLine();
                }
                else
                {
                    addError(6);//缺少id
                    while(unit.val != ")" && unit.key != "id" && unit.val != ";" && unit.val != "const" && unit.val != "var" && unit.val != "procedure")
                        ReadLine();
                    if(unit.key == "id")
                        ReadLine();
                }
            }
            if(unit.val != ")")
            {   
                addError(3);//缺少)
            }
            else
                ReadLine();
        }
        else if(unit.val == ")")
            ReadLine();
        else
        {
            addError(3);//缺少)
            ReadLine();
            while(unit.val != ")" && unit.val != ";" && unit.val != "const" && unit.val != "var" && unit.val != "procedure")
                ReadLine();
        }
    }
    else
    {
        addError(6);//缺少id
        while(unit.val != "(" && unit.val != ")" && unit.val != ";")
            ReadLine();
        if(unit.val == "(")
        {
            ReadLine();
            para_func();
        }
    }

    if(unit.val == ";")
        ReadLine();
    else
    {
        addError(2);//缺少;
        test(2);//同步
    }

    if(unit.val == "const" || unit.val == "var" || unit.val == "procedure" || unit.val == "begin" || unit.key == "id" || unit.val == "if" || unit.val == "while" || unit.val == "call" || unit.val == "read" || unit.val == "write")
        Block();

    while(unit.val == ";")
    {
        ReadLine();
        Proc();
    }
    
}

void Var()//-------应该ok-----------
{
    if(unit.key == "id")
    {
        string name = unit.val;
		if (is_the_same_level(name, lev))
		{
			addError(23);//重复定义
		}

		addVar(name, lev, dx);
		dx++;
        ReadLine();
    }
    else
    {
        addError(6);//var后面应为标识符
        if(unit.key == "INT")
            ReadLine();
        while(unit.val != "," && unit.key != "id" && unit.val != ";")
            ReadLine();
    }
}

/**
 * <vardecl> → var <id>{,<id>};
 * <id> → l{l|d}
 * l represent letter.
 */
void Vardecl()//-------应该ok-----------
{
    ReadLine();
    Var();
    while(unit.val == "," || unit.key == "id")
    {
        if(unit.key == "id")
        {
            string name = unit.val;
            if (is_the_same_level(name, lev))
            {
                addError(23);//重复定义
            }

            addVar(name, lev, dx);
            dx++;
            addError(5);//缺少,
        }    
        else
        {
        	ReadLine();
        	Var();
		}    
    }

    if(unit.val == ";")
        ReadLine();
    else
        addError(2);//缺少;

}

/**
 * Const variables declaration.
 * <const> → <id>:=<integer>
 * <id> → l{l|d}
 * l represent letter.
 */
void Const()//-------应该ok-----------
{
    ReadLine();
    string name;
    if(unit.key == "id")
    {
        name = unit.val;
        ReadLine();
    }
    else
    {
        addError(6);//缺少id
        while(unit.val != ":=" && unit.val != "=" && unit.val != ";")
            ReadLine();
        if(unit.val == ";")
            return;
    }

    if(unit.val == ":=")
    {
        ReadLine();
        if(unit.key == "INT")
        {
            int value = stringtoint(unit.val);

            if(is_the_same_level(name, lev))
                addError(23);//多重定义
            addConst(name, lev, value);
			ReadLine();
		}    
        else
        {
            addError(13);//常量赋值号后面应为数字
            if(unit.key == "id")
                ReadLine();
        }
    }
    else
    {
        if(unit.val == "=")
        {
            addError(14);//缺少:
            ReadLine();

            if(unit.key == "INT")
            {
                int value = stringtoint(unit.val);

                if(is_the_same_level(name, lev))
                    addError(23);//多重定义
                addConst(name, lev, value);
                ReadLine();
            } 
            else
                addError(13);//赋值号后面应为数字
        }
        else
        {
            addError(15);//缺少:=
            while(unit.val != ":=" && unit.key != "INT" && unit.val != ",")
                ReadLine();
            if(unit.key != "INT")
                return;
        }
    }

}

/**
 * Const variables declaration.
 * <condecl> → const <const>{,<const>};
 */
void Condecl()//-------应该ok-----------
{
    Const();
    bool flag = false;
    while(unit.val == "," || unit.key == "id")
    {
        flag = true;
        if(unit.key == "id")
            addError(5);//缺少逗号 
        else
            ;//无需Readline(),因为在Const函数内调用了Readline
        Condecl();
    }
    
    if(flag == false)
    {
        if(unit.val == ";")
            ReadLine();
        else
            addError(2);//缺少;
    }
}

/**
 * <block> → [<condecl>][<vardecl>][<proc>]<body>
 */
void Block()//-------应该ok-----------
{
    int dx0 = dx;
	int tx0 = tx + 1;
	int n = 0;

    if (tx0 > 1)
	{
		n = findproc();
		tx0 -= SymTable[n].size;//减去形参个数
        dx = 3 + SymTable[n].size;
	}
	else
		dx = 3;

	int cx0 = cx;//记录跳转指令的位置
	gen(JMP, 0, 0);

    if(unit.val == "const")
        Condecl();
    if(unit.val == "var")
        Vardecl();
    if(unit.val == "procedure")
    {
        Proc();
        lev--;
    }

    if(tx0 > 1)
    {
        n = findproc();
        for(int i = 0; i < SymTable[n].size; i++)
            gen(STO, 0, SymTable[n].size + 3 - 1 - i);//？？？？
    }

    Pcode[cx0].a = cx;//回填
    INT_pos = cx;//记录等待回填的位置
    gen(INT, 0, dx);

    if(tx != 1)
    {
        SymTable[n].value = cx - 1 - SymTable[n].size;  //过程入口地址, 会生成SymTable[n].size个指令
    }

    Body();
    gen(OPR, 0, 0);

    tx = tx0;   // clear Sym
	dx = dx0;
}

/**
 * <prog> → program <id>; <block>
 */
void Prog()//-------应该ok-----------
{
    ReadLine();
    if(unit.val != "program")
    {
        addError(16);//缺少program
        if(unit.key != "id" && unit.val != ";")
            ReadLine();
    }
    else
    {
        ReadLine();
    }

    if(unit.val != ";")
    {
        if(unit.key != "id")
        {
            ReadLine();
            addError(6);//缺少id
        }
        else
        {
            ReadLine();            
            if(unit.val != ";")
                addError(2);//缺少;
            else
                ReadLine();
        }
    }
    else
        addError(6);//缺少id
    
    while(unit.val != "const" && unit.val != "var" && unit.val != "procedure" && unit.val != "begin" && unit.val != ";")
    {
//        addError(17);//存在多余的单词
        ReadLine();
    }
    if(unit.val == ";")
        ReadLine();
    Block();
}

/**
 * Open IO files.
 */
void OpenFile()
{
    in.open("C:\\Users\\forever\\Desktop\\compile\\GA\\la_output.txt", ios::in);
//    out.open("ga_output.txt", ios::out);
    if(!in)
    {
        cout << "输入文件打开失败" << endl;
        exit(-1);
    }
//    if(!out)
//    {
//        cout << "输出文件打开失败" << endl;
//        exit(-1);
//    }
}

/**
 * Close IO files.
 */
void CloseFile()
{
    in.close();
    out.close();
}

int main()
{
	LA();
    OpenFile();
	Prog();
	
    int count = 0; //记录未被使用的变量个数
	int i = 1;
    while (SymTable[i].num && !SymTable[i].level)  //在符号表中遍历第0层主函数的符号
	{
		if (SymTable[i].num == 1 && SymTable[i].type == VAR) //如果是变量且只定义过一次，后续没有使用，进行后续优化
		{
			int j = i + 1;                                      
			while (SymTable[j].num && !SymTable[j].level)    //后续所有定义的变量相对地址都要减一
			{
				SymTable[j].adr--;                    //
				j++;
			}
			count++;    //未被使用的变量个数加一
		}
		i++;
	}
    
	Pcode[INT_pos].a -= count;     //回填INT指令开辟的空间数，一边扫描之后，解释器之前
	cout << "是否输出Pcode:1 or 0" << endl;
	int flag;
	cin >> flag;
	if (flag)
	{
		printPcode();
	}
	cout << "是否输出符号表:1 or 0" << endl;
	cin >> flag;
	if (flag)
	{
		printTable();
		cout << endl;
	}
    interpreter(); 
    
    CloseFile();
	return 0;
}
