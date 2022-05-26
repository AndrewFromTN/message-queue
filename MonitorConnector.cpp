#include <iostream>
#include <string>
#include <deque>
#include <unistd.h>
#include <time.h>
#include "zmq.hpp"

int adapter_csv_to_plot(char* line, std::string& converted);

int main (int argc, char** argv)
{
    std::deque<std::string> local_queue;

    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_SUB);
    socket.connect("tcp://127.0.0.1:5557");
    int connected = zmq_setsockopt((void*)socket, ZMQ_SUBSCRIBE, "MSG_MEMSTAT", 11);

    assert(connected == 0);

    int pip[2];
    int result = pipe(pip);

    int buffer_size = 200;

    char* buffer = new char[buffer_size];

    for (int i = 0; i < buffer_size; ++i)
    {
        buffer[i] = '\0';
    }

    if (result == -1)
    {
        puts("Failed to create the pipe, quiting.");
        return -1;
    }

    pid_t pid = fork();
    if (pid != 0)
    {
        close(pip[0]);

        write(pip[1], (void*)"set term x11\n", 13);

        fsync(pip[1]);

        std::string write_buff("plot \"-\" with linespoints\n");
        while (true)
        {
            socket.recv(buffer, buffer_size, 0);

            std::string converted;
            int size = adapter_csv_to_plot(buffer, converted);

            local_queue.push_back(converted);

            if (local_queue.size() > 20)
            {
                local_queue.pop_front();
            }

            write(pip[1], (void*)write_buff.c_str(), write_buff.length());

            std::for_each(local_queue.begin(), local_queue.end(), [&](std::string str)
            {
                write(pip[1], (void*)str.c_str(), str.length());
                write(pip[1], "\n", 1);
            });

            write(pip[1], (void*)"e\n", 2);
            fsync(pip[1]);
        }
    }
    else
    {
        close(pip[1]);

        dup2(pip[0], 0);

        close(pip[0]);

        char *const params[] = { "/usr/bin/gnuplot", NULL };
        execv("/usr/bin/gnuplot", params);

        puts("execv failed, exiting.");
        return -1;
    }

    return 0;
}

int adapter_csv_to_plot(char* line, std::string& converted)
{
    std::string str(line);
    str.erase(0,12); // Remove MSG_MEMSTAT,
;
    std::string epoch;
    std::string free_mem;

    while (str[0] != ',')
    {
        epoch += str[0];
        str.erase(0, 1);
    }
    
    str.erase(0, 1); // Erase the comma

    std::time_t t = atoi(epoch.c_str());
    struct tm* time = std::gmtime(&t);

    int day_epoch = time->tm_hour * 3600 + time->tm_min * 60 + time->tm_sec;

    converted.append(std::to_string(day_epoch));
    converted.append(" ");

    int i = 0;
    while (str[i] != '\n')
    {
        converted += str[i];
        ++i;
    }

    converted.append("\n0");

    return converted.length();
}