#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/fs.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>

//Buff size must be multiple of 4
#define BUFF_SIZE 1024*1024*5

static void die (const char * format, ...) {
    va_list vargs;
    va_start (vargs, format);
    vfprintf (stderr, format, vargs);
    if (errno != 0) //Is there no constant for ERROR SUCCESS?
        fprintf (stderr, ": %s",strerror(errno));
    
    fprintf (stderr, "\n");
    exit(1);
}


int main(int argc, char **argv)
{
  time_t t,start,end;
  int fd,i;
  long long length;
  char buffer[BUFF_SIZE];
  unsigned int seed = (unsigned) time(&t);
  char *device;
  int die_at_error = 1;
  if (argc == 3 && !strcmp(argv[1],"--print-range")) {
    die_at_error = 0;
    device =  argv[2];
  } else if (argc == 2) {
    device = argv[1];
  } else {
    die("Usage: ./%s [--print-range] device",argv[0]);
  }
  
  
  
  fd=open(device,O_WRONLY);
  if (fd < 0) die("Failed to open device!");
  if(ioctl(fd,BLKGETSIZE64,&length)==-1) die("Failed to get device size");
  printf("Device size: %lld\n",length);
  
  //length=1024*1024*26;
  
  srand(seed); 
  long long written = 0;
  time(&start);
  long long percent = length/100;
  while (written < length) {
      if (written > percent) {
        printf(".");
        fflush(stdout);
        percent = percent+length/100;
      }
      //printf("written: %lld\n",written);
      for (i = 0; i < BUFF_SIZE; i+=4) {
        *((unsigned int*) &buffer[i]) = rand();
      }
      int l = write(fd,buffer,BUFF_SIZE);
      written += l;
      if (l < 0) die("\nFailed to write to device");
      if (l != BUFF_SIZE && written != length) die("\nFailed to write full int");
  }
  
  printf("\nManaged to write all data!\n");
  close(fd);
  sync();
  time(&end);
  double seconds = difftime(end,start);
  printf("Wrote %lld bytes in %d seconds (%.2f MB/s)\n",written,(int)seconds,((double)written/(1024.0*1024.0))/(seconds));
  
  
  fd = open(device,O_RDONLY);
  if (fd < 0) die("Failed to open device");
  srand(seed);
  time(&start);
  long long readbytes = 0;
  long long failstart = -1;
  long long failend = -1;
  percent = length/100;
  while (readbytes < length) {
        if (readbytes > percent) {
            printf(failstart==-1?".":",");
            fflush(stdout);
            percent = percent+length/100;
          }
          
        //printf("read: %lld\n",readbytes);
        int l = read(fd,buffer,BUFF_SIZE);
        readbytes += l;
        if (l < 0) die("\nFailed to read from device");
        if (l != BUFF_SIZE && readbytes != length) die("\nFailed to read full int");
        for (i = 0; i < BUFF_SIZE; i+=4) {
            if (*((unsigned int*) &buffer[i]) != rand()) {
              if (failstart == -1) failstart = readbytes-l+i;
              if (die_at_error) die("\nFailed to validate byte %lld",readbytes-l+i);
            } else if (failstart != -1) {
              failend = readbytes-l+i;
              printf("\nFail at range: %lld-%lld\n",failstart,failend);
              failstart = -1;
            }
        }
        
  }
  printf("\nManaged to read all data correctly\n");
  close(fd);
  time(&end);
  seconds = difftime(end,start);
  printf("Read %lld bytes in %d seconds (%.2f MB/s)\n",readbytes,(int)seconds,((double)readbytes/(1024.0*1024.0))/(seconds));
  return 0;

}
