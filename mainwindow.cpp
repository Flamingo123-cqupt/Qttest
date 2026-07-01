/**
 * @file mainwindow.cpp
 * @brief 主窗口模块实现
 * 
 * 实现主窗口的UI构建、定时器管理、用户交互处理和结果展示逻辑。
 * 作为应用的核心控制中心，协调数据采集、分析和展示的完整流程。
 */

#include "mainwindow.h"
#include "spectrumplotwidget.h"
#include "waterfallwidget.h"

#include <QComboBox>
#include <QDateTime>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QTableWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>

/**
 * @brief 构造函数
 * 
 * 初始化所有UI组件指针，构建用户界面，设置定时器（80ms间隔）。
 * 
 * @param parent 父对象指针
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_timer(new QTimer(this))           // 创建定时器（父对象管理生命周期）
    , m_spectrumPlot(0)                   // 频谱图组件指针（初始为空）
    , m_waterfall(0)                      // 瀑布图组件指针（初始为空）
    , m_scenarioCombo(0)                  // 场景选择下拉框（初始为空）
    , m_thresholdSlider(0)                // 门限滑块（初始为空）
    , m_thresholdLabel(0)                 // 门限标签（初始为空）
    , m_statusLabel(0)                    // 状态标签（初始为空）
    , m_peakLabel(0)                      // 峰值标签（初始为空）
    , m_bandwidthLabel(0)                 // 带宽标签（初始为空）
    , m_confidenceLabel(0)                // 置信度标签（初始为空）
    , m_runButton(0)                      // 运行按钮（初始为空）
    , m_clearButton(0)                    // 清空按钮（初始为空）
    , m_eventTable(0)                     // 事件表格（初始为空）
    , m_running(true)                     // 默认运行状态：true（运行中）
{
    // 构建用户界面
    buildUi();
    
    // 连接定时器信号到槽函数
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimerTick()));
    
    // 启动定时器（80ms间隔，约12.5Hz的帧速率）
    m_timer->start(80);
}

/**
 * @brief 析构函数
 * 
 * 无需手动释放资源，Qt父对象机制会自动清理子对象。
 */
MainWindow::~MainWindow()
{
}

/**
 * @brief 构建用户界面
 * 
 * 创建并布局所有UI组件：
 * 1. 创建中心Widget和根布局
 * 2. 左侧布局：控制面板、结果面板、事件表格
 * 3. 右侧布局：频谱图、瀑布图
 * 4. 设置窗口标题
 */
