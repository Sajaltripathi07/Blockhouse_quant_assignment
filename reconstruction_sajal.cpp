#include "orderbook.h"
#include "csv_parser.h"
#include <iostream>
#include <vector>
#include <map>
#include <chrono>

class MBPReconstructor
{
private:
    OrderBook my_orderbook;
    CSVParser my_csv_parser;

    std::vector<std::pair<uint64_t, std::vector<MBPLevel>>> all_bid_snapshots;
    std::vector<std::pair<uint64_t, std::vector<MBPLevel>>> all_ask_snapshots;

    struct TradeInProgress
    {
        MBOAction trade_action;
        MBOAction fill_action;
        bool got_trade = false;
        bool got_fill = false;
    };

    std::map<uint64_t, TradeInProgress> trades_waiting_for_completion;

    void takeSnapshot(uint64_t timestamp)
    {
        auto bids = my_orderbook.getBidLevels(10);
        auto asks = my_orderbook.getAskLevels(10);

        all_bid_snapshots.emplace_back(timestamp, bids);
        all_ask_snapshots.emplace_back(timestamp, asks);
    }

    void processAction(const MBOAction &action)
    {
        switch (action.action)
        {
        case 'R':
            break;
        case 'A':
            my_orderbook.addOrder(action.side, action.price, action.size, action.order_id);
            takeSnapshot(action.timestamp);
            break;
        case 'C':
        {
            auto it = trades_waiting_for_completion.find(action.order_id);
            if (it != trades_waiting_for_completion.end())
            {
                auto &info = it->second;
                if (info.got_trade && info.got_fill)
                {
                    my_orderbook.processTradeSequence(info.trade_action, info.fill_action, action);
                    trades_waiting_for_completion.erase(it);
                    takeSnapshot(action.timestamp);
                    break;
                }
            }
            my_orderbook.cancelOrder(action.order_id);
            takeSnapshot(action.timestamp);
            break;
        }
        case 'T':
            if (action.side != 'N')
            {
                auto &info = trades_waiting_for_completion[action.order_id];
                info.trade_action = action;
                info.got_trade = true;
            }
            break;
        case 'F':
        {
            auto &info = trades_waiting_for_completion[action.order_id];
            info.fill_action = action;
            info.got_fill = true;
            break;
        }
        default:
            std::cerr << "Unknown action: " << action.action << std::endl;
            break;
        }
    }

public:
    // Main reconstruction function
    void reconstruct(const std::string &input_file, const std::string &output_file)
    {
        auto start_time = std::chrono::high_resolution_clock::now();

        auto actions = my_csv_parser.parseCSV(input_file);

        for (const auto &action : actions)
        {
            processAction(action);
        }

        my_csv_parser.writeMBP(output_file, all_bid_snapshots, all_ask_snapshots);

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);

        std::cout << "Reconstruction completed in " << duration.count() << " microseconds\n";
        std::cout << "Generated " << all_bid_snapshots.size() << " snapshots\n";
    }
};

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <input_mbo.csv>\n";
        return 1;
    }

    std::string input_file = argv[1];
    std::string output_file = "mbp_output.csv";

    try
    {
        MBPReconstructor reconstructor;
        reconstructor.reconstruct(input_file, output_file);
        std::cout << "Reconstruction successful!\n";
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
