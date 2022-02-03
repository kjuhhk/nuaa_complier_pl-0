#include<iostream>
#include<fstream>
#include<cstdio>
#include<cstdlib>
#include<cstring>

using namespace std;

FILE *fp = fopen("C:\\Users\\forever\\Desktop\\compile\\pl-0\\la_input.txt", "r");
fstream output("C:\\Users\\forever\\Desktop\\compile\\pl-0\\la_output.txt", ios::out | ios::trunc); // 写文件;

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

bool isBC(char ch)
{
    if(ch == ' '){ // 空格
        col++;
        return true;
    }
    else if(ch == '\t'){ // tab
        col += 4;
        return true;
    }
    else if(ch == '\r' || ch == '\n') // 回车
    {
        row++;
        col = 1;
        return true;
    }
    else{
        return false;
    }
}

void Concat(string& strToken, char ch) {
	strToken.push_back(ch);
}

bool IsLetter(char ch)
{
    if(ch >= 'a' && ch <= 'z'){
        return true;
    }
    else if(ch >= 'A' && ch <= 'Z'){
        return true;
    }
    else return false;
}

bool IsDigit(char ch)
{
    if(ch >= '0' && ch <= '9'){
        return true;
    }
    else return false;
}

int Reserve(string strToken)
{
	for(int i = 1; i <= 15; i++) {
		if (strToken == key[i]) {
			return i;
		}
	}
	return 0;
}

void Retract(char ch)
{
    if (ch != EOF){
        ungetc(ch, fp);
    }
}

int LA()
{
    if(!fp){
        cout << "输入文件打开失败" << endl;
        exit(1);
    }

    if(!output.is_open())
    {
        cout << "输出文件打开失败" << endl;
        exit(2);
    }

    row = col = 1;
    string strToken;
    char ch;

    while(1)
    {
        ch = fgetc(fp);
        if(ch == EOF) break;

        //1.判断是否为空白
        if(isBC(ch)){
            strToken = "";
        }
        //2.判断是否为字母
        else if(IsLetter(ch)){
            //将所有字符连接起来
            while(IsDigit(ch) || IsLetter(ch))
            {
                Concat(strToken, ch);
                col++;
                ch = fgetc(fp);
            }
            
            //判断是关键字还是ID
            if (Reserve(strToken)){ 
                output << strToken << " RESERVED " << row << ' ' << col << endl;
            }
			else{
                output << strToken << " id " << row << ' ' << col << endl;
            }

            strToken = "";
            Retract(ch);
        }
        //3.判断是否为数字
        else if (IsDigit(ch)){
            //将所有数字连接起来
			while (IsDigit(ch)) {
				Concat(strToken, ch);
                col++;
				ch = fgetc(fp);
			}

            //以数字开头的ID，报错！
            if (IsLetter(ch)) {

                //继续读完字符串
                while (IsLetter(ch) || IsDigit(ch)){
                    Concat(strToken, ch);
                    col++;
                    ch = fgetc(fp);
                }
				cout << "[Lexical ERROR]" << " [" << row << "," << col <<"] " << "Invalid ID: " << strToken << endl;
                output << strToken << " id " << row << ' ' << col << endl;
            } 
            else {
                
                output << strToken << " INT " << row << ' ' << col << endl;
            }

            Retract(ch);
            strToken = "";
        }
        //4.判断是否为其他字符
        else{
            if(ch == '=')
            {
                col++;
                output << ch << " COP " << row << ' ' << col << endl; 
            }
            else if(ch == '<')
            {
                col++;
                ch = fgetc(fp);
                if(ch == '>')
                {
                    col++;
                    output << "<> COP " << row << ' ' << col << endl;
                }
                else if(ch == '=')
                {
                    col++;
                    output << "<= COP " << row << ' ' << col << endl;
                }
                else
                {
                    output << "< COP " << row << ' ' << col << endl;
                    Retract(ch);
                }
            }
            else if(ch == '>')
            {
                col++;
                ch = fgetc(fp);
                if(ch == '=')
                {
                    col++;
                    output << ">= COP " << row << ' ' << col << endl;
                }
                else
                {
                    output << "> COP " << row << ' ' << col << endl;
                    Retract(ch);
                }
            }
            else if(ch == ':')
            {
                col++;
                ch = fgetc(fp);
                if(ch == '=')
                {
                    col++;
                    output << ":= AOP " << row << ' ' << col << endl; 
                }
                else
                {
                	cout << "[LEXICAL ERROR]" << " [" << row << "," << col << "] " << "Missing \"=\" near the \":\" " << endl;
                    output << ":= AOP " << row << ' ' << col << endl; 
					Retract(ch);
                }
            }
            else if(ch == '+' || ch == '-' || ch == '*' || ch == '/')
            {
                col++;
                output << ch << " OOP " << row << ' ' << col << endl; 
            }
            else if(ch == ';')
            {
                col++;
                output << ch << " EOP " << row << ' ' << col << endl; 
            }
            else if(ch == '(' || ch == ')' || ch == ',' || ch == '.')
            {
                col++;
                output << ch << " SOP " << row << ' ' << col << endl; 
            }
            else
            {
                col++;
                output << ch << " UNKNOWN " << row << ' ' << col << endl;
            }
        }
    }
    fclose(fp);
    output.close();

    printf("-----词法分析已完成，结果存至la_output.txt文件中-----\n");

    return 0;
}
