#include "Maze.h"
#include "mazegenerator.h"

/**
 * @brief 构造函数
 *
 * 设置默认可见步数和视野半径，清空历史记录，并生成随机地图。
 */
Maze::Maze()
{
    maxVisibleSteps = DEFAULT_VISIBLE_STEPS;
    viewRadius = DEFAULT_VIEW_RADIUS;
    historyCount = 0;
    generateRandomMap();
}

/**
 * @brief 初始化测试用固定地图
 *
 * 手动设置草地、路径、树木、水面、石头等元素的位置，
 * 用于调试和测试，不依赖随机生成。
 */
void Maze::initTestMap()
{
    for(int row = 0; row < ROWS; row++)
    {
        for(int col = 0; col < COLS; col++)
        {
            map[row][col] = Grass;
        }
    }

    for(int col = 0; col < COLS; col++)
    {
        map[0][col] = Path;
    }

    for(int row = 0; row < ROWS; row++)
    {
        map[row][COLS - 1] = Path;
    }

    for(int i = 3; i < 10; i++)
    {
        map[5][i] = Tree;
    }

    for(int i = 12; i < 19; i++)
    {
        map[12][i] = Tree;
    }

    for(int i = 2; i < 8; i++)
    {
        map[9][i] = Water;
    }

    for(int i = 13; i < 20; i++)
    {
        map[i][5] = Water;
    }

    for(int i = 6; i < 13; i++)
    {
        map[7][i] = Stone;
    }

    map[0][0] = Start;

    map[ROWS - 1][COLS - 1] = End;
}

/**
 * @brief 生成随机地图
 *
 * 创建 MazeGenerator 实例并调用其 generate 方法生成随机迷宫。
 */
void Maze::generateRandomMap()
{
    MazeGenerator generator;
    generator.generate(*this);
}

/**
 * @brief 判断指定单元格是否可行走
 *
 * 检查边界后判断单元格类型：草地、路径、起点、终点、钥匙、草莓、炸弹均可通行；
 * 门仅在玩家持有钥匙时可通行。
 *
 * @param row 行号
 * @param col 列号
 * @param hasKey 玩家是否持有钥匙
 * @return 是否可行走
 */
bool Maze::isWalkable(int row, int col, bool hasKey) const
{
    if(row < 0 || row >= ROWS || col < 0 || col >= COLS)
    {
        return false;
    }

    CellType type = map[row][col];

    return type == Grass ||
           type == Path ||
           type == Start ||
           type == End ||
           (type == Key) ||
           (type == Strawberry) ||
           (type == Bomb) ||
           (type == Door && hasKey);
}

/**
 * @brief 初始化迷雾状态
 *
 * 将所有单元格设置为不可见、迷雾透明度 255（全雾）、无动画，
 * 仅起点和终点默认可见，并清空移动历史。
 */
void Maze::initVisibility()
{
    for(int row = 0; row < ROWS; row++)
    {
        for(int col = 0; col < COLS; col++)
        {
            visible[row][col] = false;
            fogAlpha[row][col] = 255;
            isFogAnimating[row][col] = false;
        }
    }

    visible[0][0] = true;
    visible[ROWS - 1][COLS - 1] = true;
    fogAlpha[0][0] = 0;
    fogAlpha[ROWS - 1][COLS - 1] = 0;

    historyCount = 0;
}

/**
 * @brief 根据玩家当前位置更新迷雾可见性
 *
 * 1. 点亮玩家视野半径内的所有单元格，并触发迷雾消散动画。
 * 2. 如果历史记录超过最大可见步数，将最早足迹处的迷雾重新覆盖（如果该单元格不再被其他足迹覆盖）。
 * 3. 将当前位置加入历史记录。
 *
 * @param playerRow 玩家当前行
 * @param playerCol 玩家当前列
 */
void Maze::updateVisibility(int playerRow, int playerCol)
{
    for(int dr = -viewRadius; dr <= viewRadius; dr++)
    {
        for(int dc = -viewRadius; dc <= viewRadius; dc++)
        {
            int row = playerRow + dr;
            int col = playerCol + dc;
            if(row >= 0 && row < ROWS && col >= 0 && col < COLS)
            {
                if(!visible[row][col])
                {
                    visible[row][col] = true;
                    isFogAnimating[row][col] = true;
                }
            }
        }
    }

    if(historyCount >= maxVisibleSteps)
    {
        Position oldest = history[0];
        for(int i = 0; i < historyCount - 1; i++)
        {
            history[i] = history[i + 1];
        }
        historyCount--;

        for(int dr = -viewRadius; dr <= viewRadius; dr++)
        {
            for(int dc = -viewRadius; dc <= viewRadius; dc++)
            {
                int row = oldest.row + dr;
                int col = oldest.col + dc;
                if(row >= 0 && row < ROWS && col >= 0 && col < COLS)
                {
                    bool stillVisible = false;
                    for(int i = 0; i < historyCount; i++)
                    {
                        Position pos = history[i];
                        for(int hd = -viewRadius; hd <= viewRadius; hd++)
                        {
                            for(int hc = -viewRadius; hc <= viewRadius; hc++)
                            {
                                if(row == pos.row + hd && col == pos.col + hc)
                                {
                                    stillVisible = true;
                                    break;
                                }
                            }
                            if(stillVisible) break;
                        }
                        if(stillVisible) break;
                    }
                    if(!stillVisible && visible[row][col])
                    {
                        visible[row][col] = false;
                        isFogAnimating[row][col] = true;
                    }
                }
            }
        }
    }

    if(historyCount < MAX_HISTORY_SIZE)
    {
        history[historyCount++] = Position(playerRow, playerCol);
    }
}

/**
 * @brief 设置最大可见步数
 *
 * 限制最小值为 1，防止无效输入。
 *
 * @param steps 新的可见步数上限
 */
void Maze::setMaxVisibleSteps(int steps)
{
    maxVisibleSteps = steps;
    if(maxVisibleSteps < 1) maxVisibleSteps = 1;
}

/**
 * @brief 设置视野半径
 *
 * 限制取值范围为 0~10。
 *
 * @param radius 新的视野半径
 */
void Maze::setViewRadius(int radius)
{
    viewRadius = radius;
    if(viewRadius < 0) viewRadius = 0;
    if(viewRadius > 10) viewRadius = 10;
}