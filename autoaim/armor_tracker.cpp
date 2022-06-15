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
    hit_score = 0;
    history_info.push_back(src);
    calcTargetScore();
}

//TODO:改变目标距中心点计算方式,可以考虑通过相机内参,xyz偏移量,yaw,pitch偏移量计算出目前瞄准点
/**
 * @brief 计算该Tracker作为目标的可能性分数
 * 
 * @return true 
 * @return false 
 */
bool ArmorTracker::calcTargetScore()
{
    vector<Point2f> points;
    float rotate_angle;
    auto horizonal_dist_to_center = abs(last_armor.center2d.x - 640);

    for(int i = 0; i < 4; i++)
    {
        points.push_back(last_armor.apex2d[i]);
    }
    RotatedRect rotated_rect = minAreaRect(points);
    //调整角度至0-90度(越水平角度越小)
    if (rotated_rect.size.width > rotated_rect.size.height)
        rotate_angle = rotated_rect.angle;
    else
        rotate_angle = 90 - rotated_rect.angle;
    
    //计算分数
    hit_score = (1.0 / rotate_angle) * 1.5 + (20.0 / horizonal_dist_to_center) * 0.92 + (1 - 1.0 / last_armor.area) * 0.4;
    // cout << "hit_socre: " << hit_score << endl;
    return true;
}

/**
 * @brief 更新Tracker信息
 * 
 * @param new_armor 该Tracker最新装甲板位置
 * @param new_timestamp 目前时间戳 
 * @return true 
 * @return false 
 */
bool ArmorTracker::update(Armor new_armor, int new_timestamp)
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

    calcTargetScore();
    
    return true;
}