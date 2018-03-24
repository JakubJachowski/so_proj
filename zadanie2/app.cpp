#include<ncurses.h>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<iostream>
#include<unistd.h>

using namespace std;
mutex mtx;
condition_variable cv;
const int thread_count = 1;
const double slower = 100000000;
const int height = 20;
bool ready = false;
bool isCashierBusy = false;
int position_array[thread_count];
WINDOW *windows[thread_count];

struct Client{
	int x=0;
	int y=0;
	bool isInQ = FALSE;
};

Client clients[thread_count];	

bool isFinished(int tab[]){
	bool result = true;
	for(int i=0;i<thread_count;i++){
		if(tab[i]<height) result=false;
	}
	return result;
}

void serveClient(){};

void calculate(int index){

	int position=0;
	//cout<<"Start thread"<<index<<endl;
	while(true){
		usleep(100000);
		mtx.lock();

		if(clients[index].x<7){
			clients[index].x++;
		}else{
			clients[index].y++;
		}
		
		position++;
		position_array[index] = position;
		//cout<<"Thread"<<index<<", position="<<position_array[index]<<endl;
		ready=true;
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

    thread threads[thread_count+1];


    for(int i=0;i<thread_count;i++){
    	threads[i] = thread(calculate, i);
    }
    threads[thread_count] = thread(draw);

    for(int i=0;i<thread_count;i++){
    	threads[i].join();
    }
    threads[thread_count].join();

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