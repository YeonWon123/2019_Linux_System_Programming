#include "ssu_backup.h"
pthread_mutex_t mutex =PTHREAD_MUTEX_INITIALIZER;
int shared_threadlock;
pthread_t backup_tid;

ssu_thread *threadHEAD=NULL,*threadTAIL=NULL;
ssu_backupList *listHEAD=NULL,*listTAIL=NULL;
void writeLog(char *order, char *filename)
{
	FILE* fp = fopen("log.txt", "a+");

	char date[17] = { 0 };
	time_t timer;
	struct tm* t;

	timer = time(NULL);
	t = localtime(&timer);

	sprintf(date, "[%02d%02d%02d %02d%02d%02d]", t->tm_year % 100, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);

	if (!strcmp(order, "add"))
		fprintf(fp, "%s %s added\n", date, filename);
	else if (!strcmp(order, "remove"))
		fprintf(fp, "%s %s deleted\n", date, filename);
	else if (!strcmp(order, "recover"))
		fprintf(fp, "%s %s recovered\n", date, filename);
	else if (!strcmp(order, "generate"))
		fprintf(fp, "%s %s generated\n", date, filename);

	fclose(fp);
}

void Eliminate(char *str,char ch)
{
	for(;*str !='\0';str ++){
		if(*str ==ch)
		{
			strcpy(str,str+1);
			str--;
		}
	}
}
void parameterInit(struct command_parameter *parameter)
{
	parameter->filename=NULL;
	parameter->commandopt=0;
	parameter->period=0;
	parameter->filecnt=0;
	parameter->npath=NULL;
}
void main(int argc, char **argv)
{
	getcwd(exePATH,PATH_MAX);
	if(argc>=3)
	{
		fprintf(stderr,"usage : %s \n",argv[0]);
		exit(1);
	}
	Init(argv[1]);
	
	int (*commandFun)(struct command_parameter * parameter);
	int filecnt=0;
	int cnt=0;
	char buf[255];
	char commandbuf[255];
	char *add="add";
	char **arglist=NULL;
	int period =0;
	int command=0;
	int commandopt=0;
	int option =0;
	int i =0;
	char filename[255];
	char recoverNbuf[255];
	char optargbuf[255];
	struct command_parameter parameter={NULL,"",0,0,0,NULL};
	while(1)
	{
		if(arglist!=NULL)
		
		{
			free(arglist);
			arglist=NULL;
		}
		parameterInit(&parameter);	
		optarg=NULL;
		optind=0;
		option=0;
		commandopt=0;
		command=0;
		printf("20190000>");
		rewind(stdin);
		memset(buf,0,255);
		memset(commandbuf,0,255);
		fgets(buf,BUFSIZ,stdin);
		strcpy(commandbuf,buf);
	
		arglist=GetSubstring(buf," ",&cnt);

		if(!strncmp(buf,commanddata[0],3)&&!strcmp(arglist[0],commanddata[0]))
		{
			command =CMD_A;
			commandFun=AddCommand;
		}
		else if(!strncmp(buf,commanddata[1],6)&&!strcmp(arglist[0],commanddata[1]))
		{
			command =CMD_RM;
			commandFun=RemoveCommand;
		}
		else if(!strncmp(buf,commanddata[4],4)&&!strcmp(arglist[0],commanddata[4]))
		{
			ListView();
			continue;
		}
		else if(!strncmp(buf,commanddata[3],6)&&!strcmp(arglist[0],commanddata[3]))
		{
			command=CMD_R;
			commandFun=RecoverCommand;
		}
		else if(!strncmp(buf,commanddata[5],4)&&!strcmp(arglist[0],commanddata[5]))
		{
			exit(1);
		}
		else if(!strncmp(buf,commanddata[6],6)&&!strcmp(arglist[0],commanddata[6]))
		{
			command= CMD_CM;
			commandFun = CompareCommand;
		}

		else if(!strncmp(buf,commanddata[7],2)&&!strcmp(arglist[0],commanddata[7]))
		{
			chdir(exePATH);
			system(commandbuf);
			continue;
		}

		else if(!strncmp(buf,commanddata[8],2)&&!strcmp(arglist[0],commanddata[8]))
		{
			chdir(exePATH);
			system(commandbuf);
			continue;
		}
		else if(!strncmp(buf,commanddata[9],3)&&!strcmp(arglist[0],commanddata[9]))
		{
			system(commandbuf);
			continue;
		}
		else
		{

			printf("command retry\n");
			command =NOT_CMD;
			continue;
		}

		if(!(cnt<2))		
			strcpy(filename, arglist[1]);

		if(arglist ==NULL || command ==NOT_CMD)
		{
			fprintf(stdout,"there is no argument or no command\n");
			continue;
		}
		int a= optionProcessing(cnt,arglist,command,&parameter,&commandopt);
		if(a ==-1 )
		{
			continue;
		}
		if(!(cnt<2))
			parameter.filename=filename;
		parameter.commandopt=commandopt;
		if(commandFun(&parameter)<1)
		{
			fprintf(stdout,"command error\n");
			continue;
		
		}
	}
		
}
int optionProcessing(int argc, char **arglist,int mode,command_parameter *parameter,int *commandopt)
{
	for(int i =1; i<argc;i++)
	{
		char *opttarg=NULL;
		int option=0;
		if(arglist[i][0]=='-'&&strlen(arglist[i])==2)
		
		{
			option =arglist[i][1];

			if(i+1<argc)
			{
				if(arglist[i+1][0]!='-')
				{
					opttarg=arglist[++i];
				}
			}
		}

		switch(mode)
		{
			case CMD_RM:
				{
					
					if(argc<2&&argc>3)
					{
						perror("usage : remove\n");
						return -1;
					}
					if(option!=0&&!(option=='a'))
					{
						perror("usage : remove option\n");
						return -1;
						
					}
					if(option =='a')
					{	
						*commandopt |=OPT_A;
						break;
					}
					parameter->filename=arglist[1];
				}break;
			case CMD_CM:
				{
					if(argc<3)
					{
						perror("usage : compare \n");
						return -1;
					}

					if(option!=0)
					{
						perror("usage : compare option\n");
						return -1;
						
					}
					parameter->argv[0]=arglist[1];
					parameter->argv[1]=arglist[2];
					
				}break;
			case CMD_R:
				{

					if(option!=0&&!(option=='n'))
					{
						perror("usage : recover option\n");
						return -1;
						
					}
					if (option =='n')
					{
						if(opttarg ==NULL)
						{
							perror("usage : recover -n \n");
							return -1;
						}
					    strcpy(parameter->recoverName,opttarg);
						*commandopt |= OPT_N;
					}

				}break; 
			case CMD_A:
				{
					if(argc<3)
					{
						printf("usage : add \n");
						return -1;
					}

					//testing 1~10 
					int ret=strToint(arglist[2],1,10);
					if(ret ==-1)
					{
						printf("add period error :not integer \n");
						return -1;
					}	
					else if(ret ==-2)
					{    

						printf("add period error : out of range \n");
						return -1;
					}
					parameter->period = ret;

					if(option!=0&&!(option=='m'||option =='n'||option=='d'|| option =='t'))
					{
						printf("usage : add option\n");
						return -1;
						
					}
					switch(option)
					{
						
						case 'd' :
							{
								struct stat statbuf;
								if(stat(arglist[1],&statbuf)<0)
								{
									printf("%s file stat error \n",opttarg);
									return -1;
								}
								if(!S_ISDIR(statbuf.st_mode))
								{
									printf("error :directory only \n");
									return -1;
								}
								
								*commandopt |= OPT_D;
							}break;
						case 'n' : 
							{
								if(opttarg ==NULL)
								{
									printf("add -n not option target \n");
									return -1;
								}
								int choptr=strToint(opttarg,1,100);
								if(choptr ==-1)
								{
									printf("add option n error :not Int \n");
									return -1;
								}
								else if(choptr==-2)
								{
									printf("add option n error :out of range \n");
									return -1;
								}
							
							  *commandopt |= OPT_N;
							  parameter->filecnt=choptr;
							}break;
						case 'm':
							{
								if(opttarg !=NULL)
								{
									printf("add option m error : no target\n");
									return -1;
								}
						
								*commandopt |= OPT_M;
							}break;
						case 't' : 
								 {
									 if(opttarg ==NULL)
									 {
										 printf("add -t not option target\n");
										 return -1;
									 }
									 //testing 1~1200
									 int choptr = strToint(opttarg,1,1200);
									 if(choptr == -1)
									 {
										 printf("add option t error :not Int \n");
										 return -1;
									 }
									 else if(choptr ==-2)
									 {
										 printf("add option t error : out of range \n");
										 return -1;
									 }
									 *commandopt |= OPT_T; 
									 parameter->timer = choptr;
								 }
					}break;
				}
		}
	}
}
int strToint(char *str, int start, int last)
{
	int i =0 ;
	
	int value=0;
	for (i =0; i<strlen(str)-1; i++)
	{
		if(str[i] <'0'||str[i]>'9')
		{
			return -1;
		}
	 
	}
	value=atoi(str);
	if(value>last|| value<start)
		return -2;

}
int CompareCommand(struct command_parameter *parameter)
{
	struct stat statbufFN1, statbufFN2;
	if(access(parameter->argv[0],F_OK)<0)
	{
		perror("<FILENAME1> not Exist\n");
		return -1;
	}
	else if(access(parameter->argv[1],F_OK)<0)
	{

		perror("<FILENAME2> not Exist\n");
		return -1;
	}
	if(stat(parameter->argv[0],&statbufFN1)<0)
	{
		fprintf(stderr,"stat error for %s \n",parameter->argv[0]);
		return -1;
	}
	
	if(stat(parameter->argv[1],&statbufFN2)<0)
	{
		
		fprintf(stderr,"stat error for %s \n",parameter->argv[1]);
		return -1;
	
	}
	if(statbufFN1.st_mtime ==statbufFN2.st_mtime && statbufFN1.st_size==statbufFN2.st_size)
		printf("Files %s and %s are equal \n",parameter->argv[0],parameter->argv[1]);
	else
		printf("FIles %s and %s are not equal \n",parameter->argv[0],parameter->argv[1]);
	
	return 1;
}
int RemoveCommand(struct  command_parameter *parameter)
{

	ssu_backupList *seek =listHEAD->next;
	ssu_backupList *temp=NULL;
	if(parameter->commandopt &OPT_A)
	{
		while(seek!=NULL)
		{
			temp=seek->next;
			RemoveFun(seek,0);
			seek=temp;
		}
		return 1;
	}
	if(access(parameter->filename,F_OK))
	{
		fprintf(stderr,"RemoveFun error :  %s is not exist \n",parameter->npath);
		return -1;
	}
	

	while(seek!=NULL)
	{
		if(!strcmp(seek->filename,parameter->filename))
		{
			printf("del name =%s\n", seek->filename);
			RemoveFun(seek,0);
			break;
		}
		seek=seek->next;
	}
		
	return 1;
}
int  RemoveFun(ssu_backupList *seek, int opt)
{
	ssu_backupList *prev=listHEAD;
	ssu_thread *threadseek;
	while(prev->next!=seek)
		prev=prev->next;
	if(seek ==NULL)
		return 1;
	if(seek->thread_list->next ==NULL)
	{
		pthread_mutex_lock(&mutex);
		writeLog("remove",seek->pathname);
		pthread_mutex_unlock(&mutex);
	
		pthread_cancel(seek->thread_list->tid);
		prev->next=seek->next;
		free(seek);
		
		return 1;
	}
	else
	{
		threadseek = seek->thread_list->next;
		while(threadseek!=NULL)
		{
			pthread_mutex_lock(&mutex);
			writeLog("remove",threadseek->pathname);
			pthread_mutex_unlock(&mutex);		
			pthread_cancel(threadseek->tid);
			free(threadseek);
			threadseek = threadseek->next;
		}
		pthread_cancel(seek->thread_list->tid);
		prev->next = seek->next;
		free(seek);
		return 1;
	}
}
int RecoverCommand(struct command_parameter *parameter)
{
	int wfd,rfd;
	int ret=0,len=0;
	int sellectindex=0;
	char inputdata[255];
	char *recoverName;
	char **datalist;
	char* pathname=NULL;
	char *noptcheck=NULL;
	char recoverPATH[500];
	char *data;
	int cnt=0;
	ssu_thread *threadseek= NULL;
	pathname=parameter->npath;

	struct stat src_st;
	if(parameter->commandopt == OPT_N)
	{
		strcpy(recoverPATH,parameter->recoverName);

		recoverName=parameter->recoverName;
		len =strlen(recoverName);
		
		if(len <1)
		{
			fprintf(stderr,"%s Invalid recover path \n",backupPATH);
			return -1;
		}
		noptcheck=strrchr(recoverName,'/');
		if(noptcheck !=NULL){
		{
			cnt =noptcheck-recoverName+1;
			recoverName[cnt-1]='\0';
		}

			
		if(access(recoverName,F_OK))
			{
				fprintf(stderr,"%s Invaild recoverpath\n",recoverName);
				return -1;
			}

		}

	}
	int findthem=0;
	ssu_backupList *seek =listHEAD->next;
	while(seek!=NULL)
	{

		if(seek->option &OPT_D)
		{
			threadseek= seek->thread_list->next;
			while(threadseek!=NULL)
			{
				if(!strcmp(threadseek->filename,parameter->filename))
				{
					findthem=1;
					break;
				}
				threadseek=threadseek->next;
				
			}
		}
		if((!strcmp(seek->filename,parameter->filename))||findthem==1)
			break;
		
		seek=seek->next;
	}
	if(seek==NULL)
	{
		printf("%s not backup file or directory\n",parameter->filename);
		return-1 ;
	}
	datalist=getBackupList(parameter->filename,&ret);	
	if(OPT_D & seek->option)
	{
		pathname=threadseek->pathname;
	}
	else
		pathname = seek->pathname;
	for(int i=0;i<ret;i++)
	{
		printf("%d :%s	",i,datalist[i]);
		if(i%3==0)
			puts("");
	
	}
	puts("\nrecover file data ==");
	scanf("%s",inputdata);
	
	if((sellectindex=strToint(inputdata,0,255))<0)
	{
		perror("recover error :Not Int or out of range :0~255 \n");
		return -1;
	}
	
	if((sellectindex>ret))
	{
		perror("recover error : sellect number out of range \n");
		return -1;
	}
	char buf[250];
	strTodirPATH(buf,backupPATH,datalist[sellectindex]);
	if((rfd=open(buf,O_RDONLY))<0)
	{
		fprintf(stderr,"recover error : %s file exist",datalist[sellectindex]);
		return -1;
	}
	
	stat(buf,&src_st);
	printf("%s \n",pathname);
	if(parameter->commandopt == OPT_N)
	{
		wfd =open(recoverPATH,O_CREAT | O_TRUNC | O_WRONLY ,666);
		if(wfd <0)
		{
			fprintf(stderr,"file name %s create failure\n",recoverPATH);
			return -1;
		}
	}
	else
	{
		wfd =open(pathname,O_CREAT | O_TRUNC | O_WRONLY ,666);
		if(wfd <0)
		{
			fprintf(stderr,"file name %s create failure\n",pathname);
			return -1;
		}
	}
	printf("size : %ld \n",src_st.st_size);
	data =(char*)malloc(src_st.st_size);
	
	while((len=read(rfd,data,src_st.st_size))>0)
	{
		write(wfd,data,len);
	}
	pthread_mutex_lock(&mutex);
	writeLog("recover",pathname);
	pthread_mutex_unlock(&mutex);
	free(data);
	close(wfd);
	close(rfd);
	return 1;
}
char **GetSubstring(char *string,char *cut,int *cnt)
{
	*cnt =0;
	int i=0;
	char *token=NULL;
	char *templist[100]={NULL,};
	token=strtok(string,cut);
	if(token ==NULL)
	{
		fprintf(stderr,"token error ");
		return NULL;
	}
	while(token != NULL)
	{
		templist[*cnt]=token;
		*cnt=*cnt +1;
		
		token=strtok(NULL," ");
	}
	
	char **temp=(char **)malloc(sizeof(char *)*(*cnt+1));
	for(i=0;i<*cnt;i++)
	{
		Eliminate(templist[i],'\n');
		temp[i]=templist[i];
	}
	return temp;
}


