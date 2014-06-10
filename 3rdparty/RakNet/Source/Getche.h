

#if   defined(_WIN32)
#include <conio.h> /* getche() */

#else
#include <termios.h>
#include <stdio.h>
#include <unistd.h>
char getche();
#endif 
