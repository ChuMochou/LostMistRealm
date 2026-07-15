#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QTimer>
#include "Maze.h"
#include "Player.h"
#include <QKeyEvent>
#include <QList>
#include <QMediaPlayer>
#include <QAudioOutput>
#include "achievementsystem.h"

/**
 * @brief 从指定子目录加载图片资源
 *
 * 在多个路径中搜索图片文件，返回第一个找到的 QPixmap。
 *
 * @param filename 文件名
 * @param subdir 子目录名（如 "floor"、"player"、"object"）
 * @return 加载的图片，未找到则返回空 QPixmap
 */
QPixmap loadImage(const QString& filename, const QString& subdir = "tiles");

/**
 * @brief 已放置的炸弹结构体
 */
struct PlacedBomb
{
    int row;    ///< 炸弹所在行
    int col;    ///< 炸弹所在列
    float timer;///< 爆炸倒计时（秒）
};

/**
 * @brief 爆炸效果结构体
 */
struct Explosion
{
    int row;          ///< 爆炸中心行
    int col;          ///< 爆炸中心列
    int currentFrame; ///< 当前动画帧
};

/**
 * @brief 游戏组件类，负责迷宫渲染、玩家控制和游戏逻辑
 *
 * 处理键盘输入、玩家移动动画、道具拾取、炸弹放置与爆炸、
 * 迷雾可见性更新，以及游戏通关/死亡判定。
 */
class GameWidget : public QWidget
{
    Q_OBJECT

public:
    /** @brief 构造函数，加载资源、初始化计时器和音频 */
    explicit GameWidget(QWidget *parent = nullptr);

    /** @brief 重新开始游戏，生成新地图并重置玩家状态 */
    void restartGame();

    /** @brief 发送初始状态信号，用于初始化右侧面板 */
    void sendInitialStatus();

    /**
     * @brief 设置数据库玩家 ID
     * @param id 玩家数据库 ID
     */
    void setDbPlayerId(int id) { dbPlayerId = id; }

protected:
    /** @brief 绘制事件，渲染迷宫地图、玩家、炸弹和爆炸效果 */
    void paintEvent(QPaintEvent *event) override;

    /** @brief 键盘按下事件处理，控制玩家移动、放置炸弹和秘籍检测 */
    void keyPressEvent(QKeyEvent *event) override;

signals:
    /** @brief 玩家状态变化信号（生命值、草莓、炸弹、钥匙） */
    void statusChanged(int health, int maxHealth, int strawberries, int maxStrawberries, int bombs, int maxBombs, int keys, int maxKeys);

    /** @brief 道具收集信号 */
    void itemCollected(QString message);

    /** @brief 请求重新开始游戏信号 */
    void restartGameRequested();

    /** @brief 玩家死亡信号（参数：是否被炸弹炸死） */
    void playerDied(bool diedByBomb);

    /** @brief 游戏通关信号（参数：生命值、草莓数、是否放置过炸弹） */
    void gameCleared(int health, int strawberries, bool bombPlacedThisRun);

    /** @brief 全部迷雾点亮信号 */
    void allFogRevealed();

    /** @brief 秘籍输入信号 */
    void secretCodeEntered();

private slots:
    /** @brief 更新玩家移动动画，平滑移动到目标位置并处理道具拾取/通关/死亡 */
    void updateAnimation();

    /** @brief 更新玩家动画帧（定时器回调） */
    void updateFrame();

    /** @brief 更新迷雾动画（定时器回调），逐步改变迷雾透明度 */
    void updateFogAnimation();

    /** @brief 更新炸弹倒计时（定时器回调），到期后触发爆炸 */
    void updateBombTimers();

    /** @brief 更新爆炸动画帧（定时器回调），动画结束后移除爆炸 */
    void updateExplosionAnimation();

private:
    Maze maze;                ///< 迷宫地图对象
    Player player;             ///< 玩家对象
    QTimer *animationTimer;    ///< 移动动画定时器（约60fps）
    QTimer *frameTimer;        ///< 动画帧切换定时器
    QTimer *fogTimer;          ///< 迷雾动画定时器
    QTimer *bombTimer;         ///< 炸弹倒计时定时器
    QTimer *explosionTimer;    ///< 爆炸动画定时器
    QMediaPlayer *bombSound;   ///< 炸弹音效播放器
    QAudioOutput *bombAudioOutput; ///< 炸弹音效输出
    QMediaPlayer *pickUpSound; ///< 拾取音效播放器
    QAudioOutput *pickUpAudioOutput; ///< 拾取音效输出
    bool soundEnabled;         ///< 音效是否开启
    bool bombPlacedThisRun;    ///< 本局是否放置过炸弹
    int currentRunSteps;       ///< 本局已走步数
    QString secretCodeBuffer;  ///< 秘籍输入缓冲区
    int dbPlayerId;            ///< 数据库玩家 ID
    const float MOVE_SPEED = 0.15f;          ///< 每帧移动速度
    const float BOMB_EXPLODE_TIME = 2.0f;    ///< 炸弹爆炸倒计时（秒）
    const int EXPLOSION_FRAMES = 6;          ///< 爆炸动画总帧数
    const int EXPLOSION_FRAME_DURATION = 50; ///< 爆炸每帧持续时间（ms）

public:
    /**
     * @brief 设置音效开关状态
     * @param enabled 是否开启音效
     */
    void setSoundEnabled(bool enabled);

    QPixmap grassPixmap;        ///< 草地贴图
    QPixmap pathPixmap;         ///< 路径贴图
    QPixmap treePixmap;         ///< 树木贴图
    QPixmap waterPixmap;        ///< 水面贴图
    QPixmap stonePixmap;        ///< 石头贴图
    QPixmap endPixmap;          ///< 终点贴图
    QPixmap doorPixmap;         ///< 门贴图
    QPixmap keyPixmap;          ///< 钥匙贴图
    QPixmap strawberryPixmap;   ///< 草莓贴图
    QPixmap bombPixmap;         ///< 炸弹贴图
    QPixmap fogPixmap;          ///< 迷雾贴图
    QPixmap playerSpriteSheet;  ///< 玩家精灵图（4x4方向帧）
    QPixmap explosionPixmaps[6];///< 爆炸动画帧贴图

    QList<PlacedBomb> placedBombs; ///< 当前放置的炸弹列表
    QList<Explosion> explosions;   ///< 当前爆炸效果列表
    bool useTextures;              ///< 是否使用贴图渲染（无贴图时使用颜色填充）

    /** @brief 在玩家当前位置放置炸弹 */
    void placeBomb();

    /**
     * @brief 引爆指定索引的炸弹
     *
     * 炸毁 3x3 范围内的障碍物，检查玩家是否被波及，
     * 并更新数据库统计。
     *
     * @param index 炸弹在 placedBombs 列表中的索引
     */
    void explodeBomb(int index);
};

#endif // GAMEWIDGET_H