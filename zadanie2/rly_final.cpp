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
const int clients_count = 1;
const int delay = 100;
const int circle_size = 6;
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
		if(c->pos_x==queue_start_positionX && c->pos_y>(start_height-circle_size) && c->pos_y<=start_height){
			c->pos_y--;
			return false;
		}else{
			if(c->pos_x<(queue_start_positionX+circle_size*2) && c->pos_y==(start_height-circle_size)){
				c->pos_x++;
				return false;
			}else{
				if(c->pos_x==(queue_start_positionX+2*circle_size) && c->pos_y<(start_height+circle_size)){
					c->pos_y++;
					return false;
				}else{
					if(c->pos_x>(queue_start_positionX) && c->pos_y==(start_height+circle_size)){
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
			if(doing_circle_clients.size()==2) {
				client->pos_x--;
				for(int i=0;i<50;i++){
					client->pos_y--;
					if(!drawer_busy) {
						draw = true;
						cv_drawer.notify_one();
					}
					client->cv_client.wait_for(lck, chrono::milliseconds(delay));
				}
				return;	
			}
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
			client->in_queue = true;
			if(!drawer_busy) {
				draw = true;
				cv_drawer.notify_one();
			}
			while(!client->is_done) client->cv_client.wait(lck);
			finished_clients.push(client);
			break;
		}
		//client->is_done=true;
	}
	client->pos_x = queue_start_positionX + 2*circle_size + 1;
	for(int i=0;i<20;i++){
		client->cv_client.wait_for(lck, chrono::milliseconds(delay));
		client->pos_x++;
		if(!drawer_busy) {
			draw = true;
			cv_drawer.notify_one();
		}
	}
	end_value++;
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
		for(int i=0;i<clients_count;i++){
			if(!client_array[i].in_queue && !client_array[i].being_served){
				//cout<<client_array[i].pos_x<<" "<<client_array[i].pos_y<<endl;
				attron(COLOR_PAIR(client_array[i].color));
				mvprintw(client_array[i].pos_y,client_array[i].pos_x,"x");								
			}
			if(client_array[i].being_served){
				attron(COLOR_PAIR(client_array[i].color));
				mvprintw(start_height, queue_start_positionX + 2*circle_size- 1, "x");
			}
			
			// if(waiting_clients.back()!=NULL){
			// 	attron(COLOR_PAIR(waiting_clients.back()->color));
			// 	mvprintw(start_height, queue_start_positionX + 1, "x");
			// }
		}
		///drawing status
		tmp_queue_status = "Queue count: " + to_string(waiting_clients.size());
		tmp_circle_status = "Circle count: " + to_string(doing_circle_clients.size());
		tmp_finished = "Finished count: " + to_string(finished_clients.size());
		tmp_spawned_clients = "Spawned clients: " + to_string(spawned_clients);
		
		
		queue_status = tmp_queue_status.c_str();
		circle_status = tmp_circle_status.c_str();
		finished_clients_status = tmp_finished.c_str();
		spawned_clients_status = tmp_spawned_clients.c_str();
		
		
		attron(COLOR_PAIR(1));
		mvprintw(0,0,queue_status);
		mvprintw(1,0,circle_status);
		mvprintw(2,0,finished_clients_status);
		mvprintw(3,0,spawned_clients_status);
		
		mvprintw(start_height-1, queue_start_positionX + 1, "Queue");
		mvprintw(start_height-1, queue_start_positionX + 2*circle_size- 1, "K");
		
		//drawing queue
		if(waiting_clients.size()==2){
			attron(COLOR_PAIR(waiting_clients.back()->color));
			mvprintw(start_height, queue_start_positionX + 1, "x");
			attron(COLOR_PAIR(waiting_clients.front()->color));
			mvprintw(start_height, queue_start_positionX + 2, "x");
		}else{
			if(waiting_clients.front()!=NULL){
				attron(COLOR_PAIR(waiting_clients.front()->color));
				mvprintw(start_height, queue_start_positionX + 2, "x");
			}
		}
		refresh();
		drawer_busy=false;
		if(end_value==clients_count) break;
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
		if(end_value==clients_count) break;
	}
	
}

int main(int argc, char *argv[])
{
	mutex mtx;
	unique_lock<mutex> lck(mtx);
	condition_variable cv;

	
	initscr();
	start_color();
	curs_set(0);
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
		client_array[i].color = rand()%6+2;
		thread_array[i] = thread(client, &client_array[i]);
		spawned_clients++;
		cv.wait_for(lck, chrono::milliseconds(rand()%5000));
	}
	
	drw.join();
	cshr.join();
	for(int i=0;i<clients_count;i++){
		thread_array[i].join();
	}

	getchar();
    endwin();
    
    return 0;
}