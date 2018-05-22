#include<ncurses.h>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<iostream>
#include<unistd.h>
#include<queue>
#include<set>
#include<chrono>    
#include<string>
#include<ctime>
using namespace std;

const int queue_start_positionX = 20;
const int start_height = 15;
const int clients_count = 100;
const int delay = 100;
const int circle_size = 6;

//pad start position
const int pad_start_x = 10;
const int pad_start_y = 30;
const int pad_width = 16;

//brick size
const int brick_width = 5;
const int brick_height = 1;

//screen size
const int screen_width =  60;
const int screen_height = 80;

//objects count
const int balls_count = 8;
const int pads_count = 1;
const int bricks_count = 60;


int spawned_clients = 0;
int end_value = 0;

struct Client{
	condition_variable cv_client;
	int pos_x = -1;
	int pos_y = start_height;
	bool is_done = false;
	bool in_queue = false;
	bool being_served = false;
	int color = 0;
};

struct Ball{
    condition_variable cv_ball;
    double pos_x = pad_start_x+(pad_width)/2;
    double pos_y = pad_start_y-3;
    double speed = 1;
    double x_mov = 0;
    double y_mov = 1;
	bool is_stuck = false;
	int ball_color;
};

struct Brick{
	Ball *stuck_ball;
	int pos_x;
	int pos_y;
	int width = brick_width;
	int height = brick_height;
	int brick_color;
};

struct Pad{
    condition_variable cv_pad;
    int pos_x = pad_start_x;
    int pos_y = pad_start_y;
    char key_pressed = 'x';
};  

mutex mtx_client;
mutex mtx_drawer;
mutex mtx_cashier;
mutex mtx_ball;
mutex mtx_pad;



//condition_variable cv_client;
condition_variable cv_drawer;
condition_variable cv_cashier;
condition_variable cv_ball;


bool drawer_busy;
bool draw;

queue<Client*> waiting_clients;
queue<Client*> doing_circle_clients;
queue<Client*> finished_clients;




Ball balls_array[balls_count];
Pad pads_array[pads_count];
Brick bricks_array[bricks_count];

thread balls_threads[balls_count];
thread pads_threads[pads_count];


int whichBrickTouching(Ball *ball){
	for(int i=0;i<bricks_count;i++){
		if(ball->pos_x>=bricks_array[i].pos_x && ball->pos_x<=(bricks_array[i].pos_x+brick_width)
			&& ball->pos_y>=bricks_array[i].pos_y && ball->pos_y<=(bricks_array[i].pos_y+brick_height)){
				if(bricks_array[i].stuck_ball==nullptr){
					bricks_array[i].stuck_ball = ball;
					bricks_array[i].brick_color = 8;
					bricks_array[i].stuck_ball->is_stuck = true;
					return i;
				}else{
					bricks_array[i].pos_x = -10;
					bricks_array[i].pos_y = -10;
					bricks_array[i].stuck_ball->is_stuck = false;
					bricks_array[i].stuck_ball->x_mov = -1*ball->x_mov;	
					bricks_array[i].stuck_ball->y_mov *= -1;
					ball->y_mov *=-1;
					bricks_array[i].stuck_ball->cv_ball.notify_one();
					bricks_array[i].stuck_ball = nullptr;
				}
		}
	}
	return -1;
}

