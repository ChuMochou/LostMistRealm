#include "databasemanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDateTime>
#include <QDebug>

/** @brief 获取数据库管理器单例实例 */
DatabaseManager& DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

/** @brief 私有构造函数 */
DatabaseManager::DatabaseManager()
{
}

/** @brief 析构函数，关闭数据库连接 */
DatabaseManager::~DatabaseManager()
{
    close();
}

/**
 * @brief 初始化数据库连接并创建表结构
 *
 * 使用 QSQLITE 驱动打开 SQLite 数据库文件，并创建所需的表结构。
 */
bool DatabaseManager::initialize(const QString& dbPath)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);

    if (!db.open())
    {
        qDebug() << "Failed to open database:" << db.lastError().text();
        return false;
    }

    if (!createTables())
    {
        return false;
    }

    return true;
}

/** @brief 关闭数据库连接 */
void DatabaseManager::close()
{
    if (db.isOpen())
    {
        db.close();
    }
}

/**
 * @brief 创建数据库表结构
 *
 * 创建 player、achievement、player_achievement 三张表（若不存在），
 * 初始化默认成就定义，并执行数据库迁移以确保旧库兼容新字段。
 */
bool DatabaseManager::createTables()
{
    QSqlQuery query;

    // 创建 player 表
    if (!query.exec(
        "CREATE TABLE IF NOT EXISTS player ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT,"
        "death_count INTEGER DEFAULT 0,"
        "clear_count INTEGER DEFAULT 0,"
        "strawberry_count INTEGER DEFAULT 0,"
        "step_count INTEGER DEFAULT 0,"
        "bomb_used INTEGER DEFAULT 0,"
        "obstacle_destroyed INTEGER DEFAULT 0,"
        "total_play_time INTEGER DEFAULT 0,"
        "consecutive_no_damage_clears INTEGER DEFAULT 0"
        ")"
    ))
    {
        qDebug() << "Failed to create player table:" << query.lastError().text();
        return false;
    }

    // 创建 achievement 表
    if (!query.exec(
        "CREATE TABLE IF NOT EXISTS achievement ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT,"
        "description TEXT,"
        "is_hidden INTEGER DEFAULT 0"
        ")"
    ))
    {
        qDebug() << "Failed to create achievement table:" << query.lastError().text();
        return false;
    }

    // 创建 player_achievement 表
    if (!query.exec(
        "CREATE TABLE IF NOT EXISTS player_achievement ("
        "player_id INTEGER,"
        "achievement_id INTEGER,"
        "unlock_time TEXT,"
        "PRIMARY KEY(player_id, achievement_id),"
        "FOREIGN KEY(player_id) REFERENCES player(id),"
        "FOREIGN KEY(achievement_id) REFERENCES achievement(id)"
        ")"
    ))
    {
        qDebug() << "Failed to create player_achievement table:" << query.lastError().text();
        return false;
    }

    // 初始化默认成就
    initDefaultAchievements();

    // 数据库迁移：确保旧数据库有新字段
    query.exec("ALTER TABLE player ADD COLUMN consecutive_no_damage_clears INTEGER DEFAULT 0");

    return true;
}

/**
 * @brief 插入默认 30 个成就定义
 *
 * 仅当成就表为空时才插入，避免重复插入。
 */
