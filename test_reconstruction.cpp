#include "orderbook.h"
#include "csv_parser.h"
#include <iostream>
#include <cassert>
#include <chrono>
#include <fstream>
#include <cmath>

class TestSuite
{
private:
    int tests_passed = 0;
    int tests_failed = 0;

    void assert_equal(double expected, double actual, const std::string &test_name)
    {
        if (std::abs(expected - actual) < 1e-6)
        {
            std::cout << "[PASS] " << test_name << std::endl;
            tests_passed++;
        }
        else
        {
            std::cout << "[FAIL] " << test_name << " - Expected: " << expected << ", Got: " << actual << std::endl;
            tests_failed++;
        }
    }

    void assert_equal(int64_t expected, int64_t actual, const std::string &test_name)
    {
        if (expected == actual)
        {
            std::cout << "[PASS] " << test_name << std::endl;
            tests_passed++;
        }
        else
        {
            std::cout << "[FAIL] " << test_name << " - Expected: " << expected << ", Got: " << actual << std::endl;
            tests_failed++;
        }
    }

    void assert_equal(char expected, char actual, const std::string &test_name)
    {
        if (expected == actual)
        {
            std::cout << "[PASS] " << test_name << std::endl;
            tests_passed++;
        }
        else
        {
            std::cout << "[FAIL] " << test_name << " - Expected: '" << expected << "', Got: '" << actual << "'" << std::endl;
            tests_failed++;
        }
    }

public:
    void test_orderbook_basic()
    {
        std::cout << "\n=== Testing Basic OrderBook Operations ===" << std::endl;

        OrderBook book;
        book.addOrder('B', 99.50, 100, 1001);
        book.addOrder('A', 100.50, 200, 1002);
        book.addOrder('B', 99.45, 150, 1003);

        auto bids = book.getBidLevels(10);
        auto asks = book.getAskLevels(10);

        assert_equal(99.50, bids[0].price, "First bid price");
        assert_equal(100, bids[0].size, "First bid size");
        assert_equal(99.45, bids[1].price, "Second bid price");
        assert_equal(150, bids[1].size, "Second bid size");

        assert_equal(100.50, asks[0].price, "First ask price");
        assert_equal(200, asks[0].size, "First ask size");

        book.cancelOrder(1001);
        bids = book.getBidLevels(10);
        assert_equal(99.45, bids[0].price, "Bid price after cancellation");
        assert_equal(150, bids[0].size, "Bid size after cancellation");
    }

    void test_trade_sequence()
    {
        std::cout << "\n=== Testing Trade Sequence (T->F->C) ===" << std::endl;

        OrderBook book;
        book.addOrder('A', 100.50, 200, 1002);

        MBOAction trade, fill, cancel;
        trade.action = 'T';
        trade.side = 'A';
        trade.price = 100.50;
        trade.size = 50;
        trade.order_id = 0;

        fill.action = 'F';
        fill.side = 'A';
        fill.price = 100.50;
        fill.size = 50;
        fill.order_id = 1002;

        cancel.action = 'C';
        cancel.side = 'A';
        cancel.price = 100.50;
        cancel.size = 50;
        cancel.order_id = 1002;

        book.processTradeSequence(trade, fill, cancel);

        auto asks = book.getAskLevels(10);
        assert_equal(100.50, asks[0].price, "Ask price after trade");
        assert_equal(150, asks[0].size, "Ask size after trade (200-50)");
    }

    void test_csv_parsing()
    {
        std::cout << "\n=== Testing CSV Parsing ===" << std::endl;

        std::ofstream test_file("test_input.csv");
        test_file << "timestamp,action,side,price,size,order_id\n";
        test_file << "1640995200000000000,R,N,0,0,0\n";
        test_file << "1640995200100000000,A,B,99.50,100,1001\n";
        test_file << "1640995200200000000,A,A,100.50,200,1002\n";
        test_file.close();

        CSVParser parser;
        auto actions = parser.parseCSV("test_input.csv");

        assert_equal(3, static_cast<int64_t>(actions.size()), "Number of parsed actions");
        assert_equal('R', actions[0].action, "First action type");
        assert_equal('A', actions[1].action, "Second action type");
        assert_equal(99.50, actions[1].price, "Second action price");
        assert_equal(100, actions[1].size, "Second action size");
    }

    void test_performance()
    {
        std::cout << "\n=== Testing Performance ===" << std::endl;

        OrderBook book;
        auto start = std::chrono::high_resolution_clock::now();

        const int num_operations = 100000;
        for (int i = 0; i < num_operations; i++)
        {
            double price = 100.0 + (i % 100) * 0.01;
            book.addOrder('B', price, 100, i);
            if (i >= 5 && i % 10 == 0)
            {
                book.cancelOrder(i - 5);
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        double ops_per_sec = num_operations * 1e6 / duration.count();

        std::cout << "Processed " << num_operations << " ops in " << duration.count() << " µs" << std::endl;
        std::cout << "Rate: " << ops_per_sec << " ops/sec" << std::endl;

        if (ops_per_sec > 10000)
        {
            std::cout << "[PASS] Performance test - " << ops_per_sec << " ops/sec" << std::endl;
            tests_passed++;
        }
        else
        {
            std::cout << "[FAIL] Performance test - Only " << ops_per_sec << " ops/sec" << std::endl;
            tests_failed++;
        }
    }

    void test_mbp_levels()
    {
        std::cout << "\n=== Testing MBP Level Generation ===" << std::endl;

        OrderBook book;
        book.addOrder('B', 99.50, 100, 1001);
        book.addOrder('B', 99.50, 50, 1002);
        book.addOrder('B', 99.45, 200, 1003);

        auto bids = book.getBidLevels(10);

        assert_equal(99.50, bids[0].price, "Aggregated price level");
        assert_equal(150, bids[0].size, "Aggregated size (100+50)");
        assert_equal(99.45, bids[1].price, "Second price level");
        assert_equal(200, bids[1].size, "Second size level");

        for (int i = 2; i < 10; i++)
        {
            assert_equal(0.0, bids[i].price, "Zero-padded price level " + std::to_string(i));
            assert_equal(0, bids[i].size, "Zero-padded size level " + std::to_string(i));
        }
    }
    void run_all_tests()
    {
        std::cout << "Starting MBP-10 Reconstruction Test Suite" << std::endl;
        std::cout << "=========================================" << std::endl;

        test_orderbook_basic();
        test_trade_sequence();
        test_csv_parsing();
        test_mbp_levels();
        test_performance();

        std::cout << "\n=== Test Results ===" << std::endl;
        std::cout << "Tests Passed: " << tests_passed << std::endl;
        std::cout << "Tests Failed: " << tests_failed << std::endl;
        std::cout << "Success Rate: " << (100.0 * tests_passed / (tests_passed + tests_failed)) << "%" << std::endl;

        if (tests_failed == 0)
        {
            std::cout << "All tests passed! ✓" << std::endl;
        }
        else
        {
            std::cout << "Some tests failed! ✗" << std::endl;
        }
    }
};

int main()
{
    TestSuite suite;
    suite.run_all_tests();
    return 0;
}
