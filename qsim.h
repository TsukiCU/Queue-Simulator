#ifndef MMC_SIMULATOR_H
#define MMC_SIMULATOR_H

#include <queue>
#include <memory>
#include <random>
#include <vector>

enum class EventType { ARRIVAL, DEPARTURE };

struct Event {
    double time;
    EventType type;

    bool operator<(const Event& other) const {
        // For min-heap priority queue
        return time > other.time;
    }
};

// High level abstraction for queues.
class IQueueSimulator {
public:
    virtual void runSimulation(int totalCustomers) = 0;
    virtual void displayResults() const = 0;
    virtual ~IQueueSimulator() = default;
};

#endif // MMC_SIMULATOR_H