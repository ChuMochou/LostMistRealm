#include "mazegenerator.h"
#include <cstdlib>
#include <ctime>

const int MazeGenerator::dx_gen[DIR_NUM] = {0, 0, -2, 2};
const int MazeGenerator::dy_gen[DIR_NUM] = {-2, 2, 0, 0};

/**
 * @brief 构造函数，使用当前时间初始化随机数种子
 */
MazeGenerator::MazeGenerator()
{
    srand(time(nullptr));
}

/**
 * @brief 随机返回一种障碍物类型
 *
 * 等概率返回树、水面或石头中的一种。
 *
 * @return 随机障碍物类型
 */
Maze::CellType MazeGenerator::getRandomObstacle()
{
    int randVal = rand() % 3;
    switch (randVal)
    {
    case 0: return Maze::Tree;
    case 1: return Maze::Water;
    case 2: return Maze::Stone;
    default: return Maze::Tree;
    }
}

/**
 * @brief 生成随机迷宫地图
 *
 * 整体流程：
 * 1. 用随机障碍物填满地图
 * 2. 使用随机化 Kruskal 算法生成迷宫主路径
 * 3. 将路径单元格转换为草地，并设置起点和终点
 * 4. 在终点附近放置门
 * 5. 通过 BFS 确保钥匙放置位置从起点可达
 * 6. 随机放置草莓（3个）和炸弹（2个）
 *
 * @param maze 待填充的迷宫对象
 */
