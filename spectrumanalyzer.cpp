#include "spectrumanalyzer.h"

#include <QtMath>

SpectrumAnalyzer::SpectrumAnalyzer(QObject *parent)
    : QObject(parent)
    , m_thresholdDbm(-68.0)
{
}

void SpectrumAnalyzer::setThresholdDbm(double thresholdDbm)
{
    m_thresholdDbm = thresholdDbm;
}

double SpectrumAnalyzer::thresholdDbm() const
{
    return m_thresholdDbm;
}

DetectionResult SpectrumAnalyzer::analyze(const SpectrumFrame &frame)
{
    DetectionResult result;
    result.detected = false;
    result.label = QStringLiteral("未发现无人机信号");
    result.detail = QStringLiteral("频谱能量低于检测门限");
    result.confidence = 0.0;
    result.peakDbm = -120.0;
    result.centerFreqMHz = frame.centerFreqMHz;
    result.bandwidthMHz = 0.0;
    result.activeBins = countActiveBins(frame.powerDbm);
    result.hopCount = 0;

    float peak = -120.0f;
    const int peakBin = findPeakBin(frame.powerDbm, &peak);
    result.peakDbm = peak;
    if (peakBin >= 0) {
        result.centerFreqMHz = frame.centerFreqMHz - frame.spanMHz / 2.0
                + (double)peakBin * frame.spanMHz / qMax(1, frame.powerDbm.size() - 1);
    }

    int firstBin = -1;
    int lastBin = -1;
    result.bandwidthMHz = estimateBandwidthMHz(frame, &firstBin, &lastBin);
    updateHopHistory(peakBin);
    result.hopCount = recentHopCount();

    if (peak < m_thresholdDbm) {
        return result;
    }

    result.detected = true;

    // 简化识别规则：用于模块演示，实际工程可替换成特征库/机器学习模型。
    if (result.hopCount >= 4 && result.bandwidthMHz < 8.0) {
        result.label = QStringLiteral("疑似无人机跳频遥控信号");
        result.confidence = 0.86;
        result.detail = QStringLiteral("窄带峰值在多个频点间快速切换，符合跳频链路特征");
    } else if (result.bandwidthMHz >= 10.0 && result.activeBins >= 35) {
        result.label = QStringLiteral("疑似无人机图传信号");
        result.confidence = 0.82;
        result.detail = QStringLiteral("检测到持续宽带能量平台，疑似视频/数传链路");
    } else if (result.bandwidthMHz > 0.4 && result.bandwidthMHz < 8.0) {
        result.label = QStringLiteral("疑似无人机遥控信号");
        result.confidence = 0.74;
        result.detail = QStringLiteral("检测到持续窄带高能量峰，疑似遥控控制链路");
    } else {
        result.label = QStringLiteral("未知无线信号");
        result.confidence = 0.45;
        result.detail = QStringLiteral("能量超过门限，但特征不足以归类为典型无人机链路");
    }

    const double margin = qBound(0.0, (result.peakDbm - m_thresholdDbm) / 24.0, 1.0);
    result.confidence = qBound(0.0, result.confidence * 0.75 + margin * 0.25, 0.98);
    return result;
}

void SpectrumAnalyzer::reset()
{
    m_recentPeakBins.clear();
}

int SpectrumAnalyzer::countActiveBins(const QVector<float> &bins) const
{
    int count = 0;
    for (int i = 0; i < bins.size(); ++i) {
        if (bins.at(i) >= m_thresholdDbm) {
            ++count;
        }
    }
    return count;
}

int SpectrumAnalyzer::findPeakBin(const QVector<float> &bins, float *peakValue) const
{
    if (bins.isEmpty()) {
        return -1;
    }

    int peakBin = 0;
    float peak = bins.at(0);
    for (int i = 1; i < bins.size(); ++i) {
        if (bins.at(i) > peak) {
            peak = bins.at(i);
            peakBin = i;
        }
    }

    if (peakValue) {
        *peakValue = peak;
    }
    return peakBin;
}

double SpectrumAnalyzer::estimateBandwidthMHz(const SpectrumFrame &frame, int *firstBin, int *lastBin) const
{
    int first = -1;
    int last = -1;
    for (int i = 0; i < frame.powerDbm.size(); ++i) {
        if (frame.powerDbm.at(i) >= m_thresholdDbm) {
            if (first < 0) {
                first = i;
            }
            last = i;
        }
    }

    if (firstBin) {
        *firstBin = first;
    }
    if (lastBin) {
        *lastBin = last;
    }
    if (first < 0 || last < first || frame.powerDbm.size() <= 1) {
        return 0.0;
    }

    return (double)(last - first + 1) * frame.spanMHz / (double)frame.powerDbm.size();
}

void SpectrumAnalyzer::updateHopHistory(int peakBin)
{
    if (peakBin < 0) {
        return;
    }
    m_recentPeakBins.append(peakBin);
    while (m_recentPeakBins.size() > 24) {
        m_recentPeakBins.removeFirst();
    }
}

int SpectrumAnalyzer::recentHopCount() const
{
    if (m_recentPeakBins.size() < 2) {
        return 0;
    }

    int changes = 0;
    int last = m_recentPeakBins.first();
    for (int i = 1; i < m_recentPeakBins.size(); ++i) {
        const int current = m_recentPeakBins.at(i);
        if (qAbs(current - last) > 18) {
            ++changes;
            last = current;
        }
    }
    return changes;
}
