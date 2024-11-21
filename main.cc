#include "mmcq.h"
#include <memory>
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <arrival_rate_lambda> <service_rate_mu> <num_servers> <warm_up_time>\n";
        return 1;
    }

    double lambda = atof(argv[1]);
    double mu = atof(argv[2]);
    int servers = atoi(argv[3]);
    double warm_up_time = atof(argv[4]);

    std::unique_ptr<IQueueSimulator> simulator = std::make_unique<MMCQueue>(lambda, mu, servers, warm_up_time);
    simulator->runSimulation(10000);  // simulate 10k people.
    simulator->displayResults();

    return 0;
}