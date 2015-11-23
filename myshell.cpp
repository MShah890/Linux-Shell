#include<iostream>
#include<vector>
#include<fstream>
#include<cstring>
#include<cstdlib>
#include<unordered_map>
#include<unistd.h>
#include<sys/wait.h>

#define MAXCOMMANDS 1000
#define MAXARGS 1000
using namespace std;

struct command
{
	ifstream input;
	ofstream output;	

	int num_commands=-1;
	int num_args[MAXCOMMANDS]={0};

	char* command_list[MAXCOMMANDS][MAXARGS];					//command and their arguments storage
	vector<string> pr_operator;							//Pipe or redirection operators storage

	bool background_task;
	bool append;

};

string line;
command s;
string opr="|<>";	

void eat_space(int &i)
{
	while(line[i]==' '&&i<line.length())
	{
		i++;
	}
}

void readline()
{	
	cout<<"COMMANDS,OPERATORS AND ARGUMENTS SHOULD BE SEPARATED BY ATLEAST ONE WHITESPACE AND ONLY SINGLE PIPE COMMANDS WILL WORK"<<endl;
	getline(cin,line);
}


void parse()
{
	s.num_commands=-1;
	s.num_args[MAXCOMMANDS]={0};
	s.pr_operator.clear();

	string com;
	string arg;

	int command_number=0;	

	int flag_parser_leave=0;
	int flag_argument=0;
	int flag_operator=0;
	int flag_argends=0;
	int flag_command=0;

	int i;
	for(i=0;i<line.length();i++)									//loop till line not over
	{
		com="";
		eat_space(i);
		if(i>line.length())
			break;	
		while(line[i]!=' '&&i<line.length())							//getting command
		{
			flag_command=1;	
			com+=line[i];
			i++;
		}
		if(flag_command)
			s.num_commands++;								//increment number of commands
		eat_space(i);		
		if(i>line.length()&&flag_command!=1)					//if no such command exists that has no arguments hence > and not >=
			break;
		while(true)								// a command can have more than one argument hence an outer loop
		{		
			arg="";		
			while((opr.find(line[i])==string::npos)&&i<line.length()&&line[i]!=' ')		//getting argument
			{
				flag_argument=1;
				flag_argends=0;
				
				if(line[i]=='\"')						//if argument of form "xyz pqr" then different way of extraction
				{
					i++;								//To get past first double quote
					while(i<line.length())
					{
						if(line[i]!='\"')
							arg+=line[i];
						else
						{
							flag_argends=1;
							i++;						//To get past ending double quote
							break;
						}
						i++;
					}

				}

				if(flag_argends||i>line.length())					//flag_argends is for quoted arguments
					break;
				
				arg+=line[i];								//appending character to argument string	
				i++;
			}	
			if(flag_argument&&arg!="")						//if argument exists
			{
				s.num_args[s.num_commands]++;				//incrementing number of arguments 
				char *cstr = new char[com.length() + 1];		//Converting command and argument from string to char*
				strcpy(cstr, com.c_str());

				char *astr = new char[arg.length() + 1];
				strcpy(astr, arg.c_str());
				
				if(flag_command)
					s.command_list[s.num_commands][0]=cstr;			//setting command
			
				s.command_list[s.num_commands][s.num_args[s.num_commands]]=astr; //setting argument for particular command in matrix
				flag_command=0;	
				flag_argument=0;
			
			}
			if(flag_command==1)
			{
				char *cstr = new char[com.length() + 1];		//Converting command from string to char*
				strcpy(cstr, com.c_str());
					
				s.command_list[s.num_commands][0]=cstr;			//when only command and no argument
			}
			if(opr.find(line[i])!=string::npos)				//if operator then break
				break;	
			eat_space(i);	
			if(i>=line.length())
			{
				break;
				flag_parser_leave=1;
			}

		}
	
		if(flag_parser_leave==1)
			break;

		if(opr.find(line[i])!=string::npos)					//when line has operator extract it and insert into pr_operator		
		{
			if(line[i]=='|')
				s.pr_operator.push_back("|");
			else if(line[i]=='<')
			{	
				i++;
				if(line[i]=='<'&&i<line.length())
					s.pr_operator.push_back("<<");
				else if(i>line.length())
					break;
				else
					s.pr_operator.push_back("<");	
			}
			else if(line[i]=='>')
			{	
				i++;
				if(line[i]=='>'&&i<line.length())
					s.pr_operator.push_back(">>");
				else if(i>line.length())
					break;
				else
					s.pr_operator.push_back(">");	
			}
			i++;								//To get past operator	
		}
	
	}
}

int execute()
{
	pid_t pid,wpid;

	int status;
	int pfds[2];

	pid=fork();

	if(pid==0)
	{
		pipe(pfds);

		pid_t pid2,wpid2;
		
		if(!s.pr_operator.empty())
		{
			//cout<<"Inside if condition checking pipe operator existence";
			pid2=fork();
	
			if(pid2==0)
			{
				//cout<<"Pipe Operator Second Child"<<endl;
				close(1);	
				dup(pfds[1]);	
				close(pfds[1]);
				close(pfds[0]);
				if(execvp(s.command_list[0][0],s.command_list[0])==-1)
				{
					perror("myshell");
				}
				cout<<"After ls"<<endl;

				exit(1);
			}
			else if(pid2<0)
			{
				perror("myshell");
			}
			else
			{
			
				close(0);
				dup(pfds[0]);
				close(pfds[0]);
				close(pfds[1]);	

				if(execvp(s.command_list[1][0],s.command_list[1])==-1)
				{
					perror("myshell");
				}

				exit(1);
			}
		}
		else
		{
				//cout<<"No Pipe Operator 1st Child";
				if(execvp(s.command_list[0][0],s.command_list[0])==-1)
				{
					perror("Here myshell");
				}
		}

		exit(1);
	}
	else if(pid<0)
	{
		perror("myshell");
	}
	else
	{
		while (wait(&status) != pid);

	}
	
	return 1;
	
}

void start_loop()
{
	int status,flag;
	do
	{
		printf("(::)");
		flag=0;

		readline();
	 	parse();

		for(int i=0;i<=s.num_commands;i++)
		{
			s.command_list[i][s.num_args[i]+1]=NULL;
			s.num_args[i]=0;		
		}

		if(!strcmp(s.command_list[0][0],"exit"))
		{
			exit(0);	
		}
		else if(!strcmp(s.command_list[0][0],"cd"))
		{
			
			chdir(s.command_list[0][1]);
			flag=1;
			status=1;
		}
		
		if(!flag)
			status=execute();	
		
	}while(status);
}


int main()
{
	start_loop();

	return 0;
}
