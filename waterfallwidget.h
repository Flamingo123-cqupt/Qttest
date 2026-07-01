#ifndef WATERFALLWIDGET_H
#define WATERFALLWIDGET_H

#include "spectrumsimulator.h"

#include <QWidget>
#include <QImage>

class WaterfallWidget : public QWidget
{
    Q_OBJECT
public:
    explicit WaterfallWidget(QWidget *parent = 0);

    void setPowerRange(float minDbm, float maxDbm);
    void addFrame(const SpectrumFrame &frame);
    void clear();

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    QColor colorForPower(float dbm) const;
    void ensureImage();

private:
    QImage m_waterfall;
    SpectrumFrame m_lastFrame;
    bool m_hasFrame;
    float m_minDbm;
    float m_maxDbm;
};

#endif // WATERFALLWIDGET_H
