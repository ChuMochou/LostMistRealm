#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>

/**
 * @brief 玩家统计数据结构体
 *
 * 封装玩家在数据库中的所有统计信息，
 * 包括死亡次数、通关次数、草莓收集量、移动步数等。
 */
struct PlayerStats
{
    int id;                        ///< 玩家数据库 ID
    QString name;                  ///< 玩家名称
    int deathCount;                ///< 累计死亡次数
    int clearCount;                ///< 累计通关次数
    int strawberryCount;           ///< 累计草莓收集数
    int stepCount;                 ///< 累计移动步数
    int bombUsed;                  ///< 累计炸弹使用数
    int obstacleDestroyed;         ///< 累计炸毁障碍数
    int totalPlayTime;             ///< 累计游戏时间（秒）
    int consecutiveNoDamageClears; ///< 连续无伤通关次数
};

/**
 * @brief 数据库管理器，采用单例模式管理 SQLite 数据库操作
 *
 * 负责玩家数据的 CRUD、统计数据的递增、成就的解锁与查询，
 * 以及进度重置等所有数据库操作。
 */
class DatabaseManager
{
public:
    /** @brief 获取数据库管理器单例实例 */
    static DatabaseManager& instance();

    /**
     * @brief 初始化数据库连接并创建表结构
     * @param dbPath 数据库文件路径
     * @return 是否初始化成功
     */
    bool initialize(const QString& dbPath = "lostmistrealm.db");

    /** @brief 关闭数据库连接 */
    void close();

    // --- 玩家数据操作 ---

    /**
     * @brief 创建新玩家记录
     * @param name 玩家名称
     * @return 新玩家的 ID，失败返回 -1
     */
    int createPlayer(const QString& name);

    /**
     * @brief 获取已有玩家 ID 或创建新玩家
     * @param name 玩家名称
     * @return 玩家 ID
     */
    int getOrCreatePlayer(const QString& name);

    /**
     * @brief 从数据库加载玩家统计数据
     * @param playerId 玩家 ID
     * @param stats 输出参数，存储加载的统计数据
     * @return 是否加载成功
     */
    bool loadPlayer(int playerId, PlayerStats& stats);

    /**
     * @brief 更新玩家的所有统计数据到数据库
     * @param stats 包含新统计数据的结构体
     * @return 是否更新成功
     */
    bool updatePlayerStats(const PlayerStats& stats);

    // --- 统计数据递增 ---

    /** @brief 将指定玩家的死亡次数加 1 */
    bool incrementDeath(int playerId);

    /** @brief 将指定玩家的通关次数加 1 */
    bool incrementClear(int playerId);

    /**
     * @brief 将指定玩家的草莓收集数增加指定数量
     * @param playerId 玩家 ID
     * @param count 增加的数量
     */
    bool addStrawberries(int playerId, int count);

    /**
     * @brief 将指定玩家的移动步数增加指定数量
     * @param playerId 玩家 ID
     * @param count 增加的数量
     */
    bool addSteps(int playerId, int count);

    /**
     * @brief 将指定玩家的炸弹使用数增加指定数量
     * @param playerId 玩家 ID
     * @param count 增加的数量
     */
    bool addBombUsed(int playerId, int count);

    /**
     * @brief 将指定玩家的炸毁障碍数增加指定数量
     * @param playerId 玩家 ID
     * @param count 增加的数量
     */
    bool addObstacleDestroyed(int playerId, int count);

    /**
     * @brief 将指定玩家的游戏时间增加指定秒数
     * @param playerId 玩家 ID
     * @param seconds 增加的秒数
     */
    bool addPlayTime(int playerId, int seconds);

    /**
     * @brief 设置指定玩家的连续无伤通关次数
     * @param playerId 玩家 ID
     * @param count 新的连续无伤通关次数
     */
    bool setConsecutiveNoDamageClears(int playerId, int count);

    // --- 成就相关 ---

    /**
     * @brief 解锁指定玩家的指定成就
     * @param playerId 玩家 ID
     * @param achievementId 成就 ID
     * @return 是否成功解锁（已解锁则返回 false）
     */
    bool unlockAchievement(int playerId, int achievementId);

    /**
     * @brief 检查指定玩家的指定成就是否已解锁
     * @param playerId 玩家 ID
     * @param achievementId 成就 ID
     * @return 是否已解锁
     */
    bool isAchievementUnlocked(int playerId, int achievementId);

    /**
     * @brief 获取指定玩家已解锁的所有成就 ID 列表
     * @param playerId 玩家 ID
     * @return 已解锁成就 ID 列表
     */
    QList<int> getUnlockedAchievements(int playerId);

    /**
     * @brief 获取指定玩家已解锁的成就总数
     * @param playerId 玩家 ID
     * @return 已解锁成就数
     */
    int getUnlockedAchievementCount(int playerId);

    // --- 重置进度 ---

    /**
     * @brief 重置指定玩家的所有统计数据为 0
     * @param playerId 玩家 ID
     * @return 是否重置成功
     */
    bool resetPlayerStats(int playerId);

    /**
     * @brief 清除指定玩家的所有成就解锁记录
     * @param playerId 玩家 ID
     * @return 是否清除成功
     */
    bool resetPlayerAchievements(int playerId);

    // --- 获取所有成就定义 ---

    /** @brief 成就定义结构体 */
    struct AchievementDef
    {
        int id;             ///< 成就 ID
        QString name;       ///< 成就名称
        QString description;///< 成就描述
        bool isHidden;      ///< 是否为隐藏成就
    };

    /**
     * @brief 获取所有成就的定义列表
     * @return 成就定义列表
     */
    QList<AchievementDef> getAllAchievements();

private:
    /** @brief 私有构造函数（单例模式） */
    DatabaseManager();
    /** @brief 私有析构函数，关闭数据库连接 */
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;            ///< 禁用拷贝构造
    DatabaseManager& operator=(const DatabaseManager&) = delete; ///< 禁用拷贝赋值

    /** @brief 创建数据库表结构（player、achievement、player_achievement） */
    bool createTables();
    /** @brief 插入默认 30 个成就定义（仅当成就表为空时执行） */
    bool initDefaultAchievements();

    QSqlDatabase db; ///< SQLite 数据库连接
};

#endif // DATABASEMANAGER_H
