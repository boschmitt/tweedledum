#include "timing.h"

/*************************************************
  These routines can be used for approximate CPU
  timing on a WIN system. DMM Sept 2002
************************************************/

long
cpuTime()
/* returns elapsed user CPU time */
{
  return(clock());
}

void
printCPUtime(clock_t x)
/* display a time value in sec. */
{
  printf("%7.2f",(float)x/CLOCKS_PER_SEC);
}

void
printCPUtime(clock_t x, std::ostream &os)
/* display a time value in sec. */
{
  char outputbuffer[30];
  sprintf(outputbuffer, "%7.2f",(float)x/CLOCKS_PER_SEC);
  os << outputbuffer;
}

void 
dateToday(char *str)
{
  time_t tt;
  struct tm *tod;
  time(&tt);
  tod=localtime(&tt);
  sprintf(str,"%2.2d/%2.2d/%d",tod->tm_mday,tod->tm_mon +1,tod->tm_year+1900);
}

void 
wallTime(char *str)
{
    time_t tt;
  struct tm *tod;
  time(&tt);
  tod=localtime(&tt);
  sprintf(str,"%2.2d:%2.2d:%2.2d",tod->tm_hour,tod->tm_min,tod->tm_sec);

}
