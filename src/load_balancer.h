#pragma once

#include <unistd.h>    
#include <sys/wait.h>   
#include <vector>
#include <functional>
#include <stdexcept>
#include <iostream>
#include <cstring>      

namespace LoadBalancing
{
    class LoadBalancer
    {
    public:
        LoadBalancer(int num_forks, std::function<void(int)> task)
            : num_forks_(num_forks), task_(task)
        {
            if (num_forks_ < 1)
            {
                throw std::invalid_argument("Number of forks must be at least 1.");
            }

            if (!task_)
            {
                throw std::invalid_argument("Task function must be valid.");
            }
        }

        void start()
        {
            for (int i = 0; i < num_forks_; ++i)
            {
                pid_t pid = fork();

                if (pid < 0)
                {
                    // Fork failed
                    throw std::runtime_error(std::string("Fork failed: ") + strerror(errno));
                }
                else if (pid == 0)
                {
                    // Child process
                    try
                    {
                        task_(i);
                    }
                    catch (const std::exception& ex)
                    {
                        std::cerr << "Exception in child process: " << ex.what() << std::endl;
                    }
                    catch (...)
                    {
                        std::cerr << "Unknown exception in child process." << std::endl;
                    }
                    _exit(0); 
                }
                else
                {
                    // Parent process
                    child_pids_.push_back(pid);
                }
            }
        }

        // This function blocks until all child processes have terminated.
        void waitForChildren()
        {
            for (pid_t pid : child_pids_)
            {
                int status;
                pid_t result = waitpid(pid, &status, 0);
                if (result == -1)
                {
                    std::cerr << "Error waiting for child process " << pid << ": " << strerror(errno) << std::endl;
                }
                else
                {
                    if (WIFEXITED(status)) // Exited
                    {
                        int exit_status = WEXITSTATUS(status);
                        std::cout << "Child process " << pid << " exited with status " << exit_status << "." << std::endl;
                    }
                    else if (WIFSIGNALED(status)) // Terminated
                    {
                        int term_signal = WTERMSIG(status);
                        std::cout << "Child process " << pid << " terminated by signal " << term_signal << "." << std::endl;
                    }
                    else // Different reason
                    {
                        std::cout << "Child process " << pid << " terminated abnormally." << std::endl;
                    }
                }
            }
        }

        ~LoadBalancer()
        {
            // Check if there are any unwaited child processes
            if (!child_pids_.empty())
            {
                waitForChildren();
            }
        }

    private:
        int num_forks_;                             
        std::function<void(int)> task_;                 
        std::vector<pid_t> child_pids_;              
    };
}
