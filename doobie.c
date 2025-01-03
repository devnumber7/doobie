
/*** includes ***/
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <errno.h>


/*** defination ***/

#define CTRL_KEY(k) ((k) & 0x1f)


/*** data ***/
struct editorConfig{
	int screenrows;
	int screencolumns;
	struct termios og_term;
};

struct editorConfig E;



/*** terminal ***/
void die(const char *s){
	perror(s);
	exit(1);
}

void disableRawMode(void){
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.og_term) == -1){
	
		die("tcsetattr");
	}
}

void enableRawMode(void){
	
	if (tcgetattr(STDIN_FILENO, &E.og_term) == -1) die("tcgetattr");
	atexit(disableRawMode);
	
	
	struct termios raw = E.og_term;
    raw.c_iflag &= ~(ICRNL|IXON|INPCK|BRKINT|ISTRIP);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME]=1;
    
    
    if (tcsetattr(STDIN_FILENO,TCSAFLUSH, &raw) == -1) die("tcsetattr");
} 



char editorReadKey(void){
	int nread;
	char c;
	while ((nread = read(STDIN_FILENO, &c, 1)) !=1){
		if (nread == -1 && errno != EAGAIN) die("read");
		
	}
	return c;
	
}



int getCursorPosition(int *rows, int *cols) {
  char buf[32];
    unsigned int i = 0;
    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;
    while (i < sizeof(buf) - 1) {
      if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
      if (buf[i] == 'R') break;
      i++;
    }
     buf[i] = '\0';
     if (buf[0] != '\x1b' || buf[1] != '[') return -1;
     if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;
    return 0;
}


int getWindowSize(int *rows, int *cols) {
  struct winsize ws;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return -1;
    return getCursorPosition(rows, cols);
  } else {
    *cols = ws.ws_col;
    *rows = ws.ws_row;
    return 0;
  }
}





/*** output ***/

void editorDrawRows(void) {
  int y;
  for (y = 0; y < E.screenrows; y++) {
    write(STDOUT_FILENO, "~", 1);
    if (y < E.screenrows - 1) {
      write(STDOUT_FILENO, "\r\n", 2);
    }
  }
}


void editorRefreshScreen(void){
	write(STDOUT_FILENO, "\x1b[2J", 4);
	write(STDOUT_FILENO, "\x1b[H", 3);

	editorDrawRows();
	write(STDOUT_FILENO, "\x1b[H", 3);
}	


/*** input ***/

void editorProcessKeypress(void){
	char c = editorReadKey();

	switch(c){
		case CTRL_KEY('q'):
			exit(0);
			break;
	}
}

/*** initialisation ***/

void initEditor(void){
	 if (getWindowSize(&E.screenrows, &E.screencolumns) == -1) die("getWindowSize");
}




/*** entry point ***/
int main(void){
	enableRawMode();
	
  while(1){
  	editorRefreshScreen();
  	editorProcessKeypress();
  }
  return 0;
}