int AddCommand(struct command_parameter *parameter)
{

	ssu_backupList *listadd =(ssu_backupList *)malloc(sizeof(ssu_backupList));
	strcpy(listadd->filename,parameter->filename);
	listadd->period=parameter->period;
	listadd->option=parameter->commandopt;
	listadd->filecnt=parameter->filecnt;
	listadd->mtime_old=0;
	listadd->next=NULL;
	if(listadd->option &OPT_T)
		listadd->timer=parameter->timer;
	chdir(exePATH);
	realpath(parameter->filename,listadd->pathname);
	if(access(listadd->pathname,F_OK)<0)
	{
		fprintf(stderr,"%s in not exist  AddCommand\n",listadd->pathname);
		free(listadd);
		return -1;
	}
	ssu_backupList *seek =listHEAD;
	while(seek->next!=NULL)
		seek=seek->next;
	seek->next=listadd;
	listadd->thread_list = create_thread(listadd);
	pthread_mutex_lock(&mutex);
	writeLog("add",listadd->pathname);
	pthread_mutex_unlock(&mutex);
	return 1;
}
void Init(char *readdir)
{
	int len;
	int cnt=0;
	char *backupdirname;
	listHEAD=(ssu_backupList*)malloc(sizeof(ssu_backupList));
	char *backupdir;
	char buf[255];
	char *temp={"backup"};
	if(readdir ==NULL)	readdir=temp;
	len =strlen(readdir);
	strcpy(backupPATH,readdir);
	if(len <1)
	{
		fprintf(stderr,"%s Invalid backupPATH \n",backupPATH);
		exit(1);
		
	}
	backupdir=strrchr(readdir,'/');
	if(backupdir !=NULL){
		cnt =backupdir-readdir+1;
		backupdirname=(char *)malloc(sizeof(char)*strlen(backupdir));
		strcpy(backupdirname,++backupdir);
		if(access(readdir,F_OK))
		{
			fprintf(stderr,"%s Invaild backupPATH\n",readdir);
			exit(1);
		}

	}
	if(access(backupPATH,F_OK))
		mkdir(backupPATH,0777);
	printf("backup dir = %s thread Initialize\n",backupPATH);
	realpath(backupPATH,buf);
	strcpy(backupPATH,buf);
	
}

