#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int reader(int fd,char *file){
	int val;
	if(read(fd,&val,sizeof(int))<0){
		fprintf(stderr,"read failed!: %s\n",file);
		exit(1);
	}

	printf("acme: %d\n",val);

	return 0;
}

int writer(int fd,int val,char *file){
	if(write(fd,&val,sizeof(int))<0){
		fprintf(stderr,"write failure!: %s\n",file);
		exit(1);
	}
		fprintf(stderr,"write success!: %s\n",file);
	return 0;
}

int main(int argc, char *argv[]){
	char *file="/dev/acme";
	int val,fd=open(file,O_RDWR);
	
	if(fd<0){
		fprintf(stderr,"open failed: %s\n",file);
		exit(1);
	}

	switch(argc){
		case 1:
			reader(fd,file);
			break;
		case 2:
			val=atoi(argv[argc-1]);
//			fprintf(stderr,"%i\n",val);
			writer(fd,val,file);
			break;
		default:
			fprintf(stderr,"Error, cowardly refusing to proceed!\n");
			exit(1);
			break;
	}

	close(fd);
	exit(0);
}