bool DatabaseManager::initDefaultAchievements()
{
    QSqlQuery query;
    query.exec("SELECT COUNT(*) FROM achievement");
    if (query.next() && query.value(0).toInt() > 0)
    {
        return true; // 已有成就数据
    }

    // 插入30个成就
    struct AchDef
    {
        int id; const char* name; const char* desc; int hidden;
    };
    QList<AchDef> achs = {
        {1,  "破晓的凯旋", "通关1次游戏", 0},
        {2,  "终将踏破万重迷雾", "通关50次游戏", 0},
        {3,  "亡者初识归途", "累计死亡1次", 0},
        {4,  "亡魂不眷未来之灾", "累计死亡50次", 0},
        {5,  "将甘甜献予终点", "收集所有草莓后通关", 0},
        {6,  "森林馈赠之始", "累计收集10颗草莓", 0},
        {7,  "繁花终将硕果累累", "累计收集100颗草莓", 0},
        {8,  "火光照亮前路", "累计使用30个炸弹", 0},
        {9,  "爆炎终将吞没群山", "累计使用300个炸弹", 0},
        {10, "碎石开辟新生", "累计炸毁100个障碍物", 0},
        {11, "群山终将在烈焰前俯首", "累计炸毁500个障碍物", 0},
        {12, "无伤的远征", "以满生命值通关1次", 0},
        {13, "圣者不染尘埃", "以满生命值通关50次", 0},
        {14, "远方仍在呼唤", "玩家共移动1000格", 0},
        {15, "脚步终将丈量世界", "玩家共移动10000格", 0},
        {16, "故土朝日复东升", "通关1000次游戏", 0},
        {17, "时间终将铭记旅人", "游戏时间累计达到100小时", 0},
        {18, "残烛仍向黎明", "以1点生命值通关", 1},
        {19, "唯有终点值得停留", "通关时未收集到草莓", 1},
        {20, "此雷鸣是天罚抑或是祝福", "把自己炸死", 1},
        {21, "无人知晓的仪式", "玩家输入\"wwssaaddbaba\"", 1},
        {22, "万籁归于寂静", "同时关闭背景音乐和游戏音效", 1},
        {23, "世界尽头的回响", "连续10局无伤通关", 1},
        {24, "与你行至世界尽头", "探索所有地图", 1},
        {25, "群星见证归人", "累计游玩365小时", 1},
        {26, "愿火焰照亮归路", "累计点燃1000次炸弹", 1},
        {27, "长夜终迎黎明", "首次死亡后再次通关", 1},
        {28, "风止于森林深处", "一局游戏内不放置炸弹通关", 1},
        {29, "愿旅人永不迷失", "第一次点亮全部迷雾", 1},
        {30, "与心爱的你行至世界尽头", "完成所有成就", 1}
    };

    QSqlQuery ins;
    ins.prepare("INSERT INTO achievement (id, name, description, is_hidden) VALUES (:id, :name, :desc, :hidden)");
    for (const auto& a : achs)
    {
        ins.bindValue(":id", a.id);
        ins.bindValue(":name", QString::fromUtf8(a.name));
        ins.bindValue(":desc", QString::fromUtf8(a.desc));
        ins.bindValue(":hidden", a.hidden);
        ins.exec();
    }

    return true;
}

/**
 * @brief 创建新玩家记录
 * @return 新玩家的 ID，失败返回 -1
 */
int DatabaseManager::createPlayer(const QString& name)
{
    QSqlQuery query;
    query.prepare("INSERT INTO player (name) VALUES (:name)");
    query.bindValue(":name", name);

    if (!query.exec())
    {
        qDebug() << "Failed to create player:" << query.lastError().text();
        return -1;
    }

    return query.lastInsertId().toInt();
}

/**
 * @brief 获取已有玩家 ID 或创建新玩家
 * @return 玩家 ID
 */
int DatabaseManager::getOrCreatePlayer(const QString& name)
{
    QSqlQuery query;
    query.prepare("SELECT id FROM player WHERE name = :name LIMIT 1");
    query.bindValue(":name", name);

    if (query.exec() && query.next())
    {
        return query.value(0).toInt();
    }

    return createPlayer(name);
}

/**
 * @brief 从数据库加载玩家统计数据
 * @return 是否加载成功
 */
bool DatabaseManager::loadPlayer(int playerId, PlayerStats& stats)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM player WHERE id = :id");
    query.bindValue(":id", playerId);

    if (!query.exec() || !query.next())
    {
        return false;
    }

    stats.id = query.value("id").toInt();
    stats.name = query.value("name").toString();
    stats.deathCount = query.value("death_count").toInt();
    stats.clearCount = query.value("clear_count").toInt();
    stats.strawberryCount = query.value("strawberry_count").toInt();
    stats.stepCount = query.value("step_count").toInt();
    stats.bombUsed = query.value("bomb_used").toInt();
    stats.obstacleDestroyed = query.value("obstacle_destroyed").toInt();
    stats.totalPlayTime = query.value("total_play_time").toInt();
    stats.consecutiveNoDamageClears = query.value("consecutive_no_damage_clears").toInt();

    return true;
}