char *strTodate(char *old_str){
	char date[14] = {0};
	time_t timer;
	struct tm *t;

	timer = time(NULL);	
	t = localtime(&timer);

	char *result = (char *)calloc(sizeof(char), PATH_MAX);
	
	strcpy(result,old_str);
	sprintf(date, "_%02d%02d%02d%02d%02d%02d",t->tm_year %100, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
	strcat(result, date);
	return result;
}
ssu_thread *create_thread(struct backupList *bklist){
	ssu_thread *temp = (ssu_thread *)malloc(sizeof(ssu_thread));
	temp->next = NULL;
	temp->own=bklist;
	if(pthread_create(&(temp->tid), NULL, (void *)(&thread_function), (void *)temp) != 0){
		perror("pthread not create");
	}
	return temp;
}
void thread_function(void* arg)
{
	ssu_thread *recvdata=(ssu_thread *)arg;
	int period = recvdata->own->period;
	int filecnt= recvdata->own->filecnt;	
	int option = recvdata->own->option;
	char *filename = recvdata->own->filename;
	char *pathname = recvdata->own->pathname;
	int ret=0;
	char ** datalist;
	recvdata->tid=pthread_self();
	recvdata->next =NULL;
	while(1)
	{
		sleep(period);
		if(option & OPT_N)
		{
				
			datalist=getBackupList(filename,&ret);	
			for(int i=0;i<ret -filecnt;i++)
			{
				char buf[255];
				
				strTodirPATH(buf,backupPATH,datalist[i]);
				unlink(buf);
			}
			

		}
	
		if(option & OPT_D)
		{
			dirreculsion(recvdata,pathname);
		
		}
		else
			file_backup(recvdata);

						

	}
}

char **getBackupList(char *name, int *ret){ 
	char *real = strTodate(name);
	int length = strlen(real);
	struct dirent **namelist;
	char buf[255];
	int cnt = scandir(backupPATH, &namelist, NULL, alphasort);
	char **list = (char **)malloc(cnt * sizeof(char *));
	int many = 0;
	char ptr[255];
	char *pptr;
	int len=0;
	for (int i = 0; i < cnt; i++)
	{
		if ((!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, "..")))
			continue;
		if(strlen(namelist[i]->d_name)!= length)
			continue;
		int namelen =strlen(namelist[i]->d_name);	
		for(int j = 0 ; j< namelen;j++)
		{
			if(namelist[i]->d_name[j] =='_')
			{
				if (!strncmp(namelist[i]->d_name,name,j))
				{
					list[many] = (char *)malloc(sizeof(char) * strlen(namelist[i]->d_name));
					strcpy(list[many],namelist[i]->d_name);
					many++;
					break;

				}
			}
		}

	}

	for (int i = 0; i < cnt; i++)
		free(namelist[i]); 
	free(namelist);
	*ret = many; 
	return list; 
}

