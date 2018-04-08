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

const int queue_start_positionX = 7;
const int start_height = 10;
const int clients_count = 100;
const int delay = 100;
const int circleSize = 5;

struct Client{
	condition_variable cv_client;
	int pos_x = 0;
	int pos_y = start_height;
	bool is_done = false;
	int color = 0;
};

mutex mtx_client;
mutex mtx_drawer;
mutex mtx_cashier;

//condition_variable cv_client;
condition_variable cv_drawer;
condition_variable cv_cashier;

bool drawer_busy;
bool draw;

queue<Client*> waiting_clients;
queue<Client*> doing_circle_clients;
queue<Client*> finished_clients;



thread thread_array[clients_count];

Client client_array[clients_count];


bool isCircleFinished(Client *c){
    if(c->pos_x==queue_start_positionX && c->pos_y==start_height){
		c->pos_y--;
		return false;
	}else{
		if(c->pos_x==queue_start_positionX && c->pos_y>(start_height-circleSize) && c->pos_y<=start_height){
			c->pos_y--;
			return false;
		}else{
			if(c->pos_x<(queue_start_positionX+circleSize*2) && c->pos_y==(start_height-circleSize)){
				c->pos_x++;
				return false;
			}else{
				if(c->pos_x==(queue_start_positionX+2*circleSize) && c->pos_y<(start_height+circleSize)){
					c->pos_y++;
					return false;
				}else{
					if(c->pos_x>(queue_start_positionX) && c->pos_y==(start_height+circleSize)){
						c->pos_x--;
						return false;
					}else{
						if(c->pos_x==queue_start_positionX && c->pos_y>start_height+1){
							c->pos_y--;
							return false;
						}else{
							c->pos_y--;
							return true;
						}
					}
				}
			}
		}
	}
}


void client(Client *client){
	unique_lock<mutex> lck(mtx_client);

	//walk to queue
	for(int i=0;i<queue_start_positionX;i++){
		client->cv_client.wait_for(lck, chrono::milliseconds(delay));
		client->pos_x++;
		//cout<<client->pos_x<<endl;
		if(!drawer_busy) {
			draw = true;
			cv_drawer.notify_one();
		}
	}

	for(int i=0;i<2;i++){
		if(waiting_clients.size()==2){
			if(doing_circle_clients.size()==2) break;
		//if(true){
			doing_circle_clients.push(client);
			while(!isCircleFinished(client)){
				//cout<<"circle"<<endl;
				client->cv_client.wait_for(lck, chrono::milliseconds(delay));
				if(!drawer_busy) {
					//cout<<"notifying drawe"<<endl;
					draw = true;
					cv_drawer.notify_one();
				}
			}
			doing_circle_clients.pop();
		}else{
			waiting_clients.push(client);
			while(!client->is_done) client->cv_client.wait(lck);
			finished_clients.push(client);
			break;
		}
		//client->is_done=true;
	}
	client->pos_x = queue_start_positionX + 2*circleSize + 1;
	for(int i=0;i<50;i++){
		client->cv_client.wait_for(lck, chrono::milliseconds(delay));
		client->pos_x++;
		if(!drawer_busy) {
			draw = true;
			cv_drawer.notify_one();
		}
	}
}

void drawer(){
	unique_lock<mutex> lck(mtx_drawer);
	while(true){
		while(!draw) cv_drawer.wait(lck);
		//cout<<"im drawing";
		draw = false;
		drawer_busy = true;
		clear();
		for(int i=0;i<clients_count;i++){
			if(!client_array[i].is_done){
				//cout<<client_array[i].pos_x<<" "<<client_array[i].pos_y<<endl;
				attron(COLOR_PAIR(client_array[i].color));
				mvprintw(client_array[i].pos_y,client_array[i].pos_x,"x");
				string tmp_queue_status = "Queue count: " + to_string(waiting_clients.size());
				string tmp_circle_status = "Circle count: " + to_string(doing_circle_clients.size());
				string tmp_finished = "Finished count: " + to_string(finished_clients.size());
				
				
				const char *queue_status = tmp_queue_status.c_str();
				const char *circle_status = tmp_circle_status.c_str();
				const char *finished_clients_status = tmp_finished.c_str();
				
				attron(COLOR_PAIR(1));
				mvprintw(0,0,queue_status);
				mvprintw(1,0,circle_status);
				mvprintw(2,0,finished_clients_status);				
								
				refresh();
			}else{
				//cout<<"dude is done"<<endl;
			}
		}
		drawer_busy=false;
	}
}

void cashier(){
	unique_lock<mutex> lck(mtx_cashier);
	while(true){
		cv_cashier.wait_for(lck, chrono::milliseconds(delay*10));
		if(waiting_clients.size()!=0) {
			waiting_clients.front()->is_done=true;
			waiting_clients.front()->cv_client.notify_one();
			waiting_clients.pop();
		}
	}
	
}

int main(int argc, char *argv[])
{
	mutex mtx;
	unique_lock<mutex> lck(mtx);
	condition_variable cv;

	
	initscr();
	start_color();
	init_pair(1, COLOR_BLACK, COLOR_CYAN);
	init_pair(2, COLOR_CYAN, COLOR_CYAN);
	init_pair(3, COLOR_BLUE, COLOR_BLUE);
	init_pair(4, COLOR_GREEN, COLOR_GREEN);
	init_pair(5, COLOR_MAGENTA, COLOR_MAGENTA);
	init_pair(6, COLOR_WHITE, COLOR_WHITE);
	init_pair(7, COLOR_YELLOW, COLOR_YELLOW);
	


	thread drw = thread(drawer);
	thread cshr = thread(cashier);
	
	srand(time(NULL));

	for(int i=0;i<clients_count;i++){
		client_array[i].color = rand()%8+2;
		thread_array[i] = thread(client, &client_array[i]);
		cv.wait_for(lck, chrono::milliseconds(delay*5));
	}
	
    endwin();
    getchar();
    return 0;
}