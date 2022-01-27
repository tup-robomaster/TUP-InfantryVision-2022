#include "general.h"
/**
 * @brief 按行读取文件中内容
 * @return 以vector形式返回文件每行内容
 * **/
std::vector<string> readLines(string file_path)
{
    std::vector<string> lines;
    string line;
    ifstream file;
    file.open(file_path, ios::in);

    while(getline(file,line))
        lines.push_back(line);

    return lines;
}

/**
 * @brief 将软链接转化为实际指向路径
 * @return 实际位置的绝对路径
 * **/
std::string symbolicToReal(string path)
{
    char path_tmp[1024];
    auto len = readlink(path.c_str(), path_tmp, 1024);
    //若路径错误返回空字符串
    if (len == -1)
        return NULL;
    string path_relative(path_tmp,0,len);

    return relativeToFull(path_relative,path);
    
}

/**
 * @brief 将相对路径转化为实际路径
 * @param relative 相对路径
 * @param origin 相对路径起始位置
 * @return 绝对路径
 * **/
std::string relativeToFull(string relative,string origin)
{
    auto tree_relative = generatePathTree(relative);
    auto tree_origin = generatePathTree(origin);

    string result;

    //记数
    int relative_cnt = 0;
    for(auto dir : tree_relative)
    {
        if (dir == "..")
            relative_cnt++;
    }

    vector<string> tree_parent (tree_origin.begin(),tree_origin.end() - relative_cnt - 1);
    vector<string> tree_no_relative (tree_relative.begin() + relative_cnt,tree_relative.end());

    result = treeToPath(tree_parent) + treeToPath(tree_no_relative);

    return result;
    
}

/**
 * @brief 生成路径的树状结构
 * @param path 路径
 * @return 路径结构
 * **/
std::vector<string> generatePathTree(string path)
{
    vector<string> tree;

    //若为相对路径，在头部插入“/”便于处理
    if (path[0] == '.')
        path.insert(0,"/");

    //使用逗号表达式控制循环
    for(auto i = 0; i = path.find("/"), i != path.npos; path = path.substr(i + 1))
    {   
        if(i == 0)
            continue;
        auto sub = path.substr(0, i);
        tree.push_back(sub);
    }

    tree.push_back(path);

    return tree;
}

/**
 * @brief 由树状结构生成路径
 * @return 生成路径
 * **/
string treeToPath(std::vector<string> &tree)
{
    string result;
    for(auto node : tree)
    {
        result += "/" + node;
    }
    return result;
}

/**
 * @brief 获取输入目录上级目录
 * @return 生成路径
 * **/
string getParent(string path)
{
    int last_slash_idx = 0;
    string path_tmp = path;

    for(auto i = 0; i = path_tmp.find("/"), i != path_tmp.npos; path_tmp = path_tmp.substr(i + 1))
    {   
        last_slash_idx += i + 1;
    }
    
    if (last_slash_idx == 0)
        return path;
    else
        return path.substr(0,last_slash_idx - 1);
    
}
