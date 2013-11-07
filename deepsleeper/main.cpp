/*===========================================
DeepSleeper
============================================*/
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <asm/ioctl.h>
//#include <linux/rtc.h>
#include <linux/android_alarm.h>

int main(int argc, char **argv) {
  int res;
  int afd;

  int val;
  
  afd = open("/dev/alarm", O_RDWR);
  if(afd < 0) {
      fprintf(stderr, "Unable to open rtc: %s\n", strerror(errno));
      return 1;
  }
  
  if(argc < 2) {
      fprintf(stderr, "Usage: deepsleeper <int value>\n");
      return 1;
  }
  
  val = atoi ( argv[1] );
  printf("Calling ioctl with value %d.\n", val);
  res = ioctl(afd, ANDROID_ALARM_IGNORE_ON_SUSPEND, &val);
  if(res < 0) {
      fprintf(stderr, "Unable to set deep sleep alarm: %s\n", strerror(errno));
		  return 1;
  }
  exit(0); // Use exit() to exit a program, do not use 'return' from main()
}
