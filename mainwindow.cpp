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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_timer(new QTimer(this))
    , m_spectrumPlot(0)
    , m_waterfall(0)
    , m_scenarioCombo(0)
    , m_thresholdSlider(0)
    , m_thresholdLabel(0)
    , m_statusLabel(0)
    , m_peakLabel(0)
    , m_bandwidthLabel(0)
    , m_confidenceLabel(0)
    , m_runButton(0)
    , m_clearButton(0)
    , m_eventTable(0)
    , m_running(true)
{
    buildUi();
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimerTick()));
    m_timer->start(80);
}

MainWindow::~MainWindow()
{
}

void MainWindow::buildUi()
{
    QWidget *central = new QWidget(this);
    QHBoxLayout *rootLayout = new QHBoxLayout(central);
    rootLayout->setContentsMargins(10, 10, 10, 10);
    rootLayout->setSpacing(10);

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->setSpacing(10);

    QGroupBox *controlBox = new QGroupBox(QStringLiteral("模拟与检测控制"), central);
    QGridLayout *controlLayout = new QGridLayout(controlBox);

    m_scenarioCombo = new QComboBox(controlBox);
    m_scenarioCombo->addItem(QStringLiteral("环境底噪"), SpectrumSimulator::IdleNoise);
    m_scenarioCombo->addItem(QStringLiteral("无人机遥控窄带链路"), SpectrumSimulator::DroneControl);
    m_scenarioCombo->addItem(QStringLiteral("无人机图传宽带链路"), SpectrumSimulator::DroneVideo);
    m_scenarioCombo->addItem(QStringLiteral("无人机跳频链路"), SpectrumSimulator::FrequencyHopping);
    m_scenarioCombo->setCurrentIndex(1);
    connect(m_scenarioCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onScenarioChanged(int)));

    m_thresholdSlider = new QSlider(Qt::Horizontal, controlBox);
    m_thresholdSlider->setRange(-90, -40);
    m_thresholdSlider->setValue((int)m_analyzer.thresholdDbm());
    connect(m_thresholdSlider, SIGNAL(valueChanged(int)), this, SLOT(onThresholdChanged(int)));

    m_thresholdLabel = new QLabel(controlBox);
    onThresholdChanged(m_thresholdSlider->value());

    m_runButton = new QPushButton(QStringLiteral("暂停"), controlBox);
    m_clearButton = new QPushButton(QStringLiteral("清空"), controlBox);
    connect(m_runButton, SIGNAL(clicked()), this, SLOT(onRunButtonClicked()));
    connect(m_clearButton, SIGNAL(clicked()), this, SLOT(onClearButtonClicked()));

    controlLayout->addWidget(new QLabel(QStringLiteral("模拟场景"), controlBox), 0, 0);
    controlLayout->addWidget(m_scenarioCombo, 0, 1, 1, 2);
    controlLayout->addWidget(new QLabel(QStringLiteral("检测门限"), controlBox), 1, 0);
    controlLayout->addWidget(m_thresholdSlider, 1, 1);
    controlLayout->addWidget(m_thresholdLabel, 1, 2);
    controlLayout->addWidget(m_runButton, 2, 1);
    controlLayout->addWidget(m_clearButton, 2, 2);

    QGroupBox *resultBox = new QGroupBox(QStringLiteral("识别结果"), central);
    QVBoxLayout *resultLayout = new QVBoxLayout(resultBox);
    m_statusLabel = new QLabel(QStringLiteral("等待数据"), resultBox);
    m_peakLabel = new QLabel(QStringLiteral("峰值功率: -"), resultBox);
    m_bandwidthLabel = new QLabel(QStringLiteral("占用带宽: -"), resultBox);
    m_confidenceLabel = new QLabel(QStringLiteral("置信度: -"), resultBox);
    m_statusLabel->setWordWrap(true);
    resultLayout->addWidget(m_statusLabel);
    resultLayout->addWidget(m_peakLabel);
    resultLayout->addWidget(m_bandwidthLabel);
    resultLayout->addWidget(m_confidenceLabel);
    resultLayout->addStretch();

    m_eventTable = new QTableWidget(0, 5, central);
    QStringList headers;
    headers << QStringLiteral("时间") << QStringLiteral("结果") << QStringLiteral("峰值dBm")
            << QStringLiteral("中心MHz") << QStringLiteral("带宽MHz");
    m_eventTable->setHorizontalHeaderLabels(headers);
    m_eventTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_eventTable->verticalHeader()->setVisible(false);
    m_eventTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_eventTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    leftLayout->addWidget(controlBox);
    leftLayout->addWidget(resultBox);
    leftLayout->addWidget(new QLabel(QStringLiteral("检测事件"), central));
    leftLayout->addWidget(m_eventTable, 1);

    m_spectrumPlot = new SpectrumPlotWidget(central);
    m_waterfall = new WaterfallWidget(central);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->setSpacing(8);
    rightLayout->addWidget(m_spectrumPlot, 0);
    rightLayout->addWidget(m_waterfall, 1);

    rootLayout->addLayout(leftLayout, 0);
    rootLayout->addLayout(rightLayout, 1);
    setCentralWidget(central);
    setWindowTitle(QStringLiteral("无人机频谱瀑布图识别检测模块 - Qt 5.12.12"));
}

