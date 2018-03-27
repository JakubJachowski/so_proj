#include<ncurses.h>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<iostream>
#include<unistd.h>
#include <queue>
#include <chrono>    

using namespace std;


const int queueSize = 3;
const int thread_count = 100;
const int stepInterval = 100000;

const int circleSize = 10;
const int circleXStart = 5;

thread threads[thread_count+2];

struct Client{
	int x=0;
	int y=circleSize;
    int positionInQ = -1;
    bool finished = false;
    bool isQuit =false;
    bool waitingForStep = false;
};

Client clients[thread_count];

condition_variable cv_client;
condition_variable cv_draw;
condition_variable cv;

mutex mtx_draw;
mutex mtx_client;
mutex mtx_cashier;

bool isClientInQueueArray[thread_count];
bool isClientFinishedArray[thread_count];
bool isReadyToDraw = true;

WINDOW *windows[thread_count];

queue<int> q;
queue<int> q_finished;



bool isQueueFull(){
    if(q.size()==queueSize) return true;
    return false;
}


bool isCircleFinished(Client *c){
    //cout<<c->x<<" "<<c->y<<"AYYYYY"<<endl;
	if(c->y==circleSize && c->x==circleXStart){
		c->y--;
		return false;
	} else{
		if(c->x==circleXStart && c->y>0 && c->y<circleSize) {
			c->y--;
			return false;
		}else{
			if(c->y==0 && c->x<(circleXStart+2*circleSize)){
				c->x++;
				return false;
			}else{
				if(c->x==(2*circleSize+circleXStart) && c->y<(2*circleSize)){
					c->y++;
					return false;
				}else{
					if(c->y==(2*circleSize) && c->x>(circleXStart)){
						c->x--;
						return false;
					}else{
						if(c->x==circleXStart  && c->y!=(circleSize+1)){
							c->y--;
							return false;
						}else{
							c->y+=10;
							return true;
						}
					}
				}
			}
		}
	}
}


void client(int id){
	unique_lock<mutex> lck(mtx_client);

	for(int i=0;i<circleXStart;i++){
		cv_client.wait_for(lck, chrono::milliseconds(500));
		//this_thread::sleep_for(chrono::milliseconds(500));
		clients[id].x++;
		isReadyToDraw = true;
		cout<<id<<endl;
		cv_draw.notify_one();
	} 
}

void drawScene(){
    unique_lock<mutex> lck(mtx_draw);
	while(true){
		cv_draw.wait(lck, []{return isReadyToDraw;});
		cout<<"draw"<<endl;
		isReadyToDraw=false;
	}
}

void cashier(){
}

int main(int argc, char *argv[])
{

    for(int i=0;i<thread_count;i++){
        threads[i] = thread(client, i);
		usleep(100000);
    }
	threads[thread_count] = thread(drawScene);


    

    // thread t1 = thread(cashier);
    // thread t2 = thread(client, 0);
    getchar();
    return 0;
}

