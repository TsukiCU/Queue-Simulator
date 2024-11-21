#ifndef MMC_QUEUE_H
#define MMC_QUEUE_H

#include "qsim.h"
#include <condition_variable>
#include <mutex>
#include <thread>

#ifndef ENABLE_PLOT
#define ENABLE_PLOT 0
#endif

#if ENABLE_PLOT
#include "matplotlibcpp.h"
namespace plt = matplotlibcpp;
#endif

class MMCQueue : public IQueueSimulator {
public:
    MMCQueue(double arrivalRate, double serviceRate, int numServers, double warmup);
    ~MMCQueue();

    void runSimulation(int totalCustomers) override;
    void displayResults() const override;

private:
    double lambda, mu;
    double currentTime;
    int servers, busyServers;
    std::queue<double> customerQueue;       // customer q (arrival times)
    std::priority_queue<Event> eventQueue;  // priority q , chronological order.

    // Statistics stuff.
    double totalQueueLengthTime;
    double serverBusyTime;
    double totalResponseTime;
    double totalWaitingTime;
    double lastEventTime;
    int totalCustomersServed;

    // Warm up
    double warmUpTime;
    bool warmUpComplete;

    // Random number generators
    std::default_random_engine generator;
    std::exponential_distribution<double> arrivalDistribution;
    std::exponential_distribution<double> serviceDistribution;

    // Multithreading stuff
    std::mutex mtx;
    std::condition_variable cv;
    std::vector<std::thread> workers;
    bool stop = false;  // Condition variable as a flag to stop threads gracefully.
    bool exit = false;  // There is something wrong and the simulation should not continue.

    // Private methods
    void handleArrival();
    void handleDeparture();
    void beginService();

    // plot
    std::vector<int> queueLengths;
};

#endif // MMC_QUEUE_H