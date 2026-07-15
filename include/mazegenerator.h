#ifndef MAZEGENERATOR_H
#define MAZEGENERATOR_H

#include "maze.h"

/**
 * @brief 迷宫生成器类，使用随机化 Kruskal 算法生成迷宫地图
 *
 * 生成迷宫路径后，还会随机放置门、钥匙、草莓和炸弹等游戏元素。
 */
class MazeGenerator
{
public:
    /** @brief 构造函数，初始化随机数种子 */
    MazeGenerator();

    /**
     * @brief 生成随机迷宫地图
     *
     * 使用随机化 Kruskal 算法生成主路径，然后随机放置障碍物、
     * 门、钥匙、草莓和炸弹等游戏元素。
     *
     * @param maze 待填充的迷宫对象
     */
    void generate(Maze& maze);

private:
    /** @brief 边结构体，用于 Kruskal 算法中表示两个单元格之间的连接 */
    struct Edge
    {
        int x1, y1;     ///< 起始单元格坐标
        int x2, y2;     ///< 终止单元格坐标
        int wallX, wallY; ///< 两者之间的墙壁坐标
    };

    static const int DIR_NUM = 4;                     ///< 方向数（上下左右）
    static const int dx_gen[DIR_NUM];                   ///< X方向偏移数组
    static const int dy_gen[DIR_NUM];                   ///< Y方向偏移数组

    /**
     * @brief 随机返回一种障碍物类型（树/水面/石头）
     * @return 随机障碍物类型
     */
    Maze::CellType getRandomObstacle();
};

#endif // MAZEGENERATOR_H