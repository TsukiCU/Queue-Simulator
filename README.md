## Queue-Simulator

This is a M/M/c simulator written in C++. The model is designed to adapt M/M/C, with different numbers of servers by utilizing multithreading, as long as modern C++ features such as mutex locks, condition variables and smart pointers to avoid data race.
The code was fully tested under **Ubuntu 22.04 (kernel 5.15.0)** , **ARM64** architecture , using **GCC** and **Clang**.

### Features

1.	Multithreading:
	•	Support servers of any number while each is simulated as an independent thread.
	•	Synchronization is ensured by using mutex locks and condition variables.
2.	Event-Driven Simulation:
	•	The simulation operates in an event-driven manner, as arrivals and departures are treated as *events*, and are processed in chronological order.
	•	This approach is efficient and mirrors real-world queueing systems.
3. Warm up stage
	•	Add warm up stage to stabalise the results.
4.	Visualization (Optional):
	•	Generate histograms of queue lengths and plots of average queue lengths over time (requires MatplotlibCPP).

### How to run

#### Compile
The code provides an option to visualize the results but needs you to install `matplotlibcpp` first, this can be done by: 

If you are on Linux(Ubuntu)
```
apt install python-dev, python-pip3
pip3 install matplotlib, nump
clone matplotlib project and place matplotlibcpp.h in the workplace.
```

If you find it annoying to do all this (which I can totally relate), the good new is by default, PLOT is set to 0, and the visualization function is disabled. You can still run the program successfully, and test all other functionalities by running : 

`make`

If you succefully install it, you can view the result by setting PLOT to 1.

`make plot`

Note: Ensure you are using **Python 3.10**, as the Makefile includes the `-lpython3.10` flag. or switch this flag manually to whichever versions of Python is installed on your machine, but it's not guaranteed that version conflicts won't happen.

#### Run

```./mmc <arrival rate> <process rate> <number of servers> <warm_up_time>```
Note, those values have to make sense. The program exits and does nothing if unreasonable inputs are received.

### Analysis

It is recommended to choose reasonable warm up period.
Tr : Response Time
Lq : Queue Length

| Arrival Rate | Process Rate | Server Number | Average Lq | Average Tr | Server Utilization |
|--------------|--------------|---------------|------------|------------|--------------------|
| 0.5          | 1.0          | 1             | 1.11137    |  1.43577	|     0.735526       |
| 0.5          | 1.0          | 2             | 0.600691   |  0.523404  | 	  0.568028	     |
| 0.7          | 1.0          | 1             | 2.2648     |  2.841971  |     0.856693       |
| 0.7          | 1.0          | 2             | 0.930956   |  0.769445  |	  0.654332		 |
| 0.9          | 1.0          | 1             | 7.99084    |  9.22552   |	  0.892315		 |
| 0.9          | 1.0          | 2             | 1.76244    |  1.41942   |	  0.717868       |

Observations

1.	Adding server number is good. It reduces queue length (Lq) and response time (Tr), e.g., for arrival rate 0.5, Tr drops from 1.44 (1 server) to 0.52 (2 servers).

2.	As arrival rates rise (e.g., from 0.5 to 0.9), Lq and Tr increase significantly, especially with fewer servers.

3.	Utilization decreases with more servers, but is oddly higher than theoratical value.

### Issues

Apparently, There are discrepencies in utilization rate that can't be ignored between theoratical values and test results. This can be due to:

1. Though carefully examined, there are still some bugs or logical errors in the code especially in multithreading context.

2. At some point, the servers are overloaded with customers.

### TODO

1. Provide a docker container to save the users from installing packages they might never need.
2. Support more models like M/M/c/k.
3. There could be performance bottleneck as the number of servers grow too high. Use techniques like thread pool to enhance performance and reduce data racing.