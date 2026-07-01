/**
 * @file mainwindow.cpp
 * @brief 主窗口模块实现
 * 
 * 实现主窗口的UI初始化、定时器管理、用户交互处理和结果展示逻辑。
 * UI界面通过 mainwindow.ui 文件定义，由Qt Designer生成代码。
 * 
 * 作为应用的核心控制中心，协调数据采集、分析和展示的完整流程。
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "spectrumplotwidget.h"
#include "waterfallwidget.h"

#include <QDateTime>
#include <QHeaderView>
#include <QTimer>

/**
 * @brief 构造函数
 * 
 * 初始化UI界面（调用Qt自动生成的setupUi），设置定时器（80ms间隔），
 * 初始化场景下拉框，连接信号槽。
 * 
 * @param parent 父对象指针
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)           // 创建UI对象
    , m_timer(new QTimer(this))        // 创建定时器（父对象管理生命周期）
    , m_running(true)                  // 默认运行状态：true（运行中）
{
    // 调用Qt自动生成的setupUi，从.ui文件初始化界面
    ui->setupUi(this);
    
    // 初始化场景下拉框（添加四个模拟场景选项）
    initScenarioCombo();
    
    // 连接定时器信号到槽函数
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimerTick()));
    
    // 启动定时器（80ms间隔，约12.5Hz的帧速率）
    m_timer->start(80);
    
    // 设置表格列自动拉伸
    ui->eventTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    // 设置门限滑块范围
    ui->thresholdSlider->setRange(-90, -40);
    ui->thresholdSlider->setValue((int)m_analyzer.thresholdDbm());
    
    // 更新门限标签初始显示
    on_thresholdSlider_valueChanged(ui->thresholdSlider->value());
    
    // 初始化频率控制控件
    ui->startFreqSpin->setValue(m_simulator.startFreqMHz());
    ui->centerFreqSpin->setValue(m_simulator.centerFreqMHz());
    ui->spanSpin->setValue(m_simulator.spanMHz());
    
    // 连接频谱图框选信号和缩放信号到瀑布图
    SpectrumPlotWidget *spectrumPlot = qobject_cast<SpectrumPlotWidget*>(ui->spectrumPlotWidget);
    WaterfallWidget *waterfall = qobject_cast<WaterfallWidget*>(ui->waterfallWidget);
    if (spectrumPlot && waterfall) {
        connect(spectrumPlot, SIGNAL(selectionChanged(double, double)), 
                waterfall, SLOT(setSelection(double, double)));
        connect(spectrumPlot, SIGNAL(zoomChanged(double, double)), 
                waterfall, SLOT(setZoomRange(double, double)));
    }
}

/**
 * @brief 析构函数
 * 
 * 释放UI对象资源。
 */
MainWindow::~MainWindow()
{
    delete ui;  // 释放UI对象
}

/**
 * @brief 初始化场景下拉框
 * 
 * 为场景选择下拉框添加四个模拟场景选项，并设置默认选中项。
 */
void MainWindow::initScenarioCombo()
{
    // 添加四个模拟场景选项（文本 + 数据值）
    ui->scenarioCombo->addItem(QStringLiteral("环境底噪"), SpectrumSimulator::IdleNoise);
    ui->scenarioCombo->addItem(QStringLiteral("无人机遥控窄带链路"), SpectrumSimulator::DroneControl);
    ui->scenarioCombo->addItem(QStringLiteral("无人机图传宽带链路"), SpectrumSimulator::DroneVideo);
    ui->scenarioCombo->addItem(QStringLiteral("无人机跳频链路"), SpectrumSimulator::FrequencyHopping);
    
    // 默认选中"无人机遥控窄带链路"
    ui->scenarioCombo->setCurrentIndex(1);
}

/**
 * @brief 定时器触发槽函数
 * 
 * 每80ms触发一次，执行核心数据处理流程：
 * 1. 检查运行状态（暂停时跳过）
 * 2. 生成下一帧模拟数据
 * 3. 分析频谱数据
 * 4. 更新频谱图和瀑布图
 * 5. 更新结果面板
 * 6. 如果检测到信号，添加事件记录
 */
