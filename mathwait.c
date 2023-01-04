#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <wait.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#define LOG_PRINT(...) log_print(__FILE__, __LINE__, __VA_ARGS__ )
void log_print(char* filename, int line, char *fmt,...);
char* print_time();
FILE *fp ;
static int SESSION_TRACKER; //Keeps track of session
int main(int args, char *argv[])
{
	LOG_PRINT("Program initialized");
	key_t key;
	int found = 0, i, j;
	int status;
	int pid;
	// ftok to generate unique key
	LOG_PRINT("Generating Unique Key");
	key = ftok("mathwait",65);
	LOG_PRINT("Key generated successfully");
	// shmget returns an identifier in shmid
	int shmid = shmget(key,1024,0666|IPC_CREAT);
	
	// shmat to attach to shared memory
	LOG_PRINT("CREATING SHARED MEMORY");
	int *num = (int*) shmat(shmid,(void*)0,0);
	num[0]=-2;
	num[1]=-2;
	LOG_PRINT("Creating Parent and Child processes");
	pid=fork();
	if(pid<0)
	{
		printf("\n Error ");
		LOG_PRINT("Error when creating parent and child processes");
		exit(1);
	}
	else if(pid==0)
	{
		// Assigning args to numbers array
		LOG_PRINT("CHILD PROCESS INITIALIZED");
		int numbers[args], num_arg;
		for (i = 1; i < args; i++){
			// If argv = -h
			if(strcmp(argv[i],"-h")==0){
				exit(1);
			}else{
				//sscanf(num_arg, "%d", &argv[i]);
				num_arg = atoi(argv[i]);
				//num_arg = (int)(argv[i])+0;
				numbers[i] = num_arg;
			}
		}
		// Assigning values to the shared memory
		LOG_PRINT("ASSIGNING VALUES TO SHARED MEMORY");
		for (i = 1; i < args; i++){
			for (j = i+1; j < args; j++){
			LOG_PRINT("Values %d %d checked",numbers[i],numbers[j]);
				if ((numbers[i]+numbers[j])==19){
					num[0]=numbers[i];
					num[1]=numbers[j];
					LOG_PRINT("Target values %d %d found",numbers[i],numbers[j]);
					found=1;
					break;
				}
			}
			if (found == 1){
				break;
			}
		}
		if (found != 1){
			LOG_PRINT("Child process found no value");
			num[0]=-1;
			num[1]=-1;
		}
		LOG_PRINT("CHILD pid is %d ",getpid());
	}
	else
	{
		LOG_PRINT("Waiting for child process to finish execution");
		wait(&status);
		LOG_PRINT("Printing out output");
		if(num[0] == -1 && num[1] == -1){
			printf("No pair found");
		}
		else if(num[0] == -2 && num[1] == -2){
			perror("num");
			exit(1);
		}else{
			printf("Pair found by child: %d %d\n",num[0],num[1]);
		}
		LOG_PRINT("Parent pid is %d ",getpid());
	}
	LOG_PRINT("Detartching the shared memory");
	shmdt(num);
	return 0;
}

char* print_time()
{
    int size = 0;
    time_t t;
    char *buf;
    
    t=time(NULL); /* get current calendar time */
    
    char *timestr = asctime( localtime(&t) );
    timestr[strlen(timestr) - 1] = 0;  //Getting rid of \n
    
    size = strlen(timestr)+ 1 + 2; //Additional +2 for square braces
    buf = (char*)malloc(size);
    
    memset(buf, 0x0, size);
    snprintf(buf,size,"[%s]", timestr);
   
    return buf;
}
void log_print(char* filename, int line, char *fmt,...)
{
    va_list         list;
    char            *p, *r;
    int             e;

    if(SESSION_TRACKER > 0)
      fp = fopen ("log.txt","a+");
    else
      fp = fopen ("log.txt","w");
    
    fprintf(fp,"%s ",print_time());
    fprintf(fp,"[%s][line: %d] ",filename,line);
    va_start( list, fmt );

    for ( p = fmt ; *p ; ++p )
    {
        if ( *p != '%' )//If simple string
        {
            fputc( *p,fp );
        }
        else
        {
            switch ( *++p )
            {
                /* string */
            case 's':
            {
                r = va_arg( list, char * );

                fprintf(fp,"%s", r);
                continue;
            }

            /* integer */
            case 'd':
            {
                e = va_arg( list, int );

                fprintf(fp,"%d", e);
                continue;
            }

            default:
                fputc( *p, fp );
            }
        }
    }
    va_end( list );
    fputc( '\n', fp );
    SESSION_TRACKER++;
    fclose(fp);
}
