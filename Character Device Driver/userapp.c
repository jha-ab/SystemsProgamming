#include <linux/ioctl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#define DEVICE "/dev/mycdrv"

#define CDRV_IOC_MAGIC 'Z'
#define ASP_CLEAR_BUF _IOW(CDRV_IOC_MAGIC, 1, int)


int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Device number not specified\n");
		return 1;
	}
	int dev_no = atoi(argv[1]);
	char dev_path[20];
	int i,fd;
	char ch, write_buf[100], read_buf[10];
	int offset, origin;
	sprintf(dev_path, "%s%d", DEVICE, dev_no);
	fd = open(dev_path, O_RDWR);
	if(fd == -1) {
		printf("File %s either does not exist or has been locked by another "
				"process\n", DEVICE);
		exit(-1);
	}
	int com=1;
	while(com){
		if(com!=2){
	printf(" r = read from device after seeking to desired offset\n"
			" w = write to device \n");
	printf(" c = Clear buffer\n");
	printf("\n\n enter command :");
		}
	scanf("%c", &ch);
	switch(ch) {
	case 'w':
		printf("Enter Data to write: ");
		scanf(" %[^\n]", write_buf);
		for(i=0;i<100;i++)
		{
			if(write_buf[i]=='\0')
				break;
		}
		write(fd, write_buf, i);
		if(com==2)
			com--;
		for(i=0;i<100;i++)
		{
			write_buf[i]=' ';
		}
		break;

	case 'c':
		printf("\n Will clear data \n");
		int rc = ioctl(fd, ASP_CLEAR_BUF, 0);
		if (rc == -1) { 
			perror("\n***error in ioctl***\n");
			return -1;
		}
		printf("\n Data cleared \n");
		if(com==2)
			com--;
		break;

	case 'r':
		printf("Origin \n 0 = beginning \n 1 = current \n 2 = end \n\n");
		printf(" enter origin :");
		scanf("%d", &origin);
		printf(" \n enter offset :");
		scanf("%d", &offset);
		lseek(fd, offset, origin);
		int n = read(fd, read_buf, sizeof(read_buf));
		if ( n> 0) {
			*(read_buf+10)='\0';
			printf("\ndevice: %s \n", read_buf );

		} else {
			fprintf(stderr, "Reading failed\n");
		}
		if(com==2)
			com--;
		for(i=0;i<10;i++)
		{
			read_buf[i]=' ';
		}
		break;

	default:
		if(com==1)
			printf("Enter e to exit");
		else if(ch=='e')
		{

		}
		else
			printf("Command not recognized\n");
	    com++;
		if(com==3)
			com=0;
		break;
	}
	}
	close(fd);
	return 0;
}