void MazeGenerator::generate(Maze& maze)
{
    const int MAZE_SIZE = 12;
    int visited[MAZE_SIZE][MAZE_SIZE] = {0};
    Edge edges[MAZE_SIZE * MAZE_SIZE * 4];
    int edgeCount = 0;

    for (int row = 0; row < Maze::ROWS; row++)
    {
        for (int col = 0; col < Maze::COLS; col++)
        {
            maze.map[row][col] = getRandomObstacle();
        }
    }

    int startI = 0;
    int startJ = 0;
    int startX = 2 * startI;
    int startY = 2 * startJ;
    visited[startI][startJ] = 1;
    maze.map[startX][startY] = Maze::Path;

    for (int d = 0; d < DIR_NUM; d++)
    {
        int ni = startI + (dx_gen[d] / 2);
        int nj = startJ + (dy_gen[d] / 2);
        if (ni >= 0 && ni < MAZE_SIZE && nj >= 0 && nj < MAZE_SIZE)
        {
            int nx = 2 * ni;
            int ny = 2 * nj;
            int wallX = (startX + nx) / 2;
            int wallY = (startY + ny) / 2;

            Edge newEdge;
            newEdge.x1 = startX;
            newEdge.y1 = startY;
            newEdge.x2 = nx;
            newEdge.y2 = ny;
            newEdge.wallX = wallX;
            newEdge.wallY = wallY;
            edges[edgeCount++] = newEdge;
        }
    }

    while (edgeCount > 0)
    {
        int randIndex = rand() % edgeCount;
        Edge e = edges[randIndex];

        int i1 = e.x1 / 2;
        int j1 = e.y1 / 2;
        int i2 = e.x2 / 2;
        int j2 = e.y2 / 2;

        if (visited[i1][j1] != visited[i2][j2])
        {
            maze.map[e.wallX][e.wallY] = Maze::Path;

            int newX, newY, newI, newJ;
            if (!visited[i1][j1])
            {
                newX = e.x1; newY = e.y1; newI = i1; newJ = j1;
            }
            else
            {
                newX = e.x2; newY = e.y2; newI = i2; newJ = j2;
            }

            visited[newI][newJ] = 1;
            maze.map[newX][newY] = Maze::Path;

            for (int d = 0; d < DIR_NUM; d++)
            {
                int ni = newI + (dx_gen[d] / 2);
                int nj = newJ + (dy_gen[d] / 2);
                if (ni >= 0 && ni < MAZE_SIZE && nj >= 0 && nj < MAZE_SIZE)
                {
                    if (!visited[ni][nj])
                    {
                        int nx = 2 * ni;
                        int ny = 2 * nj;
                        int wallX = (newX + nx) / 2;
                        int wallY = (newY + ny) / 2;

                        Edge newEdge;
                        newEdge.x1 = newX;
                        newEdge.y1 = newY;
                        newEdge.x2 = nx;
                        newEdge.y2 = ny;
                        newEdge.wallX = wallX;
                        newEdge.wallY = wallY;
                        edges[edgeCount++] = newEdge;
                    }
                }
            }
        }

        edges[randIndex] = edges[--edgeCount];
    }

    for (int row = 0; row < Maze::ROWS; row++)
    {
        for (int col = 0; col < Maze::COLS; col++)
        {
            if (maze.map[row][col] == Maze::Path)
            {
                maze.map[row][col] = Maze::Grass;
            }
        }
    }

    int endX = Maze::ROWS - 1;
    int endY = Maze::COLS - 1;
    maze.map[endX][endY] = Maze::Grass;

    maze.map[0][0] = Maze::Start;
    maze.map[endX][endY] = Maze::End;

    // ===== 添加门 =====
    if (endX > 0)
    {
        maze.map[endX - 1][endY] = Maze::Door;
    }
    if (endY > 0)
    {
        maze.map[endX][endY - 1] = Maze::Door;
    }

    // ===== 添加钥匙 =====
    bool reachable[Maze::ROWS][Maze::COLS] = {false};
    int dx[4] = {0, 0, -1, 1};
    int dy[4] = {-1, 1, 0, 0};

    int queue[Maze::ROWS * Maze::COLS][2];
    int front = 0, rear = 0;

    queue[rear][0] = 0;
    queue[rear][1] = 0;
    rear++;
    reachable[0][0] = true;

    while (front < rear)
    {
        int r = queue[front][0];
        int c = queue[front][1];
        front++;

        for (int d = 0; d < 4; d++)
        {
            int nr = r + dx[d];
            int nc = c + dy[d];
            if (nr >= 0 && nr < Maze::ROWS && nc >= 0 && nc < Maze::COLS)
            {
                if (!reachable[nr][nc] && maze.map[nr][nc] == Maze::Grass)
                {
                    reachable[nr][nc] = true;
                    queue[rear][0] = nr;
                    queue[rear][1] = nc;
                    rear++;
                }
            }
        }
    }

    int validPositions[Maze::ROWS * Maze::COLS][2];
    int validCount = 0;
    for (int r = 0; r < Maze::ROWS; r++)
    {
        for (int c = 0; c < Maze::COLS; c++)
        {
            if (reachable[r][c] && (r != 0 || c != 0) && (r >= 10 || c >= 10))
            {
                validPositions[validCount][0] = r;
                validPositions[validCount][1] = c;
                validCount++;
            }
        }
    }

    if (validCount > 0)
    {
        int randPos = rand() % validCount;
        int keyRow = validPositions[randPos][0];
        int keyCol = validPositions[randPos][1];
        maze.map[keyRow][keyCol] = Maze::Key;
    }

    // ===== 添加三个草莓 =====
    int strawberryValidPositions[Maze::ROWS * Maze::COLS][2];
    int strawberryValidCount = 0;
    for (int r = 0; r < Maze::ROWS; r++)
    {
        for (int c = 0; c < Maze::COLS; c++)
        {
            if (reachable[r][c] && maze.map[r][c] == Maze::Grass && (r >= 10 || c >= 10))
            {
                strawberryValidPositions[strawberryValidCount][0] = r;
                strawberryValidPositions[strawberryValidCount][1] = c;
                strawberryValidCount++;
            }
        }
    }

    int strawberriesPlaced = 0;
    while (strawberriesPlaced < 3 && strawberryValidCount > 0)
    {
        int randPos = rand() % strawberryValidCount;
        int berryRow = strawberryValidPositions[randPos][0];
        int berryCol = strawberryValidPositions[randPos][1];
        maze.map[berryRow][berryCol] = Maze::Strawberry;

        strawberryValidPositions[randPos][0] = strawberryValidPositions[strawberryValidCount - 1][0];
        strawberryValidPositions[randPos][1] = strawberryValidPositions[strawberryValidCount - 1][1];
        strawberryValidCount--;

        strawberriesPlaced++;
    }

    // ===== 添加炸弹 =====
    int bombValidPositions[Maze::ROWS * Maze::COLS][2];
    int bombValidCount = 0;
    for (int r = 0; r < Maze::ROWS; r++)
    {
        for (int c = 0; c < Maze::COLS; c++)
        {
            if (reachable[r][c] && maze.map[r][c] == Maze::Grass && (r >= 10 || c >= 10))
            {
                bombValidPositions[bombValidCount][0] = r;
                bombValidPositions[bombValidCount][1] = c;
                bombValidCount++;
            }
        }
    }

    const int BOMB_COUNT = 2;
    for (int i = 0; i < BOMB_COUNT && bombValidCount > 0; i++)
    {
        int randPos = rand() % bombValidCount;
        int bombRow = bombValidPositions[randPos][0];
        int bombCol = bombValidPositions[randPos][1];
        maze.map[bombRow][bombCol] = Maze::Bomb;

        for (int j = randPos; j < bombValidCount - 1; j++)
        {
            bombValidPositions[j][0] = bombValidPositions[j + 1][0];
            bombValidPositions[j][1] = bombValidPositions[j + 1][1];
        }
        bombValidCount--;
    }
}