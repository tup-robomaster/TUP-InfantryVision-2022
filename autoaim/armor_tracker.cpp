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
    key = src.key;
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
    // auto horizonal_dist_to_center = abs(last_armor.center2d.x - 640);


    RotatedRect rotated_rect = last_armor.rrect;
    //调整角度至0-90度(越水平角度越小)
    if (rotated_rect.size.width > rotated_rect.size.height)
        rotate_angle = rotated_rect.angle;
    else
        rotate_angle = 90 - rotated_rect.angle;
    
    //计算分数
    //使用log函数压缩角度权值范围
    hit_score = log(0.15 * (90 - rotate_angle) + 10) * (last_armor.area);
    // cout << "hit_socre: " <<rotate_angle<<" "<<" : "<<last_armor.area<<" "<< hit_score << endl;
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