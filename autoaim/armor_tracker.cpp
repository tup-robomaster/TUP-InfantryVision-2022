#include "armor_tracker.h"
/**
 * @brief 构造一个ArmorTracker对象
 * 
 * @param src Armor对象
 */
ArmorTracker::ArmorTracker(Armor src, int src_timestamp)
{
    last_armor = src;
    last_timestamp = src_timestamp;
    is_initialized = false;

    history_info.push_back(src);
}

bool ArmorTracker::update(Armor new_armor,int new_timestamp)
{
    if (history_info.size() <= max_history_len)
    {
        history_info.push_back(new_armor);
    }
    else
    {
        history_info.pop_front();
        history_info.push_back(new_armor);
    }

    is_initialized = true;
    prev_armor = last_armor;
    prev_timestamp = last_timestamp;
    
    last_armor = new_armor;
    last_timestamp = new_timestamp;

    return true;
}