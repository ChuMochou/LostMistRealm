#include "MainWindow.h"
#include "GameWidget.h"
#include "databasemanager.h"
#include <QMessageBox>
#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QInputDialog>
#include <QDialog>
#include <QTableWidget>
#include <QHeaderView>
#include <QLineEdit>
#include <QTextEdit>
#include <QPointer>
#include <QSettings>

/**
 * @brief 构造主窗口
 *
 * 初始化数据库、成就系统、AI 助手管理器、游戏组件和右侧控制面板，
 * 连接所有信号与槽，并启动背景音乐和游戏时间计时器。
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), achievementSystem(nullptr), aiManager(nullptr), currentPlayerId(-1), musicEnabled(true), soundEnabled(true)
{
    setWindowTitle("失落迷雾之境");

    // 初始化数据库
    if (!DatabaseManager::instance().initialize()) {
        QMessageBox::critical(this, "错误", "数据库初始化失败！");
    } else {
        // 创建成就系统
        achievementSystem = new AchievementSystem(this);
        connect(achievementSystem, SIGNAL(achievementUnlocked(QString, QString)),
                this, SLOT(onAchievementUnlocked(QString, QString)));

        // 获取或创建玩家
        currentPlayerId = DatabaseManager::instance().getOrCreatePlayer("Player");
        achievementSystem->setCurrentPlayer(currentPlayerId);
    }

    // 初始化 AI 助手管理器
    aiManager = new AIManager(this);
    // 设置 API Key（可从配置文件、环境变量或用户输入获取）
    // 从本地持久化存储中恢复上次保存的 API Key
    QSettings settings("LostMistRealm", "LostMistRealm");
    QString savedApiKey = settings.value("ai/apiKey").toString();
    if (!savedApiKey.isEmpty()) {
        aiManager->setApiKey(savedApiKey);
    }
    aiManager->setBaseUrl(QStringLiteral("https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions"));
    aiManager->setModel(QStringLiteral("qwen-turbo"));

    connect(aiManager, &AIManager::answerReady, this, &MainWindow::onAIAnswerReady);
    connect(aiManager, &AIManager::errorOccurred, this, &MainWindow::onAIError);

    const int MAP_SIZE = 23 * 32;
    const int PANEL_WIDTH = 200;

    heartPixmap = loadImage("heart.png", "player");
    if (!heartPixmap.isNull()) {
        heartPixmap = heartPixmap.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    strawberryPixmap = loadImage("strawberry.png", "object");
    if (!strawberryPixmap.isNull()) {
        strawberryPixmap = strawberryPixmap.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    bombPixmap = loadImage("bomb.png", "object");
    if (!bombPixmap.isNull()) {
        bombPixmap = bombPixmap.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    keyPixmap = loadImage("key.png", "object");
    if (!keyPixmap.isNull()) {
        keyPixmap = keyPixmap.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    gameWidget = new GameWidget();
    gameWidget->setDbPlayerId(currentPlayerId);
    gameWidget->setFixedSize(MAP_SIZE, MAP_SIZE);
    mainLayout->addWidget(gameWidget);

    QWidget *rightPanel = new QWidget();
    rightPanel->setFixedSize(PANEL_WIDTH, MAP_SIZE);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(10, 10, 10, 10);
    rightLayout->setSpacing(10);

    QGroupBox *statusGroup = new QGroupBox("玩家状态");
    QVBoxLayout *statusLayout = new QVBoxLayout(statusGroup);
    statusLayout->setContentsMargins(5, 5, 5, 5);
    statusLayout->setSpacing(5);

    QWidget *healthRow = new QWidget();
    QHBoxLayout *healthLayout = new QHBoxLayout(healthRow);
    healthLayout->setContentsMargins(0, 0, 0, 0);
    healthLayout->setSpacing(5);
    healthIcon = new QLabel();
    healthIcon->setPixmap(heartPixmap);
    healthText = new QLabel("3/3");
    healthBar = new QProgressBar();
    healthBar->setMaximum(100);
    healthBar->setValue(100);
    healthBar->setFixedHeight(16);
    healthBar->setFixedWidth(80);
    healthBar->setTextVisible(false);
    setupProgressBar(healthBar, "#ff4444");
    healthLayout->addWidget(healthIcon);
    healthLayout->addWidget(healthText);
    healthLayout->addWidget(healthBar);
    statusLayout->addWidget(healthRow);

    QWidget *strawberryRow = new QWidget();
    QHBoxLayout *strawberryLayout = new QHBoxLayout(strawberryRow);
    strawberryLayout->setContentsMargins(0, 0, 0, 0);
    strawberryLayout->setSpacing(5);
    strawberryIcon = new QLabel();
    strawberryIcon->setPixmap(strawberryPixmap);
    strawberryText = new QLabel("0/3");
    strawberryBar = new QProgressBar();
    strawberryBar->setMaximum(100);
    strawberryBar->setValue(0);
    strawberryBar->setFixedHeight(16);
    strawberryBar->setFixedWidth(80);
    strawberryBar->setTextVisible(false);
    setupProgressBar(strawberryBar, "#ff69b4");
    strawberryLayout->addWidget(strawberryIcon);
    strawberryLayout->addWidget(strawberryText);
    strawberryLayout->addWidget(strawberryBar);
    statusLayout->addWidget(strawberryRow);

    QWidget *bombRow = new QWidget();
    QHBoxLayout *bombLayout = new QHBoxLayout(bombRow);
    bombLayout->setContentsMargins(0, 0, 0, 0);
    bombLayout->setSpacing(5);
    bombIcon = new QLabel();
    bombIcon->setPixmap(bombPixmap);
    bombText = new QLabel("0/6");
    bombBar = new QProgressBar();
    bombBar->setMaximum(100);
    bombBar->setValue(0);
    bombBar->setFixedHeight(16);
    bombBar->setFixedWidth(80);
    bombBar->setTextVisible(false);
    setupProgressBar(bombBar, "#888888");
    bombLayout->addWidget(bombIcon);
    bombLayout->addWidget(bombText);
    bombLayout->addWidget(bombBar);
    statusLayout->addWidget(bombRow);

    QWidget *keyRow = new QWidget();
    QHBoxLayout *keyLayout = new QHBoxLayout(keyRow);
    keyLayout->setContentsMargins(0, 0, 0, 0);
    keyLayout->setSpacing(5);
    keyIcon = new QLabel();
    keyIcon->setPixmap(keyPixmap);
    keyText = new QLabel("0/1");
    keyBar = new QProgressBar();
    keyBar->setMaximum(100);
    keyBar->setValue(0);
    keyBar->setFixedHeight(16);
    keyBar->setFixedWidth(80);
    keyBar->setTextVisible(false);
    setupProgressBar(keyBar, "#ffd700");
    keyLayout->addWidget(keyIcon);
    keyLayout->addWidget(keyText);
    keyLayout->addWidget(keyBar);
    statusLayout->addWidget(keyRow);

    rightLayout->addWidget(statusGroup);

    QGroupBox *controlGroup = new QGroupBox("按键操作");
    QVBoxLayout *controlLayout = new QVBoxLayout(controlGroup);
    controlLayout->setContentsMargins(5, 5, 5, 5);
    controlLayout->setSpacing(3);

    QLabel *upLabel = new QLabel("W/↑    : 向上走");
    QLabel *downLabel = new QLabel("S/↓     : 向下走");
    QLabel *leftLabel = new QLabel("A/←   : 向左走");
    QLabel *rightLabel = new QLabel("D/→   : 向右走");
    QLabel *bombLabel = new QLabel("Space : 放置炸弹");
    QLabel *bombDescLabel = new QLabel("(炸弹：炸毁3*3范围内的障碍物)");
    bombDescLabel->setStyleSheet("font-size: 10px; color: #888888;");

    controlLayout->addWidget(upLabel);
    controlLayout->addWidget(downLabel);
    controlLayout->addWidget(leftLabel);
    controlLayout->addWidget(rightLabel);
    controlLayout->addWidget(bombLabel);
    controlLayout->addWidget(bombDescLabel);

    rightLayout->addWidget(controlGroup);

    collectionGroup = new QGroupBox("道具收集");
    collectionLayout = new QVBoxLayout(collectionGroup);
    collectionLayout->setContentsMargins(5, 5, 5, 5);
    collectionLayout->setSpacing(3);
    rightLayout->addWidget(collectionGroup);

    rightLayout->addStretch(1);

    QGroupBox *menuGroup = new QGroupBox("功能菜单");
    QVBoxLayout *menuLayout = new QVBoxLayout(menuGroup);
    menuLayout->setContentsMargins(5, 5, 5, 5);
    menuLayout->setSpacing(5);

    newGameBtn = new QPushButton("新游戏");
    menuLayout->addWidget(newGameBtn);

    achievementBtn = new QPushButton("成就");
    menuLayout->addWidget(achievementBtn);

    toggleMusicBtn = new QPushButton("关闭背景音乐");
    menuLayout->addWidget(toggleMusicBtn);

    toggleSoundBtn = new QPushButton("关闭音效");
    menuLayout->addWidget(toggleSoundBtn);

    QPushButton *aiAssistantBtn = new QPushButton("AI 助手");
    menuLayout->addWidget(aiAssistantBtn);

    exitGameBtn = new QPushButton("退出游戏");
    menuLayout->addWidget(exitGameBtn);

    rightLayout->addWidget(menuGroup);

    mainLayout->addWidget(rightPanel);

    connect(newGameBtn, SIGNAL(clicked()), this, SLOT(onNewGameClicked()));
    connect(achievementBtn, SIGNAL(clicked()), this, SLOT(onAchievementClicked()));
    connect(toggleMusicBtn, SIGNAL(clicked()), this, SLOT(onToggleMusicClicked()));
    connect(toggleSoundBtn, SIGNAL(clicked()), this, SLOT(onToggleSoundClicked()));
    connect(exitGameBtn, SIGNAL(clicked()), this, SLOT(onExitGameClicked()));
    connect(gameWidget, SIGNAL(statusChanged(int, int, int, int, int, int, int, int)),
            this, SLOT(onStatusChanged(int, int, int, int, int, int, int, int)));
    connect(gameWidget, SIGNAL(itemCollected(QString)),
            this, SLOT(onItemCollected(QString)));
    connect(gameWidget, SIGNAL(restartGameRequested()),
            this, SLOT(onRestartGameRequested()));
    connect(gameWidget, SIGNAL(playerDied(bool)),
            this, SLOT(onPlayerDied(bool)));
    connect(gameWidget, SIGNAL(gameCleared(int, int, bool)),
            this, SLOT(onGameCleared(int, int, bool)));
    connect(gameWidget, SIGNAL(allFogRevealed()),
            this, SLOT(onAllFogRevealed()));
    connect(gameWidget, SIGNAL(secretCodeEntered()),
            this, SLOT(onSecretCodeEntered()));
    connect(aiAssistantBtn, &QPushButton::clicked, this, &MainWindow::onAIAssistantClicked);

    gameWidget->sendInitialStatus();

    playBackgroundMusic();

    // 游戏时间计时器（每60秒触发一次）
    playTimeTimer = new QTimer(this);
    connect(playTimeTimer, SIGNAL(timeout()), this, SLOT(onPlayTimeTick()));
    playTimeTimer->start(60000);

    setFixedSize(MAP_SIZE + PANEL_WIDTH, MAP_SIZE);
}

/**
 * @brief 响应游戏状态变化
 *
 * 根据当前生命值、草莓数、炸弹数和钥匙数更新右侧面板的文本标签和进度条。
 */
