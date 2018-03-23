#include<ncurses.h>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<iostream>
#include<unistd.h>

using namespace std;
mutex mtx;
condition_variable cv;
const int thread_count = 4;
const double slower = 100000000;
const int height = 20;
bool ready = false;
int position_array[thread_count];

struct Client{
    int x;
    int y;
    int r;
    int g;
    int b;
};
// bool isFinished(int tab[]){
// 	bool result = true;
// 	for(int i=0;i<thread_count;i++){
// 		if(tab[i]<height) result=false;
// 	}
// 	return result;
// }

// void calculate(int index){
	
// 	while(true){
// 		counter+=((index+1)/slower);
// 		//counter+=(1/slower);

// 		//cout<<"counter="<<counter;
// 		if(counter>position){
// 			//cout<<"if";
// 			mtx.lock();
// 			position++;
// 			position_array[index] = position;
// 			//cout<<"Thread"<<index<<", position="<<position_array[index]<<endl;
// 			ready=true;
// 			mtx.unlock();
// 			cv.notify_all();
// 			if(position==height) return;
// 		}
// 	}
// }

// void draw(){
// 	unique_lock<mutex> lck(mtx);
// 	while(true){
// 		while(!ready) cv.wait(lck);
// 		for(int i=0;i<thread_count;i++){
// 			erase();
// 			WINDOW *win = newwin(2,2,position_array[i]*2, i*2);
// 			box(win, '|', '-');
// 			touchwin(win);
//     		wrefresh(win);
			
// 		}
// 		ready=false;

// 		if(position_array[0]==height){
// 			clear();
// 			return;
// 		} 
// 	}
// }

void create_client(int client_id){
    Client client;
    client.x=0;
    client.y=0;
    while(true){
        client.x++;
        client.y++;
        usleep(1000000)
    }
}




int main(int argc, char *argv[])
{
    initscr();

    WINDOW *win = newwin(5, 5, 1, 1);
    box(win, '|', '-');
    wrefresh(win);
    usleep(1000000);
    wclear(win);
    wrefresh(win);
    usleep(1000000);
    mvwin(win, 10, 10);
    box(win, '|', '-');
    wrefresh(win);

    // thread threads[thread_count+1];


    // for(int i=0;i<thread_count;i++){
    // 	threads[i] = thread(calculate, i);
    // }
    // threads[thread_count] = thread(draw);

    // for(int i=0;i<thread_count;i++){
    // 	threads[i].join();
    // }
    // threads[thread_count].join();

    getchar();
    endwin();
    return 0;
}