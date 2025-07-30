#pragma once

#include <map>
#include <vector>
#include <string>
#include <cstdint>

struct Order
{
    double price;
    int64_t size;
    uint64_t order_id;

    Order() : price(0.0), size(0), order_id(0) {}
    Order(double p, int64_t s, uint64_t id) : price(p), size(s), order_id(id) {}
};

struct MBOAction
{
    uint64_t timestamp;
    char action; // R, A, T, F, C
    char side;   // B, A, N
    double price;
    int64_t size;
    uint64_t order_id;

    MBOAction() : timestamp(0), action(0), side(0), price(0.0), size(0), order_id(0) {}
};

struct MBPLevel
{
    double price;
    int64_t size;

    MBPLevel() : price(0.0), size(0) {}
    MBPLevel(double p, int64_t s) : price(p), size(s) {}
};

class OrderBook
{
private:
    // Use maps for efficient price-level operations
    // Key: price, Value: total size at that price
    std::map<double, int64_t, std::greater<double>> bids; // Descending order
    std::map<double, int64_t> asks;                       // Ascending order

    // Track individual orders for cancellations
    std::map<uint64_t, Order> orders;

public:
    OrderBook();
    ~OrderBook();

    void clear();
    void addOrder(char side, double price, int64_t size, uint64_t order_id);
    void cancelOrder(uint64_t order_id);
    void processTradeSequence(const MBOAction &trade, const MBOAction &fill, const MBOAction &cancel);

    std::vector<MBPLevel> getBidLevels(int max_levels = 10) const;
    std::vector<MBPLevel> getAskLevels(int max_levels = 10) const;

    void printBook() const; // For debugging
};