void MainWindow::onStatusChanged(int health, int maxHealth, int strawberries, int maxStrawberries, int bombs, int maxBombs, int keys, int maxKeys)
{
    healthText->setText(QString("%1/%2").arg(health).arg(maxHealth));
    healthBar->setValue(maxHealth > 0 ? (health * 100) / maxHealth : 0);

    strawberryText->setText(QString("%1/%2").arg(strawberries).arg(maxStrawberries));
    strawberryBar->setValue(maxStrawberries > 0 ? (strawberries * 100) / maxStrawberries : 0);

    bombText->setText(QString("%1/%2").arg(bombs).arg(maxBombs));
    bombBar->setValue(maxBombs > 0 ? (bombs * 100) / maxBombs : 0);

    keyText->setText(QString("%1/%2").arg(keys).arg(maxKeys));
    keyBar->setValue(maxKeys > 0 ? (keys * 100) / maxKeys : 0);
}

/**
 * @brief 为进度条设置自定义样式
 *
 * 通过 QSS 设置进度条的边框、圆角和填充颜色。
 */
void MainWindow::setupProgressBar(QProgressBar *bar, const QString& color)
{
    QString styleSheet = QString(
        "QProgressBar {"
        "    border: 1px solid #cccccc;"
        "    border-radius: 4px;"
        "    background-color: #e0e0e0;"
        "}"
        "QProgressBar::chunk {"
        "    background-color: %1;"
        "    border-radius: 3px;"
        "}"
    ).arg(color);
    bar->setStyleSheet(styleSheet);
}

