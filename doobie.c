#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>





struct termios og_term;

void enableRawMode(void){
    struct termios raw = og_term;
    tcgetattr(STDIN_FILENO, &raw);
    raw.c_iflag &= ~(ICRNL|IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    
    tcsetattr(STDIN_FILENO,TCSAFLUSH, &raw);
} 


void disableRawMode(void){
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &og_term);
}



int main(void){
	char c;
  while (read(STDIN_FILENO, &c, 1)==1 && c !='q'){
    if (iscntrl(c)) {
      printf("%d\n", c);
    } else {
      printf("%d ('%c')\n", c, c);
    }
  }
  return 0;
}