void MainWindow::buildUi()
{
    // 创建中心Widget（QMainWindow必须设置中心Widget）
    QWidget *central = new QWidget(this);
    QHBoxLayout *rootLayout = new QHBoxLayout(central);
    rootLayout->setContentsMargins(10, 10, 10, 10);  // 设置边距
    rootLayout->setSpacing(10);                      // 设置间距

    // 左侧布局（控制面板、结果面板、事件表格）
    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->setSpacing(10);

    // ===== 控制面板 =====
    QGroupBox *controlBox = new QGroupBox(QStringLiteral("模拟与检测控制"), central);
    QGridLayout *controlLayout = new QGridLayout(controlBox);

    // 场景选择下拉框
    m_scenarioCombo = new QComboBox(controlBox);
    m_scenarioCombo->addItem(QStringLiteral("环境底噪"), SpectrumSimulator::IdleNoise);
    m_scenarioCombo->addItem(QStringLiteral("无人机遥控窄带链路"), SpectrumSimulator::DroneControl);
    m_scenarioCombo->addItem(QStringLiteral("无人机图传宽带链路"), SpectrumSimulator::DroneVideo);
    m_scenarioCombo->addItem(QStringLiteral("无人机跳频链路"), SpectrumSimulator::FrequencyHopping);
    m_scenarioCombo->setCurrentIndex(1);  // 默认选中"无人机遥控窄带链路"
    connect(m_scenarioCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onScenarioChanged(int)));

    // 门限滑块（范围：-90dBm 到 -40dBm）
    m_thresholdSlider = new QSlider(Qt::Horizontal, controlBox);
    m_thresholdSlider->setRange(-90, -40);
    m_thresholdSlider->setValue((int)m_analyzer.thresholdDbm());  // 使用分析器默认门限
    connect(m_thresholdSlider, SIGNAL(valueChanged(int)), this, SLOT(onThresholdChanged(int)));

    // 门限标签（显示当前门限值）
    m_thresholdLabel = new QLabel(controlBox);
    onThresholdChanged(m_thresholdSlider->value());  // 初始化显示

    // 运行/暂停按钮和清空按钮
    m_runButton = new QPushButton(QStringLiteral("暂停"), controlBox);
    m_clearButton = new QPushButton(QStringLiteral("清空"), controlBox);
    connect(m_runButton, SIGNAL(clicked()), this, SLOT(onRunButtonClicked()));
    connect(m_clearButton, SIGNAL(clicked()), this, SLOT(onClearButtonClicked()));

    // 布局控制面板组件（网格布局）
    controlLayout->addWidget(new QLabel(QStringLiteral("模拟场景"), controlBox), 0, 0);
    controlLayout->addWidget(m_scenarioCombo, 0, 1, 1, 2);  // 跨2列
    controlLayout->addWidget(new QLabel(QStringLiteral("检测门限"), controlBox), 1, 0);
    controlLayout->addWidget(m_thresholdSlider, 1, 1);
    controlLayout->addWidget(m_thresholdLabel, 1, 2);
    controlLayout->addWidget(m_runButton, 2, 1);
    controlLayout->addWidget(m_clearButton, 2, 2);

    // ===== 结果面板 =====
    QGroupBox *resultBox = new QGroupBox(QStringLiteral("识别结果"), central);
    QVBoxLayout *resultLayout = new QVBoxLayout(resultBox);
    
    // 状态标签（显示检测结果和场景信息）
    m_statusLabel = new QLabel(QStringLiteral("等待数据"), resultBox);
    // 峰值功率标签
    m_peakLabel = new QLabel(QStringLiteral("峰值功率: -"), resultBox);
    // 占用带宽标签
    m_bandwidthLabel = new QLabel(QStringLiteral("占用带宽: -"), resultBox);
    // 置信度标签
    m_confidenceLabel = new QLabel(QStringLiteral("置信度: -"), resultBox);
    
    m_statusLabel->setWordWrap(true);  // 允许自动换行
    
    // 布局结果面板组件
    resultLayout->addWidget(m_statusLabel);
    resultLayout->addWidget(m_peakLabel);
    resultLayout->addWidget(m_bandwidthLabel);
    resultLayout->addWidget(m_confidenceLabel);
    resultLayout->addStretch();  // 添加拉伸因子，使内容靠上

    // ===== 事件表格 =====
    m_eventTable = new QTableWidget(0, 5, central);  // 0行5列
    QStringList headers;
    headers << QStringLiteral("时间") << QStringLiteral("结果") << QStringLiteral("峰值dBm")
            << QStringLiteral("中心MHz") << QStringLiteral("带宽MHz");
    m_eventTable->setHorizontalHeaderLabels(headers);
    m_eventTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);  // 自动拉伸列宽
    m_eventTable->verticalHeader()->setVisible(false);  // 隐藏行号
    m_eventTable->setEditTriggers(QAbstractItemView::NoEditTriggers);  // 禁止编辑
    m_eventTable->setSelectionBehavior(QAbstractItemView::SelectRows);  // 选择整行

    // 将左侧组件添加到左侧布局
    leftLayout->addWidget(controlBox);
    leftLayout->addWidget(resultBox);
    leftLayout->addWidget(new QLabel(QStringLiteral("检测事件"), central));
    leftLayout->addWidget(m_eventTable, 1);  // 拉伸因子1，占满剩余空间

    // ===== 右侧组件 =====
    m_spectrumPlot = new SpectrumPlotWidget(central);  // 实时频谱图
    m_waterfall = new WaterfallWidget(central);        // 频谱瀑布图

    // 右侧布局（频谱图在上，瀑布图在下）
    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->setSpacing(8);
    rightLayout->addWidget(m_spectrumPlot, 0);  // 拉伸因子0，固定高度
    rightLayout->addWidget(m_waterfall, 1);     // 拉伸因子1，占满剩余空间

    // 将左右布局添加到根布局
    rootLayout->addLayout(leftLayout, 0);   // 拉伸因子0，固定宽度
    rootLayout->addLayout(rightLayout, 1);  // 拉伸因子1，占满剩余空间
    
    // 设置中心Widget
    setCentralWidget(central);
    
    // 设置窗口标题
    setWindowTitle(QStringLiteral("无人机频谱瀑布图识别检测模块 - Qt 5.12.12"));
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
    m_spectrumPlot->setFrame(frame, result);
    
    // 4. 更新瀑布图（添加新帧）
    m_waterfall->addFrame(frame);
    
    // 5. 更新结果面板显示
    updateResultPanel(result, frame);

    // 6. 如果检测到信号，添加事件记录到表格
    if (result.detected) {
        appendEventRow(result, frame);
    }
}

