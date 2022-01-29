#include <stdio.h>
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>

WINDOW *pad1,*ball1;
int row, col;
int wx = 10;
int wy = 10;
int bx = 20;
int by = 20;
int xdir = 1;
int ydir = 1;

int wins[20][4] = {
	{6,4,20,20},
	{7,4,20,20},
	{8,4,20,20},
	{9,4,20,20},
	{10,4,20,20},
	{11,4,20,20},
	{6,5,20,20},
	{11,5,20,20},
	{6,6,20,20},
	{11,6,20,20},
	{6,7,20,20},
	{11,7,20,20},
	{6,8,20,20},
	{11,8,20,20},
	{6,9,20,20},
	{7,9,20,20},
	{8,9,20,20},
	{9,9,20,20},
	{10,9,20,20},
	{11,9,20,20}
};

WINDOW *blocks[20];

void test(){
        int on = 1;
	initscr();
	noecho();

	for(int i = 0; i < 20; i++){
	 blocks[i] = newwin(1,1,wins[i][0],wins[i][1]);
	 box(blocks[i],ACS_VLINE, ACS_HLINE);
	}


	pad1 = newwin(8,8,20,20);
	
	box(pad1,ACS_VLINE,ACS_HLINE);
	curs_set(0);
	refresh();
	wrefresh(pad1);
	
	while(on){
	  switch(getch()){
		case 'q':
		  on=0;
		  break;
		case 'w':
		  wy-=1;
		  break;
		case 's':
		  
		  wy+=1;
		  break;
	}
	//box(stdscr,ACS_VLINE, ACS_HLINE);
	mvwin(pad1,wy,wx);
	refresh();
	wrefresh(pad1);
	 for(int i = 0; i<20;i++){
  	   wrefresh(blocks[i]);
	 }
	}
	endwin();
}

void init(){

	int ch;
	initscr();
	noecho();
	pad1 = newwin(6,2,wx,wy);
	ball1 = newwin(2,2,bx,by);
	getmaxyx(stdscr,row,col);

	box(pad1,ACS_VLINE, ACS_HLINE);
	box(ball1,ACS_VLINE, ACS_HLINE);	
	box(stdscr,ACS_VLINE, ACS_HLINE);
        start_color();
	init_pair(1,COLOR_BLUE,7);
	wbkgd(pad1, COLOR_PAIR(1));
	wbkgd(ball1, COLOR_PAIR(1));
	refresh();
	wrefresh(pad1);
	wrefresh(ball1);
	nodelay(stdscr,TRUE);
	curs_set(0);

	while(1){
	  ch = getch();
	  if(ch=='q') break;
	  else if(ch=='w') wy-=1;
	  else if(ch=='s') wy+=1;
	  mvwin(pad1,wy,wx);
	  if(bx>col-4) xdir = -1;
	  else if(bx==0) xdir = 1;
	  if(by>row-4) ydir=-1;
	  else if(by==0) ydir=1;
	  bx+=xdir;
	  by+=ydir;
	  mvwin(ball1,by,bx);
	  box(stdscr,ACS_VLINE, ACS_HLINE);
	  refresh();
	  wrefresh(pad1);
	  wrefresh(ball1);
	  usleep(25000); 
	}
	endwin();


}


int main(int args, char* argv[]){

	test();

 return 0;
}

