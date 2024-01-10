#include <bits/stdc++.h>

using namespace std;

class Customer
{
public:
    // Arrival time
    double arrival_time;

    // Start time of service
    double service_start_time;

    // Departure time
    double departure_time;

    // Server that served this customer
    int server_index;

    // Serial based on arrival time
    int serial;

    Customer() {}

    // Creates Customer object from arrival to system
    Customer(double arrival_time)
    {
        this->arrival_time = arrival_time;
    }
};

class Server
{
public:
    int status = 0;
    int id;
    Customer current_customer;
    int buffer;
    queue<Customer> q;
    double last_op = 0;
    double q_sum = 0;
    double busy_time = 0;
    double prev_free_time = 0;
    Server() {}
    Server(int status, int id, int buffer)
    {
        this->status = status;
        this->id = id;
        this->buffer = buffer;
    }
};

class Event
{
public:
    string type;
    double invoke_time;
    int target_server;
    Event() {}
    Event(string type, double invoke_time, int target_server)
    {
        this->type = type;
        this->invoke_time = invoke_time;
        this->target_server = target_server;
    }
};

class Compare
{
public:
    bool operator()(Event A, Event B)
    {
        return A.invoke_time > B.invoke_time;
    }
};

// Parameters for simulation
int arrival_rate;
int service_rate;
double inter_arrival_time_mean;
double inter_service_time_mean;
int num_servers;
int tot_passengers;
double current_time = 0;
int dropped = 0;

// Priority queue for events
priority_queue<Event, vector<Event>, Compare> pq;

// Vectors to store servers, customers, and buffer fill status
vector<Server> servers;
vector<Customer> customers;
vector<int> buffer_fill;

// Function to get the available server with the shortest queue or an idle server
int getAvailableServer()
{
    int availableServer = -1;
    int minSize = 1e9;

    // Find server with the shortest queue
    for (int i = 0; i < servers.size(); i++)
    {
        if (servers[i].q.size() < minSize)
        {
            minSize = servers[i].q.size();
            availableServer = i;
        }
    }

    // If all queues have the same size, find an idle server
    for (int i = 0; i < servers.size(); i++)
    {
        if (servers[i].status == 0)
        {
            availableServer = i;
        }
    }

    return availableServer;
}

