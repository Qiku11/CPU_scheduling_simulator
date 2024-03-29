// Yuen Pak Hong (a1820486)
// Ze Kai Chong (a1786571)
// Shuhao Duan (a1807323)
/*
based on "baseline.cpp" by Andrey Kan
andrey.kan@adelaide.edu.au
2021
*/
#include <iostream>
#include <fstream>
#include <deque>
#include <vector>
#include <queue>

using namespace std;
// std is a namespace: https://www.cplusplus.com/doc/oldtutorial/namespaces/
const int TIME_ALLOWANCE = 8;  // allow to use up to this number of time slots at once
const int PRINT_LOG = 0; // print detailed execution trace

class Customer
{
public:
    std::string name;
    int priority;
    int arrival_time;
    int slots_remaining; // how many time slots are still needed
    int playing_since;

    Customer(std::string par_name, int par_priority, int par_arrival_time, int par_slots_remaining)
    {
        name = par_name;
        priority = par_priority;
        arrival_time = par_arrival_time;
        slots_remaining = par_slots_remaining;
        playing_since = -1;
    }

    Customer()
    {
        name = "";
        priority = -1;
        arrival_time = -1;
        slots_remaining = 0;
        playing_since = -1;
    }
};

class Event
{
public:
    int event_time;
    int customer_id;  // each event involes exactly one customer

    Event(int par_event_time, int par_customer_id)
    {
        event_time = par_event_time;
        customer_id = par_customer_id;
    }
};

void initialize_system(
    std::ifstream &in_file,
    std::deque<Event> &arrival_events,
    std::vector<Customer> &customers)
{
    std::string name;
    int priority, arrival_time, slots_requested;

    // read input file line by line
    // https://stackoverflow.com/questions/7868936/read-file-line-by-line-using-ifstream-in-c
    int customer_id = 0;
    while (in_file >> name >> priority >> arrival_time >> slots_requested)
    {
        Customer customer_from_file(name, priority, arrival_time, slots_requested);
        customers.push_back(customer_from_file);

        // new customer arrival event
        Event arrival_event(arrival_time, customer_id);
        arrival_events.push_back(arrival_event);

        customer_id++;
    }
}

void print_state(
    std::ofstream &out_file,
    int current_time,
    int current_id,
    const std::deque<Event> &arrival_events,
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> &customer_queue_high,
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> &customer_queue_low)
{
    out_file << current_time << " " << current_id << '\n';
    if (PRINT_LOG == 0)
    {
        return;
    }
    std::cout << current_time << ", " << current_id << '\n';
    for (int i = 0; i < arrival_events.size(); i++)
    {
        std::cout << "\t" << arrival_events[i].event_time << ", " << arrival_events[i].customer_id << ", ";
    }
    std::cout << '\n';
    /*for (int i = 0; i < customer_queue.size(); i++)
    {
        std::cout << "\t" << customer_queue[i] << ", ";
    }*/

    while(!customer_queue_high.empty()){
      cout << "\t" << customer_queue_high.top().second << ", ";
      customer_queue_high.pop();
    }

    while(!customer_queue_low.empty()){
      cout << "\t" << customer_queue_low.top().second << ", ";
      customer_queue_low.pop();
    }
    std::cout << '\n';
}

// process command line arguments
// https://www.geeksforgeeks.org/command-line-arguments-in-c-cpp/
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Provide input and output file names." << std::endl;
        return -1;
    }
    std::ifstream in_file(argv[1]);
    std::ofstream out_file(argv[2]);
    if ((!in_file) || (!out_file))
    {
        std::cerr << "Cannot open one of the files." << std::endl;
        return -1;
    }

    // deque: https://www.geeksforgeeks.org/deque-cpp-stl/
    // vector: https://www.geeksforgeeks.org/vector-in-cpp-stl/
    std::deque<Event> arrival_events; // new customer arrivals
    std::vector<Customer> customers; // information about each customer

    // read information from file, initialize events queue
    initialize_system(in_file, arrival_events, customers);

    int current_id = -1; // who is using the machine now, -1 means nobody
    int time_out = -1; // time when current customer will be preempted

    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> queue_high;
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> queue_low;



    // step by step simulation of each time slot
    bool all_done = false;
    for (int current_time = 0; !all_done; current_time++)
    {
        // welcome newly arrived customers
        while (!arrival_events.empty() && (current_time == arrival_events[0].event_time))
        {
          // loop thorugh the queue and insert the arriving job its corresponding location
          // so that we have our waiting queue set up in an ascending order to job lengths
          int cust_id = arrival_events[0].customer_id;
          if (customers[cust_id].priority == 0){
            queue_high.push({customers[cust_id].slots_remaining, cust_id});
          } else {
            queue_low.push({customers[cust_id].slots_remaining, cust_id});
          }


          // sort(vec.begin(), vec.end(), [](const Customer& lhs, const Customer& rhs) {
          //
          //   return lhs.key < rhs.key;
          // });

          arrival_events.pop_front();
        }
        // check if we need to take a customer off the machine
        if (current_id >= 0)
        {
            if (current_time == time_out)
            {
                int last_run = current_time - customers[current_id].playing_since;
                customers[current_id].slots_remaining -= last_run;
                if (customers[current_id].slots_remaining > 0)
                {
                    // customer is not done yet, waiting for the next chance to play
                    if (customers[current_id].priority == 0){
                      queue_low.push({customers[current_id].slots_remaining, current_id});
                    } else {
                      queue_high.push({customers[current_id].slots_remaining, current_id});
                    }
                }
                current_id = -1; // the machine is free now
            }
        }
        // if machine is empty, schedule a new customer
        if (current_id == -1)
        {
            if (!queue_high.empty()) // is anyone waiting?
            {
                current_id = queue_high.top().second;
                queue_high.pop();
                if (TIME_ALLOWANCE > customers[current_id].slots_remaining)
                {
                    time_out = current_time + customers[current_id].slots_remaining;
                }
                else
                {
                    time_out = current_time + TIME_ALLOWANCE;
                }
                customers[current_id].playing_since = current_time;
            } else if (!queue_low.empty()){
              current_id = queue_low.top().second;
              queue_low.pop();
              if (TIME_ALLOWANCE > customers[current_id].slots_remaining){
                time_out = current_time + customers[current_id].slots_remaining;
              } else {
                time_out = current_time + TIME_ALLOWANCE;
              }
              customers[current_id].playing_since = current_time;
            } else {

            }
        }
        print_state(out_file, current_time, current_id, arrival_events, queue_high, queue_low);

        // exit loop when there are no new arrivals, no waiting and no playing customers
        all_done = (arrival_events.empty() && queue_high.empty() && queue_low.empty() && (current_id == -1));
    }

    return 0;
}
