/******************************************************************************
ps -f : process view in full format
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <dirent.h>
#include <libgen.h>
#include <pwd.h>
#include <unistd.h>
#include <fcntl.h>
 
int check_if_number (char *str)
{
  int i;
  for (i=0; str[i] != '\0'; i++)
  {
    if (!isdigit (str[i]))
    {
      return 0;
    }
  }
  return 1;
}
 
#define MAX_BUF 1024
#define INT_SIZE_BUF 6
#define PID_LIST_BLOCK 32
#define UP_TIME_SIZE 10

const char *getUserName(int uid)
{
  struct passwd *pw = getpwuid(uid);
  if (pw)
  {
    return pw->pw_name;
  }

  return "";
}
void pidaux ()
{
  DIR *dirp;
  FILE *fp;
  struct dirent *entry;
  char path[MAX_BUF], read_buf[MAX_BUF],temp_buf[MAX_BUF];
  char uid_int_str[INT_SIZE_BUF]={0},*line;
  
  char uptime_str[UP_TIME_SIZE];
  char *user,*command;
  size_t len=0;
  dirp = opendir ("/proc/");
  if (dirp == NULL)
  {
         perror ("Fail");
         exit(0);
  } 
  strcpy(path,"/proc/");
  strcat(path,"uptime");
  
  fp=fopen(path,"r");
 
  if(fp!=NULL)
  {
	getline(&line,&len,fp);
	sscanf(line,"%s ",uptime_str);
  }
  
  long uptime=atof(uptime_str);
  long Hertz=sysconf(_SC_CLK_TCK); 
 
  while ((entry = readdir (dirp)) != NULL)
  {
    if (check_if_number (entry->d_name))
    {
	strcpy(path,"/proc/");
	strcat(path,entry->d_name);
	strcat(path,"/status");
	
	fp=fopen(path,"r");
	unsigned long long vmsize;

	if(fp!=NULL)
	{
		vmsize=0;
		getline(&line,&len,fp);
		getline(&line,&len,fp);
		getline(&line,&len,fp);
		getline(&line,&len,fp);
		getline(&line,&len,fp);
		getline(&line,&len,fp);
		getline(&line,&len,fp);
		getline(&line,&len,fp);
		sscanf(line,"Uid:    %s ",uid_int_str);
	}
	else
	{
		fprintf(stdout,"FP is NULL\n");
	}
	
	strcpy(path,"/proc/");
	strcat(path,entry->d_name);
	strcat(path,"/stat");
	fp=fopen(path,"r");
	getline(&line,&len,fp);
	char comm[10],state;
	unsigned int flags;
	int pid,ppid,pgrp,session,tty_nr,tpgid;
	unsigned long minflt,cminflt,majflt,cmajflt,utime,stime;
	unsigned long long starttime, starttime1;
	long cutime,cstime,priority,nice,num_threads,itreavalue;
	
//PPID
	sscanf(line,"%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld  %ld %llu",&pid,comm,&state,&ppid,&pgrp,&session,&tty_nr,&tpgid,&flags,&minflt,&cminflt,&majflt,&cmajflt,&utime,&stime,&cutime,&cstime,&priority,&nice,&num_threads,&itreavalue,&starttime);
	
	unsigned long total_time=utime+stime;
//CPU Time computation
	total_time=total_time+(unsigned long)cutime+(unsigned long)cstime;
	float seconds=uptime-(starttime/Hertz);
// Start time of the system
	starttime1=starttime/Hertz;
// Percentage of CPU_Usage
	float cpu_usage=100*((total_time/Hertz)/seconds);
	if(isnan(cpu_usage))
	{
		cpu_usage=0.0;
	}
	strcpy (path, "/proc/");
	strcat (path, entry->d_name);
	strcat (path, "/comm");
// User ID	 
    
    fp = fopen (path, "r");
      if (fp != NULL)
      	{
        	fscanf (fp, "%s", read_buf);
		fclose(fp);
      	}
      	char *userName=getUserName(atoi(uid_int_str));
     	if(strlen(userName)<9)
      	{
		user=userName;	
      	}
      	else
      	{
		user=uid_int_str;
      	}
//Terminal location computation      	
   char *tty;
   int fd;
   char path1[MAX_BUF];
  sprintf(path1, "/proc/%s/fd/0", entry->d_name);
  fd = open(path1, O_RDONLY);
  tty = ttyname(fd);
      	fprintf(stdout,"%s %s %d %0.1f %02llu:%02llu %s %02lu:%02lu:%02lu %s\n",user,entry->d_name,ppid,cpu_usage,(starttime1/3600)%3600,(starttime1/60)%60,tty,(total_time / 3600) % 3600, (total_time/ 60) % 60, total_time % 60,read_buf);
     	 
    }
  } 
  closedir (dirp);
}
 
int main (int argc, char *argv[])
{
 printf("Process view in full format\n");
 printf("UID PID PPID CPU STIME TTY TIME CMD\n");
  pidaux();	
  return 0;
}







