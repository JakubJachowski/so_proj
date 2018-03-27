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
condition_variable cv3;

mutex mtx_draw;
mutex mtx_client;
mutex mtx_cashier;

bool isClientInQueueArray[thread_count];
bool isClientFinishedArray[thread_count];
bool isReadyToDraw = true;

WINDOW *windows[thread_count];

queue<int> q;
queue<int> q_finished;



void initIsClientInQueueArray(){
    for(int i=0;i<thread_count;i++){
        isClientInQueueArray[i] = false;
        isClientFinishedArray[i] = false;
    }
}

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

    bool finishedDoingCircle;

    unique_lock<mutex> lck(mtx_client);

    for(int i=0;i<circleXStart;i++){
        cv_client.wait_for(lck, chrono::milliseconds(500));
        clients[id].x++;
        isReadyToDraw = true;
        cv_draw.notify_one();
    }

    while(true){
        if(isClientFinishedArray[id]) {
            //cout<<"client leaving"<<endl;
            isReadyToDraw = true;
            cv_draw.notify_one();
            break;
        }
        
        //cout<<"client walks..."<<endl;

        isReadyToDraw = true;
        cv_draw.notify_one();
        if(q.size()==queueSize){
            //cout<<"queue is full"<<endl;
            finishedDoingCircle = false;
            while(!finishedDoingCircle) {

                finishedDoingCircle = isCircleFinished(&clients[id]);
                isReadyToDraw = true;
                cv_draw.notify_one();
                cv_client.wait_for(lck, chrono::milliseconds(100));
            }
        }else{
            //cout<<"in queue..."<<endl;
            clients[id].positionInQ=q.size();
            q.push(id);
            isClientInQueueArray[id]= true;
            while(isClientInQueueArray[id]) {
                cv_client.wait(lck);
                //cout<<"client  waiting"<<endl;
            }
            //cout<<"client served"<<endl;
            isReadyToDraw = true;
            cv_draw.notify_one();
        }
    }
    
}

void drawScene(){
    unique_lock<mutex> lck(mtx_draw);
    while(true){
        while(!isReadyToDraw) cv_draw.wait(lck);
        //cout<<"IM DRAWING BRO"<<endl;
        for(int i=0;i<thread_count;i++){
            if(windows[i]==NULL) windows[i] = newwin(2,2,0,0);
            if(clients[i].isQuit) continue;
            if(clients[i].positionInQ!=-1 && !clients[i].finished){
                wclear(windows[i]);
                wrefresh(windows[i]);
                mvwin(windows[i],2,2+clients[i].positionInQ);    
                box(windows[i], '|', '-');
    		    wrefresh(windows[i]);
            }else{
                if(!clients[i].finished){
                    wclear(windows[i]);
                    wrefresh(windows[i]);
                    mvwin(windows[i],clients[i].y+i,clients[i].x);
                    box(windows[i], '|', '-');
    		        wrefresh(windows[i]);
                }else{
                    wclear(windows[i]);
                    wrefresh(windows[i]);
                    mvwin(windows[i],5+(q_finished.size()*2),20);
                    box(windows[i], '|', '-');
    		        wrefresh(windows[i]);
                    clients[i].isQuit = true;
                }
                
            }
			
        }
        isReadyToDraw = false;
    }
}

void cashier(){
    // unique_lock<mutex> lck(mtx_cashier);
    while(true){
        this_thread::sleep_for(chrono::milliseconds(1000));
        if(!q.empty()){
            this_thread::sleep_for(chrono::milliseconds(1000));
            isClientInQueueArray[q.front()] = false;
            isClientFinishedArray[q.front()] = true;
            if(q.size()==queueSize){
                clients[q.back()].positionInQ=1;
                clients[q.front()].finished = true;
                q_finished.push(q.front());
                q.pop();
                clients[q.front()].positionInQ=0;
            }else{
                if(q.size()==queueSize-1){
                    clients[q.front()].finished = true;
                    q_finished.push(q.front());
                    q.pop();
                    clients[q.front()].positionInQ=0;
                }else{
                    clients[q.front()].finished = true;
                    q_finished.push(q.front());
                    q.pop();
                }
            }
            
            cv_draw.notify_all();

            this_thread::sleep_for(chrono::milliseconds(1000));
            cv_client.notify_all();
        }
    }

}

int main(int argc, char *argv[])
{
    initscr();

    initIsClientInQueueArray();

    threads[thread_count] = thread(cashier);    
    threads[thread_count+1] = thread(drawScene);

    for(int i=0;i<thread_count;i++){
        threads[i] = thread(client, i);
       // this_thread::sleep_for(chrono::milliseconds(1000));

       usleep(100000);
    }

    for(int i=0;i<thread_count+2;i++){
        threads[i].join();
        usleep(100000);
    }
    

    

    // thread t1 = thread(cashier);
    // thread t2 = thread(client, 0);
    getchar();
    endwin();
    return 0;
}
