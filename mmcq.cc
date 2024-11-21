#include "mmcq.h"
#include <iostream>
#include <cmath>

MMCQueue::MMCQueue(double arrivalRate, double serviceRate, int numServers, double warmup)
    : lambda(arrivalRate), mu(serviceRate), currentTime(0.0),
      servers(numServers), busyServers(0), totalQueueLengthTime(0.0),
      serverBusyTime(0.0), totalResponseTime(0.0), totalWaitingTime(0.0),
      lastEventTime(0.0), totalCustomersServed(0), warmUpTime(warmup), warmUpComplete(false),
      arrivalDistribution(arrivalRate), serviceDistribution(serviceRate)
{
    if (numServers <= 0) {
        exit = true;
        std::cerr << "Server number doesn't make sense. Exiting.. " << std::endl;
    }
    else if (arrivalRate >= serviceRate * numServers) {
        exit = true;
        std::cerr << "Can't deal with that, can we? Exiting.. " << std::endl;
    }
    else if (warmUpTime <= 0) {
        exit = true;
        std::cerr << "Gonna need a nice warm up time. Exiting.. " << std::endl;
    }
}

MMCQueue::~MMCQueue() { 
    {
        std::unique_lock<std::mutex> lock(mtx);
        stop = true;
    }
    cv.notify_all();
    for (auto& worker : workers) {
        worker.join();
    }
}

void MMCQueue::runSimulation(int totalCustomers) {
    if (exit)
        return;
    // First event upon arriavl
    double firstArrivalTime = arrivalDistribution(generator);
    eventQueue.push({firstArrivalTime, EventType::ARRIVAL});

    for (int i = 0; i < servers; ++i) {
        workers.emplace_back([this, totalCustomers]()
        {
            for(;;) 
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [this]() { return !eventQueue.empty() || stop; });

                // stop if notified by other threads.
                if (stop)
                    break;

                // Update some statistical stuff.
                double timeSinceLastEvent = currentTime - lastEventTime;
                lastEventTime = currentTime;
                if (warmUpComplete) {
                    totalQueueLengthTime += customerQueue.size() * timeSinceLastEvent;
                    serverBusyTime += busyServers * timeSinceLastEvent;
                }

                if (!eventQueue.empty()) {
                    Event event = eventQueue.top();
                    eventQueue.pop();
                    currentTime = event.time;

                    // Why unlock then lock?
                    // 1. prevent deadlock if shared resource is used in handleArrival() or handleDeparture().
                    // 2. decouple event processing with shared resource protection.
                    lock.unlock();

                    if (event.type == EventType::ARRIVAL) {
                        handleArrival();
                    } else if (event.type == EventType::DEPARTURE) {
                        handleDeparture();
                    }

                    lock.lock();
                } else {
                    // cv will be checked again upon wakeup, that's why this "else" is necessary here.
                    // in case of spurious wakeup, release the lock so that other threads may proceed.
                    lock.unlock();
                }

                // Are all the customers served?
                if (totalCustomersServed >= totalCustomers) {
                    stop = true;
                    cv.notify_all();
                }

                // if warm up stage is done, clear everything and start over.
                if (!warmUpComplete && currentTime >= warmUpTime) {
                    warmUpComplete = true;
                    totalQueueLengthTime = 0.0;
                    serverBusyTime = 0.0;
                    totalResponseTime = 0.0;
                    totalWaitingTime = 0.0;
                    totalCustomersServed = 0;
                }
            }
        });
    }

    // Main thread waiting all customers to be served.
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this, totalCustomers]() { return totalCustomersServed >= totalCustomers; });
        stop = true;
        cv.notify_all(); // Actually not quite necessary.
    }
}

void MMCQueue::handleArrival() {
    std::unique_lock<std::mutex> lock(mtx);
    customerQueue.push(currentTime);

    double nextArrivalTime = currentTime + arrivalDistribution(generator);
    eventQueue.push({nextArrivalTime, EventType::ARRIVAL});

    // If there is an idle server start right away.
    if (busyServers < servers) {
        // QUEUE_LOG(LOG_INFO, "Begin serving a customer in process %d .", m_id);
        beginService();
    }
}
void MMCQueue::handleDeparture() {
    std::unique_lock<std::mutex> lock(mtx);

    busyServers--;
    totalCustomersServed++;

    if (!customerQueue.empty()) {
        double arrivalTime = customerQueue.front();
        customerQueue.pop();

        double responseTime = currentTime - arrivalTime;
        totalResponseTime += responseTime;

        if (busyServers < servers) {
            beginService();
        }
    }

    cv.notify_all();
}

void MMCQueue::beginService() {
    busyServers++;
    double serviceTime = serviceDistribution(generator);
    eventQueue.push({currentTime + serviceTime, EventType::DEPARTURE});
    cv.notify_all(); // Prevent thread from starving.
}

void MMCQueue::displayResults() const {
    if (exit)
        return;
    // Unlikely, but
    if (!warmUpComplete) {
        std::cerr << "The system hasn't finished warming up. " << std::endl;
        return;
    }

    // currentTime is total elapsed time.
    double averageQueueLength = totalQueueLengthTime / currentTime;
    double serverUtilization = serverBusyTime / (currentTime * servers);
    double averageResponseTime = totalResponseTime / totalCustomersServed;

    std::cout << "Simulation Results for M/M/" << servers << ":\n";
    std::cout << "Average queue length: " << averageQueueLength << "\n";
    std::cout << "Server utilization: " << serverUtilization << "\n";
    std::cout << "Average response time: " << averageResponseTime << " units\n";

    // Visualization fro the result.
    #if ENABLE_PLOT
    std::vector<int> queueLengths;  // Collect queue length data for histogram

    // std::queue doesn't support for loop...
    std::queue<double> tempQueue = customerQueue;
    while (!tempQueue.empty()) {
        queueLengths.push_back(tempQueue.size());
        tempQueue.pop();
    }

    // plt hist
    plt::hist(queueLengths, 10);  // 10 bins
    plt::title("Queue Length Distribution");
    plt::xlabel("Queue Length");
    plt::ylabel("Frequency");
    plt::show();

    std::vector<double> time;
    std::vector<double> avgQueueLengths;
    double cumulativeQueueLength = 0;
    for (size_t i = 0; i < queueLengths.size(); ++i) {
        time.push_back(i);
        cumulativeQueueLength += queueLengths[i];
        avgQueueLengths.push_back(cumulativeQueueLength / (i + 1));
    }

    plt::plot(time, avgQueueLengths);
    plt::title("Average Queue Length Over Time");
    plt::xlabel("Time");
    plt::ylabel("Average Queue Length");
    plt::save("result.png");

    std::cout << "\nResult saved to the current working directory." << std::endl;
    #endif
}