void ball(Ball *ball, Pad *pad){
    unique_lock<mutex> lck(mtx_ball);
    while(true){
        ball->cv_ball.wait_for(lck, chrono::milliseconds(delay));
        ball->pos_x += ball->x_mov;
        ball->pos_y += ball->y_mov;

		if(ball->pos_y<=0){
			ball->y_mov *=-1;
		}
		if(ball->pos_x<=0){
			ball->x_mov*=-1;
			ball->pos_x = 0;
		}

		if(ball->pos_x>=screen_width){
			ball->x_mov*=-1;
			ball->pos_x = screen_width;
		}

		if(ball->pos_y>=pad_start_y && ball->pos_y<=pad_start_y+1 && ball->pos_x>=pad->pos_x && ball->pos_x<=(pad->pos_x+pad_width)){
			ball->y_mov*=-1;
			ball->x_mov = (ball->pos_x-pad->pos_x-(pad_width/2))/10*3;
		}

		if(ball->pos_y>=pad_start_y+2) {
			ball->pos_x = -1;
			ball->pos_y = -1;
			return;
		}
        
		// if(whichBrickTouching(ball)!=-1) ball->is_stuck=true;
		whichBrickTouching(ball);

		while(ball->is_stuck) ball->cv_ball.wait(lck);

        if(!drawer_busy){
            draw = true;
            cv_drawer.notify_one();
        }
    }
}

void pad(Pad *pad){
    unique_lock<mutex> lck(mtx_pad);
    while(true){
        pad->cv_pad.wait_for(lck, chrono::milliseconds(delay));
        if(pad->key_pressed == 'a' && pad->pos_x>=4) pad->pos_x-=4;
        if(pad->key_pressed == 'd'&& pad->pos_x<=screen_width-4) pad->pos_x+=4;
        pad->key_pressed = 'z';
        
        if(!drawer_busy){
            draw = true;
            cv_drawer.notify_one();
        }
    }
}

void drawer(){
	unique_lock<mutex> lck(mtx_drawer);

	string tmp_queue_status;
	string tmp_circle_status;
	string tmp_finished;
	string tmp_spawned_clients;
	
					
	const char *queue_status;
	const char *circle_status;
	const char *finished_clients_status;
	const char *spawned_clients_status;
	

	while(true){
		while(!draw) cv_drawer.wait(lck);

		draw = false;
		drawer_busy = true;
		clear();
		clear();

        //draw balls
        for(int i=0;i<balls_count;i++){
			attron(COLOR_PAIR(balls_array[i].ball_color));
			mvprintw(balls_array[i].pos_y,balls_array[i].pos_x,"x");	            							
        }

		//draw pads
        for(int i=0;i<pads_count;i++){
            attron(COLOR_PAIR(3));
            mvprintw(pads_array[i].pos_y,pads_array[i].pos_x,"xxxxxxxxxxxxxxxx");
        }

		//draw bricks
		for(int i=0;i<bricks_count;i++){
            attron(COLOR_PAIR(bricks_array[i].brick_color));
            mvprintw(bricks_array[i].pos_y, bricks_array[i].pos_x, "xxxxx");
        }

		// tmp_queue_status = "Queue count: " + to_string(waiting_clients.size());
		// tmp_circle_status = "Circle count: " + to_string(doing_circle_clients.size());
		// tmp_finished = "Finished count: " + to_string(finished_clients.size());
		// tmp_spawned_clients = "Spawned clients: " + to_string(spawned_clients);
		
		
		// queue_status = tmp_queue_status.c_str();
		// circle_status = tmp_circle_status.c_str();
		// finished_clients_status = tmp_finished.c_str();
		// spawned_clients_status = tmp_spawned_clients.c_str();
		
		
		// attron(COLOR_PAIR(1));
		// mvprintw(0,0,queue_status);
		// mvprintw(1,0,circle_status);
		// mvprintw(2,0,finished_clients_status);
		// mvprintw(3,0,spawned_clients_status);
		
		// mvprintw(start_height-1, queue_start_positionX + 1, "Queue");
		// mvprintw(start_height-1, queue_start_positionX + 2*circle_size- 1, "K");
		
		// //drawing queue
		// if(waiting_clients.size()==2){
		// 	attron(COLOR_PAIR(waiting_clients.back()->color));
		// 	mvprintw(start_height, queue_start_positionX + 1, "x");
		// 	attron(COLOR_PAIR(waiting_clients.front()->color));
		// 	mvprintw(start_height, queue_start_positionX + 2, "x");
		// }else{
		// 	if(waiting_clients.front()!=NULL){
		// 		attron(COLOR_PAIR(waiting_clients.front()->color));
		// 		mvprintw(start_height, queue_start_positionX + 2, "x");
		// 	}
		// }
		refresh();
		drawer_busy=false;
		//if(end_value==clients_count) break;
	}
}

