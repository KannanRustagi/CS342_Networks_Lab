#include <bits/stdc++.h>
#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include <thread>

using namespace std;

// time till which the channel is busy
int busy = 0;
// number of tried transmissions
int num_try_nodes = 0;
// total number of collisions that occured
int tot_collisions = 0;
// total number of successful transmissions
int tot_succ_transmissions = 0;
// total number of times all the nodes backed off
int tot_backoffs = 0;
// initializing timestamp
int t = 0;

// initializing the maximum time for which any node can transmit data onto any channel
int max_trans_time = 10;

// initializing the absolute times at which the nodes start transmission
vector<double> trans_times;

// class for the shared channel
class channel
{
    // current node that is transmitting
    int curr_node_id = -1;

public:
    // function for transmitting data onto the channel
    bool transmit_channel(int node_id, string data)
    {
        num_try_nodes++;
        // if the channel is currently not busy
        if (busy < t)
        {
            busy = t + (rand() % max_trans_time) + 1;
            this->curr_node_id = node_id;
            tot_succ_transmissions++;
            cout << "Node with ID " << node_id << " is transmitting the following data-" << endl;
            cout << data << endl;
            return 1;
        }
        // if the channel is busy
        else
        {
            cout << "Node with ID " << node_id << " cannot transmit beacuse Node with ID " << curr_node_id << " is already transmitting, try again later\n";
            tot_backoffs++;
            return 0;
        }
    }
};

channel shared_channel;

class node
{
public:
    int id;
    // maximum backoff interval for a node
    int max_back_int;
    int num_backoffs = 0;
    int suc_transmit = 0;

    // constructor
    node(int id, int max_back_int)
    {
        this->id = id;
        this->max_back_int = max_back_int;
    }

    // transmitting data to channel
    void transmit()
    {
        if (shared_channel.transmit_channel(this->id, "Hello"))
        {
            cout << "Node " << id << " successfully transmitted data\n";
            this->suc_transmit += 1;
        }
        else
        {
            backoff();
        }
    }

    // backing off when channek is busy
    void backoff()
    {

        int back_time = (rand() % max_back_int) + 1;
        this->num_backoffs += 1;
        cout << "Node " << id << " is backing off for " << back_time << "s\n";
        trans_times[id - 1] += back_time;
    }
};

void handle_channel(int n, vector<node> &nodes)
{
    while (1)
    {
        int node_id = -1;
        // for detecting collision
        int c = 0;
        // count of nodes that have successfully transmitted
        int cnt = 0;

        // iterating over each node and checking whether or not it wants to transmit at a given time
        for (int i = 0; i < n; i++)
        {
            if (trans_times[i] == t + 1)
            {
                if (node_id == -1)
                    node_id = i + 1;
                else
                {
                    cout << "Collision Detected\n";
                    nodes[i].backoff();
                    c = 1;
                }
            }
            if (trans_times[i] < t)
                cnt++;
        }
        // incrementing time
        t++;
        // if a collision has occured
        if (c)
        {
            if (node_id != -1)
                nodes[node_id - 1].backoff();
            tot_collisions++;
        }
        else
        {
            if (node_id != -1)
                nodes[node_id - 1].transmit();
        }
        // all the nodes have successfully transmitted
        if (cnt == n)
            break;
        sleep(1);
    }
}

int main()
{
    srand(time(0));
    int N;
    cout << "Enter number of nodes: ";
    cin >> N;
    cout << endl;
    vector<node> nodes;

    trans_times.resize(N);
    
    for (int i = 0; i < N; i++)
    {
        node station(i + 1, 8);
        nodes.push_back(station);
        cout << "Enter the time of transmission for node " << i + 1 << ":";
        cin >> trans_times[i];
    }

    // simulation of csma/cd protocol
    handle_channel(N, nodes);

    cout << endl
         << endl
         << "Statistics:\n";
    cout << "Successfully finished all transmissions\n";
    cout << "Total Number of combined tried transmissions by all the nodes- " << num_try_nodes << endl;
    cout << "Total Number of combined successful transmissions- " << tot_succ_transmissions << endl;
    cout << "Total Number of collisions detected- " << tot_collisions << endl;
    cout<<endl;
    cout << "Number of backoffs detected for each node-"<< endl;

    for(auto i: nodes){
        cout<<"For node with id "<<i.id<<", number of times it had to backoff= "<<i.num_backoffs<<endl;
    }

    return 0;
}