#ifndef SPECTRUMPLOTWIDGET_H
#define SPECTRUMPLOTWIDGET_H

#include "spectrumsimulator.h"
#include "spectrumanalyzer.h"

#include <QWidget>

class SpectrumPlotWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SpectrumPlotWidget(QWidget *parent = 0);

    void setPowerRange(float minDbm, float maxDbm);
    void setFrame(const SpectrumFrame &frame, const DetectionResult &result);
    void clear();

protected:
    void paintEvent(QPaintEvent *event);

private:
    QPointF pointForBin(const QRect &plotRect, int bin, float dbm) const;
    void drawGrid(QPainter &painter, const QRect &plotRect) const;
    void drawSpectrum(QPainter &painter, const QRect &plotRect) const;
    void drawDetectionMarker(QPainter &painter, const QRect &plotRect) const;

private:
    SpectrumFrame m_frame;
    DetectionResult m_result;
    bool m_hasFrame;
    float m_minDbm;
    float m_maxDbm;
};

#endif // SPECTRUMPLOTWIDGET_H
