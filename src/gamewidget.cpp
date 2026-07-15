#include "GameWidget.h"
#include <QPainter>
#include <QKeyEvent>
#include <QMessageBox>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>

const int CELL_SIZE = 32;
const int TILE_SIZE = 16;

/**
 * @brief 从指定子目录加载图片资源
 *
 * 在应用程序目录、当前目录等多个路径中搜索图片文件，
 * 返回第一个成功加载的 QPixmap。
 *
 * @param filename 文件名
 * @param subdir 子目录名
 * @return 加载的图片，未找到则返回空 QPixmap
 */
QPixmap loadImage(const QString& filename, const QString& subdir)
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString filePath;

    filePath = appDir + "/resources/images/" + subdir + "/" + filename;
    QPixmap pixmap(filePath);
    if(!pixmap.isNull())
        return pixmap;

    filePath = appDir + "/../resources/images/" + subdir + "/" + filename;
    pixmap.load(filePath);
    if(!pixmap.isNull())
        return pixmap;

    filePath = appDir + "/../../resources/images/" + subdir + "/" + filename;
    pixmap.load(filePath);
    if(!pixmap.isNull())
        return pixmap;

    filePath = appDir + "/../../../resources/images/" + subdir + "/" + filename;
    pixmap.load(filePath);
    if(!pixmap.isNull())
        return pixmap;

    filePath = appDir + "/../../../../resources/images/" + subdir + "/" + filename;
    pixmap.load(filePath);
    if(!pixmap.isNull())
        return pixmap;

    filePath = QDir::currentPath() + "/resources/images/" + subdir + "/" + filename;
    pixmap.load(filePath);
    if(!pixmap.isNull())
        return pixmap;

    filePath = QDir::currentPath() + "/../resources/images/" + subdir + "/" + filename;
    pixmap.load(filePath);
    if(!pixmap.isNull())
        return pixmap;

    filePath = QDir::currentPath() + "/../../resources/images/" + subdir + "/" + filename;
    pixmap.load(filePath);
    return pixmap;
}

/**
 * @brief 从精灵图中提取指定行列的单个图块，并缩放到单元格尺寸
 *
 * @param spriteSheet 精灵图源图
 * @param row 行索引
 * @param col 列索引
 * @param tileSize 原始图块尺寸
 * @return 缩放后的图块 QPixmap
 */
