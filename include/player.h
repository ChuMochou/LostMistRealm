#ifndef PLAYER_H
#define PLAYER_H

#include <QPoint>

/**
 * @brief 玩家类，封装玩家在迷宫中的状态和行为
 *
 * 包含位置、朝向、动画帧、生命值、道具数量等信息，
 * 以及移动动画的控制方法。
 */
class Player
{
public:
    /** @brief 玩家朝向枚举 */
    enum Direction
    {
        Down = 0,   ///< 朝下
        Up = 1,     ///< 朝上
        Left = 2,   ///< 朝左
        Right = 3   ///< 朝右
    };

    /** @brief 构造函数，初始化玩家默认状态（位置、生命值、炸弹数等） */
    Player();

    QPoint position;        ///< 玩家当前网格坐标（col, row）
    float x, y;             ///< 玩家平滑移动时的浮点坐标
    Direction direction;    ///< 玩家当前朝向
    int currentFrame;       ///< 当前动画帧索引
    bool hasKey;            ///< 是否已拾取钥匙
    int strawberryCount;    ///< 当前草莓收集数
    int bombCount;          ///< 当前炸弹数量
    int health;             ///< 当前生命值
    bool isMoving;          ///< 是否正在移动中
    int targetRow, targetCol; ///< 移动目标格子坐标

    /** @brief 开始移动动画，将 isMoving 置为 true 并重置动画帧 */
    void startMoveAnimation();

    /** @brief 停止移动动画，将 isMoving 置为 false 并重置动画帧 */
    void stopMoveAnimation();

    /** @brief 推进动画帧，在移动状态下循环切换 0~3 帧 */
    void nextFrame();

    /**
     * @brief 设置玩家朝向
     * @param dir 新的朝向
     */
    void setDirection(Direction dir);
};

#endif // PLAYER_H