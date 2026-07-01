#include "spectrumplotwidget.h"

#include <QPainter>
#include <QPainterPath>
#include <QtMath>

SpectrumPlotWidget::SpectrumPlotWidget(QWidget *parent)
    : QWidget(parent)
    , m_hasFrame(false)
    , m_minDbm(-100.0f)
    , m_maxDbm(-35.0f)
{
    setMinimumHeight(230);
    setAutoFillBackground(false);
}

void SpectrumPlotWidget::setPowerRange(float minDbm, float maxDbm)
{
    m_minDbm = minDbm;
    m_maxDbm = qMax(minDbm + 1.0f, maxDbm);
    update();
}

void SpectrumPlotWidget::setFrame(const SpectrumFrame &frame, const DetectionResult &result)
{
    m_frame = frame;
    m_result = result;
    m_hasFrame = true;
    update();
}

void SpectrumPlotWidget::clear()
{
    m_hasFrame = false;
    update();
}

void SpectrumPlotWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), QColor(18, 22, 28));

    const int leftMargin = 62;
    const int rightMargin = 18;
    const int topMargin = 28;
    const int bottomMargin = 36;
    const QRect plotRect(leftMargin, topMargin,
                         qMax(1, width() - leftMargin - rightMargin),
                         qMax(1, height() - topMargin - bottomMargin));

    drawGrid(painter, plotRect);
    drawSpectrum(painter, plotRect);
    drawDetectionMarker(painter, plotRect);
}

QPointF SpectrumPlotWidget::pointForBin(const QRect &plotRect, int bin, float dbm) const
{
    const int count = m_frame.powerDbm.size();
    const double xRatio = (count > 1) ? (double)bin / (double)(count - 1) : 0.0;
    const double yRatio = qBound(0.0, ((double)dbm - (double)m_minDbm) / ((double)m_maxDbm - (double)m_minDbm), 1.0);
    const double x = plotRect.left() + xRatio * plotRect.width();
    const double y = plotRect.bottom() - yRatio * plotRect.height();
    return QPointF(x, y);
}

void SpectrumPlotWidget::drawGrid(QPainter &painter, const QRect &plotRect) const
{
    painter.save();
    painter.setPen(QPen(QColor(48, 58, 70), 1));
    for (int i = 0; i <= 10; ++i) {
        const int x = plotRect.left() + i * plotRect.width() / 10;
        painter.drawLine(x, plotRect.top(), x, plotRect.bottom());
    }
    for (int i = 0; i <= 6; ++i) {
        const int y = plotRect.top() + i * plotRect.height() / 6;
        painter.drawLine(plotRect.left(), y, plotRect.right(), y);
    }

    painter.setPen(QColor(120, 135, 155));
    painter.drawRect(plotRect);

    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);
    painter.setPen(QColor(188, 198, 210));
    painter.drawText(10, 18, QStringLiteral("实时频谱图"));
    painter.drawText(8, plotRect.top() + 6, QString::number(m_maxDbm, 'f', 0) + QStringLiteral(" dBm"));
    painter.drawText(8, plotRect.bottom(), QString::number(m_minDbm, 'f', 0) + QStringLiteral(" dBm"));

    if (m_hasFrame) {
        const double startMHz = m_frame.centerFreqMHz - m_frame.spanMHz / 2.0;
        const double endMHz = m_frame.centerFreqMHz + m_frame.spanMHz / 2.0;
        painter.drawText(plotRect.left(), height() - 10, QString::number(startMHz, 'f', 1) + QStringLiteral(" MHz"));
        painter.drawText(plotRect.center().x() - 42, height() - 10, QString::number(m_frame.centerFreqMHz, 'f', 1) + QStringLiteral(" MHz"));
        painter.drawText(plotRect.right() - 78, height() - 10, QString::number(endMHz, 'f', 1) + QStringLiteral(" MHz"));
    }
    painter.restore();
}

void SpectrumPlotWidget::drawSpectrum(QPainter &painter, const QRect &plotRect) const
{
    if (!m_hasFrame || m_frame.powerDbm.isEmpty()) {
        painter.save();
        painter.setPen(QColor(145, 155, 170));
        painter.drawText(plotRect, Qt::AlignCenter, QStringLiteral("等待频谱数据"));
        painter.restore();
        return;
    }

    QPainterPath fillPath;
    QPainterPath linePath;
    const QPointF firstPoint = pointForBin(plotRect, 0, m_frame.powerDbm.first());
    linePath.moveTo(firstPoint);
    fillPath.moveTo(plotRect.left(), plotRect.bottom());
    fillPath.lineTo(firstPoint);

    for (int i = 1; i < m_frame.powerDbm.size(); ++i) {
        const QPointF p = pointForBin(plotRect, i, m_frame.powerDbm.at(i));
        linePath.lineTo(p);
        fillPath.lineTo(p);
    }
    fillPath.lineTo(plotRect.right(), plotRect.bottom());
    fillPath.closeSubpath();

    painter.save();
    painter.setClipRect(plotRect.adjusted(1, 1, -1, -1));
    painter.fillPath(fillPath, QColor(0, 190, 210, 42));
    painter.setPen(QPen(QColor(30, 235, 220), 1.4));
    painter.drawPath(linePath);

    // 逐点叠加亮点，让用户能看到频谱 bin 的离散采样。
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(235, 255, 210));
    const int step = qMax(1, m_frame.powerDbm.size() / qMax(80, plotRect.width() / 4));
    for (int i = 0; i < m_frame.powerDbm.size(); i += step) {
        const QPointF p = pointForBin(plotRect, i, m_frame.powerDbm.at(i));
        painter.drawEllipse(p, 1.8, 1.8);
    }
    painter.restore();
}

void SpectrumPlotWidget::drawDetectionMarker(QPainter &painter, const QRect &plotRect) const
{
    if (!m_hasFrame || m_frame.powerDbm.isEmpty()) {
        return;
    }

    int peakBin = 0;
    float peak = m_frame.powerDbm.first();
    for (int i = 1; i < m_frame.powerDbm.size(); ++i) {
        if (m_frame.powerDbm.at(i) > peak) {
            peak = m_frame.powerDbm.at(i);
            peakBin = i;
        }
    }

    const QPointF peakPoint = pointForBin(plotRect, peakBin, peak);
    painter.save();
    painter.setPen(QPen(m_result.detected ? QColor(255, 90, 60) : QColor(210, 210, 120), 1, Qt::DashLine));
    painter.drawLine(QPointF(peakPoint.x(), plotRect.top()), QPointF(peakPoint.x(), plotRect.bottom()));
    painter.setBrush(m_result.detected ? QColor(255, 90, 60) : QColor(210, 210, 120));
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(peakPoint, 4.0, 4.0);

    painter.setPen(QColor(230, 235, 240));
    const QString text = QStringLiteral("峰值 %1 dBm  %2 MHz")
            .arg(peak, 0, 'f', 1)
            .arg(m_result.centerFreqMHz, 0, 'f', 2);
    painter.drawText(plotRect.adjusted(8, 6, -8, -6), Qt::AlignTop | Qt::AlignRight, text);
    painter.restore();
}