void dirreculsion(ssu_thread* backupthread,char *pathname)
{
	int fd = open(pathname, O_RDONLY);
	ssu_thread *seek=backupthread,* temp;
	struct stat stbuf;
	if(stat(pathname,&stbuf)<0)
	{
		printf("error : dirstat\n");
		pthread_exit(0);
	}
	if(S_ISCHR(stbuf.st_mode))
	{
		return;
	}

	if(S_ISBLK(stbuf.st_mode))
	{
		return;
	}

	if(S_ISFIFO(stbuf.st_mode))
	{
		return;
	}

	if(S_ISSOCK(stbuf.st_mode))
	{
		return;
	}
	if(S_ISDIR(stbuf.st_mode))
	{
		
		DIR *dir_info;
		struct dirent *dir_entry;
		char root[1024];
		dir_info = opendir(pathname);
		if(dir_info != NULL)
		{
			while(dir_entry=readdir(dir_info))
			{
				if(!strcmp(dir_entry->d_name,".")||! strcmp(dir_entry->d_name,".."))
					continue;

				memset(root,0,sizeof(root));
				strcpy(root,pathname);
				strcat(root,"/");
				strcat(root,dir_entry->d_name);
				temp = (ssu_thread*)malloc(sizeof(ssu_thread));
				temp->pathname=(char *)malloc(sizeof(strlen(root)));
				temp->filename =(char *)malloc(sizeof(strlen(dir_entry->d_name)));
				strcpy(temp->filename,dir_entry->d_name);
				strcpy(temp->pathname,root);
				temp->next=NULL;
				temp->own=backupthread->own;
				backupthread->next=temp;
				backupthread=temp;
	
				if(backupthread->own->option & OPT_N)
				{
					int ret =0;
					char **datalist=NULL;
					int filecnt= backupthread->own->filecnt;	
					datalist=getBackupList(temp->filename,&ret);	
					for(int i=0;i<ret -filecnt;i++)
					{
						char buf[255];
				

						strTodirPATH(buf,backupPATH,datalist[i]);
						unlink(buf);
					}
			
	
				}

				file_backup(backupthread);
				dirreculsion(backupthread,root);
			}
			closedir(dir_info);
			return;

		}
	}
}

