#include<ncurses.h>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<iostream>
#include<unistd.h>

using namespace std;


condition_variable cv1;
condition_variable cv2;
mutex mtx1;
mutex mtx2;
const int thread_count=3;
bool fun1locked = false;
bool fun2locked = false;


void fun1(){
    unique_lock<mutex> lck(mtx1);
    while(true){
        while(fun1locked) {
            cv1.wait(lck);
            cout<<"fun1 lock"<<endl;
        }
        fun1locked = true;
        cout<<"fun1 not locked"<<endl;
    }
    
}

void fun2(){
    unique_lock<mutex> lck(mtx2);
    while(true){
        while(fun2locked) {
            cv2.wait(lck);
            cout<<"fun2 lock"<<endl;
            cout<<this_thread::get_id();
        }
        fun2locked = true;
        if(fun1locked){
            cout<<"fun2 unlocking fun1"<<endl;
            fun1locked = false;
            usleep(1000000);
            cv1.notify_one();
        }
    }
}

void fun3(){
    while(true){
        if(fun2locked){
            cout<<"fun3 unlocking fun2"<<endl;
            fun2locked = false;
            usleep(2000000);
            cv2.notify_one();
        }
        
    }
}

int main(int argc, char *argv[])
{
    thread threads[thread_count];

    threads[0] = thread(fun1);
    threads[1] = thread(fun2);
    threads[2] = thread(fun3);
    
    

    for(int i=0;i<thread_count;i++){
    	threads[i].join();
    }
    getchar();
    return 0;
}