/**
 * @brief 响应道具收集事件
 *
 * 在右侧“道具收集”区域添加一条带绿色字样的提示标签。
 */
void MainWindow::onItemCollected(QString message)
{
    QLabel *label = new QLabel(message);
    label->setStyleSheet("font-size: 12px; color: #448844;");
    collectionLayout->addWidget(label);
}

/**
 * @brief 析构函数
 *
 * 停止音乐播放器并释放播放器和音频输出资源。
 */
MainWindow::~MainWindow()
{
    if (musicPlayer)
    {
        musicPlayer->stop();
        musicPlayer->deleteLater();
    }
    if (audioOutput)
    {
        audioOutput->deleteLater();
    }
}

/**
 * @brief 处理“新游戏”按钮点击
 *
 * 清空道具收集区域并调用 GameWidget 重新开始游戏。
 */
void MainWindow::onNewGameClicked()
{
    clearCollectionBar();
    gameWidget->restartGame();
}

/**
 * @brief 处理游戏重启请求
 *
 * 清空道具收集区域，以便新一轮游戏的道具信息重新显示。
 */
void MainWindow::onRestartGameRequested()
{
    clearCollectionBar();
}

/**
 * @brief 清空道具收集区域
 *
 * 遍历并删除 collectionLayout 中的所有子控件和布局项。
 */
