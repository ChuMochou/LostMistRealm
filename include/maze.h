#ifndef MAZE_H
#define MAZE_H

/**
 * @brief 迷宫类，封装迷宫地图数据、迷雾可见性和视野系统
 *
 * 包含 23×23 的地图网格、战争迷雾的透明度和动画状态，
 * 以及基于玩家移动历史的视野管理机制。
 */
class Maze
{
public:
    static const int ROWS = 23;              ///< 地图行数
    static const int COLS = 23;              ///< 地图列数
    static const int DEFAULT_VISIBLE_STEPS = 1000; ///< 默认可见步数上限
    static const int DEFAULT_VIEW_RADIUS = 2;    ///< 默认视野半径

    /** @brief 地图单元格类型枚举 */
    enum CellType
    {
        Grass,       ///< 草地（可通行）
        Path,        ///< 路径（可通行）
        Tree,        ///< 树木（不可通行，可被炸弹炸毁）
        Water,       ///< 水面（不可通行，不可被炸弹炸毁）
        Stone,       ///< 石头（不可通行，可被炸弹炸毁）
        Start,       ///< 起点
        End,         ///< 终点
        Door,        ///< 门（需要钥匙才能通行）
        Key,         ///< 钥匙（可拾取）
        Strawberry,  ///< 草莓（可拾取）
        Bomb         ///< 炸弹（可拾取）
    };

    /** @brief 位置结构体，用于记录玩家移动历史 */
    struct Position
    {
        int row;
        int col;
        Position() : row(0), col(0) {}
        Position(int r, int c) : row(r), col(c) {}
    };

public:
    /** @brief 构造函数，初始化视野参数并生成随机地图 */
    Maze();

    CellType map[ROWS][COLS];             ///< 地图网格数据
    bool visible[ROWS][COLS];             ///< 各单元格是否可见
    int fogAlpha[ROWS][COLS];             ///< 迷雾透明度（0=无雾，255=全雾）
    bool isFogAnimating[ROWS][COLS];      ///< 各单元格迷雾是否正在动画过渡

    int maxVisibleSteps;                  ///< 最大可见步数（超出后旧足迹的迷雾会重新覆盖）
    int viewRadius;                       ///< 视野半径

    /** @brief 初始化测试用固定地图（用于调试） */
    void initTestMap();

    /** @brief 生成随机地图，委托给 MazeGenerator */
    void generateRandomMap();

    /**
     * @brief 判断指定单元格是否可行走
     * @param row 行号
     * @param col 列号
     * @param hasKey 玩家是否持有钥匙（用于判断门是否可通行）
     * @return 是否可行走
     */
    bool isWalkable(int row, int col, bool hasKey = false) const;

    /** @brief 初始化迷雾状态，全部置为不可见并重置历史记录 */
    void initVisibility();

    /**
     * @brief 根据玩家当前位置更新迷雾可见性
     *
     * 点亮视野范围内的单元格，并在超过最大可见步数时
     * 重新覆盖最早足迹处的迷雾。
     *
     * @param playerRow 玩家当前行
     * @param playerCol 玩家当前列
     */
    void updateVisibility(int playerRow, int playerCol);

    /**
     * @brief 设置最大可见步数
     * @param steps 新的可见步数上限
     */
    void setMaxVisibleSteps(int steps);

    /**
     * @brief 设置视野半径
     * @param radius 新的视野半径（0~10）
     */
    void setViewRadius(int radius);

private:
    static const int MAX_HISTORY_SIZE = 100;
    Position history[MAX_HISTORY_SIZE];
    int historyCount;
};

#endif // MAZE_H