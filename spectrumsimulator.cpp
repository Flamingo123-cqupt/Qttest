#include "spectrumsimulator.h"

#include <QtMath>
#include <QRandomGenerator>

SpectrumSimulator::SpectrumSimulator(QObject *parent)
    : QObject(parent)
    , m_scenario(DroneControl)
    , m_binCount(512)
    , m_centerFreqMHz(2400.0)
    , m_spanMHz(100.0)
    , m_timeSec(0.0)
    , m_hopIndex(0)
{
}

void SpectrumSimulator::setScenario(SpectrumSimulator::Scenario scenario)
{
    m_scenario = scenario;
    m_hopIndex = 0;
}

SpectrumSimulator::Scenario SpectrumSimulator::scenario() const
{
    return m_scenario;
}

SpectrumFrame SpectrumSimulator::nextFrame()
{
    QVector<float> bins(m_binCount);
    addNoise(bins);

    QString name;
    switch (m_scenario) {
    case IdleNoise:
        name = QStringLiteral("环境底噪");
        break;
    case DroneControl:
        name = QStringLiteral("无人机遥控窄带链路");
        // 遥控链路常表现为较窄带、持续存在的能量峰。
        addGaussianSignal(bins, m_binCount * 0.42 + 8.0 * qSin(m_timeSec * 1.3), 5.0, -48.0f);
        addGaussianSignal(bins, m_binCount * 0.61, 3.0, -55.0f);
        break;
    case DroneVideo:
        name = QStringLiteral("无人机图传宽带链路");
        // 图传链路通常带宽更宽，功率平台更连续。
        addFlatSignal(bins, m_binCount * 0.50 + 14.0 * qSin(m_timeSec * 0.5), 70.0, -50.0f);
        addGaussianSignal(bins, m_binCount * 0.50, 35.0, -54.0f);
        break;
    case FrequencyHopping: {
        name = QStringLiteral("无人机跳频链路");
        const int hopBins[] = {90, 145, 210, 270, 330, 405};
        if (((int)(m_timeSec * 10.0)) % 2 == 0) {
            m_hopIndex = (m_hopIndex + 1) % 6;
        }
        addGaussianSignal(bins, hopBins[m_hopIndex], 4.0, -45.0f);
        addGaussianSignal(bins, hopBins[(m_hopIndex + 2) % 6], 3.0, -58.0f);
        break;
    }
    }

    SpectrumFrame frame;
    frame.powerDbm = bins;
    frame.centerFreqMHz = m_centerFreqMHz;
    frame.spanMHz = m_spanMHz;
    frame.timeSec = m_timeSec;
    frame.scenarioName = name;

    m_timeSec += 0.08;
    return frame;
}

int SpectrumSimulator::binCount() const
{
    return m_binCount;
}

double SpectrumSimulator::binToFreqMHz(int bin) const
{
    if (m_binCount <= 1) {
        return m_centerFreqMHz;
    }
    const double startMHz = m_centerFreqMHz - m_spanMHz / 2.0;
    return startMHz + (double)bin * m_spanMHz / (double)(m_binCount - 1);
}

void SpectrumSimulator::addNoise(QVector<float> &bins) const
{
    for (int i = 0; i < bins.size(); ++i) {
        const float slowRipple = 2.5f * (float)qSin((double)i * 0.035 + m_timeSec * 0.8);
        bins[i] = -92.0f + slowRipple + randomFloat(-4.0f, 4.0f);
    }
}

void SpectrumSimulator::addGaussianSignal(QVector<float> &bins, double centerBin, double widthBins, float peakDbm) const
{
    const double sigma = qMax(1.0, widthBins);
    for (int i = 0; i < bins.size(); ++i) {
        const double d = ((double)i - centerBin) / sigma;
        const float shape = (float)qExp(-0.5 * d * d);
        const float value = -95.0f + (peakDbm + 95.0f) * shape;
        bins[i] = qMax(bins[i], value + randomFloat(-1.2f, 1.2f));
    }
}

void SpectrumSimulator::addFlatSignal(QVector<float> &bins, double centerBin, double widthBins, float peakDbm) const
{
    const double half = widthBins / 2.0;
    for (int i = 0; i < bins.size(); ++i) {
        const double distance = qAbs((double)i - centerBin);
        if (distance <= half) {
            bins[i] = qMax(bins[i], peakDbm + randomFloat(-3.0f, 2.0f));
        } else if (distance <= half + 18.0) {
            const double edge = 1.0 - (distance - half) / 18.0;
            bins[i] = qMax(bins[i], -92.0f + (peakDbm + 92.0f) * (float)edge);
        }
    }
}

float SpectrumSimulator::randomFloat(float minValue, float maxValue) const
{
    const double u = QRandomGenerator::global()->generateDouble();
    return minValue + (maxValue - minValue) * (float)u;
}
