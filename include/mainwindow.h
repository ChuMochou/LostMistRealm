#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QPixmap>
#include <QProgressBar>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QTimer>
#include "achievementsystem.h"
#include "aimanager.h"

class GameWidget;

/**
 * @brief 主窗口类，负责整个游戏界面的布局和流程协调
 *
 * 包含游戏区域、右侧控制面板（玩家状态、按键说明、功能菜单等），
 * 并协调游戏事件与成就系统、AI 助手之间的交互。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief 构造主窗口，初始化所有子模块和UI组件
     * @param parent 父组件指针
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /**
     * @brief 析构函数，释放音乐播放器和音频输出资源
     */
    ~MainWindow();

private slots:
    /** @brief 处理“新游戏”按钮点击，重置游戏状态并生成新地图 */
    void onNewGameClicked();

    /** @brief 处理“退出游戏”按钮点击，弹出确认对话框后退出应用 */
    void onExitGameClicked();

    /** @brief 切换背景音乐的播放/暂停状态 */
    void onToggleMusicClicked();

    /** @brief 切换游戏音效的开启/关闭状态 */
    void onToggleSoundClicked();

    /**
     * @brief 响应游戏状态变化，更新右侧面板的生命值/草莓/炸弹/钥匙进度条
     * @param health 当前生命值
     * @param maxHealth 最大生命值
     * @param strawberries 当前草莓数
     * @param maxStrawberries 最大草莓数
     * @param bombs 当前炸弹数
     * @param maxBombs 最大炸弹数
     * @param keys 当前钥匙数
     * @param maxKeys 最大钥匙数
     */
    void onStatusChanged(int health, int maxHealth, int strawberries, int maxStrawberries, int bombs, int maxBombs, int keys, int maxKeys);

    /**
     * @brief 响应道具收集事件，在“道具收集”区域显示提示信息
     * @param message 收集提示文本
     */
    void onItemCollected(QString message);

    /** @brief 响应游戏重启请求，清空道具收集区域 */
    void onRestartGameRequested();

    /** @brief 打开成就系统对话框，展示游戏统计、成就列表和重置进度功能 */
    void onAchievementClicked();

    /**
     * @brief 响应玩家死亡事件，转发给成就系统进行成就检查
     * @param diedByBomb 是否被炸弹炸死
     */
    void onPlayerDied(bool diedByBomb);

    /**
     * @brief 响应游戏通关事件，构造上下文并转发给成就系统
     * @param health 通关时的生命值
     * @param strawberries 本局收集的草莓数
     * @param bombPlacedThisRun 本局是否放置过炸弹
     */
    void onGameCleared(int health, int strawberries, bool bombPlacedThisRun);

    /** @brief 响应全部迷雾点亮事件，转发给成就系统 */
    void onAllFogRevealed();

    /** @brief 响应秘籍输入事件，转发给成就系统 */
    void onSecretCodeEntered();

    /** @brief 计时器定时触发（每60秒），累加游戏时间并检查成就 */
    void onPlayTimeTick();

    /**
     * @brief 响应成就解锁事件，弹出成就解锁提示框
     * @param name 成就名称
     * @param description 成就描述
     */
    void onAchievementUnlocked(const QString& name, const QString& description);

    /** @brief 打开 AI 助手对话框，支持用户与 AI 进行游戏相关问答 */
    void onAIAssistantClicked();

    /**
     * @brief 全局 AI 回答就绪槽函数（保留用于扩展，如弹出通知）
     * @param answer AI 回答文本
     */
    void onAIAnswerReady(const QString &answer);

    /**
     * @brief 全局 AI 错误槽函数（保留用于扩展）
     * @param errorMessage 错误信息
     */
    void onAIError(const QString &errorMessage);

private:
    GameWidget *gameWidget;
    QPushButton *newGameBtn;
    QPushButton *achievementBtn;
    QPushButton *toggleMusicBtn;
    QPushButton *toggleSoundBtn;
    QPushButton *exitGameBtn;

    QLabel *healthIcon;
    QLabel *healthText;
    QProgressBar *healthBar;
    QLabel *strawberryIcon;
    QLabel *strawberryText;
    QProgressBar *strawberryBar;
    QLabel *bombIcon;
    QLabel *bombText;
    QProgressBar *bombBar;
    QLabel *keyIcon;
    QLabel *keyText;
    QProgressBar *keyBar;

    QGroupBox *collectionGroup;
    QVBoxLayout *collectionLayout;

    QPixmap heartPixmap;
    QPixmap strawberryPixmap;
    QPixmap bombPixmap;
    QPixmap keyPixmap;

    QMediaPlayer *musicPlayer;
    QAudioOutput *audioOutput;

    AchievementSystem *achievementSystem;
    AIManager *aiManager;
    int currentPlayerId;
    bool musicEnabled;
    bool soundEnabled;
    QTimer *playTimeTimer;

    /**
     * @brief 为进度条设置自定义样式（边框、圆角、填充颜色）
     * @param bar 目标进度条指针
     * @param color 进度条填充颜色（十六进制字符串）
     */
    void setupProgressBar(QProgressBar *bar, const QString& color);

    /**
     * @brief 清空右侧“道具收集”区域的所有标签
     */
    void clearCollectionBar();

    /**
     * @brief 初始化并播放背景音乐，自动搜索多个路径以定位音频文件
     */
    void playBackgroundMusic();
};

#endif // MAINWINDOW_H