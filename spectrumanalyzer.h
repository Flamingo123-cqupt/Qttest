#ifndef SPECTRUMANALYZER_H
#define SPECTRUMANALYZER_H

#include "spectrumsimulator.h"

#include <QObject>
#include <QVector>
#include <QString>

struct DetectionResult
{
    bool detected;
    QString label;
    QString detail;
    double confidence;
    double peakDbm;
    double centerFreqMHz;
    double bandwidthMHz;
    int activeBins;
    int hopCount;
};

class SpectrumAnalyzer : public QObject
{
    Q_OBJECT
public:
    explicit SpectrumAnalyzer(QObject *parent = 0);

    void setThresholdDbm(double thresholdDbm);
    double thresholdDbm() const;
    DetectionResult analyze(const SpectrumFrame &frame);
    void reset();

private:
    int countActiveBins(const QVector<float> &bins) const;
    int findPeakBin(const QVector<float> &bins, float *peakValue) const;
    double estimateBandwidthMHz(const SpectrumFrame &frame, int *firstBin, int *lastBin) const;
    void updateHopHistory(int peakBin);
    int recentHopCount() const;

private:
    double m_thresholdDbm;
    QVector<int> m_recentPeakBins;
};

#endif // SPECTRUMANALYZER_H
