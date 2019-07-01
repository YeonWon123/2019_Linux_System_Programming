#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<string.h>
#include <dirent.h>
#include <time.h>
#include <wait.h>
#include <pthread.h>
#include <sys/stat.h>
#include<fcntl.h>

#define CMD_A 01000000
#define CMD_R 00100000
#define CMD_C 00010000
#define CMD_L 00001000
#define CMD_E 00000100
#define CMD_RM 00000010
#define CMD_CM 00000001
#define NOT_CMD 00000000
#define OPT_D 00000001
#define OPT_N 00000010
#define OPT_DSB 00001000
#define OPT_M 01000000
#define OPT_T 00000100
#define OPT_A 0010000
char backupPATH[PATH_MAX];
char exePATH[PATH_MAX];
char *commanddata[10]={"add","remove","compare","recover","list","exit","compare","ls","vi","vim"};
typedef struct thread_struct
{
	struct thread_struct *next;
	pthread_t tid;
	char *filename;
	char *pathname;
	struct backupList* own;
}ssu_thread;
typedef struct backupList
{
	struct backupList* next;
	char filename[PATH_MAX];
	char pathname[500];
	char recoverName[500];
	int period;
	int option;
	int filecnt;
	int mtime_old;
	int timer;
	ssu_thread* thread_list;
	
}ssu_backupList;
typedef struct command_parameter
{
	char *filename;
	char recoverName[500];
	int commandopt;
	int period;
	int filecnt;
	char *npath;
	int mtime_old;
	int timer;
	char *argv[10];
}command_parameter;
command_parameter* getOption(char **options, int cnt,command_parameter *comm);
char **GetSubstring(char *string,char *cut,int *cnt);
int list_checiking();
void Init();
void *Backup_Thread();
void parameterInit(struct command_parameter * parameter);
void thread_function(void* arg);
int AddCommand(struct command_parameter * parameter);
int RecoverCommand(struct command_parameter *parameter);
int RemoveCommand(struct command_parameter *parameter);
int CompareCommand(struct command_parameter *parameter);
void AddList(char *data,int period);
ssu_thread *create_thread(struct backupList *bklist);
void file_backup(ssu_thread *backupthread);
void dir_backup(ssu_thread *threadunit);
char **getBackupList(char *name, int *ret);
void ListView();
char *hexTodate(char *hex_str);
int strToint(char *str,int start, int last);
int optionProcessing(int argc,char **arglist,int mode,command_parameter *parameter,int* commandopt);
int RemoveFun(ssu_backupList *seek,int opt);
char *strTodirPATH(char* local,char *path,char *addpathname);
void dirreculsion(ssu_thread* backupthread ,char *pathname);
char **getTimeBackupList(char *name, int *ret,int timer);
