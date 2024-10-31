#include <iostream>
#include <chrono>

#define NO_PYTHON 0 // Set to 0 to ENABLE python and generate plot with results
#if NO_PYTHON == 0
#include <Python.h> // for matplotlib and pandas
#endif

#include "hasher.h"
#include "generator.h"
#include "load_balancer.h"
#include "results_writer.h"

int main()
{
    Hashing::Md5 md5{};
    size_t max_size = 5;
    int max_num_forks = 80;
    std::string charset = "abcdefghijklmnopqrstuvwxyz";

    for (size_t size = 1; size <= max_size; size++)
    {
        size_t total = Generators::StringGenerator::totalCombinations(size, charset);
        Results::ResultsTable table{};
        
        std::cout << "### Size: " << size << " Total number of combinations: " << total << std::endl;

        for (int num_forks = 1; num_forks <= max_num_forks; num_forks++)
        {
            try
            {
                auto start_time = std::chrono::high_resolution_clock::now();

                {
                    LoadBalancing::LoadBalancer lb(num_forks, [&md5, size, &charset, num_forks, total, max_num_forks](int fork_id)
                                                   {
                        pid_t pid = getpid();
                        size_t start_idx = (total / num_forks) * fork_id;
                        size_t end_idx = (fork_id + 1 == num_forks) ? total - 1 : start_idx + (total / num_forks) - 1;
                        
                        Generators::StringGenerator generator(
                            size, charset,
                            Generators::StringGenerator::numberToString(size, start_idx, charset),
                            Generators::StringGenerator::numberToString(size, end_idx, charset));

                        std::cout << "Child process PID:" << pid << " started working at index " << start_idx << std::endl;

                        while (generator.hasNext())
                        {
                            std::string s = generator.next();
                            //std::cout << s << " " << md5.hash(s) << std::endl;
                        }

                        std::cout << "Child process PID: " << pid << " finished work at index " << end_idx << std::endl; });

                    // Fork child processes
                    lb.start();

                } // Main process is blocked here until all children are finished

                auto end_time = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> duration = end_time - start_time;
                table[num_forks] = duration.count();

                std::cout << "All child processes have completed in " << duration.count() << " ms" << std::endl;
            }
            catch (const std::exception &ex)
            {
                std::cerr << "Error: " << ex.what() << std::endl;
            }
        }

        try
        {
            Results::ResultsWriter::writeResultsCsv(table, "data/" + std::to_string(size) + "znaky.csv");
        }
        catch (const std::runtime_error &e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    // Create a results plot
    #if NO_PYTHON == 0
    {
        // Initialize the Python interpreter
        Py_Initialize();
        FILE* file = fopen("src/plot.py", "r");
        if (file != nullptr) {
            // Run the Python script file
            PyRun_SimpleFile(file, "src/plot.py");
            fclose(file);
        }

        // Finalize the Python interpreter
        Py_Finalize();
    }
    #else
    {
        std::cout << "Python was not enabled. No plot was generated." << std::endl;
    }
    #endif

    return 0;
}