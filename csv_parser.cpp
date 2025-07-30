#include "csv_parser.h"
#include <sstream>
#include <iostream>
#include <iomanip>

CSVParser::CSVParser() {}

CSVParser::~CSVParser() {}

std::string CSVParser::trim(const std::string &str)
{
    size_t first = str.find_first_not_of(' ');
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}

std::vector<std::string> CSVParser::split(const std::string &line, char delimiter)
{
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string token;

    while (std::getline(ss, token, delimiter))
    {
        tokens.push_back(trim(token));
    }

    return tokens;
}

std::vector<MBOAction> CSVParser::parseCSV(const std::string &filename)
{
    std::vector<MBOAction> actions;
    std::ifstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return actions;
    }

    std::string line;
    bool first_line = true;

    while (std::getline(file, line))
    {
        if (first_line)
        {
            first_line = false;
            continue; // Skip header
        }

        if (line.empty())
            continue;

        std::vector<std::string> tokens = split(line, ',');
        if (tokens.size() < 6)
            continue;

        MBOAction action;
        try
        {
            action.timestamp = std::stoull(tokens[0]);
            action.action = tokens[1][0];
            action.side = tokens[2][0];
            action.price = std::stod(tokens[3]);
            action.size = std::stoll(tokens[4]);
            action.order_id = std::stoull(tokens[5]);

            actions.push_back(action);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error parsing line: " << line << " - " << e.what() << std::endl;
            continue;
        }
    }

    file.close();
    return actions;
}

void CSVParser::writeMBP(const std::string &filename,
                         const std::vector<std::pair<uint64_t, std::vector<MBPLevel>>> &bid_snapshots,
                         const std::vector<std::pair<uint64_t, std::vector<MBPLevel>>> &ask_snapshots)
{
    std::ofstream file(filename);

    if (!file.is_open())
    {
        std::cerr << "Error: Could not create output file " << filename << std::endl;
        return;
    }

    // Write header
    file << "timestamp";
    for (int i = 1; i <= 10; i++)
    {
        file << ",bid_price_" << i << ",bid_size_" << i;
    }
    for (int i = 1; i <= 10; i++)
    {
        file << ",ask_price_" << i << ",ask_size_" << i;
    }
    file << std::endl;

    // Merge and sort snapshots by timestamp
    std::vector<std::pair<uint64_t, std::pair<std::vector<MBPLevel>, std::vector<MBPLevel>>>> all_snapshots;

    // Combine bid and ask snapshots
    size_t bid_idx = 0, ask_idx = 0;
    std::vector<MBPLevel> current_bids(10), current_asks(10);

    while (bid_idx < bid_snapshots.size() || ask_idx < ask_snapshots.size())
    {
        uint64_t next_timestamp = UINT64_MAX;
        bool use_bid = false, use_ask = false;

        if (bid_idx < bid_snapshots.size())
        {
            next_timestamp = std::min(next_timestamp, bid_snapshots[bid_idx].first);
        }
        if (ask_idx < ask_snapshots.size())
        {
            next_timestamp = std::min(next_timestamp, ask_snapshots[ask_idx].first);
        }

        if (bid_idx < bid_snapshots.size() && bid_snapshots[bid_idx].first == next_timestamp)
        {
            current_bids = bid_snapshots[bid_idx].second;
            bid_idx++;
            use_bid = true;
        }
        if (ask_idx < ask_snapshots.size() && ask_snapshots[ask_idx].first == next_timestamp)
        {
            current_asks = ask_snapshots[ask_idx].second;
            ask_idx++;
            use_ask = true;
        }

        if (use_bid || use_ask)
        {
            all_snapshots.emplace_back(next_timestamp, std::make_pair(current_bids, current_asks));
        }
    }

    // Write snapshots
    file << std::fixed << std::setprecision(2);
    for (const auto &snapshot : all_snapshots)
    {
        file << snapshot.first;

        // Write bid levels
        for (int i = 0; i < 10; i++)
        {
            const auto &level = snapshot.second.first[i];
            file << "," << level.price << "," << level.size;
        }

        // Write ask levels
        for (int i = 0; i < 10; i++)
        {
            const auto &level = snapshot.second.second[i];
            file << "," << level.price << "," << level.size;
        }

        file << std::endl;
    }

    file.close();
}