void cashier(){
	unique_lock<mutex> lck(mtx_cashier);
	Client *temp;
	while(true){
		if(waiting_clients.size()!=0) {
			temp = waiting_clients.front();
			waiting_clients.pop();
			temp->being_served=true;			
			temp->in_queue=false;
			while(drawer_busy) cv_cashier.wait_for(lck, chrono::milliseconds(delay/10));
			if(!drawer_busy) {
				draw = true;
				cv_drawer.notify_one();
			}
			cv_cashier.wait_for(lck, chrono::milliseconds(delay*10));
			temp->is_done=true;
			temp->being_served=false;						
			temp->cv_client.notify_one();
			while(drawer_busy) cv_cashier.wait_for(lck, chrono::milliseconds(delay/10));
			if(!drawer_busy) {
				draw = true;
				cv_drawer.notify_one();
			}			
		}
		//if(end_value==clients_count) break;
	}
	
}

int main(int argc, char *argv[])
{
	mutex mtx;
	unique_lock<mutex> lck(mtx);
	condition_variable cv;


    initscr();			/* Start curses mode 		*/
	raw();				/* Line buffering disabled	*/
	keypad(stdscr, TRUE);		/* We get F1, F2 etc..		*/
	noecho();			/* Don't echo() while we do getch */
	start_color();
	curs_set(0);

	init_pair(1, COLOR_BLACK, COLOR_CYAN);
	init_pair(2, COLOR_CYAN, COLOR_CYAN);
	init_pair(3, COLOR_BLUE, COLOR_BLUE);
	init_pair(4, COLOR_GREEN, COLOR_GREEN);
	init_pair(5, COLOR_MAGENTA, COLOR_MAGENTA);
	init_pair(6, COLOR_WHITE, COLOR_WHITE);
	init_pair(7, COLOR_YELLOW, COLOR_YELLOW);
	init_pair(8, COLOR_RED, COLOR_RED);
	


	thread drw = thread(drawer);

	
	srand(time(NULL));

	for(int i=0;i<bricks_count;i++){
		bricks_array[i].brick_color = 7;
		if(i==0){
			bricks_array[i].pos_x = 0;
			bricks_array[i].pos_y = 0;
		}else{
			if(bricks_array[i-1].pos_x+brick_width<=screen_width){
				bricks_array[i].pos_x = bricks_array[i-1].pos_x+brick_width+1;
				bricks_array[i].pos_y = bricks_array[i-1].pos_y;
			}else{
				bricks_array[i].pos_x = 0;
				bricks_array[i].pos_y = bricks_array[i-1].pos_y+brick_height+1;
			}
		}
	}

	for(int i=0;i<pads_count;i++){
        pads_threads[i] = thread(pad, &pads_array[i]);
    }

    for(int i=0;i<balls_count;i++){
		balls_array[i].ball_color = rand()%6+2;
        balls_threads[i] = thread(ball, &balls_array[i], &pads_array[0]);
		cv.wait_for(lck, chrono::milliseconds(1000));
    }


    
    

	// for(int i=0;i<clients_count;i++){
	// 	client_array[i].color = rand()%6+2;
	// 	thread_array[i] = thread(client, &client_array[i]);
	// 	spawned_clients++;
	// 	cv.wait_for(lck, chrono::milliseconds(rand()%1000));
	// }
	
	// drw.join();
	// cshr.join();
	// for(int i=0;i<clients_count;i++){
	// 	thread_array[i].join();
	// }

    char aux = 'a';
    while(true){
        aux =getchar();
        if(aux=='x') break;
        pads_array[0].key_pressed = aux;
    };
    endwin();
    
    return 0;
}