/**
 * @brief 更新玩家的所有统计数据到数据库
 * @return 是否更新成功
 */
bool DatabaseManager::updatePlayerStats(const PlayerStats& stats)
{
    QSqlQuery query;
    query.prepare(
        "UPDATE player SET "
        "name = :name, "
        "death_count = :death_count, "
        "clear_count = :clear_count, "
        "strawberry_count = :strawberry_count, "
        "step_count = :step_count, "
        "bomb_used = :bomb_used, "
        "obstacle_destroyed = :obstacle_destroyed, "
        "total_play_time = :total_play_time, "
        "consecutive_no_damage_clears = :consecutive_no_damage_clears "
        "WHERE id = :id"
    );
    query.bindValue(":name", stats.name);
    query.bindValue(":death_count", stats.deathCount);
    query.bindValue(":clear_count", stats.clearCount);
    query.bindValue(":strawberry_count", stats.strawberryCount);
    query.bindValue(":step_count", stats.stepCount);
    query.bindValue(":bomb_used", stats.bombUsed);
    query.bindValue(":obstacle_destroyed", stats.obstacleDestroyed);
    query.bindValue(":total_play_time", stats.totalPlayTime);
    query.bindValue(":consecutive_no_damage_clears", stats.consecutiveNoDamageClears);
    query.bindValue(":id", stats.id);

    return query.exec();
}

/** @brief 将玩家死亡次数加 1 */
bool DatabaseManager::incrementDeath(int playerId)
{
    QSqlQuery query;
    query.prepare("UPDATE player SET death_count = death_count + 1 WHERE id = :id");
    query.bindValue(":id", playerId);
    return query.exec();
}

/** @brief 将玩家通关次数加 1 */
bool DatabaseManager::incrementClear(int playerId)
{
    QSqlQuery query;
    query.prepare("UPDATE player SET clear_count = clear_count + 1 WHERE id = :id");
    query.bindValue(":id", playerId);
    return query.exec();
}

/** @brief 将玩家草莓收集数增加指定数量 */
bool DatabaseManager::addStrawberries(int playerId, int count)
{
    QSqlQuery query;
    query.prepare("UPDATE player SET strawberry_count = strawberry_count + :count WHERE id = :id");
    query.bindValue(":count", count);
    query.bindValue(":id", playerId);
    return query.exec();
}

/** @brief 将玩家移动步数增加指定数量 */
bool DatabaseManager::addSteps(int playerId, int count)
{
    QSqlQuery query;
    query.prepare("UPDATE player SET step_count = step_count + :count WHERE id = :id");
    query.bindValue(":count", count);
    query.bindValue(":id", playerId);
    return query.exec();
}

/** @brief 将玩家炸弹使用数增加指定数量 */
bool DatabaseManager::addBombUsed(int playerId, int count)
{
    QSqlQuery query;
    query.prepare("UPDATE player SET bomb_used = bomb_used + :count WHERE id = :id");
    query.bindValue(":count", count);
    query.bindValue(":id", playerId);
    return query.exec();
}

/** @brief 将玩家炸毁障碍数增加指定数量 */
bool DatabaseManager::addObstacleDestroyed(int playerId, int count)
{
    QSqlQuery query;
    query.prepare("UPDATE player SET obstacle_destroyed = obstacle_destroyed + :count WHERE id = :id");
    query.bindValue(":count", count);
    query.bindValue(":id", playerId);
    return query.exec();
}

/** @brief 将玩家游戏时间增加指定秒数 */
bool DatabaseManager::addPlayTime(int playerId, int seconds)
{
    QSqlQuery query;
    query.prepare("UPDATE player SET total_play_time = total_play_time + :seconds WHERE id = :id");
    query.bindValue(":seconds", seconds);
    query.bindValue(":id", playerId);
    return query.exec();
}

/** @brief 设置玩家连续无伤通关次数 */
bool DatabaseManager::setConsecutiveNoDamageClears(int playerId, int count)
{
    QSqlQuery query;
    query.prepare("UPDATE player SET consecutive_no_damage_clears = :count WHERE id = :id");
    query.bindValue(":count", count);
    query.bindValue(":id", playerId);
    return query.exec();
}