void MainWindow::onTimerTick()
{
    if (!m_running) {
        return;
    }

    const SpectrumFrame frame = m_simulator.nextFrame();
    const DetectionResult result = m_analyzer.analyze(frame);
    m_spectrumPlot->setFrame(frame, result);
    m_waterfall->addFrame(frame);
    updateResultPanel(result, frame);

    if (result.detected) {
        appendEventRow(result, frame);
    }
}

void MainWindow::onScenarioChanged(int index)
{
    const int value = m_scenarioCombo->itemData(index).toInt();
    m_simulator.setScenario((SpectrumSimulator::Scenario)value);
    m_analyzer.reset();
}

void MainWindow::onThresholdChanged(int value)
{
    m_analyzer.setThresholdDbm(value);
    if (m_thresholdLabel) {
        m_thresholdLabel->setText(QString::number(value) + QStringLiteral(" dBm"));
    }
}

void MainWindow::onRunButtonClicked()
{
    m_running = !m_running;
    m_runButton->setText(m_running ? QStringLiteral("暂停") : QStringLiteral("继续"));
}

void MainWindow::onClearButtonClicked()
{
    m_spectrumPlot->clear();
    m_waterfall->clear();
    m_eventTable->setRowCount(0);
    m_analyzer.reset();
}

void MainWindow::updateResultPanel(const DetectionResult &result, const SpectrumFrame &frame)
{
    QString status = result.label + QStringLiteral("\n") + result.detail
            + QStringLiteral("\n当前场景: ") + frame.scenarioName;
    m_statusLabel->setText(status);
    m_peakLabel->setText(QStringLiteral("峰值功率: %1 dBm @ %2 MHz")
                         .arg(result.peakDbm, 0, 'f', 1)
                         .arg(result.centerFreqMHz, 0, 'f', 2));
    m_bandwidthLabel->setText(QStringLiteral("占用带宽: %1 MHz    活跃频点: %2    跳频次数: %3")
                              .arg(result.bandwidthMHz, 0, 'f', 2)
                              .arg(result.activeBins)
                              .arg(result.hopCount));
    m_confidenceLabel->setText(QStringLiteral("置信度: %1%")
                               .arg(result.confidence * 100.0, 0, 'f', 0));
}

void MainWindow::appendEventRow(const DetectionResult &result, const SpectrumFrame &frame)
{
    Q_UNUSED(frame);

    // 控制表格长度，避免长时间运行后表格越来越大。
    if (m_eventTable->rowCount() > 80) {
        m_eventTable->removeRow(m_eventTable->rowCount() - 1);
    }

    m_eventTable->insertRow(0);
    m_eventTable->setItem(0, 0, new QTableWidgetItem(QDateTime::currentDateTime().toString("HH:mm:ss.zzz")));
    m_eventTable->setItem(0, 1, new QTableWidgetItem(result.label));
    m_eventTable->setItem(0, 2, new QTableWidgetItem(QString::number(result.peakDbm, 'f', 1)));
    m_eventTable->setItem(0, 3, new QTableWidgetItem(QString::number(result.centerFreqMHz, 'f', 2)));
    m_eventTable->setItem(0, 4, new QTableWidgetItem(QString::number(result.bandwidthMHz, 'f', 2)));
}
