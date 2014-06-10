

#if   defined(_WIN32)
#include <conio.h> /* getche() */
#elif  defined(__S3E__)

#else

#include "Getche.h"

char getche()
{


  struct termios oldt,
                 newt;
  char            ch;
  tcgetattr( STDIN_FILENO, &oldt );
  newt = oldt;
  newt.c_lflag &= ~( ICANON | ECHO );
  tcsetattr( STDIN_FILENO, TCSANOW, &newt );
  ch = getchar();
  tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
  return ch;

} 
#endif
