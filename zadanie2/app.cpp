#include<ncurses.h>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<iostream>
#include<unistd.h>

using namespace std;
mutex mtx;
mutex que_mtx;
condition_variable cv;
condition_variable que_cv;
const int thread_count = 15;
const int circleSize = 10;
const int circleXStart = 10;
int queStatus = 0;
bool isManagerBusy = false;

const double slower = 100000000;
const int height = 90;
bool ready = false;
bool aux_ready = true;
bool isCashierBusy = false;
int position_array[thread_count];
WINDOW *windows[thread_count];

struct Client{
	int x=0;
	int y=circleSize;
};

Client clients[thread_count];	

bool isFinished(int tab[]){
	bool result = true;
	for(int i=0;i<thread_count;i++){
		if(tab[i]<height) result=false;
	}
	return result;
}

bool isCircleFinished(Client *c){
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



void manageQueue(){

}

void calculate(int index){
	unique_lock<mutex> lck_calculate(que_mtx);
	int position=0;
	bool inQueue = false;
	//cout<<"Start thread"<<index<<endl;
	while(true){

		//while(!aux_ready) cv_aux.wait(lck);
		usleep(100000);
		mtx.lock();


		if(clients[index].x<circleXStart){
			clients[index].x++;
		}else{
			if(queStatus<3){
				isInQueue = true;
				while(isManagerBusy) que_cv.wait(lck_calculate);

			}else{
				isCircleFinished(&clients[index]);
			}
		}
		
		position++;
		position_array[index] = position;
		//cout<<"Thread"<<index<<", position="<<position_array[index]<<endl;
		ready = true;
		mtx.unlock();
		cv.notify_all();
		if(position==height) return;
	}
}

void draw(){
	unique_lock<mutex> lck(mtx);
	while(true){
		while(!ready) cv.wait(lck);
		for(int i=0;i<thread_count;i++){

			if(windows[i]==NULL) windows[i] = newwin(2,2,0,0);
            wclear(windows[i]);
            wrefresh(windows[i]);
            mvwin(windows[i],clients[i].y,clients[i].x);
			box(windows[i], '|', '-');
    		wrefresh(windows[i]);
			
		}
		ready=false;

		if(position_array[0]==height){
			clear();
			return;
		} 
	}
}




int main(int argc, char *argv[])
{
    initscr();

    thread threads[thread_count+2];


    for(int i=0;i<thread_count;i++){
    	threads[i] = thread(calculate, i);
    }
    threads[thread_count] = thread(draw);
	//threads[thread_count+1] = thread(interrupter);

    for(int i=0;i<thread_count;i++){
    	threads[i].join();
    }
    threads[thread_count].join();
	// threads[thread_count+1].join();

    getchar();

    endwin();
    return 0;
}



// #include<ncurses.h>
// #include<thread>
// #include<mutex>
// #include<condition_variable>
// #include<iostream>
// #include<unistd.h>

// using namespace std;
// mutex mtx;
// condition_variable cv;
// const int thread_count = 4;
// const double slower = 100000000;
// const int height = 20;
// bool ready = false;
// int position_array[thread_count];
// WINDOW *windows[thread_count];
// struct Client{
//     int x;
//     int y;
//     int r;
//     int g;
//     int b;
// };
// //Client clients[thread_count];

// // bool isFinished(int tab[]){
// // 	bool result = true;
// // 	for(int i=0;i<thread_count;i++){
// // 		if(tab[i]<height) result=false;
// // 	}
// // 	return result;
// // }

// void calculate(int index){
// 	while(true){
// 		usleep(100000);
// 		//if(//clients[index].x<7){
// 			//clients[index].x++;
// 			cv.notify_all();
// 			ready=true;
// 		//}else{
// 			//break;
// 		//}

// 		//cout<<"counter="<<counter;
// 			//cout<<"if";

// 			// mtx.lock();
// 			// position++;
// 			// position_array[index] = position;
// 			// //cout<<"Thread"<<index<<", position="<<position_array[index]<<endl;
// 			// ready=true;
// 			// mtx.unlock();
// 			// cv.notify_all();
			
// 	}
// }

// void draw(){
// 	unique_lock<mutex> lck(mtx);
// 	int x=0;
// 	while(true){
// 		while(!ready) cv.wait(lck);
// 		for(int i=0;i<thread_count;i++){
// 			if(windows[i]==NULL){
//                 windows[i] = newwin(2,2,1,1);

//             }
//             wclear(windows[i]);
//             wrefresh(windows[i]);
//             mvwin(windows[i],1,1);
// 			box(windows[i], '|', '-');
//     		wrefresh(windows[i]);
			
// 		}
// 		ready=false;
// 		if(x=10000) break;
// 	}
// }






// int main(int argc, char *argv[])
// {
//     initscr();

//     // WINDOW *win = newwin(5, 5, 1, 1);
//     // box(win, '|', '-');
//     // wrefresh(win);
//     // usleep(1000000);
//     // wclear(win);
//     // wrefresh(win);
//     // usleep(1000000);
//     // mvwin(win, 10, 10);
//     // box(win, '|', '-');
//     // wrefresh(win);

//     thread threads[thread_count+1];

// 	// for(int i=0;i<thread_count;i++){
// 	// 	clients[i].x = 0;
// 	// 	clients[i].y = 0;
//     // }
//     threads[thread_count] = thread(draw);

//     for(int i=0;i<thread_count;i++){
//     	threads[i] = thread(calculate, i);
//     }
//     threads[thread_count] = thread(draw);

//     for(int i=0;i<thread_count;i++){
//     	threads[i].join();
//     }
//     threads[thread_count].join();

//     getchar();
//     endwin();
//     return 0;
// }