void MainWindow::clearCollectionBar()
{
    if (collectionLayout)
    {
        while (QLayoutItem *item = collectionLayout->takeAt(0))
        {
            delete item->widget();
            delete item;
        }
    }
}

/**
 * @brief 切换背景音乐的播放/暂停状态
 *
 * 如果当前正在播放则暂停，否则恢复播放。
 * 同时检查“万籁归于寂静”成就（音乐和音效均关闭时触发）。
 */
void MainWindow::onToggleMusicClicked()
{
    if (!musicPlayer) return;

    if (musicPlayer->playbackState() == QMediaPlayer::PlayingState)
    {
        musicPlayer->pause();
        toggleMusicBtn->setText("开启背景音乐");
        musicEnabled = false;
    }
    else
    {
        musicPlayer->play();
        toggleMusicBtn->setText("关闭背景音乐");
        musicEnabled = true;
    }

    // 检查“万籁归于寂静”成就
    if (!musicEnabled && !soundEnabled && achievementSystem) {
        achievementSystem->onBothSoundsOff();
    }
}

/**
 * @brief 切换游戏音效的开启/关闭状态
 *
 * 更新音效状态并同步到 GameWidget，同时检查“万籁归于寂静”成就。
 */
void MainWindow::onToggleSoundClicked()
{
    soundEnabled = !soundEnabled;
    gameWidget->setSoundEnabled(soundEnabled);

    if (soundEnabled)
    {
        toggleSoundBtn->setText("关闭音效");
    }
    else
    {
        toggleSoundBtn->setText("开启音效");
    }

    // 检查“万籁归于寂静”成就
    if (!musicEnabled && !soundEnabled && achievementSystem) {
        achievementSystem->onBothSoundsOff();
    }
}

/**
 * @brief 处理“退出游戏”按钮点击
 *
 * 弹出确认对话框，用户点击“是”则退出应用程序。
 */
void MainWindow::onExitGameClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                               "退出游戏",
                                                               "确定要退出游戏吗？",
                                                               QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        QApplication::quit();
    }
}

/**
 * @brief 初始化并播放背景音乐
 *
 * 创建音频输出和媒体播放器，设置无限循环播放，
 * 在多个可能路径中搜索背景音乐文件并播放。
 */
void MainWindow::playBackgroundMusic()
{
    audioOutput = new QAudioOutput(this);
    musicPlayer = new QMediaPlayer(this);
    musicPlayer->setAudioOutput(audioOutput);
    musicPlayer->setLoops(QMediaPlayer::Infinite);
    audioOutput->setVolume(0.7);

    QString appDir = QCoreApplication::applicationDirPath();
    QString projectRoot = QDir(appDir).absoluteFilePath("../..");

    QStringList paths = {
        "resources/audio/bgm.mp3",
        "resources/audio/bgm.ogg",
        "resources/audio/bgm.wav",
        appDir + "/resources/audio/bgm.mp3",
        appDir + "/resources/audio/bgm.ogg",
        appDir + "/resources/audio/bgm.wav",
        QDir::currentPath() + "/resources/audio/bgm.mp3",
        QDir::currentPath() + "/resources/audio/bgm.ogg",
        QDir::currentPath() + "/resources/audio/bgm.wav",
        projectRoot + "/resources/audio/bgm.mp3",
        projectRoot + "/resources/audio/bgm.ogg",
        projectRoot + "/resources/audio/bgm.wav"
    };

    QString musicPath;
    for (const QString& path : paths)
    {
        if (QFile::exists(path))
        {
            musicPath = path;
            break;
        }
    }

    if (!musicPath.isEmpty())
    {
        musicPlayer->setSource(QUrl::fromLocalFile(musicPath));
        musicPlayer->play();
    }
}

