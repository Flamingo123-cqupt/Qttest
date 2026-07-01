#ifndef SPECTRUMSIMULATOR_H
#define SPECTRUMSIMULATOR_H

#include <QObject>
#include <QVector>
#include <QString>

struct SpectrumFrame
{
    QVector<float> powerDbm;      // 每个频点的功率，单位 dBm。
    double centerFreqMHz;         // 当前分析带宽中心频率。
    double spanMHz;               // 当前分析带宽。
    double timeSec;               // 模拟时间戳。
    QString scenarioName;         // 当前模拟场景名称。
};

class SpectrumSimulator : public QObject
{
    Q_OBJECT
public:
    enum Scenario
    {
        IdleNoise = 0,       // 只有底噪。
        DroneControl,        // 窄带遥控链路。
        DroneVideo,          // 宽带图传链路。
        FrequencyHopping     // 跳频链路。
    };

    explicit SpectrumSimulator(QObject *parent = 0);

    void setScenario(Scenario scenario);
    Scenario scenario() const;
    SpectrumFrame nextFrame();

    int binCount() const;
    double binToFreqMHz(int bin) const;

private:
    void addNoise(QVector<float> &bins) const;
    void addGaussianSignal(QVector<float> &bins, double centerBin, double widthBins, float peakDbm) const;
    void addFlatSignal(QVector<float> &bins, double centerBin, double widthBins, float peakDbm) const;
    float randomFloat(float minValue, float maxValue) const;

private:
    Scenario m_scenario;
    int m_binCount;
    double m_centerFreqMHz;
    double m_spanMHz;
    double m_timeSec;
    int m_hopIndex;
};

#endif // SPECTRUMSIMULATOR_H
