#include "achievementsystem.h"
#include <QDebug>

/** @brief 构造函数，初始化玩家 ID 为 -1（未设置） */
AchievementSystem::AchievementSystem(QObject *parent)
    : QObject(parent), m_playerId(-1)
{
}

/** @brief 设置当前玩家 ID */
void AchievementSystem::setCurrentPlayer(int playerId)
{
    m_playerId = playerId;
}

/**
 * @brief 获取所有成就及其解锁状态
 *
 * 从数据库获取全部成就定义和已解锁列表，
 * 组合为 AchievementInfo 列表返回。
 */
QList<AchievementInfo> AchievementSystem::getAllAchievementsWithStatus()
{
    QList<AchievementInfo> result;
    if (m_playerId < 0) return result;

    QList<DatabaseManager::AchievementDef> allAchievements =
        DatabaseManager::instance().getAllAchievements();
    QList<int> unlockedIds = DatabaseManager::instance().getUnlockedAchievements(m_playerId);

    for (const auto& def : allAchievements)
    {
        AchievementInfo info;
        info.id = def.id;
        info.name = def.name;
        info.description = def.description;
        info.isHidden = def.isHidden;
        info.isUnlocked = unlockedIds.contains(def.id);
        result.append(info);
    }

    return result;
}

/** @brief 获取当前玩家已解锁的成就数量 */
int AchievementSystem::getUnlockedCount() const
{
    if (m_playerId < 0) return 0;
    return DatabaseManager::instance().getUnlockedAchievementCount(m_playerId);
}

/** @brief 获取成就总数 */
int AchievementSystem::getTotalCount() const
{
    return DatabaseManager::instance().getAllAchievements().size();
}

/**
 * @brief 通用成就检查
 *
 * 从数据库加载玩家统计数据，逐一检查各项成就的解锁条件，
 * 最后检查元成就。
 */
void AchievementSystem::checkAchievements()
{
    if (m_playerId < 0) return;

    PlayerStats stats;
    if (!DatabaseManager::instance().loadPlayer(m_playerId, stats)) {
        return;
    }

    // 1. 破晓的凯旋 - 通关1次
    if (stats.clearCount >= 1)
        tryUnlock(1, "破晓的凯旋", "通关1次游戏");

    // 2. 终将踏破万重迷雾 - 通关50次
    if (stats.clearCount >= 50)
        tryUnlock(2, "终将踏破万重迷雾", "通关50次游戏");

    // 3. 亡者初识归途 - 死亡1次
    if (stats.deathCount >= 1)
        tryUnlock(3, "亡者初识归途", "累计死亡1次");

    // 4. 亡魂不眷未来之灾 - 死亡50次
    if (stats.deathCount >= 50)
        tryUnlock(4, "亡魂不眷未来之灾", "累计死亡50次");

    // 6. 森林馈赠之始 - 收集10颗草莓
    if (stats.strawberryCount >= 10)
        tryUnlock(6, "森林馈赠之始", "累计收集10颗草莓");

    // 7. 繁花终将硕果累累 - 收集100颗草莓
    if (stats.strawberryCount >= 100)
        tryUnlock(7, "繁花终将硕果累累", "累计收集100颗草莓");

    // 8. 火光照亮前路 - 使用30个炸弹
    if (stats.bombUsed >= 30)
        tryUnlock(8, "火光照亮前路", "累计使用30个炸弹");

    // 9. 爆炎终将吞没群山 - 使用300个炸弹
    if (stats.bombUsed >= 300)
        tryUnlock(9, "爆炎终将吞没群山", "累计使用300个炸弹");

    // 10. 碎石开辟新生 - 炸毁100个障碍物
    if (stats.obstacleDestroyed >= 100)
        tryUnlock(10, "碎石开辟新生", "累计炸毁100个障碍物");

    // 11. 群山终将在烈焰前俯首 - 炸毁500个障碍物
    if (stats.obstacleDestroyed >= 500)
        tryUnlock(11, "群山终将在烈焰前俯首", "累计炸毁500个障碍物");

    // 14. 远方仍在呼唤 - 移动1000格
    if (stats.stepCount >= 1000)
        tryUnlock(14, "远方仍在呼唤", "玩家共移动1000格");

    // 15. 脚步终将丈量世界 - 移动10000格
    if (stats.stepCount >= 10000)
        tryUnlock(15, "脚步终将丈量世界", "玩家共移动10000格");

    // 16. 故土朝日复东升 - 通关1000次
    if (stats.clearCount >= 1000)
        tryUnlock(16, "故土朝日复东升", "通关1000次游戏");

    // 17. 时间终将铭记旅人 - 100小时 = 360000秒
    if (stats.totalPlayTime >= 360000)
        tryUnlock(17, "时间终将铭记旅人", "游戏时间累计达到100小时");

    // 23. 世界尽头的回响 - 连续10局无伤通关
    if (stats.consecutiveNoDamageClears >= 10)
        tryUnlock(23, "世界尽头的回响", "连续10局无伤通关");

    // 25. 群星见证归人 - 365小时 = 1314000秒
    if (stats.totalPlayTime >= 1314000)
        tryUnlock(25, "群星见证归人", "累计游玩365小时");

    // 26. 愿火焰照亮归路 - 使用1000个炸弹
    if (stats.bombUsed >= 1000)
        tryUnlock(26, "愿火焰照亮归路", "累计点燃1000次炸弹");

    // 检查"完成所有成就"
    checkMetaAchievement();
}

/**
 * @brief 处理游戏通关事件
 *
 * 更新连续无伤通关计数，检查与通关相关的成就，
 * 然后运行通用成就检查。
 */
