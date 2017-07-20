//  SIC/XE Assember with support for control sections
//  Authors: Varan Shukla and Hrushikesh Hanbar
#include<iostream>
#include<stdio.h>
#include<map>
#include<fstream>
#include<sstream>
#include<vector>
#include<algorithm>
using namespace std;

map<string, int> format, opcode;							//Maps to store INstruction Format ans OpCodes
vector<map<string,int>> symtab;								//Vector of maps representing SYMTAB for each control section
vector<string> programName, baseAddress, extdefs, extrefs;	//Vector to store strings of program names, base address, extdefs, extrefs	
map<char, int> regcode;										//map storing code for each register
vector<int> sizeOfCsect;									//vector storing number of executable instructions in each control section

struct  statement{											// structure to store instruction statement
    int address;
    string label;
    string instruction;
    string parameter;
    int objectCode;
    int e;
};
vector<vector<statement>>   completeArray;					//2D vector to store instruction statements for each control statement

void initOpTab(){											//To Read The Instruction Set File and initialize format and opcode maps
    FILE *fp;
    fp=fopen("INS.txt","r");
    char ins[8];   int frmt;  int opc;    int c;
    while(1){
        fscanf(fp,"%s",ins);
        fscanf(fp,"%d",&frmt);
        fscanf(fp,"%x",&opc);
        format[ins] = frmt;
        opcode[ins]= opc;
        if((c=fgetc(fp))==EOF)	break;
    }
}

void setCsect(vector<string> &list,int count, int &i, int nCsect){	//funtion to populate the completeArray
    completeArray[nCsect-1].push_back(*(new statement()));
	if(i!=0){
        if(completeArray[nCsect-1][i-1].instruction!="RESW" && completeArray[nCsect-1][i-1].instruction!="RESB" && completeArray[nCsect-1][i-1].instruction!="BYTE" && completeArray[nCsect-1][i-1].instruction!="WORD")
			if(completeArray[nCsect-1][i-1].e==1)
                completeArray[nCsect-1][i].address = completeArray[nCsect-1][i-1].address + 4;
            else
                completeArray[nCsect-1][i].address = completeArray[nCsect-1][i-1].address + format[completeArray[nCsect-1][i-1].instruction];
        else{
            if(completeArray[nCsect-1][i-1].instruction=="RESW")
                completeArray[nCsect-1][i].address = completeArray[nCsect-1][i-1].address + (3*stoi(completeArray[nCsect-1][i-1].parameter) );
            else if(completeArray[nCsect-1][i-1].instruction=="RESB")
                completeArray[nCsect-1][i].address = completeArray[nCsect-1][i-1].address + (stoi(completeArray[nCsect-1][i-1].parameter) );
            else if(completeArray[nCsect-1][i-1].instruction=="WORD")
                completeArray[nCsect-1][i].address = completeArray[nCsect-1][i-1].address + 3;
            else if(completeArray[nCsect-1][i-1].instruction=="BYTE")
                completeArray[nCsect-1][i].address = completeArray[nCsect-1][i-1].address + 1;
        }
    }
	else
		completeArray[nCsect-1][i].address = 0;
    if(count == 4){
        if(symtab[nCsect-1].find(list[0])==symtab[nCsect-1].end())
            completeArray[nCsect-1][i].label = list[0];
        else cout<<"Error : Duplicate Labels used\n";
        if(list[1][0]=='+'){
            completeArray[nCsect-1][i].instruction = list[1].substr(1);
            completeArray[nCsect-1][i].e=1;
        }else{
            completeArray[nCsect-1][i].e=0;
            completeArray[nCsect-1][i].instruction = list[1];
        }
        completeArray[nCsect-1][i].parameter = list[2];
		symtab[nCsect-1][completeArray[nCsect-1][i].label]=completeArray[nCsect-1][i].address;
    }
    else if(count == 3){
        completeArray[nCsect-1][i].label = "\0";
        if(list[0][0]=='+'){
            completeArray[nCsect-1][i].instruction = list[0].substr(1);
            completeArray[nCsect-1][i].e=1;
        }else{
            completeArray[nCsect-1][i].e=0;
            completeArray[nCsect-1][i].instruction = list[0];
        }
        completeArray[nCsect-1][i].parameter = list[1];
    }
    else if(count == 2){
        //completeArray[nCsect-1].push_back(*(new statement()));
        completeArray[nCsect-1][i].label = "\0";
        completeArray[nCsect-1][i].instruction = list[0];
        completeArray[nCsect-1][i].parameter = "\0";
    }
    else
        cout<<"Error invalid Instruction "<<list[0]<<endl;     
    i++;
}

