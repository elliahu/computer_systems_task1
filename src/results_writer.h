#pragma once
#include <map>
#include <string>
#include <fstream>
#include <stdexcept>
#

namespace Results
{
    typedef std::map<int, double> ResultsTable;

    class ResultsWriter
    {
    public:
        static void writeResultsCsv(ResultsTable table, std::string filename)
        {
            std::ofstream csvFile(filename);

            if (!csvFile.is_open())
            {
                throw std::runtime_error("Unable to open file '1znaky.csv'");
            }

            csvFile << "FORKS,TIME\n";

            for (const auto &pair : table)
            {
                csvFile << pair.first << "," << pair.second << "\n";
            }
            
            csvFile.close();
        }
    };
}