void AchievementSystem::onGameCleared(const GameClearContext& ctx)
{
    if (m_playerId < 0) return;

    PlayerStats stats;
    if (!DatabaseManager::instance().loadPlayer(m_playerId, stats))
        return;

    // 更新连续无伤通关计数
    if (ctx.healthAtEnd >= ctx.maxHealth)
    {
        stats.consecutiveNoDamageClears++;
        DatabaseManager::instance().setConsecutiveNoDamageClears(m_playerId, stats.consecutiveNoDamageClears);
    }
    else
    {
        stats.consecutiveNoDamageClears = 0;
        DatabaseManager::instance().setConsecutiveNoDamageClears(m_playerId, 0);
    }

    // 5. 将甘甜献予终点 - 收集所有草莓后通关
    if (ctx.strawberriesCollected >= ctx.maxStrawberries && ctx.maxStrawberries > 0)
        tryUnlock(5, "将甘甜献予终点", "收集所有草莓后通关");

    // 12. 无伤的远征 - 满生命值通关1次
    if (ctx.healthAtEnd >= ctx.maxHealth)
        tryUnlock(12, "无伤的远征", "以满生命值通关1次");

    // 13. 圣者不染尘埃 - 满生命值通关50次
    if (ctx.healthAtEnd >= ctx.maxHealth && stats.consecutiveNoDamageClears >= 50)
        tryUnlock(13, "圣者不染尘埃", "以满生命值通关50次");

    // 18. 残烛仍向黎明 - 以1点生命值通关
    if (ctx.healthAtEnd == 1)
        tryUnlock(18, "残烛仍向黎明", "以1点生命值通关");

    // 19. 唯有终点值得停留 - 通关时未收集草莓
    if (ctx.strawberriesCollected == 0)
        tryUnlock(19, "唯有终点值得停留", "通关时未收集到草莓");

    // 23. 世界尽头的回响 - 连续10局无伤通关
    if (stats.consecutiveNoDamageClears >= 10)
        tryUnlock(23, "世界尽头的回响", "连续10局无伤通关");

    // 27. 长夜终迎黎明 - 死亡后再次通关
    if (stats.deathCount >= 1)
        tryUnlock(27, "长夜终迎黎明", "首次死亡后再次通关");

    // 28. 风止于森林深处 - 不放置炸弹通关
    if (!ctx.bombPlacedThisRun)
        tryUnlock(28, "风止于森林深处", "一局游戏内不放置炸弹通关");

    // 运行通用检查
    checkAchievements();
}

/**
 * @brief 处理玩家死亡事件
 *
 * 重置连续无伤通关计数，并检查“把自己炸死”成就。
 */
void AchievementSystem::onPlayerDied(bool diedByBomb)
{
    if (m_playerId < 0) return;

    // 死亡后重置连续无伤通关计数
    DatabaseManager::instance().setConsecutiveNoDamageClears(m_playerId, 0);

    // 20. 此雷鸣是天罚抑或是祝福 - 把自己炸死
    if (diedByBomb)
        tryUnlock(20, "此雷鸣是天罚抑或是祝福", "把自己炸死");

    checkAchievements();
}

/** @brief 处理全部迷雾点亮事件，触发“愿旅人永不迷失”成就 */
void AchievementSystem::onAllFogRevealed()
{
    if (m_playerId < 0) return;

    // 29. 愿旅人永不迷失 - 第一次点亮全部迷雾
    tryUnlock(29, "愿旅人永不迷失", "第一次点亮全部迷雾");
}

/** @brief 处理秘籍输入事件，触发“无人知晓的仪式”成就 */
void AchievementSystem::onSecretCodeEntered()
{
    if (m_playerId < 0) return;

    // 21. 无人知晓的仪式
    tryUnlock(21, "无人知晓的仪式", "玩家输入\"wwssaaddbaba\"");
}

/** @brief 处理音乐和音效同时关闭事件，触发“万籁归于寂静”成就 */
void AchievementSystem::onBothSoundsOff()
{
    if (m_playerId < 0) return;

    // 22. 万籁归于寂静
    tryUnlock(22, "万籁归于寂静", "同时关闭背景音乐和游戏音效");
}

/**
 * @brief 检查元成就“与心爱的你行至世界尽头”
 *
 * 当除成就 30 以外的所有成就均已解锁时触发解锁。
 */
void AchievementSystem::checkMetaAchievement()
{
    if (m_playerId < 0) return;

    int total = getTotalCount();
    int unlocked = getUnlockedCount();

    // 30. 与心爱的你行至世界尽头 - 完成所有其他成就（29/30时触发，因为30本身不算在内）
    // 当除了成就30以外的所有成就都已解锁时触发
    if (total > 0 && unlocked >= total - 1)
    {
        // 检查是否只差成就30
        if (!DatabaseManager::instance().isAchievementUnlocked(m_playerId, 30))
        {
            tryUnlock(30, "与心爱的你行至世界尽头", "完成所有成就");
        }
    }
}

/**
 * @brief 尝试解锁指定成就
 *
 * 通过 DatabaseManager 尝试解锁，成功后发射成就解锁信号
 * 并检查元成就。
 */
void AchievementSystem::tryUnlock(int achievementId, const QString& name, const QString& description)
{
    if (DatabaseManager::instance().unlockAchievement(m_playerId, achievementId))
    {
        emit achievementUnlocked(name, description);
        qDebug() << "Achievement unlocked:" << name;

        // 每次解锁新成就后检查"完成所有成就"
        checkMetaAchievement();
    }
}
