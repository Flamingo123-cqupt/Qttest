#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "spectrumsimulator.h"
#include "spectrumanalyzer.h"

#include <QMainWindow>

class SpectrumPlotWidget;
class WaterfallWidget;
class QComboBox;
class QLabel;
class QPushButton;
class QSlider;
class QTableWidget;
class QTimer;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onTimerTick();
    void onScenarioChanged(int index);
    void onThresholdChanged(int value);
    void onRunButtonClicked();
    void onClearButtonClicked();

private:
    void buildUi();
    void updateResultPanel(const DetectionResult &result, const SpectrumFrame &frame);
    void appendEventRow(const DetectionResult &result, const SpectrumFrame &frame);

private:
    SpectrumSimulator m_simulator;
    SpectrumAnalyzer m_analyzer;
    QTimer *m_timer;
    SpectrumPlotWidget *m_spectrumPlot;
    WaterfallWidget *m_waterfall;
    QComboBox *m_scenarioCombo;
    QSlider *m_thresholdSlider;
    QLabel *m_thresholdLabel;
    QLabel *m_statusLabel;
    QLabel *m_peakLabel;
    QLabel *m_bandwidthLabel;
    QLabel *m_confidenceLabel;
    QPushButton *m_runButton;
    QPushButton *m_clearButton;
    QTableWidget *m_eventTable;
    bool m_running;
};

#endif // MAINWINDOW_H
