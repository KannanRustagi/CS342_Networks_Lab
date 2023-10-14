#include <bits/stdc++.h>
#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include <thread>
#include <chrono>

using namespace std;

//time for which the channel is busy
int busy=0;
int num_try_nodes=0;
int tot_collisions=0;
int transmission_time=10;
int tot_succ_transmissions=0;
int tot_backoffs=0;
int t=0;
int max_trans_time=10;

vector<double>trans_times;

class channel{
    int curr_node_id=-1;
    public:
        bool transmit_channel(int node_id, string data){

            num_try_nodes++;
            if(busy<t){
                busy=t+(rand()%max_trans_time)+1;
                this->curr_node_id=node_id;
                tot_succ_transmissions++;
                cout<<"Node with ID "<<node_id<<" is transmitting the following data-"<<endl;
                cout<<data<<endl;
                return 1;
            }
            else{
                cout<<"Node with ID "<<node_id<<" cannot transmit beacuse Node with ID "<<curr_node_id<<" is already transmitting, try again later\n";
                tot_backoffs++;
                return 0;
            }
        }

};

channel shared_channel;

class node{
    public:
        int id;
        int max_back_int;
        int num_backoffs=0;
        int suc_transmit=0;
        string send_data;
        node(int id, int max_back_int){
            this->id=id;
            this->max_back_int=max_back_int;
        }

        void transmit(){
            if(shared_channel.transmit_channel(this->id, "Hello")){
                cout<<"Node "<<id<<" successfully transmitted data\n";
                this->suc_transmit+=1;
            }
            else{
                this->backoff();
            }
        }

        void backoff(){
            int back_time=(rand()%max_back_int) +1;
            this->num_backoffs+=1;
            cout<<"Node "<<id<<" is backing off for "<<back_time<<"s\n";
            trans_times[id-1]+=back_time;
        }

};

// void handle_node(node station){
//     while(1){
//         if(t==trans_times[station.id-1]){
//             station.transmit();
//         }
//     }
// }

void handle_channel(int n, vector<node> nodes){
    while(1){
        int node_id=-1;
        int c=0;
        int cnt=0;
        for(int i=0; i<n; i++){
            if(trans_times[i]==t+1){
                if(node_id==-1)node_id=i+1;
                else{
                    cout<<"Collision Detected\n";
                    nodes[i].backoff();
                    c=1;
                }
            }
            if(trans_times[i]<t)cnt++;
        }
        t++;
        if(c){
            if(node_id!=-1)nodes[node_id-1].backoff();
            tot_collisions++;
        }
        else{
            if(node_id!=-1)nodes[node_id-1].transmit();
        }
        if(cnt==n)break;
        sleep(1);
    }
}

int main(){
    srand(time(0));
    int N;
    cout<<"Enter number of nodes: ";
    cin>>N;
    cout<<endl;
    vector<node> nodes;
    
    trans_times.resize(N);
    for(int i=0; i<N; i++){
        node station(i+1, 8);
        nodes.push_back(station);
        cout<<"Enter the time of transmission for node "<<i+1<<":";
        cin>>trans_times[i];
        // thread(handle_node,station).detach();
    }

    thread(handle_channel, N, nodes).join();

    
    cout<<endl<<endl<<"Statistics:\n";
    cout<<"Successfully finished transmission\n";
    cout<<"Number of tried transmissions- "<<num_try_nodes<<endl;
    cout<<"Number of successful transmissions- "<<tot_succ_transmissions<<endl;
    cout<<"Number of collisions detected- "<<tot_collisions<<endl;
    cout<<"Number of backoffs- "<<tot_backoffs<<endl;
    return 0;
}