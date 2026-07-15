#include "mainwindow.h"

#include <QApplication>
#include <QLoggingCategory>

/**
 * @brief 程序入口函数
 *
 * 初始化 Qt 应用程序，屏蔽多媒体相关日志输出，
 * 创建并显示主窗口，然后进入事件循环。
 *
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * @return 应用程序退出码
 */
int main(int argc, char *argv[])
{
    QLoggingCategory::setFilterRules(
        "qt.multimedia.ffmpeg=false\n"
        "qt.multimedia=false"
    );

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return QApplication::exec();
}