/**
 * @brief 场景切换槽函数
 * 
 * 用户切换模拟场景时触发：
 * 1. 获取选中场景的枚举值
 * 2. 更新频谱模拟器的场景设置
 * 3. 重置频谱分析器（清除跳频历史）
 * 
 * @param index 场景索引
 */
void MainWindow::onScenarioChanged(int index)
{
    // 获取场景枚举值
    const int value = m_scenarioCombo->itemData(index).toInt();
    
    // 更新模拟器场景
    m_simulator.setScenario((SpectrumSimulator::Scenario)value);
    
    // 重置分析器状态（清除跳频历史等）
    m_analyzer.reset();
}

/**
 * @brief 门限调整槽函数
 * 
 * 用户调整检测门限时触发：
 * 1. 更新分析器的门限值
 * 2. 更新门限标签显示
 * 
 * @param value 门限功率值（dBm）
 */
void MainWindow::onThresholdChanged(int value)
{
    // 更新分析器门限值
    m_analyzer.setThresholdDbm(value);
    
    // 更新门限标签显示
    if (m_thresholdLabel) {
        m_thresholdLabel->setText(QString::number(value) + QStringLiteral(" dBm"));
    }
}

/**
 * @brief 运行/暂停按钮点击槽函数
 * 
 * 切换运行状态：
 * - 运行中 → 暂停（按钮文字变为"继续"）
 * - 暂停 → 运行中（按钮文字变为"暂停"）
 */
void MainWindow::onRunButtonClicked()
{
    // 切换运行状态
    m_running = !m_running;
    
    // 更新按钮文字
    m_runButton->setText(m_running ? QStringLiteral("暂停") : QStringLiteral("继续"));
}

/**
 * @brief 清空按钮点击槽函数
 * 
 * 清空所有数据和状态：
 * 1. 清空频谱图
 * 2. 清空瀑布图
 * 3. 清空事件表格
 * 4. 重置分析器状态
 */
void MainWindow::onClearButtonClicked()
{
    // 清空频谱图
    m_spectrumPlot->clear();
    
    // 清空瀑布图
    m_waterfall->clear();
    
    // 清空事件表格（设置行数为0）
    m_eventTable->setRowCount(0);
    
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
    m_statusLabel->setText(status);
    
    // 峰值功率：格式为 "峰值功率: X.X dBm @ Y.YY MHz"
    m_peakLabel->setText(QStringLiteral("峰值功率: %1 dBm @ %2 MHz")
                         .arg(result.peakDbm, 0, 'f', 1)
                         .arg(result.centerFreqMHz, 0, 'f', 2));
    
    // 占用带宽：格式为 "占用带宽: X.YY MHz    活跃频点: X    跳频次数: X"
    m_bandwidthLabel->setText(QStringLiteral("占用带宽: %1 MHz    活跃频点: %2    跳频次数: %3")
                              .arg(result.bandwidthMHz, 0, 'f', 2)
                              .arg(result.activeBins)
                              .arg(result.hopCount));
    
    // 置信度：格式为 "置信度: XX%"
    m_confidenceLabel->setText(QStringLiteral("置信度: %1%")
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
    if (m_eventTable->rowCount() > 80) {
        m_eventTable->removeRow(m_eventTable->rowCount() - 1);  // 移除最底部（最旧）的行
    }

    // 在表格顶部插入新行（行索引0）
    m_eventTable->insertRow(0);
    
    // 填充各列数据
    m_eventTable->setItem(0, 0, new QTableWidgetItem(QDateTime::currentDateTime().toString("HH:mm:ss.zzz")));  // 时间（毫秒精度）
    m_eventTable->setItem(0, 1, new QTableWidgetItem(result.label));                                           // 检测结果
    m_eventTable->setItem(0, 2, new QTableWidgetItem(QString::number(result.peakDbm, 'f', 1)));               // 峰值功率
    m_eventTable->setItem(0, 3, new QTableWidgetItem(QString::number(result.centerFreqMHz, 'f', 2)));         // 中心频率
    m_eventTable->setItem(0, 4, new QTableWidgetItem(QString::number(result.bandwidthMHz, 'f', 2)));          // 带宽
}