#include<ncurses.h>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<iostream>
#include<unistd.h>
#include <queue>
#include <chrono>    
using namespace std;

const int queue_start_positionX = 7;
const int start_height = 7;
const int clients_count = 10;
const int delay = 500;

struct Client{
	condition_variable cv_client;
	int pos_x = 0;
	int pos_y = start_height;
	bool is_done = false;
};

mutex mtx_client;
mutex mtx_drawer;

//condition_variable cv_client;
condition_variable cv_drawer;


bool drawer_busy;
bool draw;

queue<Client*> waiting_clients;

thread thread_array[clients_count];

Client client_array[clients_count];



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
			//do circle
		}else{
			waiting_clients.push(client);
			while(!client->is_done) client->cv_client.wait(lck);
			break;
		}
		client->is_done=true;
	}
}

void drawer(){
	unique_lock<mutex> lck(mtx_drawer);
	while(true){
		while(!draw) cv_drawer.wait(lck);
		draw = false;
		drawer_busy = true;
		for(int i=0;i<clients_count;i++){
			if(!client_array[i].is_done) cout<<client_array[i].pos_x<<endl;
		}
		drawer_busy=false;
	}
}

int main(int argc, char *argv[])
{
	thread drw = thread(drawer);
	for(int i=0;i<clients_count;i++){
		thread_array[i] = thread(client, &client_array[i]);
	}
	
    
    getchar();
    return 0;
}