char **getTimeBackupList(char *name, int *ret,int timer){ 
	char *real = strTodate(name);
	int length = strlen(real);
	struct dirent **namelist;
	char buf[255];
	int cnt = scandir(backupPATH, &namelist, NULL, alphasort);
	char **list = (char **)malloc(cnt * sizeof(char *));
	int many = 0;
	char ptr[255];
	char *pptr;
	int len=0;
	struct tm strparsing={0},strnowparsing={0}, *t;
	time_t now;
	long long int nowtime,filetime;
	now =time(NULL);
	t=localtime(&now);
	char nowtimestamp[255];
	sprintf(nowtimestamp, "%02d%02d%02d%02d%02d%02d",t->tm_year %100, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
	for (int i = 0; i < cnt; i++)
	{
		if ((!strcmp(namelist[i]->d_name, ".") || !strcmp(namelist[i]->d_name, "..")))
			continue;
		if(strlen(namelist[i]->d_name)!= length)
			continue;
		int namelen =strlen(namelist[i]->d_name);	
		for(int j = 0 ; j< namelen;j++)
		{
			if(namelist[i]->d_name[j] =='_')
			{
				if (!strncmp(namelist[i]->d_name,name,j))
				{
					
					
					int timercnt=0;
					char timestamp[255];
					char datelist[6][3];
					char nowlist[6][3];
					strcpy(timestamp,(namelist[i]->d_name)+j+1);
					timercnt=strlen(timestamp);
					int iter =0,datelevel=0;
					int nowyear=0, fileyear=0;
					while(datelevel<6)
					{

						datelist[datelevel][0]=timestamp[iter];
						nowlist[datelevel][0]=nowtimestamp[iter++];						
						datelist[datelevel][1]=timestamp[iter];
						nowlist[datelevel][1]=nowtimestamp[iter++];
						datelist[datelevel][2]='\0';
						nowlist[datelevel][2]='\0';
						datelevel++;	
					}
					nowyear =atoi(nowlist[0]);
					fileyear=atoi(datelist[0]);
					strparsing.tm_mon=atoi(datelist[1]);
					strparsing.tm_mday=atoi(datelist[2]);
					strparsing.tm_hour=atoi(datelist[3]);
					strparsing.tm_min=atoi(datelist[4]);
					strparsing.tm_sec=atoi(datelist[5]);


					strnowparsing.tm_mon=atoi(nowlist[1]);
					strnowparsing.tm_mday=atoi(nowlist[2]);
					strnowparsing.tm_hour=atoi(nowlist[3]);
					strnowparsing.tm_min=atoi(nowlist[4]);
					strnowparsing.tm_sec=atoi(nowlist[5]);
					nowtime=0;
					filetime=0;
					nowtime +=(2592000*12) * nowyear;
					nowtime +=2592000 *strnowparsing.tm_mon;
					nowtime +=86400 *strnowparsing.tm_mday;
					nowtime +=3600 *strnowparsing.tm_hour;
					nowtime +=60 *strnowparsing.tm_min;
					nowtime +=strnowparsing.tm_sec;
					filetime +=(2592000 *12) *fileyear;
					filetime +=2592000 *strparsing.tm_mon;
					filetime +=86400 *strparsing.tm_mday;
					filetime +=3600 *strparsing.tm_hour;
					filetime +=60 *strparsing.tm_min;
					filetime +=strparsing.tm_sec+timer;
					if(nowtime>filetime)
					{
						
						list[many] = (char *)malloc(sizeof(char) * strlen(namelist[i]->d_name));
						strcpy(list[many],(namelist[i]->d_name));
						many++;
						break;
					}

				}
			}
		}

	}

	for (int i = 0; i < cnt; i++)
		free(namelist[i]); 
	free(namelist);
	*ret = many; // return number of list by pointer
	return list; // return list
}

void file_backup(ssu_thread* backupthread) 
{


	int period = backupthread->own->period;
	int filecnt= backupthread->own->filecnt;	
	int option = backupthread->own->option;
	char *filename;
	char *pathname;
	if(!(OPT_D &backupthread->own->option))
	{
		filename = backupthread->own->filename;
		pathname = backupthread->own->pathname;
	}
	else
	{
		pathname=backupthread->pathname;
		filename =backupthread->filename;
	}
	int len,fd,nfd;
	char *data;
	struct stat src_st;
	//dentry
	struct dirent *dentry;
	struct stat statbuf;
	int mnew =0,mold=backupthread->own->mtime_old;
	DIR *dirp;
	char buf[PATH_MAX];

	/// add option -T
	if(backupthread->own->option & OPT_T)
	{
		int tcnt=0;
		char **gettime =getTimeBackupList(filename,&tcnt,backupthread->own->timer);
		
		for(int i = 0 ; i< tcnt ; i++)
		{

			char buf[255];
			strTodirPATH(buf,backupPATH,gettime[i]);
			unlink(buf);
		}
	}
	if((fd=open(pathname,O_RDONLY))<0)
	{
		fprintf(stderr,"%s is not exit \n",pathname);	
		pthread_exit(0);
		return;
	}
	stat(pathname,&src_st);
	if(S_ISDIR(src_st.st_mode))
	{
		close(fd);
		return; 
	}
	mnew = src_st.st_mtime;
	if(option &OPT_M&& mnew ==mold)
		return;
	else
		backupthread->own->mtime_old=mnew;
	char local[255];
	char *backuptime=strTodate(filename);
    char *pstr= strTodirPATH(local,backupPATH,backuptime);
	nfd =open(pstr,O_CREAT | O_TRUNC | O_WRONLY ,666);
	if(nfd <0)
	{
		printf("strtohex error %s \n",pstr);
		pthread_exit(0);
		return;
	}
	data =(char*)malloc(src_st.st_size);
	while((len=read(fd,data,src_st.st_size))>0)
	{
		write(nfd,data,len);
	}
	pthread_mutex_lock(&mutex);
	writeLog("generate",pstr);
	pthread_mutex_unlock(&mutex);
	close(fd);
}
char *strTodirPATH(char* local,char* path,char *addpathname)
{
	strcpy(local,path);
	strcat(local,"/");
	if(addpathname !=NULL)
		strcat(local,addpathname);
	return local;
}
void ListView()
{
	ssu_backupList *seek =listHEAD->next;
	while(seek!=NULL)
	{
		printf("file path = %s PERIOD= %d OPTION = %d\n" ,seek->pathname,seek->period,seek->option);
		if(seek->option &OPT_D)
		{
			ssu_thread *threadseek= seek->thread_list->next;
			while(threadseek!=NULL)
			{
				printf("\t->filepath = %s filename =%s\n",threadseek->pathname,threadseek->filename);
				threadseek=threadseek->next;
				
			}
		}

		seek=seek->next;
	}
}