int main()
{
    // Seed for random number generation
    srand(time(0));

    // Input simulation parameters from user
    cout << "Enter the number of security checkpoints:\n";
    cin >> num_servers;
    cout << "Enter the rate at which passengers arrive:\n";
    cin >> arrival_rate;
    cout << "Enter the rate at which passengers are processed:\n";
    cin >> service_rate;
    cout << "Enter the total number of passengers: \n";
    cin >> tot_passengers;

    // Calculate mean inter-arrival and inter-service times
    inter_arrival_time_mean = (double)1 / (double)arrival_rate;
    inter_service_time_mean = (double)1 / (double)service_rate;

    // Arrays to store buffer sizes for each server
    int buffer_sz[num_servers];

    // Input buffer sizes for each server and initialize server objects
    for (int i = 0; i < num_servers; i++)
    {
        cout << "Enter the buffer size for Checkpoint " << i + 1 << ": \n";
        cin >> buffer_sz[i];
        servers.push_back(Server(0, i, buffer_sz[i]));
        buffer_fill.push_back(0);
    }

    // Set up random number generators for inter-arrival and inter-service times
    std::random_device rd;
    std::exponential_distribution<> rng(arrival_rate);
    std::mt19937 rnd_gen(rd());

    std::random_device rd1;
    std::exponential_distribution<> rng1(service_rate);
    std::mt19937 rnd_gen1(rd1());

    int customer_no = 0;
    double prev_time = 0;

    // Generate arrival events for passengers and push them into the priority queue
    for (int i = 0; i < tot_passengers; i++)
    {
        prev_time += rng(rnd_gen);
        pq.push(Event("Arrival", prev_time, -1));
    }

    // Simulation loop
    while (!pq.empty())
    {
        Event e = pq.top();
        pq.pop();
        current_time = e.invoke_time;
        cout << "Time: " << current_time << "\n";

        if (e.type == "Arrival")
        {
            // Process Arrival event
            customer_no++;
            Customer customer(current_time);

            // Get available server with the shortest queue or an idle server
            int availableServer = getAvailableServer();

            if (servers[availableServer].status == 0)
            {
                // If the server is idle, assign the customer to the server
                customer.server_index = availableServer;
                customer.service_start_time = current_time;
                customer.serial = customer_no;
                cout << "Customer number " << customer.serial << " sent to Checkpoint " << customer.server_index << "\n";
                servers[availableServer].current_customer = customer;
                pq.push(Event("Departure", current_time + rng1(rnd_gen1), availableServer));
                servers[availableServer].status = 1;
                servers[availableServer].prev_free_time = current_time;
            }
            else if (buffer_fill[availableServer] < servers[availableServer].buffer)
            {
                // If the server has a non-full buffer, enqueue the customer
                customer.serial = customer_no;
                customer.server_index = availableServer;
                servers[availableServer].q.push(customer);
                servers[availableServer].q_sum += (buffer_fill[availableServer]) * (current_time - servers[availableServer].last_op);
                servers[availableServer].last_op = current_time;
                buffer_fill[availableServer]++;
            }
            else
            {
                // If the server's buffer is full, drop the customer
                dropped++;
            }
        }
        else
        {
            // Process Departure event
            int server = e.target_server;
            cout << "Checkpoint " << server << " finished serving customer " << servers[server].current_customer.serial << "\n";
            Customer customer = servers[server].current_customer;
            customer.departure_time = current_time;
            customers.push_back(customer);

            if (servers[server].q.empty())
            {
                // If the server's queue is empty, set the server to idle
                servers[server].status = 0;
                servers[server].busy_time += (current_time - servers[server].prev_free_time);
            }
            else
            {

                // If the server's queue is not empty, dequeue the next customer and assign to the server
                servers[server].q_sum += (buffer_fill[server]) * (current_time - servers[server].last_op);
                servers[server].last_op = current_time;
                buffer_fill[server]--;
                Customer customer = servers[server].q.front();
                servers[server].q.pop();
                customer.server_index = server;
                customer.service_start_time = current_time;
                servers[server].current_customer = customer;
                cout << "Customer number " << customer.serial << " sent to checkpoint " << customer.server_index << "\n";
                pq.push(Event("Departure", current_time + rng1(rnd_gen1), server));
                servers[server].status = 1;
            }
        }
    }

    // Calculate performance metrics and print results
    map<int, int> no_services_by_server;
    double total_queue_time = 0;
    double total_service_time = 0;

    for (auto i : customers)
    {
        no_services_by_server[i.server_index]++;
        total_queue_time += (i.service_start_time - i.arrival_time);
        total_service_time += (i.departure_time - i.service_start_time);
    }

    cout << endl;
    cout << "Total waiting time: " << total_queue_time << endl;
    cout << "Total service time: " << total_service_time << endl;
    cout << endl;

    double avg_waiting_time = (double)total_queue_time / (double)customer_no;
    double avg_service_time = (double)total_service_time / (double)customer_no;

    cout << "Average waiting time: " << avg_waiting_time << endl;
    cout << "Average service time: " << avg_service_time << endl;
    cout << endl;

    for (auto i : servers)
    {
        cout << "Average queue length for checkpoint " << i.id << " is " << i.q_sum / current_time << endl;
        cout << "System utilization for checkpoint " << i.id << " is " << i.busy_time / current_time << endl;
    }
    cout << endl;

    for (auto i : no_services_by_server)
    {
        cout << "Number of passengers served by checkpoint " << i.first << ": " << i.second << endl;
    }

    cout << endl;
    cout << "Number of passengers denied service: " << dropped << endl;
}
