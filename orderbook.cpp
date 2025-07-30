#include "orderbook.h"
#include <iostream>
#include <algorithm>

OrderBook::OrderBook()
{
    clear();
}

OrderBook::~OrderBook()
{
    clear();
}

void OrderBook::clear()
{
    bids.clear();
    asks.clear();
    orders.clear();
}

void OrderBook::addOrder(char side, double price, int64_t size, uint64_t order_id)
{
    if (size <= 0)
        return;

    // Store the order
    orders[order_id] = Order(price, size, order_id);

    if (side == 'B')
    {
        bids[price] += size;
    }
    else if (side == 'A')
    {
        asks[price] += size;
    }
}

void OrderBook::cancelOrder(uint64_t order_id)
{
    auto it = orders.find(order_id);
    if (it == orders.end())
        return;

    const Order &order = it->second;
    double price = order.price;
    int64_t size = order.size;

    if (bids.count(price))
    {
        bids[price] -= size;
        if (bids[price] <= 0)
            bids.erase(price);
    }
    else if (asks.count(price))
    {
        asks[price] -= size;
        if (asks[price] <= 0)
            asks.erase(price);
    }

    orders.erase(it);
}

void OrderBook::processTradeSequence(const MBOAction &trade, const MBOAction &fill, const MBOAction &cancel)
{
    auto it = orders.find(cancel.order_id);
    if (it == orders.end())
        return;

    Order &order = it->second;
    double price = order.price;
    int64_t trade_size = trade.size;
    char actual_side = cancel.side;

    if (actual_side == 'B')
    {
        if (bids.count(price))
        {
            bids[price] -= trade_size;
            if (bids[price] <= 0)
                bids.erase(price);
        }
    }
    else if (actual_side == 'A')
    {
        if (asks.count(price))
        {
            asks[price] -= trade_size;
            if (asks[price] <= 0)
                asks.erase(price);
        }
    }

    // Update or erase order
    if (order.size <= trade_size)
    {
        orders.erase(it);
    }
    else
    {
        order.size -= trade_size;
    }
}

std::vector<MBPLevel> OrderBook::getBidLevels(int max_levels) const
{
    std::vector<MBPLevel> levels;
    levels.reserve(max_levels);

    int count = 0;
    for (auto it = bids.rbegin(); it != bids.rend() && count < max_levels; ++it)
    {
        if (it->second > 0)
        {
            levels.emplace_back(it->first, it->second);
            count++;
        }
    }

    while (levels.size() < max_levels)
    {
        levels.emplace_back(0.0, 0);
    }

    return levels;
}

std::vector<MBPLevel> OrderBook::getAskLevels(int max_levels) const
{
    std::vector<MBPLevel> levels;
    levels.reserve(max_levels);

    int count = 0;
    for (auto it = asks.begin(); it != asks.end() && count < max_levels; ++it)
    {
        if (it->second > 0)
        {
            levels.emplace_back(it->first, it->second);
            count++;
        }
    }

    while (levels.size() < max_levels)
    {
        levels.emplace_back(0.0, 0);
    }

    return levels;
}

void OrderBook::printBook() const
{
    std::cout << "=== ORDER BOOK ===" << std::endl;
    std::cout << "BIDS:" << std::endl;
    for (auto it = bids.rbegin(); it != bids.rend(); ++it)
    {
        std::cout << "  " << it->first << " : " << it->second << std::endl;
    }

    std::cout << "ASKS:" << std::endl;
    for (const auto &level : asks)
    {
        std::cout << "  " << level.first << " : " << level.second << std::endl;
    }

    std::cout << "==================" << std::endl;
}
