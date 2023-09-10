// including all the required libraries
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <bits/stdc++.h>
using namespace std;

// defining the maximum capacity of our LRU cache
#define CACHE_SIZE 2
// defining the maximum size of the page content that can be retrieved
#define MAX_PAGE_SIZE 4096

// defining a structure to represent a HTTP response
typedef struct
{
    char *url;
    char *content;
} HttpResponse;

// Definition of a doubly linked list node used for implementing LRU cache.
class node
{
public:
    HttpResponse *val; // The HTTP response value.
    string key;        // The URL key associated with the response.
    node *prev;        // Pointer to the previous node in the linked list.
    node *next;        // Pointer to the next node in the linked list.
    node()
    {
        key = "";
        val = NULL;
    }
    node(string l, HttpResponse *r)
    {
        key = l;
        val = r;
    }
    node(char *l, HttpResponse *r)
    {
        string s(l);
        key = s;
        val = r;
    }
};

// Definition of the LRUCache class for managing the cache.
class LRUCache
{
public:
    node *head = new node(); // Dummy head node for easy management.
    node *tail = new node(); // Dummy tail node for easy management.

    int cap;                // Capacity of the cache.
    map<string, node *> mp; // Map for quick lookup of cache entries.

    LRUCache(int capacity)
    {
        cap = capacity;
        head->next = tail;
        tail->prev = head;
    }

    // Function to add a node to the front of the linked list.
    void addnode(node *first)
    {
        node *t = head->next;
        head->next = first;
        first->prev = head;
        t->prev = first;
        first->next = t;
    }

    // Function to delete a node from the linked list.
    void deletenode(node *nod)
    {
        node *delprev = nod->prev;
        node *delnext = nod->next;
        delprev->next = delnext;
        delnext->prev = delprev;
    }

    // Function to get a cached HTTP response by URL.
    HttpResponse *get(char *key)
    {
        string s(key);
        if (mp.find(s) != mp.end())
        {
            node *t = mp[s];
            HttpResponse *ans = t->val;
            mp.erase(s);
            deletenode(t);
            addnode(t);
            mp[s] = head->next;
            return ans;
        }
        return NULL;
    }

    // Function to put a new HTTP response into the cache.
    void put(char *s, HttpResponse *value)
    {
        string key(s);
        if (mp.find(key) != mp.end())
        {
            node *t = mp[key];
            mp.erase(key);
            deletenode(t);
        }
        if (mp.size() == cap)
        {
            node *t = tail->prev;
            mp.erase(t->key);
            deletenode(t);
        }
        addnode(new node(key, value));
        mp[key] = head->next;
    }
};

// initializing the linked list to implement the LRU cache
LRUCache cache = LRUCache(CACHE_SIZE);
// initializing the number of entries currently present in the cache
int cache_count = 0;

// function to create a new HTTP response object
HttpResponse *create_response(char *url, char *content)
{
    HttpResponse *response = (HttpResponse *)malloc(sizeof(HttpResponse));
    response->url = strdup(url);
    response->content = strdup(content);
    return response;
}

// function to create a new cache entry and add it to the front of the linked list
void add_cache_entry(char *url, char *content)
{
    HttpResponse *htp = create_response(url, content);
    cache.put(url, htp);
}

// defining a function to retrieve a web page from the cache or fetch it via HTTP
char *get_or_fetch_page(char url[])
{
    printf("\n\n%s\n\n", url);
    HttpResponse *resp = cache.get(url);
    if (resp)
    {
        std::cout << "Served from Cache\n";
        return resp->content;
    }
    // If not in cache, fetching the page via HTTP GET request to the web server
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("Socket creation failed\n");
        return NULL;
    }

    char first[100];
    memset(first, 0, sizeof(first));
    int i = 7;
    int j = 0;
    int k = 0;
    char second[100];
    memset(second, 0, sizeof(second));

    // Extract the hostname and resource path from the URL.
    while (url[i] != '/')
    {
        first[j] = url[i];
        j++;
        i++;
    }
    if (url[4] != ':')
    {
        printf("Incorrect URL\n");
        close(sockfd);
        return NULL;
    }
    while (i < strlen(url))
    {
        second[k] = url[i];
        k++;
        i++;
    }

    // Resolve the hostname to an IP address.
    struct hostent *server = gethostbyname(first);
    if (server == NULL)
    {
        printf("Host not found\n");
        close(sockfd);
        return NULL;
    }

    // Configure the server address for the HTTP request.
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(80); // HTTP typically uses port 80.
    server_addr.sin_addr = *((struct in_addr *)server->h_addr);
    memset(server_addr.sin_zero, '\0', sizeof(server_addr.sin_zero));

    // Connect to the web server.
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        printf("Connection failed\n");
        close(sockfd);
        return NULL;
    }

    char request[MAX_PAGE_SIZE];
    snprintf(request, sizeof(request), "GET %s/ HTTP/1.1\r\nhost: %s \r\n\r\n", second, first);
    printf("%s\n", request);

    // Send the HTTP GET request.
    if (send(sockfd, request, strlen(request), 0) == -1)
    {
        printf("Send failed");
        close(sockfd);
        return NULL;
    }

    char response[MAX_PAGE_SIZE];
    int bytes_received = recv(sockfd, response, sizeof(response), 0);
    if (bytes_received == -1)
    {
        printf("Receive failed");
        close(sockfd);
        return NULL;
    }

    close(sockfd);

    // Extract and store the content in the cache.
    char *content = NULL;
    if (bytes_received > 0)
    {
        content = (char *)malloc(bytes_received + 1);
        strncpy(content, response, bytes_received);
        content[bytes_received] = '\0';
        add_cache_entry(url, content);
    }

    return content;
}

// Function to display the contents of the cache
void display_cache()
{
    node *current = cache.head;
    current = current->next;
    while (current != cache.tail)
    {
        printf("URL: %s\n", current->val->url);
        // printf("Content: %s\n", current->val->content);
        current = current->next;
    }
}

int main()
{
    while (1)
    {
        char url[100];
        memset(url, 0, sizeof url);
        printf("Please enter the url: ");
        scanf("%s", url);
        if (strcmp(url, "exit") == 0)
        {
            break;
            return 0; // Exit the program.
        }
        char *page1 = get_or_fetch_page(url);
        if (page1 != NULL)
        {
            printf("Page 1 content:\n%s\n\n\n", page1);
        }
        printf("Cache Contents:\n\n");
        display_cache();
        printf("\n\n\n\n\n\n\n");
    }
}