/**
 * @brief 打开成就系统对话框
 *
 * 展示玩家的游戏统计信息、成就进度和完整成就列表，
 * 并提供“重置进度”和“关闭”按钮。
 */
void MainWindow::onAchievementClicked()
{
    if (!achievementSystem) return;

    QDialog dialog(this);
    dialog.setWindowTitle("成就系统");
    dialog.setFixedSize(500, 600);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    // 统计信息
    PlayerStats stats;
    if (DatabaseManager::instance().loadPlayer(currentPlayerId, stats)) {
        QGroupBox *statsGroup = new QGroupBox("游戏统计");
        QVBoxLayout *statsLayout = new QVBoxLayout(statsGroup);
        statsLayout->addWidget(new QLabel(QString("死亡次数: %1").arg(stats.deathCount)));
        statsLayout->addWidget(new QLabel(QString("通关次数: %1").arg(stats.clearCount)));
        statsLayout->addWidget(new QLabel(QString("收集草莓: %1").arg(stats.strawberryCount)));
        statsLayout->addWidget(new QLabel(QString("使用炸弹: %1").arg(stats.bombUsed)));
        statsLayout->addWidget(new QLabel(QString("炸毁障碍: %1").arg(stats.obstacleDestroyed)));
        statsLayout->addWidget(new QLabel(QString("移动步数: %1").arg(stats.stepCount)));
        layout->addWidget(statsGroup);
    }

    // 成就进度
    int unlocked = achievementSystem->getUnlockedCount();
    int total = achievementSystem->getTotalCount();
    QLabel *progressLabel = new QLabel(QString("成就进度: %1/%2").arg(unlocked).arg(total));
    progressLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    layout->addWidget(progressLabel);

    // 成就列表
    QTableWidget *table = new QTableWidget();
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels({"成就", "描述", "状态"});
    table->setColumnWidth(0, 180);
    table->setColumnWidth(1, 180);
    table->setColumnWidth(2, 77);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);

    QList<AchievementInfo> achievements = achievementSystem->getAllAchievementsWithStatus();
    table->setRowCount(achievements.size());

    for (int i = 0; i < achievements.size(); i++) {
        const AchievementInfo& a = achievements[i];

        QString displayName = a.name;
        QString displayDesc = (a.isHidden && !a.isUnlocked) ? "？？？" : a.description;

        table->setItem(i, 0, new QTableWidgetItem(displayName));
        table->setItem(i, 1, new QTableWidgetItem(displayDesc));
        table->setItem(i, 2, new QTableWidgetItem(a.isUnlocked ? "✓ 已解锁" : "未解锁"));
    }

    layout->addWidget(table);

    // 底部按钮行：重置进度（左） + 关闭（右），重置按钮尺寸为关闭按钮的一半
    QWidget *buttonRow = new QWidget();
    QHBoxLayout *buttonLayout = new QHBoxLayout(buttonRow);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(8);

    QPushButton *resetBtn = new QPushButton("重置进度");
    resetBtn->setFixedSize(155, 28);
    resetBtn->setStyleSheet(
        "QPushButton {"
        "    background-color: #cc4444;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 4px;"
        "    font-size: 11px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #aa2222;"
        "}"
    );

    QPushButton *closeBtn = new QPushButton("关闭");
    closeBtn->setFixedSize(310, 28);

    buttonLayout->addStretch(1);
    buttonLayout->addWidget(resetBtn);
    buttonLayout->addWidget(closeBtn);

    layout->addWidget(buttonRow);

    // 关闭按钮
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);

    // 重置进度按钮
    connect(resetBtn, &QPushButton::clicked, [&]() {
        QMessageBox::StandardButton reply = QMessageBox::question(
            &dialog,
            "重置进度",
            "这将清除你所有的统计信息和已完成的成就，你确定吗？",
            QMessageBox::Yes | QMessageBox::No
        );
        if (reply == QMessageBox::Yes) {
            DatabaseManager::instance().resetPlayerStats(currentPlayerId);
            DatabaseManager::instance().resetPlayerAchievements(currentPlayerId);
            dialog.accept();
        }
    });

    dialog.exec();
}