QPixmap extractTile(const QPixmap& spriteSheet, int row, int col, int tileSize = TILE_SIZE)
{
    if(spriteSheet.isNull())
        return QPixmap();

    int x = col * tileSize;
    int y = row * tileSize;

    if(x + tileSize > spriteSheet.width() || y + tileSize > spriteSheet.height())
        return QPixmap();

    QPixmap tile = spriteSheet.copy(x, y, tileSize, tileSize);
    return tile.scaled(CELL_SIZE, CELL_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

/**
 * @brief 游戏组件构造函数
 *
 * 加载所有贴图和音效资源，初始化各种定时器（动画、迷雾、炸弹、爆炸），
 * 设置焦点策略并初始化迷雾可见性。
 */
GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent), useTextures(true)
{
    setFocusPolicy(Qt::StrongFocus);

    grassPixmap = loadImage("grass.png", "floor");
    if(!grassPixmap.isNull())
    {
        grassPixmap = grassPixmap.scaled(CELL_SIZE, CELL_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    pathPixmap = loadImage("path.png");
    if(!pathPixmap.isNull())
        pathPixmap = pathPixmap.scaled(CELL_SIZE, CELL_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    treePixmap = loadImage("tree.png", "floor");
    if(!treePixmap.isNull())
    {
        treePixmap = treePixmap.scaled(CELL_SIZE, CELL_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    waterPixmap = loadImage("water.png", "floor");
    if(!waterPixmap.isNull())
    {
        waterPixmap = waterPixmap.scaled(CELL_SIZE, CELL_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    stonePixmap = loadImage("stone.png", "floor");
    if(!stonePixmap.isNull())
    {
        stonePixmap = stonePixmap.scaled(CELL_SIZE, CELL_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    endPixmap = loadImage("house.png", "floor");
    if(!endPixmap.isNull())
        endPixmap = endPixmap.scaled(CELL_SIZE, CELL_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    doorPixmap = loadImage("door.png", "floor");
    if(!doorPixmap.isNull())
    {
        doorPixmap = doorPixmap.scaled(CELL_SIZE, CELL_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    keyPixmap = loadImage("key.png", "object");
    if(!keyPixmap.isNull())
    {
        keyPixmap = keyPixmap.scaled(CELL_SIZE, CELL_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    strawberryPixmap = loadImage("strawberry.png", "object");
    if(!strawberryPixmap.isNull())
    {
        strawberryPixmap = strawberryPixmap.scaled(CELL_SIZE, CELL_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    bombPixmap = loadImage("bomb.png", "object");
    if(!bombPixmap.isNull())
    {
        bombPixmap = bombPixmap.scaled(CELL_SIZE, CELL_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    fogPixmap = loadImage("cloud.png", "floor");
    if(!fogPixmap.isNull())
    {
        fogPixmap = fogPixmap.scaled(CELL_SIZE, CELL_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }

    playerSpriteSheet = loadImage("player.png", "player");

    for (int i = 0; i < 6; i++)
    {
        QString filename = QString("bomb_%1.png").arg(i + 1);
        explosionPixmaps[i] = loadImage(filename, "VFX");
        if (!explosionPixmaps[i].isNull())
        {
            explosionPixmaps[i] = explosionPixmaps[i].scaled(CELL_SIZE, CELL_SIZE, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    }

    if(grassPixmap.isNull() && pathPixmap.isNull() && treePixmap.isNull())
    {
        useTextures = false;
    }

    animationTimer = new QTimer(this);
    connect(animationTimer, SIGNAL(timeout()), this, SLOT(updateAnimation()));
    animationTimer->start(16);

    frameTimer = new QTimer(this);
    connect(frameTimer, SIGNAL(timeout()), this, SLOT(updateFrame()));

    fogTimer = new QTimer(this);
    connect(fogTimer, SIGNAL(timeout()), this, SLOT(updateFogAnimation()));
    fogTimer->start(20);

    bombTimer = new QTimer(this);
    connect(bombTimer, SIGNAL(timeout()), this, SLOT(updateBombTimers()));
    bombTimer->start(16);

    explosionTimer = new QTimer(this);
    connect(explosionTimer, SIGNAL(timeout()), this, SLOT(updateExplosionAnimation()));
    explosionTimer->start(EXPLOSION_FRAME_DURATION);

    QString appDir = QCoreApplication::applicationDirPath();
    QString projectRoot = QDir(appDir).absoluteFilePath("../..");

    bombAudioOutput = new QAudioOutput(this);
    bombSound = new QMediaPlayer(this);
    bombSound->setAudioOutput(bombAudioOutput);
    bombAudioOutput->setVolume(0.5);
    QStringList bombPaths = {
        "resources/audio/bomb.wav",
        appDir + "/resources/audio/bomb.wav",
        QDir::currentPath() + "/resources/audio/bomb.wav",
        projectRoot + "/resources/audio/bomb.wav"
    };
    for (const QString& path : bombPaths)
    {
        if (QFile::exists(path))
        {
            bombSound->setSource(QUrl::fromLocalFile(path));
            break;
        }
    }

    pickUpAudioOutput = new QAudioOutput(this);
    pickUpSound = new QMediaPlayer(this);
    pickUpSound->setAudioOutput(pickUpAudioOutput);
    pickUpAudioOutput->setVolume(0.5);
    QStringList pickUpPaths = {
        "resources/audio/pick_up.wav",
        appDir + "/resources/audio/pick_up.wav",
        QDir::currentPath() + "/resources/audio/pick_up.wav",
        projectRoot + "/resources/audio/pick_up.wav"
    };
    for (const QString& path : pickUpPaths)
    {
        if (QFile::exists(path))
        {
            pickUpSound->setSource(QUrl::fromLocalFile(path));
            break;
        }
    }

    soundEnabled = true;
    bombPlacedThisRun = false;
    currentRunSteps = 0;
    secretCodeBuffer = "";
    dbPlayerId = -1;

    maze.initVisibility();
    maze.updateVisibility(player.position.y(), player.position.x());

    emit statusChanged(player.health, 3, player.strawberryCount, 3, player.bombCount, 6, player.hasKey ? 1 : 0, 1);
}

/**
 * @brief 发送初始状态信号
 *
 * 用于在主窗口初始化完成后，触发右侧面板的初始显示。
 */
void GameWidget::sendInitialStatus()
{
    emit statusChanged(player.health, 3, player.strawberryCount, 3, player.bombCount, 6, player.hasKey ? 1 : 0, 1);
}

/**
 * @brief 绘制事件
 *
 * 渲染迷宫地图的所有单元格（含迷雾效果）、玩家精灵、
 * 已放置的炸弹和爆炸效果。支持贴图渲染和颜色填充两种模式。
 */
void GameWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    for (int row = 0; row < Maze::ROWS; row++)
    {
        for (int col = 0; col < Maze::COLS; col++)
        {
            QRect cell(col * CELL_SIZE,
                       row * CELL_SIZE,
                       CELL_SIZE,
                       CELL_SIZE);

            float fogOpacity = maze.fogAlpha[row][col] / 255.0f;
            float contentOpacity = 1.0f - fogOpacity;

            if(useTextures)
            {
                if(!grassPixmap.isNull())
                    painter.drawPixmap(cell, grassPixmap);
                else
                    painter.fillRect(cell, QColor(170, 215, 120));

                switch (maze.map[row][col])
                {
                case Maze::Grass:
                    break;

                case Maze::Path:
                    painter.setOpacity(contentOpacity);
                    if(!pathPixmap.isNull())
                        painter.drawPixmap(cell, pathPixmap);
                    else
                        painter.fillRect(cell, QColor(182, 152, 102));
                    painter.setOpacity(1.0);
                    break;

                case Maze::Tree:
                    painter.setOpacity(contentOpacity);
                    if(!treePixmap.isNull())
                        painter.drawPixmap(cell, treePixmap);
                    else
                        painter.fillRect(cell, QColor(48, 110, 52));
                    painter.setOpacity(1.0);
                    break;

                case Maze::Water:
                    painter.setOpacity(contentOpacity);
                    if(!waterPixmap.isNull())
                        painter.drawPixmap(cell, waterPixmap);
                    else
                        painter.fillRect(cell, QColor(78, 165, 235));
                    painter.setOpacity(1.0);
                    break;

                case Maze::Stone:
                    painter.setOpacity(contentOpacity);
                    if(!stonePixmap.isNull())
                        painter.drawPixmap(cell, stonePixmap);
                    else
                        painter.fillRect(cell, QColor(135, 135, 140));
                    painter.setOpacity(1.0);
                    break;

                case Maze::Start:
                    break;

                case Maze::End:
                    painter.setOpacity(contentOpacity);
                    if(!endPixmap.isNull())
                        painter.drawPixmap(cell, endPixmap);
                    else
                        painter.fillRect(cell, QColor(250, 230, 90));
                    painter.setOpacity(1.0);
                    break;

                case Maze::Door:
                    painter.setOpacity(contentOpacity);
                    if(!doorPixmap.isNull())
                        painter.drawPixmap(cell, doorPixmap);
                    else
                        painter.fillRect(cell, QColor(30, 30, 30));
                    painter.setOpacity(1.0);
                    break;

                case Maze::Key:
                    painter.setOpacity(contentOpacity);
                    if(!keyPixmap.isNull())
                        painter.drawPixmap(cell, keyPixmap);
                    else
                        painter.fillRect(cell, QColor(255, 215, 0));
                    painter.setOpacity(1.0);
                    break;

                case Maze::Strawberry:
                    painter.setOpacity(contentOpacity);
                    if(!strawberryPixmap.isNull())
                        painter.drawPixmap(cell, strawberryPixmap);
                    else
                        painter.fillRect(cell, QColor(255, 105, 180));
                    painter.setOpacity(1.0);
                    break;

                case Maze::Bomb:
                    painter.setOpacity(contentOpacity);
                    if(!bombPixmap.isNull())
                        painter.drawPixmap(cell, bombPixmap);
                    else
                        painter.fillRect(cell, QColor(50, 50, 50));
                    painter.setOpacity(1.0);
                    break;

                default:
                    break;
                }

                if (maze.fogAlpha[row][col] > 0)
                {
                    painter.setOpacity(fogOpacity);
                    if(!fogPixmap.isNull())
                        painter.drawPixmap(cell, fogPixmap);
                    else
                        painter.fillRect(cell, QColor(255, 255, 255));
                    painter.setOpacity(1.0);
                }
            }
            else
            {
                painter.fillRect(cell, QColor(170, 215, 120));

                switch (maze.map[row][col])
                {
                case Maze::Grass:
                    break;

                case Maze::Path:
                    painter.setOpacity(contentOpacity);
                    painter.fillRect(cell, QColor(182, 152, 102));
                    painter.setOpacity(1.0);
                    break;

                case Maze::Tree:
                    painter.setOpacity(contentOpacity);
                    painter.fillRect(cell, QColor(48, 110, 52));
                    painter.setOpacity(1.0);
                    break;

                case Maze::Water:
                    painter.setOpacity(contentOpacity);
                    painter.fillRect(cell, QColor(78, 165, 235));
                    painter.setOpacity(1.0);
                    break;

                case Maze::Stone:
                    painter.setOpacity(contentOpacity);
                    painter.fillRect(cell, QColor(135, 135, 140));
                    painter.setOpacity(1.0);
                    break;

                case Maze::Start:
                    painter.setOpacity(contentOpacity);
                    painter.fillRect(cell, QColor(250, 230, 90));
                    painter.setOpacity(1.0);
                    break;

                case Maze::End:
                    painter.setOpacity(contentOpacity);
                    painter.fillRect(cell, QColor(250, 230, 90));
                    painter.setOpacity(1.0);
                    break;

                case Maze::Door:
                    painter.setOpacity(contentOpacity);
                    painter.fillRect(cell, QColor(30, 30, 30));
                    painter.setOpacity(1.0);
                    break;

                case Maze::Key:
                    painter.setOpacity(contentOpacity);
                    painter.fillRect(cell, QColor(255, 215, 0));
                    painter.setOpacity(1.0);
                    break;

                case Maze::Strawberry:
                    painter.setOpacity(contentOpacity);
                    painter.fillRect(cell, QColor(255, 105, 180));
                    painter.setOpacity(1.0);
                    break;

                case Maze::Bomb:
                    painter.setOpacity(contentOpacity);
                    painter.fillRect(cell, QColor(50, 50, 50));
                    painter.setOpacity(1.0);
                    break;
                }

                if (maze.fogAlpha[row][col] > 0)
                {
                    painter.setOpacity(fogOpacity);
                    painter.fillRect(cell, QColor(255, 255, 255));
                    painter.setOpacity(1.0);
                }
            }
        }
    }

    float playerScale = 3.0f;
    int playerSize = (int)(CELL_SIZE * playerScale);
    int offset = (playerSize - CELL_SIZE) / 2;

    QRect playerRect((int)(player.x * CELL_SIZE) - offset,
                     (int)(player.y * CELL_SIZE) - offset,
                     playerSize,
                     playerSize);

    if(useTextures && !playerSpriteSheet.isNull())
    {
        int frameWidth = playerSpriteSheet.width() / 4;
        int frameHeight = playerSpriteSheet.height() / 4;

        int sourceX = player.currentFrame * frameWidth;
        int sourceY = player.direction * frameHeight;

        QRect sourceRect(sourceX, sourceY, frameWidth, frameHeight);
        painter.drawPixmap(playerRect, playerSpriteSheet, sourceRect);
    }
    else
    {
        painter.setBrush(QColor(255, 60, 60));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(playerRect.adjusted(6,6,-6,-6));
    }

    for (const PlacedBomb& bomb : placedBombs)
    {
        QRect bombRect(bomb.col * CELL_SIZE,
                       bomb.row * CELL_SIZE,
                       CELL_SIZE,
                       CELL_SIZE);

        if (useTextures && !bombPixmap.isNull())
        {
            painter.drawPixmap(bombRect, bombPixmap);
        }
        else
        {
            painter.setBrush(QColor(80, 80, 80));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(bombRect.adjusted(4, 4, -4, -4));
        }

        float progress = bomb.timer / BOMB_EXPLODE_TIME;
        int ringSize = CELL_SIZE - static_cast<int>(progress * CELL_SIZE * 0.3f);
        QRect ringRect((bomb.col * CELL_SIZE) + (CELL_SIZE - ringSize) / 2,
                       (bomb.row * CELL_SIZE) + (CELL_SIZE - ringSize) / 2,
                       ringSize,
                       ringSize);

        QColor ringColor(255, 0, 0);
        ringColor.setAlpha(100);
        painter.setBrush(Qt::NoBrush);
        painter.setPen(QPen(ringColor, 2));
        painter.drawEllipse(ringRect);
    }

    for (const Explosion& explosion : explosions)
    {
        int x = (explosion.col - 1) * CELL_SIZE;
        int y = (explosion.row - 1) * CELL_SIZE;
        int size = CELL_SIZE * 3;
        QRect explosionRect(x, y, size, size);

        if (useTextures && explosion.currentFrame >= 0 && explosion.currentFrame < 6 && !explosionPixmaps[explosion.currentFrame].isNull())
        {
            painter.drawPixmap(explosionRect, explosionPixmaps[explosion.currentFrame]);
        }
        else
        {
            QColor color(255, 100, 0);
            float alpha = 1.0f - (explosion.currentFrame / (float)EXPLOSION_FRAMES);
            color.setAlpha(static_cast<int>(alpha * 255));
            painter.setBrush(color);
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(explosionRect);
        }
    }
}

/**
 * @brief 键盘按下事件处理
 *
 * 处理 WASD/方向键移动、空格放置炸弹、B键参与秘籍检测。
 * 同时更新秘籍输入缓冲区并检查是否匹配 "wwssaaddbaba"。
 */
void GameWidget::keyPressEvent(QKeyEvent *event)
{
    int newRow = player.position.y();
    int newCol = player.position.x();
    Player::Direction newDirection = player.direction;

    switch(event->key())
    {
    case Qt::Key_W:
    case Qt::Key_Up:
        newRow--;
        newDirection = Player::Up;
        secretCodeBuffer += 'w';
        break;

    case Qt::Key_S:
    case Qt::Key_Down:
        newRow++;
        newDirection = Player::Down;
        secretCodeBuffer += 's';
        break;

    case Qt::Key_A:
    case Qt::Key_Left:
        newCol--;
        newDirection = Player::Left;
        secretCodeBuffer += 'a';
        break;

    case Qt::Key_D:
    case Qt::Key_Right:
        newCol++;
        newDirection = Player::Right;
        secretCodeBuffer += 'd';
        break;

    case Qt::Key_Space:
        placeBomb();
        return;

    case Qt::Key_B:
        secretCodeBuffer += 'b';
        if (secretCodeBuffer.length() > 12)
            secretCodeBuffer = secretCodeBuffer.right(12);
        if (secretCodeBuffer == "wwssaaddbaba") {
            secretCodeBuffer = "";
            emit secretCodeEntered();
        }
        return;

    default:
        return;
    }

    // 检查秘籍 "wwssaaddbaba"
    if (secretCodeBuffer.length() > 12)
        secretCodeBuffer = secretCodeBuffer.right(12);
    if (secretCodeBuffer == "wwssaaddbaba") {
        secretCodeBuffer = "";
        emit secretCodeEntered();
    }

    player.setDirection(newDirection);

    if(maze.isWalkable(newRow, newCol, player.hasKey))
    {
        if(!player.isMoving)
        {
            player.targetRow = newRow;
            player.targetCol = newCol;
            player.startMoveAnimation();
            frameTimer->start(50);
        }
    }
    else if(maze.map[newRow][newCol] == Maze::Door)
    {
        QMessageBox::information(this,
                                 "提示",
                                 "需要钥匙才能打开这扇门！");
    }

    update();
}

/**
 * @brief 更新玩家动画帧
 *
 * 由 frameTimer 触发，推进玩家精灵的动画帧。
 */
void GameWidget::updateFrame()
{
    player.nextFrame();
    update();
}

/**
 * @brief 更新玩家移动动画
 *
 * 由 animationTimer 触发，平滑移动玩家到目标位置。
 * 到达目标后处理道具拾取、迷雾更新、通关/死亡判定等逻辑。
 */
void GameWidget::updateAnimation()
{
    if(player.isMoving)
    {
        float targetX = player.targetCol;
        float targetY = player.targetRow;

        float dx = targetX - player.x;
        float dy = targetY - player.y;

        float distance = sqrt(dx * dx + dy * dy);

        if(distance < MOVE_SPEED)
        {
            player.x = targetX;
            player.y = targetY;
            player.position = QPoint(player.targetCol, player.targetRow);
            player.stopMoveAnimation();
            frameTimer->stop();

            // 追踪步数
            currentRunSteps++;
            DatabaseManager::instance().addSteps(dbPlayerId, 1);

            int row = player.position.y();
            int col = player.position.x();

            maze.updateVisibility(row, col);

            // 检查是否全部迷雾点亮
            {
                bool allVisible = true;
                for (int r = 0; r < Maze::ROWS && allVisible; r++) {
                    for (int c = 0; c < Maze::COLS && allVisible; c++) {
                        if (!maze.visible[r][c]) allVisible = false;
                    }
                }
                if (allVisible) emit allFogRevealed();
            }

            if(maze.map[row][col] == Maze::Key)
            {
                player.hasKey = true;
                maze.map[row][col] = Maze::Grass;
                if (soundEnabled && pickUpSound)
                {
                    pickUpSound->stop();
                    pickUpSound->setPosition(0);
                    pickUpSound->play();
                }
                emit itemCollected("收集到1把钥匙");
                emit statusChanged(player.health, 3, player.strawberryCount, 3, player.bombCount, 6, 1, 1);
            }
            else if(maze.map[row][col] == Maze::Strawberry)
            {
                player.strawberryCount++;
                maze.map[row][col] = Maze::Grass;
                DatabaseManager::instance().addStrawberries(dbPlayerId, 1);
                if (soundEnabled && pickUpSound)
                {
                    pickUpSound->stop();
                    pickUpSound->setPosition(0);
                    pickUpSound->play();
                }
                emit itemCollected("收集到1颗草莓");
                emit statusChanged(player.health, 3, player.strawberryCount, 3, player.bombCount, 6, player.hasKey ? 1 : 0, 1);
            }
            else if(maze.map[row][col] == Maze::Bomb)
            {
                player.bombCount += 2;
                maze.map[row][col] = Maze::Grass;
                if (soundEnabled && pickUpSound)
                {
                    pickUpSound->stop();
                    pickUpSound->setPosition(0);
                    pickUpSound->play();
                }
                emit itemCollected("收集到2个炸弹");
                emit statusChanged(player.health, 3, player.strawberryCount, 3, player.bombCount, 6, player.hasKey ? 1 : 0, 1);
            }
            else if(maze.map[row][col] == Maze::Door)
            {
                maze.map[row][col] = Maze::Grass;
            }
            else if(maze.map[row][col] == Maze::End)
            {
                DatabaseManager::instance().incrementClear(dbPlayerId);
                emit gameCleared(player.health, player.strawberryCount, bombPlacedThisRun);

                QString message = QString("成功走出迷宫！\n本次收集了 %1/3 颗草莓\n").arg(player.strawberryCount);

                if(player.strawberryCount == 0)
                {
                    message += "没有找到任何草莓，这对你来说太难了？";
                }
                else if(player.strawberryCount == 1)
                {
                    message += "只找到了一颗草莓，继续努力！";
                }
                else if(player.strawberryCount == 2)
                {
                    message += "找到了两颗草莓，差一点就完美了！";
                }
                else if(player.strawberryCount == 3)
                {
                    message += "找到了所有草莓！这对你来说轻而易举！";
                }

                QMessageBox::information(this,
                                         "恭喜",
                                         message);
            }
        }
        else
        {
            player.x += (dx / distance) * MOVE_SPEED;
            player.y += (dy / distance) * MOVE_SPEED;

            maze.updateVisibility((int)round(player.y), (int)round(player.x));
        }

        update();
    }
}

/**
 * @brief 更新迷雾动画
 *
 * 由 fogTimer 触发，逐步改变正在动画中的单元格的迷雾透明度，
 * 实现迷雾消散和重新覆盖的平滑过渡效果。
 */
void GameWidget::updateFogAnimation()
{
    bool needsUpdate = false;
    const int FOG_STEP = 15;

    for (int row = 0; row < Maze::ROWS; row++)
    {
        for (int col = 0; col < Maze::COLS; col++)
        {
            if (maze.isFogAnimating[row][col])
            {
                needsUpdate = true;

                if (maze.visible[row][col])
                {
                    if (maze.fogAlpha[row][col] > 0)
                    {
                        maze.fogAlpha[row][col] -= FOG_STEP;
                        if (maze.fogAlpha[row][col] < 0)
                            maze.fogAlpha[row][col] = 0;
                    }
                    else
                    {
                        maze.isFogAnimating[row][col] = false;
                    }
                }
                else
                {
                    if (maze.fogAlpha[row][col] < 255)
                    {
                        maze.fogAlpha[row][col] += FOG_STEP;
                        if (maze.fogAlpha[row][col] > 255)
                            maze.fogAlpha[row][col] = 255;
                    }
                    else
                    {
                        maze.isFogAnimating[row][col] = false;
                    }
                }
            }
        }
    }

    if (needsUpdate)
    {
        update();
    }
}

/**
 * @brief 重新开始游戏
 *
 * 生成新的随机地图，重置玩家位置/状态、炸弹和爆炸列表，
 * 初始化迷雾，并发射状态变更和重启信号。
 */
void GameWidget::restartGame()
{
    maze.generateRandomMap();
    maze.initVisibility();

    placedBombs.clear();
    explosions.clear();

    player.position = QPoint(0, 0);
    player.x = 0;
    player.y = 0;
    player.direction = Player::Down;
    player.currentFrame = 0;
    player.hasKey = false;
    player.strawberryCount = 0;
    player.bombCount = 2;
    player.health = 3;
    player.isMoving = false;
    player.targetRow = 0;
    player.targetCol = 0;

    bombPlacedThisRun = false;
    currentRunSteps = 0;

    maze.updateVisibility(player.position.y(), player.position.x());

    emit statusChanged(player.health, 3, player.strawberryCount, 3, player.bombCount, 6, 0, 1);
    emit restartGameRequested();

    update();
}

/**
 * @brief 在玩家当前位置放置炸弹
 *
 * 检查炸弹数量和重复放置，消耗一个炸弹并设置爆炸倒计时。
 * 更新数据库炸弹使用统计。
 */
void GameWidget::placeBomb()
{
    if (player.bombCount <= 0)
        return;

    int row = qRound(player.y);
    int col = qRound(player.x);

    for (const PlacedBomb& bomb : placedBombs)
    {
        if (bomb.row == row && bomb.col == col)
            return;
    }

    player.bombCount--;
    bombPlacedThisRun = true;
    DatabaseManager::instance().addBombUsed(dbPlayerId, 1);

    PlacedBomb bomb;
    bomb.row = row;
    bomb.col = col;
    bomb.timer = BOMB_EXPLODE_TIME;
    placedBombs.append(bomb);

    emit statusChanged(player.health, 3, player.strawberryCount, 3, player.bombCount, 6, player.hasKey ? 1 : 0, 1);

    update();
}

/**
 * @brief 更新所有炸弹的倒计时
 *
 * 由 bombTimer 触发，每个炸弹的倒计时减少 16ms，
 * 到期后调用 explodeBomb 触发爆炸。
 */
void GameWidget::updateBombTimers()
{
    for (int i = placedBombs.size() - 1; i >= 0; i--)
    {
        placedBombs[i].timer -= 0.016f;
        if (placedBombs[i].timer <= 0)
        {
            explodeBomb(i);
        }
    }
    update();
}

/**
 * @brief 引爆指定索引的炸弹
 *
 * 播放爆炸音效，创建爆炸效果动画，炸毁 3x3 范围内的树木和石头，
 * 检查玩家是否被波及（减血或死亡），并更新数据库统计。
 *
 * @param index 炸弹在 placedBombs 列表中的索引
 */
void GameWidget::explodeBomb(int index)
{
    PlacedBomb bomb = placedBombs.takeAt(index);
    if (soundEnabled && bombSound)
    {
        bombSound->stop();
        bombSound->setPosition(0);
        bombSound->play();
    }

    Explosion explosion;
    explosion.row = bomb.row;
    explosion.col = bomb.col;
    explosion.currentFrame = 0;
    explosions.append(explosion);

    int playerRow = qRound(player.y);
    int playerCol = qRound(player.x);
    bool playerHit = false;

    int obstaclesDestroyed = 0;
    for (int dr = -1; dr <= 1; dr++)
    {
        for (int dc = -1; dc <= 1; dc++)
        {
            int r = bomb.row + dr;
            int c = bomb.col + dc;

            if (r >= 0 && r < Maze::ROWS && c >= 0 && c < Maze::COLS)
            {
                if (maze.map[r][c] == Maze::Tree || maze.map[r][c] == Maze::Stone)
                {
                    maze.map[r][c] = Maze::Grass;
                    obstaclesDestroyed++;
                }

                if (r == playerRow && c == playerCol)
                {
                    playerHit = true;
                }
            }
        }
    }

    if (obstaclesDestroyed > 0)
    {
        DatabaseManager::instance().addObstacleDestroyed(dbPlayerId, obstaclesDestroyed);
    }

    if (playerHit)
    {
        player.health--;

        if (player.health <= 0)
        {
            DatabaseManager::instance().incrementDeath(dbPlayerId);
            emit playerDied(true); // 被炸弹炸死
            QMessageBox::critical(this, "游戏结束", "你被炸死了！");
            restartGame();
        }
        else
        {
            emit statusChanged(player.health, 3, player.strawberryCount, 3, player.bombCount, 6, player.hasKey ? 1 : 0, 1);
        }
    }

    update();
}

/**
 * @brief 设置音效开关状态
 * @param enabled 是否开启音效
 */
void GameWidget::setSoundEnabled(bool enabled)
{
    soundEnabled = enabled;
}

/**
 * @brief 更新爆炸动画
 *
 * 由 explosionTimer 触发，递增爆炸动画帧，
 * 动画播放完成后移除爆炸效果。
 */
void GameWidget::updateExplosionAnimation()
{
    for (int i = explosions.size() - 1; i >= 0; i--)
    {
        explosions[i].currentFrame++;
        if (explosions[i].currentFrame >= EXPLOSION_FRAMES)
        {
            explosions.removeAt(i);
        }
    }
    update();
}