#include "fan_tracker.h"

/**
 * @brief 构造一个ArmorTracker对象
 * 
 * @param src Armor对象
 */
FanTracker::FanTracker(Fan src, int src_timestamp)
{
    last_fan = src;
    last_timestamp = src_timestamp;
    is_initialized = false;
    is_last_fan_exists = false;
    history_info.push_back(src);
}

bool FanTracker::update(Fan new_fan,int new_timestamp)
{
    is_last_fan_exists = true;
    if (history_info.size() < max_history_len)
    {
        history_info.push_back(new_fan);
    }
    else
    {
        is_initialized = true;
        history_info.pop_front();
        history_info.push_back(new_fan);
    }

    prev_fan = last_fan;
    prev_timestamp = last_timestamp;
    
    last_fan = new_fan;
    last_timestamp = new_timestamp;

    return true;
}