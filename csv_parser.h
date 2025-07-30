#pragma once

#include "orderbook.h"
#include <vector>
#include <string>
#include <fstream>

class CSVParser
{
private:
    std::string trim(const std::string &str);
    std::vector<std::string> split(const std::string &line, char delimiter);

public:
    CSVParser();
    ~CSVParser();

    std::vector<MBOAction> parseCSV(const std::string &filename);
    void writeMBP(const std::string &filename, const std::vector<std::pair<uint64_t, std::vector<MBPLevel>>> &bid_snapshots,
                  const std::vector<std::pair<uint64_t, std::vector<MBPLevel>>> &ask_snapshots);
};
