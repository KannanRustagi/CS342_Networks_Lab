#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include <limits.h>

using namespace std;

class Router
{
private:
    int routerId;
    vector<pair<Router *, int>> neighbours;
    map<int, int> routingTable;

public:
    Router(int id) : routerId(id){};

    void addNeighbour(Router *neighbour, int cost)
    {
        neighbours.push_back({neighbour, cost});
        neighbour->neighbours.push_back({this, cost});
    }

    int getRoutingInfo(int x)
    {
        if (routingTable.count(x))
            return routingTable[x];
        else
            return -1;
    }

    void updateRoutingTable()
    {
        map<int, pair<int, int>> dist;
        priority_queue<pair<Router *, int>, vector<pair<Router *, int>>, greater<pair<Router *, int>>> pq;

        for (auto &i : neighbours)
        {
            dist[i.first->routerId] = {i.second, i.first->routerId};
            pq.push({i.first, dist[i.first->routerId].first});
        }

        while (!pq.empty())
        {
            auto curr = pq.top();
            pq.pop();
            Router *node = curr.first;
            int weight = curr.second;

            if (weight > dist[node->routerId].first)
                continue;

            for (auto &i : node->neighbours)
            {
                int currDist = INT_MAX;
                if (dist.count(i.first->routerId))
                    currDist = dist[i.first->routerId].first;

                if (currDist > dist[node->routerId].first + i.second)
                {
                    dist[i.first->routerId].first = dist[node->routerId].first + i.second;
                    dist[i.first->routerId].second = dist[node->routerId].second;
                    pq.push({i.first, dist[i.first->routerId].first});
                }
            }
        }

        routingTable.clear();
        for (auto &i : dist)
        {
            if (i.first != routerId)
                routingTable[i.first] = i.second.second;
        }
    }
    void printRoutingTable()
    {
        cout << "Routing Table for router id: " << routerId << endl;
        for (auto &i : routingTable)
        {
            cout << i.first << ": " << i.second << endl;
        }
    }
};

int main()
{

    int numRouters, numEdges;
    // enter number of routers
    cout << "Enter number of routers: ";
    cin >> numRouters;
    vector<Router> myRouters;
    for (int i = 0; i < numRouters; i++)
    {
        Router x(i);
        myRouters.push_back(x);
    }

    cout << "Enter number of edges: ";
    cin >> numEdges;
    for (int i = 0; i < numEdges; i++)
    {
        int x, y, cost;
        cout << "Enter edges as \"routerA routerB cost\": ";
        cin >> x >> y >> cost;
        myRouters[x].addNeighbour(&myRouters[y], cost);
    }

    for (int i = 0; i < numRouters; i++)
    {
        myRouters[i].updateRoutingTable();
        myRouters[i].printRoutingTable();
    }

    while (1)
    {
        cout << "Enter \"routerA routerB\" to simulate path or -1 to exit:";
        int x, y;
        cin >> x;
        if (x == -1)
            break;
        cin >> y;
        int curr = x;
        vector<int> path;
        while (curr != y)
        {
            curr = myRouters[curr].getRoutingInfo(y);
            if (curr == -1)
                break;
            path.push_back(curr);
        }
        if (curr == -1)
            continue;
        cout << "The path is: " << x;
        for (auto i : path)
            cout << " --> " << i;
        cout << endl;
    }

    return 0;
}
// 0 1 4
// Enter edges as "routerA routerB cost": 0 2 2
// Enter edges as "routerA routerB cost": 1 2 1
// Enter edges as "routerA routerB cost": 1 3 5
// Enter edges as "routerA routerB cost": 2 3 8
// Enter edges as "routerA routerB cost": 2 4 10
// Enter edges as "routerA routerB cost": 3 4 2
// Enter edges as "routerA routerB cost": 3 5 6
// Enter edges as "routerA routerB cost": 4 5 5