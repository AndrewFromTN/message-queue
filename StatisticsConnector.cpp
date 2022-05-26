#include <iostream>
#include <string>
#include <unistd.h>
#include <regex>
#include "zmq.hpp"

int vmstat_to_csv(char* line, std::string& converted);

int main (int argc, char** argv)
{
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_PUB);
    socket.connect("tcp://127.0.0.1:5556");

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
        close(pip[1]);

        while (true)
        {
            read(pip[0], buffer, 1);

            if (buffer[0] == '\n')
            {
                buffer[0] = '\0';
                break;
            }
        }

        while (true)
        {
            int count = 0;
            while (count < 2)
            {
                read(pip[0], buffer, 1);

                if (buffer[0] == '\n')
                {
                    ++count;
                    buffer[0] = '\0';
                }
            }

            size_t num_read = read(pip[0], buffer, buffer_size);

            /*if (num_read == buffer_size)
            {
                char* tmp = buffer;

                buffer_size *= 2;

                buffer = new char[buffer_size];

                for (int i = 0; i < num_read; ++i)
                {
                    buffer[i] = tmp[i];
                }

                delete[] tmp;
                tmp = nullptr;
            }*/

            int start = 0;
            for (int i = 0; i < num_read; ++i)
            {
                if (buffer[i] == '\n')
                {
                    char* tmp = new char[i - start];
                    strncpy(tmp, buffer + start, i - start);

                    std::string converted;
                    size_t len = vmstat_to_csv(tmp, converted);

                    socket.send(converted.c_str(), len, 0);

                    delete[] tmp;
                    tmp = nullptr;

                    start = i + 1;
                }
            }
        }

        delete[] buffer;
        buffer = nullptr;
    } 
    else
    {
        close(pip[0]);

        dup2(pip[1], 1);

        close(pip[1]);

        char *const parameters[] = {"/usr/bin/vmstat", "2", "-n", NULL};
        execv("/usr/bin/vmstat", parameters);

        puts("execv failed");
        
        return -1;
    }

    return 0;
}

int vmstat_to_csv(char* line, std::string& converted)
{
    int epoch_time = time(NULL);
    std::string mem_amount;

    std::regex free_mem("\\d+\\s+\\d+\\s+\\d+\\s+\\d+");
    std::smatch match;

    const std::string str(line);
    if (std::regex_search(str.begin(), str.end(), match, free_mem))
    {
        std::string parsed_line = match[0].str();
        int i = parsed_line.length() - 1;
        while (parsed_line[i] != ' ')
        {
            mem_amount.insert(mem_amount.begin(), parsed_line[i]);

            --i;
        }
    }

    converted = "MSG_MEMSTAT" + std::to_string(epoch_time) + "," + mem_amount + "\n0";

    return converted.length();
}