void MainWindow::onTimerTick()
{
    // 暂停状态时跳过处理
    if (!m_running) {
        return;
    }

    // 1. 获取下一帧模拟频谱数据
    const SpectrumFrame frame = m_simulator.nextFrame();
    
    // 2. 分析频谱数据，获取检测结果
    const DetectionResult result = m_analyzer.analyze(frame);
    
    // 3. 更新频谱图（传递帧数据和检测结果）
    SpectrumPlotWidget *spectrumPlot = qobject_cast<SpectrumPlotWidget*>(ui->spectrumPlotWidget);
    if (spectrumPlot) {
        spectrumPlot->setFrame(frame, result);
    }
    
    // 4. 更新瀑布图（添加新帧）
    WaterfallWidget *waterfall = qobject_cast<WaterfallWidget*>(ui->waterfallWidget);
    if (waterfall) {
        waterfall->addFrame(frame);
    }
    
    // 5. 更新结果面板显示
    updateResultPanel(result, frame);

    // 6. 如果检测到信号，添加事件记录到表格
    if (result.detected) {
        appendEventRow(result, frame);
    }
}

/**
 * @brief 场景切换槽函数（Qt自动连接）
 * 
 * 用户切换模拟场景时触发：
 * 1. 获取选中场景的枚举值
 * 2. 更新频谱模拟器的场景设置
 * 3. 重置频谱分析器（清除跳频历史）
 * 
 * @param index 场景索引
 */
void MainWindow::on_scenarioCombo_currentIndexChanged(int index)
{
    // 获取场景枚举值
    const int value = ui->scenarioCombo->itemData(index).toInt();
    
    // 更新模拟器场景
    m_simulator.setScenario((SpectrumSimulator::Scenario)value);
    
    // 重置分析器状态（清除跳频历史等）
    m_analyzer.reset();
}

/**
 * @brief 门限调整槽函数（Qt自动连接）
 * 
 * 用户调整检测门限时触发：
 * 1. 更新分析器的门限值
 * 2. 更新门限标签显示
 * 
 * @param value 门限功率值（dBm）
 */
void MainWindow::on_thresholdSlider_valueChanged(int value)
{
    // 更新分析器门限值
    m_analyzer.setThresholdDbm(value);
    
    // 更新门限标签显示
    ui->thresholdLabel->setText(QString::number(value) + QStringLiteral(" dBm"));
}

/**
 * @brief 运行/暂停按钮点击槽函数（Qt自动连接）
 * 
 * 切换运行状态：
 * - 运行中 → 暂停（按钮文字变为"继续"）
 * - 暂停 → 运行中（按钮文字变为"暂停"）
 */
void MainWindow::on_runButton_clicked()
{
    // 切换运行状态
    m_running = !m_running;
    
    // 更新按钮文字
    ui->runButton->setText(m_running ? QStringLiteral("暂停") : QStringLiteral("继续"));
}

/**
 * @brief 清空按钮点击槽函数（Qt自动连接）
 * 
 * 清空所有数据和状态：
 * 1. 清空频谱图
 * 2. 清空瀑布图
 * 3. 清空事件表格
 * 4. 重置分析器状态
 */
void MainWindow::on_clearButton_clicked()
{
    // 清空频谱图
    SpectrumPlotWidget *spectrumPlot = qobject_cast<SpectrumPlotWidget*>(ui->spectrumPlotWidget);
    if (spectrumPlot) {
        spectrumPlot->clear();
    }
    
    // 清空瀑布图
    WaterfallWidget *waterfall = qobject_cast<WaterfallWidget*>(ui->waterfallWidget);
    if (waterfall) {
        waterfall->clear();
    }
    
    // 清空事件表格（设置行数为0）
    ui->eventTable->setRowCount(0);
    
    // 重置分析器状态
    m_analyzer.reset();
}

/**
 * @brief 更新结果面板
 * 
 * 根据检测结果更新UI上的结果显示：
 * - 状态标签：显示检测结果、详细描述和当前场景
 * - 峰值标签：显示峰值功率和中心频率
 * - 带宽标签：显示占用带宽、活跃频点和跳频次数
 * - 置信度标签：显示置信度百分比
 * 
 * @param result 检测结果
 * @param frame 频谱帧数据
 */
void MainWindow::updateResultPanel(const DetectionResult &result, const SpectrumFrame &frame)
{
    // 状态文本：检测结果 + 详细描述 + 当前场景
    QString status = result.label + QStringLiteral("\n") + result.detail
            + QStringLiteral("\n当前场景: ") + frame.scenarioName;
    ui->statusLabel->setText(status);
    
    // 峰值功率：格式为 "峰值功率: X.X dBm @ Y.YY MHz"
    ui->peakLabel->setText(QStringLiteral("峰值功率: %1 dBm @ %2 MHz")
                         .arg(result.peakDbm, 0, 'f', 1)
                         .arg(result.centerFreqMHz, 0, 'f', 2));
    
    // 占用带宽：格式为 "占用带宽: X.YY MHz    活跃频点: X    跳频次数: X"
    ui->bandwidthLabel->setText(QStringLiteral("占用带宽: %1 MHz    活跃频点: %2    跳频次数: %3")
                              .arg(result.bandwidthMHz, 0, 'f', 2)
                              .arg(result.activeBins)
                              .arg(result.hopCount));
    
    // 置信度：格式为 "置信度: XX%"
    ui->confidenceLabel->setText(QStringLiteral("置信度: %1%")
                               .arg(result.confidence * 100.0, 0, 'f', 0));
}

