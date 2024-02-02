// Ze Kai Chong (a1786571)
// Yuen Pak Hong (a1820486)
// Shuhao Duan (a1807323)

#include <iostream>
#include <fstream>
#include <deque>
#include <vector>
#include <algorithm>

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
    int id;

    Customer(std::string par_name, int par_priority, int par_arrival_time, int par_slots_remaining, int par_id)
    {
        name = par_name;
        priority = par_priority;
        arrival_time = par_arrival_time;
        slots_remaining = par_slots_remaining;
        playing_since = -1;
        id = par_id;
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
  std::vector<Customer> &customers,
  std::vector<Customer> &customers2)
  {
    std::string name;
    int priority, arrival_time, slots_requested;

    // read input file line by line
    // https://stackoverflow.com/questions/7868936/read-file-line-by-line-using-ifstream-in-c
    int customer_id = 0;
    in_file >> name >> priority >> arrival_time >> slots_requested;
    Customer customer_from_file(name, priority, arrival_time, slots_requested, customer_id);
    customers.push_back(customer_from_file);
    customers2.push_back(customer_from_file);
    Event arrival_event(arrival_time, customer_id);
    arrival_events.push_back(arrival_event);
    customer_id = 1;

    while (in_file >> name >> priority >> arrival_time >> slots_requested)
    {
      Customer customer_from_file(name, priority, arrival_time, slots_requested, customer_id);
      customers.push_back(customer_from_file);

      // we insert customers to locations corresponding to their slots_remaining value
      for (int j = 0; j < customers2.size(); j ++){
        //cout<<j<<endl;
        if (customers2[j].slots_remaining < customer_from_file.slots_remaining ){
          customers2.insert(customers2.begin()+j, customer_from_file);
          break;
        }
        if (j == customers2.size()-1){
          customers2.push_back(customer_from_file);
          break;
        }
      }
      //cout<<"done"<<endl;
      //customers2.push_back(customer_from_file);

      // struct sort_helper
      // {
      //   inline bool operator() (const Customer& struct1, const Customer& struct2)
      //   {
      //     return (struct1.slots_remaining > struct2.slots_remaining); // descending order in terms of slots requested
      //   }
      // };
      //
      // std::sort(customers2.begin(), customers2.end(), sort_helper());

      // new customer arrival event
      Event arrival_event(arrival_time, customer_id);
      arrival_events.push_back(arrival_event);

      customer_id++;
    }
    // while (!customers2.empty()){
    //   Customer current_customer = customers2.front();
    //   customers2.erase(customers2.begin());
    //   cout<<current_customer.slots_remaining<<endl;
    // }
  }

