#include <bits/stdc++.h>
using namespace std;

// Class representing an HTTP request
class HttpRequest
{
public:
    int id;
    int websiteID;
    double arrivalTime;
    double processingTime;

    // Constructor to initialize HttpRequest object
    HttpRequest(int id, int websiteID, double arrivalTime, double processingTime)
        : id(id), websiteID(websiteID), arrivalTime(arrivalTime), processingTime(processingTime) {}
};

// Class representing a website
class Website
{
public:
    int id;
    string owner;
    double bandwidth;
    double processingPower;
    std::queue<HttpRequest> requests;

    // Constructor to initialize Website object
    Website(int id, string owner, double bandwidth, double processingPower)
        : id(id), owner(owner), bandwidth(bandwidth), processingPower(processingPower) {}
};

// Class representing a Load Balancer
class LoadBalancer
{
public:
    std::vector<Website> websites;
    // Priority queue to implement Weighted Fair Queueing (WFQ) algorithm
    std::priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> pq;

    // Method to add a new website to the load balancer
    void add_website(int websiteID, string owner, int bandwidth, int processing_power)
    {
        websites.emplace_back(websiteID, owner, bandwidth, processing_power);
    }

    // Method to enqueue an HTTP request to the corresponding website's queue
    void enqueueRequest(HttpRequest httpRequest)
    {
        // Find the corresponding website and enqueue the request
        for (auto &website : websites)
        {
            if (website.id == httpRequest.websiteID)
            {
                website.requests.push(httpRequest);
            }
        }
    }

    // Weighted Fair Queueing algorithm
    void wfq()
    {
        while (!pq.empty())
            pq.pop();

        // Calculate finish times using WFQ for each website's request queue
        for (auto i : websites)
        {
            double w = (i.bandwidth + i.processingPower);
            double t = 0;
            auto temp = i.requests;

            while (!temp.empty())
            {
                auto j = temp.front();
                temp.pop();
                double ft = max(j.arrivalTime, t) + (j.processingTime / w);
                t = ft;
                pq.push({ft, i.id});
            }
        }

        // Debugging code (commented out)
        // auto chk = pq;
        // while (!chk.empty())
        // {
        //     cout << chk.top().first << " " << chk.top().second << endl;
        //     chk.pop();
        //  }
    }

    // Method to dequeue the request with the smallest finish time
    void dequeueRequest()
    {
        // Check if there are requests in the priority queue
        if (!pq.empty())
            for (auto &website : websites)
            {
                // Find the website with the smallest finish time and dequeue its request
                if (website.id == pq.top().second)
                {
                    pq.pop();
                    auto x = website.requests.front();
                    website.requests.pop();
                    cout << "Http request with id " << x.id << " dequeued" << endl;
                    break;
                }
            }
        else
        {
            cout << "No requests pending\n";
        }
    }
};

int main()
{
    LoadBalancer loadBalancer;
    int numWebsites, numRequests;

    // User input for the number of websites and their attributes
    cout << "Enter no. of websites: ";
    cin >> numWebsites;
    for (int i = 0; i < numWebsites; i++)
    {
        int websiteID;
        string owner;
        double bandwidth, processingPower;
        cout << "Enter [websiteID] [Owner] [Bandwidth] [Processing Power]: ";
        cin >> websiteID >> owner >> bandwidth >> processingPower;
        loadBalancer.add_website(websiteID, owner, bandwidth, processingPower);
    }

    // User input for the number of HTTP requests and their attributes
    cout << "Enter no. of httpRequests: ";
    cin >> numRequests;
    cout << "Enter all requests in increasing order of arrival time as \n [ID] [websiteID] [arrival Time] [Processing Time]\n";
    for (int i = 0; i < numRequests; i++)
    {
        int id, websiteID, arrivalTime, processingTime;
        cin >> id >> websiteID >> arrivalTime >> processingTime;
        loadBalancer.enqueueRequest(HttpRequest(id, websiteID, arrivalTime, processingTime));
    }

    // Main loop for user interaction
    while (1)
    {
        int x;
        cout << "Enter 1 to enqueue new request, 2 to dequeue request, 3 to exit: ";
        cin >> x;
        if (x == 3)
            break;
        else if (x == 2)
        {
            // Execute WFQ and dequeue the request
            loadBalancer.wfq();
            loadBalancer.dequeueRequest();
        }
        else
        {
            // User input to enqueue a new request
            cout << "Enter new request as \n [ID] [websiteID] [arrival Time] [Processing Time]\n";
            int id, websiteID, arrivalTime, processingTime;
            cin >> id >> websiteID >> arrivalTime >> processingTime;
            loadBalancer.enqueueRequest(HttpRequest(id, websiteID, arrivalTime, processingTime));
        }
    }
}
