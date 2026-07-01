#include "waterfallwidget.h"

#include <QPainter>
#include <QPaintEvent>
#include <QtMath>

WaterfallWidget::WaterfallWidget(QWidget *parent)
    : QWidget(parent)
    , m_hasFrame(false)
    , m_minDbm(-100.0f)
    , m_maxDbm(-35.0f)
{
    setMinimumSize(640, 420);
    setAutoFillBackground(false);
}

void WaterfallWidget::setPowerRange(float minDbm, float maxDbm)
{
    m_minDbm = minDbm;
    m_maxDbm = qMax(minDbm + 1.0f, maxDbm);
    update();
}

void WaterfallWidget::addFrame(const SpectrumFrame &frame)
{
    if (frame.powerDbm.isEmpty()) {
        return;
    }

    ensureImage();
    if (m_waterfall.isNull()) {
        return;
    }

    // 旧图整体下移一行，最上面写入当前帧，形成随时间滚动的瀑布图。
    QPainter imagePainter(&m_waterfall);
    imagePainter.drawImage(QPoint(0, 1), m_waterfall, QRect(0, 0, m_waterfall.width(), m_waterfall.height() - 1));

    for (int x = 0; x < m_waterfall.width(); ++x) {
        const int rawBin = (int)((qint64)x * frame.powerDbm.size() / qMax(1, m_waterfall.width()));
        const int bin = qBound(0, rawBin, frame.powerDbm.size() - 1);
        m_waterfall.setPixelColor(x, 0, colorForPower(frame.powerDbm.at(bin)));
    }

    m_lastFrame = frame;
    m_hasFrame = true;
    update();
}

void WaterfallWidget::clear()
{
    if (!m_waterfall.isNull()) {
        m_waterfall.fill(QColor(8, 12, 18));
    }
    m_hasFrame = false;
    update();
}

void WaterfallWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    ensureImage();

    QPainter painter(this);
    painter.fillRect(rect(), QColor(18, 22, 28));

    const int leftMargin = 58;
    const int rightMargin = 16;
    const int topMargin = 18;
    const int bottomMargin = 34;
    const QRect plotRect(leftMargin, topMargin,
                         qMax(1, width() - leftMargin - rightMargin),
                         qMax(1, height() - topMargin - bottomMargin));

    if (!m_waterfall.isNull()) {
        painter.drawImage(plotRect, m_waterfall);
    }

    painter.setPen(QColor(95, 110, 130));
    painter.drawRect(plotRect);

    painter.setPen(QColor(180, 190, 205));
    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);
    painter.drawText(8, topMargin + 12, QStringLiteral("最新"));
    painter.drawText(8, plotRect.bottom(), QStringLiteral("历史"));

    if (m_hasFrame) {
        const double startMHz = m_lastFrame.centerFreqMHz - m_lastFrame.spanMHz / 2.0;
        const double endMHz = m_lastFrame.centerFreqMHz + m_lastFrame.spanMHz / 2.0;
        painter.drawText(plotRect.left(), height() - 10, QString::number(startMHz, 'f', 1) + QStringLiteral(" MHz"));
        painter.drawText(plotRect.center().x() - 42, height() - 10, QString::number(m_lastFrame.centerFreqMHz, 'f', 1) + QStringLiteral(" MHz"));
        painter.drawText(plotRect.right() - 78, height() - 10, QString::number(endMHz, 'f', 1) + QStringLiteral(" MHz"));
    }
}

void WaterfallWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    m_waterfall = QImage();
    ensureImage();
}

QColor WaterfallWidget::colorForPower(float dbm) const
{
    const float t = qBound(0.0f, (dbm - m_minDbm) / (m_maxDbm - m_minDbm), 1.0f);

    // 分段色带：低能量黑蓝，中等能量青绿，高能量黄红。
    if (t < 0.25f) {
        const float k = t / 0.25f;
        return QColor(5, 10 + (int)(50 * k), 35 + (int)(90 * k));
    }
    if (t < 0.55f) {
        const float k = (t - 0.25f) / 0.30f;
        return QColor(0, 60 + (int)(150 * k), 130 + (int)(90 * k));
    }
    if (t < 0.80f) {
        const float k = (t - 0.55f) / 0.25f;
        return QColor((int)(240 * k), 210 + (int)(35 * k), 80 - (int)(40 * k));
    }
    const float k = (t - 0.80f) / 0.20f;
    return QColor(240 + (int)(15 * k), 170 - (int)(140 * k), 35 - (int)(25 * k));
}

void WaterfallWidget::ensureImage()
{
    const int leftMargin = 58;
    const int rightMargin = 16;
    const int topMargin = 18;
    const int bottomMargin = 34;
    const int imageWidth = qMax(1, width() - leftMargin - rightMargin);
    const int imageHeight = qMax(1, height() - topMargin - bottomMargin);

    if (m_waterfall.size() == QSize(imageWidth, imageHeight)) {
        return;
    }

    QImage newImage(imageWidth, imageHeight, QImage::Format_RGB32);
    newImage.fill(QColor(8, 12, 18));
    if (!m_waterfall.isNull()) {
        QPainter painter(&newImage);
        painter.drawImage(QRect(0, 0, imageWidth, imageHeight), m_waterfall);
    }
    m_waterfall = newImage;
}
