#include<ncurses.h>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<iostream>
#include<unistd.h>

using namespace std;


const int queueSize = 3;
const int thread_count = 5;
thread threads[thread_count+1];

condition_variable cv_client;
condition_variable cv2;
condition_variable cv3;

mutex mtx_draw;
mutex mtx_client;
mutex mtx_cashier;
int queueCount = 0;
int queue[3];
bool isClientInQueueArray[thread_count];
bool isClientFinishedArray[thread_count];


void doCircle(){};

void initQueue(){
    for(int i=0;i<queueSize;i++){
        queue[i] = -1;
    }
}

void initIsClientInQueueArray(){
    for(int i=0;i<thread_count;i++){
        isClientInQueueArray[i] = false;
        isClientFinishedArray[i] = false;
    }
}

bool isQueueFull(){
    int count = 0;
    for(int i=0;i<queueSize;i++){
        if(queue[i]!=-1) count++;
        cout<<queue[i];
    }
    cout<<endl;
    if(count==queueSize) return true;
    return false;
}

void manageQueue(){
    for(int i=queueSize-1;i>=0;i--){
        for(int j=queueSize-2;j>=0;j--){
            if(queue[j]==-1) {
                queue[j] = queue[j+1];
                queue[j+1] = -1;
            }
        }
    }
}


void client(int id){

    unique_lock<mutex> lck(mtx_client);
    while(true){
        if(isClientFinishedArray[id]) {
            cout<<"client leaving"<<endl;
            break;
        }
        
        usleep(100000);
        cout<<"client walks..."<<endl;
        if(isQueueFull()){
            //doCircle();
            cout<<"does circle..."<<endl;
        }else{
            cout<<"in queue..."<<endl;
            queue[2] = id;
            isClientInQueueArray[id]= true;
            while(isClientInQueueArray[id]) {
                cv_client.wait(lck);
                cout<<"client  waiting"<<endl;
            }
            cout<<"client served"<<endl;
        }
    }
    
}

// void drawScene(){
//     unique_lock<mutex> lck(mtx_draw);
//     while(true){

//     }
// }

void cashier(){
    // unique_lock<mutex> lck(mtx_cashier);
    while(true){
        usleep(100000);
        manageQueue();
        if(queue[0]!=-1){
            usleep(100000);
            cout<<"serving client"<<endl;
            usleep(100000);          
            isClientInQueueArray[queue[0]] = false;
            //isClientFinishedArray[queue[0]] = true;
            queue[0] = -1; 
            manageQueue(); 
            cv_client.notify_one();
        }
    }

}

int main(int argc, char *argv[])
{
    initQueue();
    initIsClientInQueueArray();

    threads[thread_count] = thread(cashier);
    for(int i=0;i<thread_count;i++){
        threads[i] = thread(client, i);
    }

    

    // thread t1 = thread(cashier);
    // thread t2 = thread(client, 0);
    getchar();
    return 0;
}