void print_state(
    std::ofstream &out_file,
    int current_time,
    int current_id,
    const std::deque<Event> &arrival_events,
    const std::deque<int> &customer_queue)
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
    for (int i = 0; i < customer_queue.size(); i++)
    {
        std::cout << "\t" << customer_queue[i] << ", ";
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
    std::vector<Customer> customers2; // our helper vector to be used to implement SJF
    int ganttchart[10000] = {-1}; // helper vector to implement SJF

    // read information from file, initialize events queue
    initialize_system(in_file, arrival_events, customers, customers2);

    // fill up the gantt chart
    int starting_point = 0;
    int chart_length = 0;

    while (!customers2.empty()){
      //cout<<customers2.size()<<endl;
      //std::cout<<"looping!"<<std::endl;
      Customer current_customer = customers2.front();
      customers2.erase(customers2.begin());
      //cout<<customers2.size()<<endl;
      starting_point = current_customer.arrival_time;
      // check if starting point has been occupied by another customer already
      // if so, calculate the remaining time slots of the previous customer
      // if remaining time slot shorter than current customer's, current customer makes way
      // and starts playing after the end of last customer's round
      if (ganttchart[starting_point] != -1){
        // calculate the remaining length of the playing period of the previous customer
        if ((customers[ganttchart[starting_point]].slots_remaining - starting_point + customers[ganttchart[starting_point]].arrival_time) < current_customer.slots_remaining){
          // ***** potentially causing a bug here. we might skip some time slots of a job in the middle and end up not finishing a job *****
          starting_point = customers[ganttchart[starting_point]].slots_remaining + customers[ganttchart[starting_point]].arrival_time;
        }
      }

      Customer helper("helper", 1, -1, -1, -1);
      int replacedid = -1;
      bool countlength = false;
      int length_of_replaced = 0;
      //std::cout<<current_customer.slots_remaining<<std::endl;
      for (int i = 0; i < current_customer.slots_remaining; i++){
        //std::cout<<"looping2!"<<std::endl;
        //std::cout<<i<<std::endl;
        // okay so during the addition of the customer to our gantt chart, if
        // we find that we are replacing a previous customer's occupation then
        // we package the replaced part as a new customer but with the same customer id
        // and the arrival time of the end of our currently inserting job
        if (ganttchart[starting_point+i] != -1 && helper.id == -1){
          replacedid = ganttchart[starting_point+i];
          cout<<replacedid<<endl;
          countlength = true;
          helper.name = customers[replacedid].name;
          helper.priority = customers[replacedid].priority;
          helper.arrival_time = customers[replacedid].arrival_time+customers[replacedid].slots_remaining;
          helper.slots_remaining = -1;
          helper.id = replacedid;
        }
        ganttchart[starting_point+i] = current_customer.id;
        if (starting_point+i > chart_length){
          chart_length = starting_point+i;
          //cout<<chart_length<<endl;
        }
        if (countlength){
          length_of_replaced ++;
        }
      }

      if (countlength){
        helper.slots_remaining = length_of_replaced;
        customers[replacedid].slots_remaining = customers[replacedid].slots_remaining + helper.slots_remaining;
        //cout<<helper.arrival_time<<endl;

        for (int j = 0; j < customers2.size(); j ++){
          //cout<<j<<endl;
          if (customers2[j].slots_remaining < helper.slots_remaining ){
            customers2.insert(customers2.begin()+j, helper);
            countlength = false;
            length_of_replaced = 0;
            helper.id = -1;
            break;
          }
          if (j == customers2.size()-1){
            customers2.push_back(helper);
            countlength = false;
            length_of_replaced = 0;
            helper.id = -1;
          }
        }

      }

    }
    cout<<"hello"<<endl;
    //
    //
    // // now we do the second sweep
    //
    //
    //
    // // the second sweep after all the jobs have been filled onto the gantt
    // int current_id = -1; // who is using the machine now, -1 means nobody
    // int time_out = -1; // time when current customer will be preempted
    // std::deque<int> queue; // waiting queue
    //
    // // step by step simulation of each time slot
    // bool all_done = false;
    // for (int current_time = 0; !all_done; current_time++)
    // {
    //     // welcome newly arrived customers
    //     while (!arrival_events.empty() && (current_time == arrival_events[0].event_time))
    //     {
    //         queue.push_back(arrival_events[0].customer_id);
    //         arrival_events.pop_front();
    //     }
    //     // check if we need to take a customer off the machine
    //     if (ganttchart[current_time] >= 0)
    //     {
    //         if (current_time == time_out)
    //         {
    //             int last_run = current_time - customers[current_id].playing_since;
    //             customers[current_id].slots_remaining -= last_run;
    //             if (customers[current_id].slots_remaining > 0)
    //             {
    //                 // customer is not done yet, waiting for the next chance to play
    //                 queue.push_back(current_id);
    //             }
    //             current_id = -1; // the machine is free now
    //         }
    //     }
    //     // if machine is empty, schedule a new customer
    //     if (ganttchart[current_time] == -1)
    //     {
    //         if (!queue.empty()) // is anyone waiting?
    //         {
    //             current_id = queue.front();
    //             queue.pop_front();
    //             if (TIME_ALLOWANCE > customers[current_id].slots_remaining)
    //             {
    //                 time_out = current_time + customers[current_id].slots_remaining;
    //             }
    //             else
    //             {
    //                 time_out = current_time + TIME_ALLOWANCE;
    //             }
    //             customers[current_id].playing_since = current_time;
    //         }
    //     }
    for (int i = 0; i <= chart_length; i ++){
      out_file << i << " " << ganttchart[i] << '\n';
      //print_state(out_file, i, ganttchart[i], arrival_events, queue);
    }
        //print_state(out_file, current_time, current_id, arrival_events, queue);

        // exit loop when there are no new arrivals, no waiting and no playing customers
        //all_done = (arrival_events.empty() && queue.empty() && (current_id == -1));


    return 0;
}