int firstPass(char* filename){										//First Pass of the assembler
    ifstream inFile(filename);										//To Read the Input File line by line and split it into list
	int i = 0,nCsect = 0;											//and use function setCsect to popuate the completeArray
    string str;
    while(getline(inFile,str))
    {	istringstream iss(str);
        vector<string> list;
        int count = 0;
        do{
            string sub;
            iss>>sub;
            list.push_back(sub);
            count++;
        }while(iss);
        if(list[0][0]=='.')											//if line is a comment
            continue;
        if(list[0] == "END")										//in case of end of program
            continue;
		if(list[1] == "CSECT")
            sizeOfCsect.push_back(i);
		if(list[1]=="START"|| list[1] == "CSECT"){					//incase of start of each new control section
            completeArray.push_back(*(new vector<statement>));
        	programName.push_back(list[0]);i=0; nCsect++;
            extdefs.push_back("");
            extrefs.push_back("");
            baseAddress.push_back("\0");
			continue;
		}
        if(list[0]=="BASE"){										//setting base address for each control section if present
            baseAddress[nCsect-1] = list[0];
            continue;       
        }
		if(list[0]=="EXTDEF"){										//putting external definiion parameter string into extdefs
			extdefs[nCsect-1] = (list[1]);
			continue;		
        }
		if(list[0]=="EXTREF"){										//putting external refernce parameter string into extrefs
			extrefs[nCsect-1] = (list[1]);
			continue;
		}
        symtab.push_back(*(new map<string,int>()));
		setCsect(list,count,i, nCsect);		
    }
    sizeOfCsect.push_back(i);
    return nCsect;
}

void secondpass(int nCsect){										//Second Pass of assembler to calculate object code for each instruction
    for(int i=0;i<nCsect;i++){
        for(int j=0;j<completeArray[i].size();j++){
            int temp = 0;
            if(completeArray[i][j].instruction=="WORD" || completeArray[i][j].instruction=="BYTE"){	//incase of WORD and BYTE instruction
                if(completeArray[i][j].parameter[0]=='X'){											//if parameter is of type X'xxxx'
                    string sub1 = completeArray[i][j].parameter.substr(2,completeArray[i][j].parameter.length() - 3);
                    completeArray[i][j].objectCode = stoi(sub1,nullptr, 16);
                }
                else if(completeArray[i][j].parameter[0]=='C'){										//if parameter is of type C'xxxx'
                    string sub1 = completeArray[i][j].parameter.substr(2,completeArray[i][j].parameter.length() - 3);
                    for(int i=0;i<sub1.length();i++){
                        temp += (int)sub1[i];
                        if(i!=sub1.length()-1)
                            temp = temp<<8;
                    }
                    completeArray[i][j].objectCode = temp;
                }
                else
                    completeArray[i][j].objectCode = stoi(completeArray[i][j].parameter);
            }
            else if(completeArray[i][j].instruction!="RESW" && completeArray[i][j].instruction!="RESB"){	//for all other instructions
                if(format[completeArray[i][j].instruction] == 3 && completeArray[i][j].e==0){				//incase of format 3
                    temp = opcode[completeArray[i][j].instruction];
                    if(completeArray[i][j].instruction=="RSUB"){
                        temp += 3;
                        temp = temp<<16;
                        completeArray[i][j].objectCode = temp;
                        continue;
                    }
                    if(completeArray[i][j].parameter[0]=='#'){												//incase of immediate addressing
                        temp += 1;
                        temp  = temp<<16;
                        string sub1 = completeArray[i][j].parameter.substr(1,completeArray[i][j].parameter.length() - 1);
                        if(symtab[i].find(sub1) != symtab[i].end())
                            temp += symtab[i][sub1];
                        else
                            temp += stoi(sub1);
                        completeArray[i][j].objectCode = temp;
                        continue;
                    }
                    else if(completeArray[i][j].parameter[0]=='@'){											//incase of indirect addressing
                        temp += 2;
                        completeArray[i][j].parameter = completeArray[i][j].parameter.substr(1,completeArray[i][j].parameter.length() - 1);
                    }
                    else{
                        temp += 3;
                    }               
                    temp = temp<<4;
                    int byte3 = 0;
                    if(completeArray[i][j].parameter[-2] == ','){											//iincase of indexed addressing
                        byte3 = 8;
                        completeArray[i][j].parameter = completeArray[i][j].parameter.substr(0,completeArray[i][j].parameter.length()-2);
                    }
                    if(baseAddress[i]=="\0" && completeArray[i][j+1].address - symtab[i][completeArray[i][j].parameter] <= 0xFFF ){
                        byte3 += 2;																			//for program counter realtive
                        temp += byte3;																		//address calculation
                        if(symtab[i][completeArray[i][j].parameter] - completeArray[i][j+1].address < 0)
                            temp++;
                        temp = temp<<12;
                        temp += symtab[i][completeArray[i][j].parameter] - completeArray[i][j+1].address;
                    }
                    else if(baseAddress[i]!="\0"){															//for base relative 
                        byte3 += 4;
                        temp += byte3;
                        temp = temp<<12;
                        temp += symtab[i][completeArray[i][j].parameter] - symtab[i][baseAddress[i]];                   
                    }
                    else
                        cout<<"Out of Bound! Use base relative or format 4 for"<<completeArray[i][j].address<<"\t"<<completeArray[i][j].label<<"\t"<<completeArray[i][j].instruction<<"\t"<<completeArray[i][j].parameter<<" "<<endl;
                    completeArray[i][j].objectCode = temp;
                }
                else if(format[completeArray[i][j].instruction] == 3 && completeArray[i][j].e==1){			//incase of format 4
                    temp = opcode[completeArray[i][j].instruction];
                    if(completeArray[i][j].parameter[0]=='#'){
                        temp += 1;
                        temp  = temp<<24;
                        string sub1 = completeArray[i][j].parameter.substr(1,completeArray[i][j].parameter.length() - 1);
                        if(symtab[i].find(sub1) != symtab[i].end())
                            temp += symtab[i][sub1];
                        else
                            temp += stoi(sub1);
                        completeArray[i][j].objectCode = temp;
                        continue;
                    }
                    else if(completeArray[i][j].parameter[0]=='@'){
                        temp += 2;
                        completeArray[i][j].parameter = completeArray[i][j].parameter.substr(1,completeArray[i][j].parameter.length() - 1);
                    }
                    else	temp += 3;
                    temp = temp<<4;
                    if(completeArray[i][j].parameter[completeArray[i][j].parameter.length()-2] == ','){
                        temp += 9;
                        completeArray[i][j].parameter = completeArray[i][j].parameter.substr(0,completeArray[i][j].parameter.length()-2);
                    }
                    else	temp += 1;
                    temp = temp<<20;
                    temp += symtab[i][completeArray[i][j].parameter];
                    completeArray[i][j].objectCode = temp;
                }	
                else if(format[completeArray[i][j].instruction] == 2){										//incase of format 2
                    temp = opcode[completeArray[i][j].instruction];
                    temp = temp<<4;
                    temp += regcode[completeArray[i][j].parameter[0]];
                    temp = temp<<4;
                    temp += regcode[completeArray[i][j].parameter[2]];
                    completeArray[i][j].objectCode = temp;
                }
                else	completeArray[i][j].objectCode = opcode[completeArray[i][j].instruction];			//incase of format 1
            }
            else	completeArray[i][j].objectCode = stoi(completeArray[i][j].parameter);
        }
    }
}

