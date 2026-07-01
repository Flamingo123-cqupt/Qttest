/**
 * @file mainwindow.h
 * @brief 主窗口模块
 * 
 * 本模块负责整合所有UI组件和业务逻辑，是整个应用的核心控制中心。
 * 主要功能包括：
 * - 构建用户界面（控制面板、结果面板、事件表格、频谱图、瀑布图）
 * - 管理定时器驱动的实时数据更新
 * - 处理用户交互（场景切换、门限调整、暂停/继续、清空）
 * - 更新检测结果展示和事件记录
 * 
 * UI界面通过 mainwindow.ui 文件定义，使用Qt Designer进行可视化编辑。
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "spectrumsimulator.h"
#include "spectrumanalyzer.h"
#include <QMainWindow>

// 前向声明（避免头文件循环依赖）
class SpectrumPlotWidget;
class WaterfallWidget;
class QTimer;

// 包含Qt自动生成的UI头文件
namespace Ui {
class MainWindow;
}

/**
 * @brief 主窗口类
 * 
 * 继承自QMainWindow，整合所有组件和业务逻辑。
 * 作为应用的核心控制中心，负责协调数据采集、分析和展示。
 * 
 * UI界面通过 mainwindow.ui 文件定义，在构造函数中调用 setupUi() 初始化。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    /**
     * @brief 构造函数
     * 
     * 初始化UI界面，设置定时器（80ms间隔），连接信号槽。
     * 
     * @param parent 父对象指针
     */
    explicit MainWindow(QWidget *parent = 0);
    
    /**
     * @brief 析构函数
     * 
     * 释放UI对象资源。
     */
    ~MainWindow();

private slots:
    /**
     * @brief 定时器触发槽函数
     * 
     * 每80ms触发一次，执行数据采集、分析和界面更新。
     */
    void onTimerTick();

    /**
     * @brief 场景切换槽函数
     * 
     * 用户切换模拟场景时触发，更新模拟器场景并重置分析器。
     * 
     * @param index 场景索引
     */
    void on_scenarioCombo_currentIndexChanged(int index);

    /**
     * @brief 门限调整槽函数
     * 
     * 用户调整检测门限时触发，更新分析器门限值。
     * 
     * @param value 门限功率值（dBm）
     */
    void on_thresholdSlider_valueChanged(int value);

    /**
     * @brief 运行/暂停按钮点击槽函数
     * 
     * 切换运行状态（暂停或继续）。
     */
    void on_runButton_clicked();

    /**
     * @brief 清空按钮点击槽函数
     * 
     * 清空所有图表和事件记录，重置分析器状态。
     */
    void on_clearButton_clicked();

    /**
     * @brief 起始频率变化槽函数
     * 
     * 用户修改起始频率时触发，联动更新中心频率。
     * 
     * @param value 起始频率值（MHz）
     */
    void on_startFreqSpin_valueChanged(double value);

    /**
     * @brief 中心频率变化槽函数
     * 
     * 用户修改中心频率时触发，联动更新起始频率。
     * 
     * @param value 中心频率值（MHz）
     */
    void on_centerFreqSpin_valueChanged(double value);

    /**
     * @brief 带宽变化槽函数
     * 
     * 用户修改带宽时触发，联动更新起始频率。
     * 
     * @param value 带宽值（MHz）
     */
    void on_spanSpin_valueChanged(double value);

    /**
     * @brief 应用频率设置按钮点击槽函数
     * 
     * 应用频率设置到模拟器，控制频谱和瀑布图显示范围。
     */
    void on_applyFreqButton_clicked();

private:
    /**
     * @brief 初始化场景下拉框
     * 
     * 为场景选择下拉框添加四个模拟场景选项。
     */
    void initScenarioCombo();

    /**
     * @brief 更新结果面板
     * 
     * 根据检测结果更新UI上的结果显示。
     * 
     * @param result 检测结果
     * @param frame 频谱帧数据
     */
    void updateResultPanel(const DetectionResult &result, const SpectrumFrame &frame);

    /**
     * @brief 添加事件记录行
     * 
     * 将检测到的信号事件添加到事件表格中。
     * 
     * @param result 检测结果
     * @param frame 频谱帧数据
     */
    void appendEventRow(const DetectionResult &result, const SpectrumFrame &frame);

private:
    Ui::MainWindow *ui;             ///< UI对象指针（由Qt自动生成）
    
    // 业务逻辑组件
    SpectrumSimulator m_simulator;  ///< 频谱数据模拟器
    SpectrumAnalyzer m_analyzer;    ///< 频谱分析器
    QTimer *m_timer;                ///< 定时器（80ms间隔）
    
    // 状态变量
    bool m_running;                 ///< 运行状态标志（true=运行中，false=暂停）
};

#endif // MAINWINDOW_H