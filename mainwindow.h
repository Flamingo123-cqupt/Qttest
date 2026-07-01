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
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "spectrumsimulator.h"
#include "spectrumanalyzer.h"
#include <QMainWindow>

// 前向声明（避免头文件循环依赖）
class SpectrumPlotWidget;
class WaterfallWidget;
class QComboBox;
class QLabel;
class QPushButton;
class QSlider;
class QTableWidget;
class QTimer;

/**
 * @brief 主窗口类
 * 
 * 继承自QMainWindow，整合所有组件和业务逻辑。
 * 作为应用的核心控制中心，负责协调数据采集、分析和展示。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    /**
     * @brief 构造函数
     * 
     * @param parent 父对象指针
     */
    explicit MainWindow(QWidget *parent = 0);
    
    /**
     * @brief 析构函数
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
    void onScenarioChanged(int index);

    /**
     * @brief 门限调整槽函数
     * 
     * 用户调整检测门限时触发，更新分析器门限值。
     * 
     * @param value 门限功率值（dBm）
     */
    void onThresholdChanged(int value);

    /**
     * @brief 运行/暂停按钮点击槽函数
     * 
     * 切换运行状态（暂停或继续）。
     */
    void onRunButtonClicked();

    /**
     * @brief 清空按钮点击槽函数
     * 
     * 清空所有图表和事件记录，重置分析器状态。
     */
    void onClearButtonClicked();

private:
    /**
     * @brief 构建用户界面
     * 
     * 创建并布局所有UI组件，包括：
     * - 控制面板（场景选择、门限滑块、按钮）
     * - 结果面板（检测状态、峰值、带宽、置信度）
     * - 事件表格（检测事件历史记录）
     * - 频谱图和瀑布图
     */
    void buildUi();

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
    // 业务逻辑组件
    SpectrumSimulator m_simulator;    ///< 频谱数据模拟器
    SpectrumAnalyzer m_analyzer;      ///< 频谱分析器

    // UI组件
    QTimer *m_timer;                  ///< 定时器（80ms间隔）
    SpectrumPlotWidget *m_spectrumPlot; ///< 实时频谱图组件
    WaterfallWidget *m_waterfall;     ///< 频谱瀑布图组件
    QComboBox *m_scenarioCombo;       ///< 场景选择下拉框
    QSlider *m_thresholdSlider;       ///< 检测门限滑块
    QLabel *m_thresholdLabel;         ///< 门限值显示标签
    QLabel *m_statusLabel;            ///< 检测状态标签
    QLabel *m_peakLabel;              ///< 峰值功率标签
    QLabel *m_bandwidthLabel;         ///< 占用带宽标签
    QLabel *m_confidenceLabel;        ///< 置信度标签
    QPushButton *m_runButton;         ///< 运行/暂停按钮
    QPushButton *m_clearButton;       ///< 清空按钮
    QTableWidget *m_eventTable;       ///< 检测事件表格

    // 状态变量
    bool m_running;                   ///< 运行状态标志（true=运行中，false=暂停）
};

#endif // MAINWINDOW_H