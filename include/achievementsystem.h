#ifndef ACHIEVEMENTSYSTEM_H
#define ACHIEVEMENTSYSTEM_H

#include <QObject>
#include <QString>
#include <QList>
#include "databasemanager.h"

/**
 * @brief 成就信息结构体，包含成就 ID、名称、描述和解锁状态
 */
struct AchievementInfo {
    int id;              ///< 成就 ID
    QString name;        ///< 成就名称
    QString description; ///< 成就描述
    bool isHidden;       ///< 是否为隐藏成就
    bool isUnlocked;     ///< 是否已解锁
};

/**
 * @brief 游戏通关上下文，用于成就检查时传递通关时的状态信息
 */
struct GameClearContext {
    int healthAtEnd;          ///< 通关时的生命值
    int maxHealth;            ///< 最大生命值
    int strawberriesCollected;///< 本局收集的草莓数
    int maxStrawberries;      ///< 地图中草莓总数
    bool bombPlacedThisRun;   ///< 本局是否放置过炸弹
    bool allFogRevealed;      ///< 是否已点亮全部迷雾
};

/**
 * @brief 成就系统类，管理成就的检查和解锁逻辑
 *
 * 根据玩家统计数据检查成就解锁条件，
 * 支持事件驱动的即时检查（如通关、死亡、秘籍输入等）。
 */
class AchievementSystem : public QObject
{
    Q_OBJECT

public:
    /** @brief 构造函数 */
    explicit AchievementSystem(QObject *parent = nullptr);

    /**
     * @brief 设置当前玩家 ID
     * @param playerId 玩家数据库 ID
     */
    void setCurrentPlayer(int playerId);

    /** @brief 获取当前玩家 ID */
    int currentPlayerId() const { return m_playerId; }

    /** @brief 获取所有成就及其解锁状态列表 */
    QList<AchievementInfo> getAllAchievementsWithStatus();

    /** @brief 获取当前玩家已解锁的成就数量 */
    int getUnlockedCount() const;

    /** @brief 获取成就总数 */
    int getTotalCount() const;

    /** @brief 通用成就检查，读取数据库统计数据并逐一检查解锁条件 */
    void checkAchievements();

    /**
     * @brief 游戏通关事件处理
     *
     * 检查与通关相关的成就（如满血通关、无草莓通关、不放置炸弹通关等），
     * 并更新连续无伤通关计数。
     */
    void onGameCleared(const GameClearContext& ctx);

    /**
     * @brief 玩家死亡事件处理
     *
     * 重置连续无伤通关计数，并检查与死亡相关的成就。
     *
     * @param diedByBomb 是否被炸弹炸死
     */
    void onPlayerDied(bool diedByBomb);

    /** @brief 全部迷雾点亮事件处理 */
    void onAllFogRevealed();

    /** @brief 秘籍输入事件处理 */
    void onSecretCodeEntered();

    /** @brief 音乐和音效同时关闭事件处理 */
    void onBothSoundsOff();

signals:
    /**
     * @brief 成就解锁时发射的信号
     * @param name 成就名称
     * @param description 成就描述
     */
    void achievementUnlocked(const QString& name, const QString& description);

private:
    int m_playerId; ///< 当前玩家数据库 ID

    /**
     * @brief 尝试解锁指定成就
     *
     * 通过 DatabaseManager 尝试解锁，成功后发射 achievementUnlocked 信号
     * 并检查元成就（完成所有成就）。
     */
    void tryUnlock(int achievementId, const QString& name, const QString& description);

    /**
     * @brief 检查元成就“与心爱的你行至世界尽头”
     *
     * 当除成就 30 以外的所有成就均已解锁时触发。
     */
    void checkMetaAchievement();
};

#endif // ACHIEVEMENTSYSTEM_H
