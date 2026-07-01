/**
 * @file main.cpp
 * @brief 无人机频谱检测系统主程序入口
 * 
 * 本程序实现无人机频谱瀑布图识别检测模块，主要功能包括：
 * - 模拟多种无人机通信场景（底噪、遥控窄带、图传宽带、跳频链路）
 * - 实时频谱分析与无人机信号识别
 * - 实时频谱图与瀑布图可视化展示
 * - 检测结果展示与事件记录
 * 
 * 系统架构：
 * - MainWindow: 主窗口，整合所有UI组件和业务逻辑
 * - SpectrumSimulator: 频谱模拟器，生成模拟频谱数据
 * - SpectrumAnalyzer: 频谱分析器，执行信号检测与分类
 * - SpectrumPlotWidget: 实时频谱图绘制组件
 * - WaterfallWidget: 频谱瀑布图绘制组件
 */

#include "mainwindow.h"
#include <QApplication>

/**
 * @brief 程序主入口函数
 * 
 * 创建Qt应用程序实例，初始化主窗口，启动事件循环。
 * 
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 * @return int 程序退出码
 */
int main(int argc, char *argv[])
{
    // 创建Qt应用程序实例
    QApplication app(argc, argv);
    
    // 创建主窗口对象
    MainWindow window;
    
    // 设置窗口初始大小（1280x760像素）
    window.resize(1280, 760);
    
    // 显示主窗口
    window.show();
    
    // 启动Qt事件循环，等待用户交互
    return app.exec();
}