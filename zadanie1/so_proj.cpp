#include<ncurses.h>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<iostream>

using namespace std;
mutex mtx;
condition_variable cv;
const int thread_count = 20;
const double slower = 10000000;
const int height = 20;
bool ready = false;
int position_array[thread_count];
WINDOW *window_array[thread_count];

bool isFinished(int tab[]){
	bool result = true;
	for(int i=0;i<thread_count;i++){
		if(tab[i]<height) result=false;
	}
	return result;
}

void calculate(int index){
	double counter = 0.0;
	int position=0;
	while(true){
		//counter+=((index+1)/slower);
		counter+=(1/slower);

		//cout<<"counter="<<counter;
		if(counter>position){
			//cout<<"if";
			mtx.lock();
			position++;
			position_array[index] = position;
			//cout<<"Thread"<<index<<", position="<<position_array[index]<<endl;
			ready=true;
			mtx.unlock();
			cv.notify_all();
			if(position==height) return;
		}
	}
}

void singleDraw(){
	for(int i=0;i<thread_count;i++){
		for(int i=0;i<thread_count;i++){
			box(window_array[i], '|', '-');
			touchwin(window_array[i]);
    		wrefresh(window_array[i]);
		}
	}
}

void initWindows(){
	for(int i=0;i<thread_count;i++){
		window_array[i] = newwin(2,2,0,i*2);
		box(window_array[i], '|', '-');
		touchwin(window_array[i]);
    	wrefresh(window_array[i]);
		position_array[i] = 0;
	}
}

void draw(){
	unique_lock<mutex> lck(mtx);
	while(true){

		if(isFinished(position_array)){
			singleDraw();
			return;
		}

		while(!ready) cv.wait(lck);
		for(int i=0;i<thread_count;i++){
			werase(window_array[i]);
			touchwin(window_array[i]);
    		wrefresh(window_array[i]);
			window_array[i] = newwin(2,2,position_array[i]*2, i*2);
			box(window_array[i], '|', '-');
			touchwin(window_array[i]);
    		wrefresh(window_array[i]);
			
		}
		ready=false;

		
	}
}




int main(int argc, char *argv[])
{
    initscr();
	initWindows();
	singleDraw();
	getchar();

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