/**
 * @brief 添加事件记录行
 * 
 * 将检测到的信号事件添加到事件表格中：
 * 1. 控制表格最大行数（超过80行时移除最旧的行）
 * 2. 在表格顶部插入新行
 * 3. 填充时间、结果、峰值、中心频率、带宽等信息
 * 
 * @param result 检测结果
 * @param frame 频谱帧数据
 */
void MainWindow::appendEventRow(const DetectionResult &result, const SpectrumFrame &frame)
{
    Q_UNUSED(frame);  // frame参数未使用

    // 控制表格长度，避免长时间运行后表格越来越大（最多80行）
    if (ui->eventTable->rowCount() > 80) {
        ui->eventTable->removeRow(ui->eventTable->rowCount() - 1);  // 移除最底部（最旧）的行
    }

    // 在表格顶部插入新行（行索引0）
    ui->eventTable->insertRow(0);
    
    // 填充各列数据
    ui->eventTable->setItem(0, 0, new QTableWidgetItem(QDateTime::currentDateTime().toString("HH:mm:ss.zzz")));  // 时间（毫秒精度）
    ui->eventTable->setItem(0, 1, new QTableWidgetItem(result.label));                                           // 检测结果
    ui->eventTable->setItem(0, 2, new QTableWidgetItem(QString::number(result.peakDbm, 'f', 1)));               // 峰值功率
    ui->eventTable->setItem(0, 3, new QTableWidgetItem(QString::number(result.centerFreqMHz, 'f', 2)));         // 中心频率
    ui->eventTable->setItem(0, 4, new QTableWidgetItem(QString::number(result.bandwidthMHz, 'f', 2)));          // 带宽
}

void MainWindow::on_startFreqSpin_valueChanged(double value)
{
    const double span = ui->spanSpin->value();
    const double newCenter = value + span / 2.0;
    
    ui->centerFreqSpin->blockSignals(true);
    ui->centerFreqSpin->setValue(qBound(500.0, newCenter, 6000.0));
    ui->centerFreqSpin->blockSignals(false);
}

void MainWindow::on_centerFreqSpin_valueChanged(double value)
{
    const double span = ui->spanSpin->value();
    const double newStart = value - span / 2.0;
    
    ui->startFreqSpin->blockSignals(true);
    ui->startFreqSpin->setValue(qBound(500.0, newStart, 6000.0 - span));
    ui->startFreqSpin->blockSignals(false);
}

void MainWindow::on_spanSpin_valueChanged(double value)
{
    const double center = ui->centerFreqSpin->value();
    const double newStart = center - value / 2.0;
    
    ui->startFreqSpin->blockSignals(true);
    ui->startFreqSpin->setValue(qBound(500.0, newStart, 6000.0 - value));
    ui->startFreqSpin->blockSignals(false);
    
    ui->centerFreqSpin->blockSignals(true);
    ui->centerFreqSpin->setValue(qBound(500.0 + value / 2.0, center, 6000.0 - value / 2.0));
    ui->centerFreqSpin->blockSignals(false);
}

void MainWindow::on_applyFreqButton_clicked()
{
    const double centerFreq = ui->centerFreqSpin->value();
    const double span = ui->spanSpin->value();
    
    m_simulator.setSpanMHz(span);
    m_simulator.setCenterFreqMHz(centerFreq);
    
    ui->startFreqSpin->setValue(m_simulator.startFreqMHz());
    ui->centerFreqSpin->setValue(m_simulator.centerFreqMHz());
    ui->spanSpin->setValue(m_simulator.spanMHz());
    
    SpectrumPlotWidget *spectrumPlot = qobject_cast<SpectrumPlotWidget*>(ui->spectrumPlotWidget);
    if (spectrumPlot) {
        spectrumPlot->clear();
    }
    
    WaterfallWidget *waterfall = qobject_cast<WaterfallWidget*>(ui->waterfallWidget);
    if (waterfall) {
        waterfall->clear();
    }
    
    m_analyzer.reset();
}