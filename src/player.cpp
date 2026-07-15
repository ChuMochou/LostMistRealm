#include "Player.h"

/**
 * @brief 构造函数，初始化玩家默认状态
 *
 * 设置初始位置为 (0,0)，朝向朝下，生命值为3，
 * 炸弹数为2，未持有钥匙，未处于移动状态。
 */
Player::Player()
{
    position = QPoint(0, 0);
    x = 0;
    y = 0;
    direction = Down;
    currentFrame = 0;
    hasKey = false;
    strawberryCount = 0;
    bombCount = 2;
    health = 3;
    isMoving = false;
    targetRow = 0;
    targetCol = 0;
}

/**
 * @brief 开始移动动画
 *
 * 将 isMoving 标志置为 true，并将动画帧重置为 0。
 */
void Player::startMoveAnimation()
{
    isMoving = true;
    currentFrame = 0;
}

/**
 * @brief 停止移动动画
 *
 * 将 isMoving 标志置为 false，并将动画帧重置为 0。
 */
void Player::stopMoveAnimation()
{
    isMoving = false;
    currentFrame = 0;
}

/**
 * @brief 推进动画帧
 *
 * 如果玩家正在移动，则将动画帧循环递增（0→ 1→ 2→ 3→ 0）。
 */
void Player::nextFrame()
{
    if(isMoving)
    {
        currentFrame = (currentFrame + 1) % 4;
    }
}

/**
 * @brief 设置玩家朝向
 * @param dir 新的朝向
 */
void Player::setDirection(Direction dir)
{
    direction = dir;
}