string emptyBlocks(int a, int b = 6){										//function to convert string to make it of length 6
    string ans = "";
    while(a++<b)
        ans += " ";
    return ans;
}

string intToString(int n, int x = 6){										//function to convert int to string of default lenght 6
    stringstream stream;
    stream<<hex<<n;
    string result( stream.str() );
    for(int y=0;y<result.length();y++)	result[y] = toupper(result[y]);
    while(result.length()<x)	result = "0" + result;
    return result;
}

void printObjectProgram(int nCsect){										//function to print the object program
    ofstream outfile;
    outfile.open("output.txt");
    for(int i=0;i<nCsect;i++){												//loop for each control section
        int sectionSize = completeArray[i].back().address;
        if(completeArray[i].back().instruction!="RESW" && completeArray[i].back().instruction!="RESB" && completeArray[i].back().instruction!="BYTE" && completeArray[i].back().instruction!="WORD")
            if(completeArray[i].back().e==1)
                sectionSize += 4;
            else
                sectionSize += format[completeArray[i].back().instruction];
        else{
            if(completeArray[i].back().instruction=="RESW")
                sectionSize += (3*completeArray[i].back().objectCode);
            else if(completeArray[i].back().instruction=="RESB")
                sectionSize += (completeArray[i].back().objectCode);
            else if(completeArray[i].back().instruction=="WORD")
                sectionSize += 3;
            else if(completeArray[i].back().instruction=="BYTE")
                sectionSize += 1;
        }
        outfile<<"H"<<programName[i];										//printing header record
        outfile<<emptyBlocks(programName[i].length())<<intToString(0);
        outfile<<hex<<intToString(sectionSize)<<dec<<"\n";
        stringstream test(extdefs[i]);
        string segment;
        vector<string> seglist;												//printing define record
        while(std::getline(test, segment, ','))
            seglist.push_back(segment);
        if(seglist.size()!=0)
        	outfile<<"D";
        for(int k=0;k<seglist.size();k++)
            outfile<<seglist[k]<<emptyBlocks(seglist[k].length())<<intToString(symtab[i][seglist[k]]);
        if(seglist.size()!=0)
        	outfile<<"\n";
        stringstream test1(extrefs[i]);										//printing refernce record
        vector<string> references;
        while(std::getline(test1, segment, ','))
            references.push_back(segment);
        if(references.size()!=0)   
        	outfile<<"R";
        for(int k=0;k<references.size();k++)
            outfile<<references[k]<<emptyBlocks(references[k].length());
        if(references.size()!=0)   outfile<<endl;
        map<int, string> modmap;
        int stat = 0;
        int limit = 0;
        for(;stat < sizeOfCsect[i];){										//printing text records
            outfile<<"T"<<intToString(completeArray[i][stat].address);
            int dummy = stat,y;
            for(y= 0;y<30;){												//loop to calculate size of each text record
                if(dummy>=sizeOfCsect[i])	break;
                if(find(references.begin(), references.end(), completeArray[i][dummy].parameter) != references.end())
				    modmap[completeArray[i][dummy].address] = completeArray[i][dummy].parameter;
				else if(completeArray[i][dummy].e==1)
					modmap[completeArray[i][dummy].address] = completeArray[i][dummy].parameter;
                if(completeArray[i][dummy].instruction=="RESW" || completeArray[i][dummy].instruction=="RESB"){
                    while(completeArray[i][dummy].instruction=="RESW" || completeArray[i][dummy].instruction=="RESB")	dummy++;
                    break;
                }
                if(format[completeArray[i][dummy].instruction] == 3 && completeArray[i][dummy].e == 0){
                    dummy++;	y+=3;
                }
                else if(format[completeArray[i][dummy].instruction] == 3 && completeArray[i][dummy].e == 1){
                    dummy++;	y+=4;                  
                }
                else if(format[completeArray[i][dummy].instruction] == 2){
                    dummy++;	y+=2; 
                }
                else{
                    dummy++;	y+=1; 
                }
                if(dummy>=sizeOfCsect[i])	break;
                if(y+format[completeArray[i][dummy].instruction]>30 || (y+format[completeArray[i][dummy].instruction]>31 && completeArray[i][dummy].e==1))
                    break;
            }
            outfile<<intToString(y,2);
            for(int x= 0;x<30;){											//printing text record
                if(stat>=sizeOfCsect[i])	break;
                if(completeArray[i][stat].instruction=="RESW" || completeArray[i][stat].instruction=="RESB"){
                    while(completeArray[i][stat].instruction=="RESW" || completeArray[i][stat].instruction=="RESB")	stat++;
                    break;
                }
                if(format[completeArray[i][stat].instruction] == 3 && completeArray[i][stat].e == 0){
                    outfile<<intToString(completeArray[i][stat].objectCode);
                    stat++;	x+=3;
                }
                else if(format[completeArray[i][stat].instruction] == 3 && completeArray[i][stat].e == 1){
                    outfile<<intToString(completeArray[i][stat].objectCode, 8);
                    stat++;	x+=4;                  
                }
                else if(format[completeArray[i][stat].instruction] == 2){
                    outfile<<intToString(completeArray[i][stat].objectCode, 4);
                    stat++;	x+=2; 
                }
                else{
                    outfile<<intToString(completeArray[i][stat].objectCode, 2);
                    stat++;	x+=1; 
                }
                if(stat>=sizeOfCsect[i])	break;
                if(x+format[completeArray[i][stat].instruction]>30 || (x+format[completeArray[i][stat].instruction]>31 && completeArray[i][stat].e==1))
                    break;
            }
            outfile<<endl;
        }
        map<int, string>::iterator it;										//printing modification records
        for(it = modmap.begin();it != modmap.end();it++)
        	outfile<<"M"<<intToString(it->first)<<intToString(5,2)<<"+"<<it->second<<endl;
        outfile<<"E"<<intToString(0,6)<<endl<<endl;							//printing end record
    }
}

int main(int argc,char *argv[]){
    regcode['A'] = 0;	regcode['X'] = 1;	regcode['L'] = 2;	regcode['B'] = 3;	regcode['S'] = 4;	regcode['T'] = 5;	regcode['F'] = 6;
    initOpTab();															//calling initoptab to populate format and opcode maps
    int nCsect=firstPass(argv[1]);											//calling first pass function, nCsect is number of control sections
    secondpass(nCsect);														//calling secong pass function
    printObjectProgram(nCsect);												//calling printObjectProgram to print object Program
    return 0;
}