/**
 * @brief 响应玩家死亡事件
 *
 * 将死亡事件转发给成就系统以检查相关成就（如被炸弹炸死等）。
 */
void MainWindow::onPlayerDied(bool diedByBomb)
{
    if (achievementSystem) {
        achievementSystem->onPlayerDied(diedByBomb);
    }
}

/**
 * @brief 响应游戏通关事件
 *
 * 构造 GameClearContext 上下文（包括生命值、草莓数、是否放置炸弹等），
 * 并转发给成就系统进行通关相关成就的检查。
 */
void MainWindow::onGameCleared(int health, int strawberries, bool bombPlacedThisRun)
{
    if (achievementSystem) {
        GameClearContext ctx;
        ctx.healthAtEnd = health;
        ctx.maxHealth = 3;
        ctx.strawberriesCollected = strawberries;
        ctx.maxStrawberries = 3;
        ctx.bombPlacedThisRun = bombPlacedThisRun;
        ctx.allFogRevealed = false;
        achievementSystem->onGameCleared(ctx);
    }
}

/**
 * @brief 响应全部迷雾点亮事件
 *
 * 将事件转发给成就系统，触发“愿旅人永不迷失”成就检查。
 */
void MainWindow::onAllFogRevealed()
{
    if (achievementSystem) {
        achievementSystem->onAllFogRevealed();
    }
}

/**
 * @brief 响应秘籍输入事件
 *
 * 将秘籍输入事件转发给成就系统，触发“无人知晓的仪式”成就检查。
 */
void MainWindow::onSecretCodeEntered()
{
    if (achievementSystem) {
        achievementSystem->onSecretCodeEntered();
    }
}

/**
 * @brief 游戏时间计时器回调（每60秒触发一次）
 *
 * 向数据库累加60秒游戏时间，并运行成就检查以触发时间相关成就。
 */
void MainWindow::onPlayTimeTick()
{
    if (currentPlayerId >= 0) {
        DatabaseManager::instance().addPlayTime(currentPlayerId, 60);
        if (achievementSystem) {
            achievementSystem->checkAchievements();
        }
    }
}

/**
 * @brief 响应成就解锁事件
 *
 * 弹出信息提示框，通知玩家解锁了哪个成就及其描述。
 */
void MainWindow::onAchievementUnlocked(const QString& name, const QString& description)
{
    QMessageBox::information(this, "成就解锁",
                             QString("恭喜解锁成就: %1\n%2").arg(name, description));
}

// ---------------------------------------------------------------------------
// AI 助手对话框
// ---------------------------------------------------------------------------
/**
 * @brief 打开 AI 助手对话框
 *
 * 创建模态对话框，包含 API Key 输入、聊天显示区域和输入区域，
 * 通过临时信号连接实现异步 AI 问答交互，对话框关闭后自动断开连接。
 */