/**
 * @brief 解锁指定成就
 *
 * 先检查是否已解锁，若未解锁则插入记录并记录解锁时间。
 *
 * @return 是否成功解锁（已解锁则返回 false）
 */
bool DatabaseManager::unlockAchievement(int playerId, int achievementId)
{
    if (isAchievementUnlocked(playerId, achievementId))
    {
        return false; // 已解锁，不再次通知
    }

    QSqlQuery query;
    query.prepare(
        "INSERT INTO player_achievement (player_id, achievement_id, unlock_time) "
        "VALUES (:player_id, :achievement_id, :unlock_time)"
    );
    query.bindValue(":player_id", playerId);
    query.bindValue(":achievement_id", achievementId);
    query.bindValue(":unlock_time", QDateTime::currentDateTime().toString(Qt::ISODate));

    return query.exec();
}

/** @brief 检查指定成就是否已解锁 */
bool DatabaseManager::isAchievementUnlocked(int playerId, int achievementId)
{
    QSqlQuery query;
    query.prepare(
        "SELECT COUNT(*) FROM player_achievement "
        "WHERE player_id = :player_id AND achievement_id = :achievement_id"
    );
    query.bindValue(":player_id", playerId);
    query.bindValue(":achievement_id", achievementId);

    if (query.exec() && query.next())
    {
        return query.value(0).toInt() > 0;
    }
    return false;
}

/** @brief 获取玩家已解锁的所有成就 ID 列表 */
QList<int> DatabaseManager::getUnlockedAchievements(int playerId)
{
    QList<int> result;
    QSqlQuery query;
    query.prepare("SELECT achievement_id FROM player_achievement WHERE player_id = :player_id");
    query.bindValue(":player_id", playerId);

    if (query.exec())
    {
        while (query.next())
        {
            result.append(query.value(0).toInt());
        }
    }
    return result;
}

/** @brief 获取玩家已解锁的成就总数 */
int DatabaseManager::getUnlockedAchievementCount(int playerId)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM player_achievement WHERE player_id = :player_id");
    query.bindValue(":player_id", playerId);

    if (query.exec() && query.next())
    {
        return query.value(0).toInt();
    }
    return 0;
}

/** @brief 获取所有成就定义列表，按 ID 升序排列 */
QList<DatabaseManager::AchievementDef> DatabaseManager::getAllAchievements()
{
    QList<AchievementDef> result;
    QSqlQuery query;
    query.exec("SELECT id, name, description, is_hidden FROM achievement ORDER BY id");

    while (query.next())
    {
        AchievementDef def;
        def.id = query.value("id").toInt();
        def.name = query.value("name").toString();
        def.description = query.value("description").toString();
        def.isHidden = query.value("is_hidden").toBool();
        result.append(def);
    }
    return result;
}

/**
 * @brief 重置玩家的所有统计数据为 0
 *
 * 将死亡次数、通关次数、草莓数、步数、炸弹数、
 * 障碍数、游戏时间和连续无伤通关数全部置零。
 */
bool DatabaseManager::resetPlayerStats(int playerId)
{
    QSqlQuery query;
    query.prepare(
        "UPDATE player SET "
        "death_count = 0, "
        "clear_count = 0, "
        "strawberry_count = 0, "
        "step_count = 0, "
        "bomb_used = 0, "
        "obstacle_destroyed = 0, "
        "total_play_time = 0, "
        "consecutive_no_damage_clears = 0 "
        "WHERE id = :id"
    );
    query.bindValue(":id", playerId);
    if (!query.exec())
    {
        qDebug() << "Failed to reset player stats:" << query.lastError().text();
        return false;
    }
    return true;
}

/**
 * @brief 清除玩家的所有成就解锁记录
 *
 * 从 player_achievement 表中删除该玩家的所有记录。
 */
bool DatabaseManager::resetPlayerAchievements(int playerId)
{
    QSqlQuery query;
    query.prepare("DELETE FROM player_achievement WHERE player_id = :player_id");
    query.bindValue(":player_id", playerId);
    if (!query.exec())
    {
        qDebug() << "Failed to reset player achievements:" << query.lastError().text();
        return false;
    }
    return true;
}