void MainWindow::onAIAssistantClicked()
{
    // 创建模态对话框
    QDialog dialog(this);
    dialog.setWindowTitle("AI 游戏助手");
    dialog.setFixedSize(520, 500);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->setSpacing(8);

    // API Key 输入区域（如果未配置）
    QLineEdit *apiKeyEdit = nullptr;
    if (aiManager && !aiManager->isBusy()) {
        QWidget *apiKeyRow = new QWidget();
        QHBoxLayout *apiKeyLayout = new QHBoxLayout(apiKeyRow);
        apiKeyLayout->setContentsMargins(0, 0, 0, 0);
        apiKeyLayout->addWidget(new QLabel("API Key:"));
        apiKeyEdit = new QLineEdit();
        apiKeyEdit->setPlaceholderText("请输入阿里百炼 API Key");
        apiKeyEdit->setEchoMode(QLineEdit::Password);
        // 自动填充上次保存的 API Key
        QSettings aiSettings("LostMistRealm", "LostMistRealm");
        QString lastKey = aiSettings.value("ai/apiKey").toString();
        if (!lastKey.isEmpty()) {
            apiKeyEdit->setText(lastKey);
        }
        apiKeyLayout->addWidget(apiKeyEdit);
        layout->addWidget(apiKeyRow);
    }

    // 聊天显示区域
    QTextEdit *chatDisplay = new QTextEdit();
    chatDisplay->setReadOnly(true);
    chatDisplay->setPlaceholderText("AI 回答将显示在这里...");
    layout->addWidget(chatDisplay);

    // 输入区域
    QWidget *inputRow = new QWidget();
    QHBoxLayout *inputLayout = new QHBoxLayout(inputRow);
    inputLayout->setContentsMargins(0, 0, 0, 0);

    QLineEdit *questionEdit = new QLineEdit();
    questionEdit->setPlaceholderText("输入你的游戏问题...");
    inputLayout->addWidget(questionEdit);

    QPushButton *sendBtn = new QPushButton("发送");
    inputLayout->addWidget(sendBtn);

    layout->addWidget(inputRow);

    // 关闭按钮
    QPushButton *closeBtn = new QPushButton("关闭");
    layout->addWidget(closeBtn);

    // 局部变量用于保存对话框指针（供 lambda 使用）
    QPointer<QDialog> dialogPtr(&dialog);
    QPointer<QTextEdit> chatDisplayPtr(chatDisplay);
    QPointer<QLineEdit> questionEditPtr(questionEdit);
    QPointer<QPushButton> sendBtnPtr(sendBtn);

    // 临时连接：回答就绪 -> 显示到聊天区域
    auto answerConn = connect(aiManager, &AIManager::answerReady,
        [chatDisplayPtr, sendBtnPtr, questionEditPtr](const QString &answer) {
            if (!chatDisplayPtr) return;
            chatDisplayPtr->append(QStringLiteral("【AI】%1\n").arg(answer));
            if (sendBtnPtr) sendBtnPtr->setEnabled(true);
            if (questionEditPtr) questionEditPtr->clear();
        });

    // 临时连接：错误 -> 显示错误信息
    auto errorConn = connect(aiManager, &AIManager::errorOccurred,
        [chatDisplayPtr, sendBtnPtr](const QString &errMsg) {
            if (!chatDisplayPtr) return;
            chatDisplayPtr->append(QStringLiteral("【错误】%1\n").arg(errMsg));
            if (sendBtnPtr) sendBtnPtr->setEnabled(true);
        });

    // 发送按钮点击
    connect(sendBtn, &QPushButton::clicked, [&]() {
        QString question = questionEdit->text().trimmed();
        if (question.isEmpty()) return;

        // 如果用户填写了 API Key，则更新配置并持久化保存
        if (apiKeyEdit && !apiKeyEdit->text().trimmed().isEmpty()) {
            QString newKey = apiKeyEdit->text().trimmed();
            aiManager->setApiKey(newKey);
            QSettings saveSettings("LostMistRealm", "LostMistRealm");
            saveSettings.setValue("ai/apiKey", newKey);
        }

        // 检查 API Key
        if (aiManager->isBusy()) {
            chatDisplay->append(QStringLiteral("【系统】正在等待AI回复，请稍候...\n"));
            return;
        }

        chatDisplay->append(QStringLiteral("【你】%1\n").arg(question));
        sendBtn->setEnabled(false);
        aiManager->ask(question);
    });

    // 回车发送
    connect(questionEdit, &QLineEdit::returnPressed, sendBtn, &QPushButton::click);

    // 关闭按钮
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);

    // 显示对话框
    dialog.exec();

    // 对话框关闭后断开临时连接
    disconnect(answerConn);
    disconnect(errorConn);
}

/**
 * @brief 全局 AI 回答就绪槽函数
 *
 * 当对话框打开时，回答已通过临时连接直接写入聊天区域。
 * 此处保留作为全局信号接口，便于后续扩展（如弹出通知）。
 */
void MainWindow::onAIAnswerReady(const QString &answer)
{
    // 当对话框打开时，回答会通过临时连接直接写入聊天区域
    // 此处保留作为全局信号接口，便于后续扩展（如弹出通知）
    Q_UNUSED(answer);
}

/**
 * @brief 全局 AI 错误槽函数
 *
 * 错误已通过临时连接显示在对话框中，此处保留用于后续扩展。
 */
void MainWindow::onAIError(const QString &errorMessage)
{
    // 同上，错误已通过临时连接显示在对话框中
    Q_UNUSED(errorMessage);
}