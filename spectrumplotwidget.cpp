/**
 * @file spectrumplotwidget.cpp
 * @brief 实时频谱图绘制组件实现
 * 
 * 实现频谱图的绘制逻辑，包括网格、频谱曲线、峰值标记等。
 * 使用QPainter进行2D绘图，支持抗锯齿渲染。
 */

#include "spectrumplotwidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QtMath>

/**
 * @brief 构造函数
 * 
 * 初始化功率显示范围（-100dBm到-35dBm），设置最小高度和背景填充方式。
 * 
 * @param parent 父对象指针
 */
SpectrumPlotWidget::SpectrumPlotWidget(QWidget *parent)
    : QWidget(parent)
    , m_hasFrame(false)          // 默认无帧数据
    , m_minDbm(-100.0f)          // 功率范围最小值：-100dBm
    , m_maxDbm(-35.0f)           // 功率范围最大值：-35dBm
    , m_mouseInWidget(false)     // 默认鼠标不在控件内
    , m_isZoomed(false)          // 默认未缩放
    , m_isSelecting(false)       // 默认未框选
    , m_selectedStartMHz(0.0)    // 默认选中起始频率
    , m_selectedEndMHz(0.0)      // 默认选中结束频率
{
    setMinimumHeight(230);       // 设置最小高度
    setAutoFillBackground(false); // 禁用自动背景填充（手动绘制）
    setMouseTracking(true);      // 启用鼠标跟踪
}

/**
 * @brief 设置功率显示范围
 * 
 * 更新功率显示范围，确保最大值大于最小值，触发重绘。
 * 
 * @param minDbm 最小功率值（dBm）
 * @param maxDbm 最大功率值（dBm）
 */
void SpectrumPlotWidget::setPowerRange(float minDbm, float maxDbm)
{
    m_minDbm = minDbm;
    m_maxDbm = qMax(minDbm + 1.0f, maxDbm);  // 确保范围有效
    update();  // 触发重绘
}

/**
 * @brief 设置当前帧数据
 * 
 * 更新频谱帧和检测结果，标记有有效帧数据，触发重绘。
 * 
 * @param frame 频谱帧数据
 * @param result 检测结果
 */
void SpectrumPlotWidget::setFrame(const SpectrumFrame &frame, const DetectionResult &result)
{
    m_frame = frame;
    m_result = result;
    m_hasFrame = true;
    update();  // 触发重绘
}

/**
 * @brief 清空显示
 * 
 * 标记无有效帧数据，触发重绘（显示"等待频谱数据"提示）。
 */
void SpectrumPlotWidget::clear()
{
    m_hasFrame = false;
    m_isZoomed = false;          // 重置缩放状态
    m_isSelecting = false;       // 重置框选状态
    m_selectedStartMHz = 0.0;    // 重置选中频率
    m_selectedEndMHz = 0.0;
    m_zoomStartMHz = 0.0;
    m_zoomEndMHz = 0.0;
    emit zoomChanged(0.0, 0.0);
    update();  // 触发重绘
}

/**
 * @brief 绘制事件处理
 * 
 * 重写QWidget的paintEvent，执行完整的频谱图绘制流程：
 * 1. 创建绘图对象，启用抗锯齿
 * 2. 填充背景
 * 3. 计算绘图区域（考虑边距）
 * 4. 绘制网格和坐标轴
 * 5. 绘制频谱曲线
 * 6. 绘制检测结果标记
 * 
 * @param event 绘制事件
 */
void SpectrumPlotWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);  // 忽略事件参数

    // 创建绘图对象，启用抗锯齿渲染
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    // 填充深色背景（RGB: 18, 22, 28）
    painter.fillRect(rect(), QColor(18, 22, 28));

    // 定义边距
    const int leftMargin = 62;    // 左边距（Y轴标签）
    const int rightMargin = 18;   // 右边距
    const int topMargin = 28;     // 上边距（标题）
    const int bottomMargin = 36;  // 下边距（X轴标签）
    
    // 计算绘图区域矩形
    const QRect plotRect(leftMargin, topMargin,
                         qMax(1, width() - leftMargin - rightMargin),
                         qMax(1, height() - topMargin - bottomMargin));

    // 绘制各个组件
    drawGrid(painter, plotRect);           // 绘制网格和坐标轴
    drawSpectrum(painter, plotRect);       // 绘制频谱曲线
    drawDetectionMarker(painter, plotRect); // 绘制检测标记
    drawSelection(painter, plotRect);      // 绘制框选区域
    drawMouseCursor(painter, plotRect);    // 绘制鼠标悬浮光标和标签
}

/**
 * @brief 将频点数据转换为绘图坐标
 * 
 * 根据频点索引和功率值，计算在绘图区域中的坐标位置：
 * - X坐标：频点索引线性映射到绘图区域宽度，支持缩放
 * - Y坐标：功率值线性映射到绘图区域高度（功率越高Y坐标越小）
 * 
 * @param plotRect 绘图区域矩形
 * @param bin 频点索引
 * @param dbm 功率值（dBm）
 * @return QPointF 绘图坐标
 */
QPointF SpectrumPlotWidget::pointForBin(const QRect &plotRect, int bin, float dbm) const
{
    const int count = m_frame.powerDbm.size();
    
    double xRatio;
    if (m_isZoomed) {
        const double totalSpan = m_frame.spanMHz;
        const double binFreq = m_frame.centerFreqMHz - totalSpan / 2.0 + (double)bin * totalSpan / (double)(count - 1);
        const double zoomSpan = m_zoomEndMHz - m_zoomStartMHz;
        xRatio = (binFreq - m_zoomStartMHz) / zoomSpan;
    } else {
        xRatio = (count > 1) ? (double)bin / (double)(count - 1) : 0.0;
    }
    
    xRatio = qBound(0.0, xRatio, 1.0);
    
    // 计算Y轴比例（(功率 - 最小值) / (最大值 - 最小值)），限制在0-1之间
    const double yRatio = qBound(0.0, ((double)dbm - (double)m_minDbm) / ((double)m_maxDbm - (double)m_minDbm), 1.0);
    
    // 计算实际坐标
    const double x = plotRect.left() + xRatio * plotRect.width();
    const double y = plotRect.bottom() - yRatio * plotRect.height();
    
    return QPointF(x, y);
}

/**
 * @brief 绘制网格和坐标轴
 * 
 * 绘制背景网格线、边框和坐标轴标签：
 * - 垂直网格线：10条
 * - 水平网格线：7条
 * - Y轴标签：功率范围（dBm）
 * - X轴标签：频率范围（MHz）
 * 
 * @param painter 绘图对象
 * @param plotRect 绘图区域矩形
 */
void SpectrumPlotWidget::drawGrid(QPainter &painter, const QRect &plotRect) const
{
    painter.save();  // 保存绘图状态

    // 绘制垂直网格线（10条）
    painter.setPen(QPen(QColor(48, 58, 70), 1));
    for (int i = 0; i <= 10; ++i) {
        const int x = plotRect.left() + i * plotRect.width() / 10;
        painter.drawLine(x, plotRect.top(), x, plotRect.bottom());
    }
    
    // 绘制水平网格线（7条）
    for (int i = 0; i <= 6; ++i) {
        const int y = plotRect.top() + i * plotRect.height() / 6;
        painter.drawLine(plotRect.left(), y, plotRect.right(), y);
    }

    // 绘制边框
    painter.setPen(QColor(120, 135, 155));
    painter.drawRect(plotRect);

    // 设置字体
    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);
    
    // 绘制标题和Y轴标签
    painter.setPen(QColor(188, 198, 210));
    painter.drawText(10, 18, QStringLiteral("实时频谱图"));
    painter.drawText(8, plotRect.top() + 6, QString::number(m_maxDbm, 'f', 0) + QStringLiteral(" dBm"));
    painter.drawText(8, plotRect.bottom(), QString::number(m_minDbm, 'f', 0) + QStringLiteral(" dBm"));

    // 绘制X轴频率标签（如果有帧数据）
    if (m_hasFrame) {
        double startMHz, endMHz, centerFreqMHz;
        if (m_isZoomed) {
            startMHz = m_zoomStartMHz;
            endMHz = m_zoomEndMHz;
            centerFreqMHz = (startMHz + endMHz) / 2.0;
        } else {
            startMHz = m_frame.centerFreqMHz - m_frame.spanMHz / 2.0;
            endMHz = m_frame.centerFreqMHz + m_frame.spanMHz / 2.0;
            centerFreqMHz = m_frame.centerFreqMHz;
        }
        
        painter.drawText(plotRect.left(), height() - 10, QString::number(startMHz, 'f', 1) + QStringLiteral(" MHz"));
        painter.drawText(plotRect.center().x() - 42, height() - 10, QString::number(centerFreqMHz, 'f', 1) + QStringLiteral(" MHz"));
        painter.drawText(plotRect.right() - 78, height() - 10, QString::number(endMHz, 'f', 1) + QStringLiteral(" MHz"));
    }

    painter.restore();  // 恢复绘图状态
}

/**
 * @brief 绘制频谱曲线
 * 
 * 绘制频谱数据的可视化表示：
 * - 频谱曲线：青色线条
 * - 填充区域：半透明青色
 * - 采样点标记：黄色小圆点（展示离散采样特性）
 * 
 * @param painter 绘图对象
 * @param plotRect 绘图区域矩形
 */
void SpectrumPlotWidget::drawSpectrum(QPainter &painter, const QRect &plotRect) const
{
    // 无帧数据时显示提示
    if (!m_hasFrame || m_frame.powerDbm.isEmpty()) {
        painter.save();
        painter.setPen(QColor(145, 155, 170));
        painter.drawText(plotRect, Qt::AlignCenter, QStringLiteral("等待频谱数据"));
        painter.restore();
        return;
    }

    // 创建路径对象
    QPainterPath fillPath;   // 填充区域路径
    QPainterPath linePath;   // 曲线路径

    // 获取第一个点的坐标
    const QPointF firstPoint = pointForBin(plotRect, 0, m_frame.powerDbm.first());
    
    // 初始化路径
    linePath.moveTo(firstPoint);
    fillPath.moveTo(plotRect.left(), plotRect.bottom());  // 从左下角开始
    fillPath.lineTo(firstPoint);

    // 遍历所有频点，构建路径
    for (int i = 1; i < m_frame.powerDbm.size(); ++i) {
        const QPointF p = pointForBin(plotRect, i, m_frame.powerDbm.at(i));
        linePath.lineTo(p);   // 添加到曲线路径
        fillPath.lineTo(p);   // 添加到填充路径
    }
    
    // 完成填充路径（闭合区域）
    fillPath.lineTo(plotRect.right(), plotRect.bottom());
    fillPath.closeSubpath();

    painter.save();
    
    // 设置裁剪区域（避免超出绘图区域）
    painter.setClipRect(plotRect.adjusted(1, 1, -1, -1));
    
    // 绘制填充区域（半透明青色，alpha=42）
    painter.fillPath(fillPath, QColor(0, 190, 210, 42));
    
    // 绘制频谱曲线（青色，线宽1.4）
    painter.setPen(QPen(QColor(30, 235, 220), 1.4));
    painter.drawPath(linePath);

    // 绘制采样点标记（黄色小圆点）
    // 计算采样步长：根据频点数量和绘图宽度，避免点过于密集
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(235, 255, 210));
    const int step = qMax(1, m_frame.powerDbm.size() / qMax(80, plotRect.width() / 4));
    for (int i = 0; i < m_frame.powerDbm.size(); i += step) {
        const QPointF p = pointForBin(plotRect, i, m_frame.powerDbm.at(i));
        painter.drawEllipse(p, 1.8, 1.8);  // 绘制小圆点
    }

    painter.restore();
}

/**
 * @brief 绘制检测结果标记
 * 
 * 绘制峰值位置的标记：
 * - 峰值垂直线：虚线（红色表示检测到信号，黄色表示未检测到）
 * - 峰值点：实心圆（红色或黄色）
 * - 峰值信息文字：显示峰值功率和中心频率
 * 
 * @param painter 绘图对象
 * @param plotRect 绘图区域矩形
 */
void SpectrumPlotWidget::drawDetectionMarker(QPainter &painter, const QRect &plotRect) const
{
    // 无帧数据时不绘制
    if (!m_hasFrame || m_frame.powerDbm.isEmpty()) {
        return;
    }

    // 查找峰值频点和峰值功率（重新计算，与分析器保持一致）
    int peakBin = 0;
    float peak = m_frame.powerDbm.first();
    for (int i = 1; i < m_frame.powerDbm.size(); ++i) {
        if (m_frame.powerDbm.at(i) > peak) {
            peak = m_frame.powerDbm.at(i);
            peakBin = i;
        }
    }

    // 计算峰值点坐标
    const QPointF peakPoint = pointForBin(plotRect, peakBin, peak);

    painter.save();
    
    // 根据检测结果选择颜色：检测到信号用红色，否则用黄色
    const QColor color = m_result.detected ? QColor(255, 90, 60) : QColor(210, 210, 120);
    
    // 绘制峰值垂直线（虚线）
    painter.setPen(QPen(color, 1, Qt::DashLine));
    painter.drawLine(QPointF(peakPoint.x(), plotRect.top()), QPointF(peakPoint.x(), plotRect.bottom()));
    
    // 绘制峰值点（实心圆）
    painter.setBrush(color);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(peakPoint, 4.0, 4.0);

    // 绘制峰值信息文字
    painter.setPen(QColor(230, 235, 240));
    const QString text = QStringLiteral("峰值 %1 dBm  %2 MHz")
            .arg(peak, 0, 'f', 1)
            .arg(m_result.centerFreqMHz, 0, 'f', 2);
    painter.drawText(plotRect.adjusted(8, 6, -8, -6), Qt::AlignTop | Qt::AlignRight, text);

    painter.restore();
}

void SpectrumPlotWidget::drawMouseCursor(QPainter &painter, const QRect &plotRect) const
{
    if (!m_mouseInWidget || !m_hasFrame || m_frame.powerDbm.isEmpty()) {
        return;
    }

    if (m_mousePos.x() < plotRect.left() || m_mousePos.x() > plotRect.right() ||
        m_mousePos.y() < plotRect.top() || m_mousePos.y() > plotRect.bottom()) {
        return;
    }

    const int count = m_frame.powerDbm.size();
    const double xRatio = (double)(m_mousePos.x() - plotRect.left()) / (double)plotRect.width();
    
    int bin;
    double freqMHz;
    if (m_isZoomed) {
        const double totalSpan = m_frame.spanMHz;
        freqMHz = m_zoomStartMHz + xRatio * (m_zoomEndMHz - m_zoomStartMHz);
        const double binPos = (freqMHz - (m_frame.centerFreqMHz - totalSpan / 2.0)) / totalSpan * (count - 1);
        bin = qBound(0, (int)binPos, count - 1);
    } else {
        bin = qBound(0, (int)(xRatio * (count - 1)), count - 1);
        const double startMHz = m_frame.centerFreqMHz - m_frame.spanMHz / 2.0;
        freqMHz = startMHz + xRatio * m_frame.spanMHz;
    }
    
    const float dbm = m_frame.powerDbm.at(bin);

    painter.save();

    painter.setPen(QPen(QColor(180, 180, 180), 1, Qt::DashLine));
    painter.drawLine(QPointF(m_mousePos.x(), plotRect.top()), QPointF(m_mousePos.x(), plotRect.bottom()));

    const QPointF dataPoint = pointForBin(plotRect, bin, dbm);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 200, 100));
    painter.drawEllipse(dataPoint, 3.5, 3.5);

    QString tooltip = QStringLiteral("频率: %1 MHz\n幅度: %2 dBm")
            .arg(freqMHz, 0, 'f', 2)
            .arg(dbm, 0, 'f', 1);

    QFont font = painter.font();
    font.setPointSize(9);
    font.setBold(true);
    painter.setFont(font);

    QFontMetrics fm(font);
    const int textWidth = fm.width(tooltip) + 12;
    const int textHeight = fm.height() * 2 + 8;

    int tooltipX = m_mousePos.x() + 12;
    int tooltipY = m_mousePos.y() - textHeight - 4;

    if (tooltipX + textWidth > width()) {
        tooltipX = m_mousePos.x() - textWidth - 12;
    }
    if (tooltipY < 0) {
        tooltipY = m_mousePos.y() + 12;
    }

    const QRect tooltipRect(tooltipX, tooltipY, textWidth, textHeight);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(25, 30, 40, 220));
    painter.drawRoundedRect(tooltipRect, 4, 4);

    painter.setPen(QColor(200, 210, 230));
    painter.drawText(tooltipRect.adjusted(6, 4, -6, -4), tooltip);

    painter.restore();
}

void SpectrumPlotWidget::drawSelection(QPainter &painter, const QRect &plotRect) const
{
    if (!m_hasFrame || m_frame.powerDbm.isEmpty()) {
        return;
    }

    const int topMargin = 28;
    const int bottomMargin = 36;

    QRect selectionRect;
    double startFreqMHz, endFreqMHz;

    if (m_isSelecting) {
        const int x1 = qMin(m_selectionStart.x(), m_selectionEnd.x());
        const int x2 = qMax(m_selectionStart.x(), m_selectionEnd.x());
        selectionRect = QRect(x1, plotRect.top(), x2 - x1, plotRect.height());
        
        const double xRatio1 = (double)(x1 - plotRect.left()) / (double)plotRect.width();
        const double xRatio2 = (double)(x2 - plotRect.left()) / (double)plotRect.width();
        
        if (m_isZoomed) {
            startFreqMHz = m_zoomStartMHz + xRatio1 * (m_zoomEndMHz - m_zoomStartMHz);
            endFreqMHz = m_zoomStartMHz + xRatio2 * (m_zoomEndMHz - m_zoomStartMHz);
        } else {
            const double totalSpan = m_frame.spanMHz;
            const double globalStart = m_frame.centerFreqMHz - totalSpan / 2.0;
            startFreqMHz = globalStart + xRatio1 * totalSpan;
            endFreqMHz = globalStart + xRatio2 * totalSpan;
        }
    } else if (m_selectedStartMHz != m_selectedEndMHz) {
        double xRatio1, xRatio2;
        if (m_isZoomed) {
            xRatio1 = (m_selectedStartMHz - m_zoomStartMHz) / (m_zoomEndMHz - m_zoomStartMHz);
            xRatio2 = (m_selectedEndMHz - m_zoomStartMHz) / (m_zoomEndMHz - m_zoomStartMHz);
        } else {
            const double totalSpan = m_frame.spanMHz;
            const double globalStart = m_frame.centerFreqMHz - totalSpan / 2.0;
            xRatio1 = (m_selectedStartMHz - globalStart) / totalSpan;
            xRatio2 = (m_selectedEndMHz - globalStart) / totalSpan;
        }
        
        xRatio1 = qBound(0.0, xRatio1, 1.0);
        xRatio2 = qBound(0.0, xRatio2, 1.0);
        
        const int x1 = plotRect.left() + (int)(xRatio1 * plotRect.width());
        const int x2 = plotRect.left() + (int)(xRatio2 * plotRect.width());
        selectionRect = QRect(x1, plotRect.top(), x2 - x1, plotRect.height());
        
        startFreqMHz = m_selectedStartMHz;
        endFreqMHz = m_selectedEndMHz;
    } else {
        return;
    }

    if (selectionRect.width() < 5) {
        return;
    }

    painter.save();

    painter.setPen(QPen(QColor(255, 180, 60), 1.5));
    painter.setBrush(QColor(255, 180, 60, 30));
    painter.drawRect(selectionRect);

    painter.setPen(QColor(255, 180, 60));
    QFont font = painter.font();
    font.setPointSize(9);
    font.setBold(true);
    painter.setFont(font);

    const double spanMHz = qAbs(endFreqMHz - startFreqMHz);
    
    QString label = QStringLiteral("带宽: %1 MHz").arg(spanMHz, 0, 'f', 1);

    QFontMetrics fm(font);
    const int textWidth = fm.width(label) + 12;
    const int textHeight = fm.height() + 8;

    int labelX = selectionRect.center().x() - textWidth / 2;
    int labelY = selectionRect.top() - textHeight - 4;

    if (labelX < plotRect.left()) {
        labelX = plotRect.left();
    }
    if (labelX + textWidth > plotRect.right()) {
        labelX = plotRect.right() - textWidth;
    }
    if (labelY < topMargin) {
        labelY = selectionRect.bottom() + 4;
    }
    if (labelY + textHeight > height() - bottomMargin) {
        labelY = selectionRect.top() - textHeight - 4;
    }

    const QRect labelRect(labelX, labelY, textWidth, textHeight);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(25, 30, 40, 230));
    painter.drawRoundedRect(labelRect, 4, 4);

    painter.setPen(QColor(255, 180, 60));
    painter.drawText(labelRect.adjusted(6, 4, -6, -4), label);

    painter.restore();
}

void SpectrumPlotWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        return;
    }

    const QRect plotRect(62, 28,
                         qMax(1, width() - 62 - 18),
                         qMax(1, height() - 28 - 36));

    if (event->pos().x() < plotRect.left() || event->pos().x() > plotRect.right() ||
        event->pos().y() < plotRect.top() || event->pos().y() > plotRect.bottom()) {
        return;
    }

    m_isSelecting = true;
    m_selectionStart = event->pos();
    m_selectionEnd = event->pos();
    update();
}

void SpectrumPlotWidget::mouseMoveEvent(QMouseEvent *event)
{
    m_mouseInWidget = true;
    m_mousePos = event->pos();
    
    if (m_isSelecting) {
        m_selectionEnd = event->pos();
    }
    
    update();
}

void SpectrumPlotWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton || !m_isSelecting) {
        return;
    }

    m_isSelecting = false;

    const QRect plotRect(62, 28,
                         qMax(1, width() - 62 - 18),
                         qMax(1, height() - 28 - 36));

    const int x1 = qMin(m_selectionStart.x(), m_selectionEnd.x());
    const int x2 = qMax(m_selectionStart.x(), m_selectionEnd.x());
    
    if (x2 - x1 < 5) {
        m_selectedStartMHz = 0.0;
        m_selectedEndMHz = 0.0;
        update();
        return;
    }

    const double xRatio1 = (double)(x1 - plotRect.left()) / (double)plotRect.width();
    const double xRatio2 = (double)(x2 - plotRect.left()) / (double)plotRect.width();

    if (m_isZoomed) {
        m_selectedStartMHz = m_zoomStartMHz + xRatio1 * (m_zoomEndMHz - m_zoomStartMHz);
        m_selectedEndMHz = m_zoomStartMHz + xRatio2 * (m_zoomEndMHz - m_zoomStartMHz);
    } else {
        const double totalSpan = m_frame.spanMHz;
        const double globalStart = m_frame.centerFreqMHz - totalSpan / 2.0;
        m_selectedStartMHz = globalStart + xRatio1 * totalSpan;
        m_selectedEndMHz = globalStart + xRatio2 * totalSpan;
    }

    emit selectionChanged(m_selectedStartMHz, m_selectedEndMHz);
    
    update();
}

void SpectrumPlotWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_mouseInWidget = false;
    update();
}

void SpectrumPlotWidget::wheelEvent(QWheelEvent *event)
{
    if (!m_hasFrame || m_frame.powerDbm.isEmpty()) {
        return;
    }

    const QRect plotRect(62, 28,
                         qMax(1, width() - 62 - 18),
                         qMax(1, height() - 28 - 36));

    if (event->pos().x() < plotRect.left() || event->pos().x() > plotRect.right()) {
        return;
    }

    const double xRatio = (event->pos().x() - plotRect.left()) / (double)plotRect.width();
    const double totalSpan = m_frame.spanMHz;
    
    double currentStartMHz, currentEndMHz;
    if (m_isZoomed) {
        currentStartMHz = m_zoomStartMHz;
        currentEndMHz = m_zoomEndMHz;
    } else {
        currentStartMHz = m_frame.centerFreqMHz - totalSpan / 2.0;
        currentEndMHz = m_frame.centerFreqMHz + totalSpan / 2.0;
    }

    const double mouseFreqMHz = currentStartMHz + xRatio * (currentEndMHz - currentStartMHz);
    
    const double zoomFactor = event->angleDelta().y() > 0 ? 0.85 : 1.15;
    const double minSpan = 1.0;
    const double maxSpan = totalSpan;

    double newSpan = (currentEndMHz - currentStartMHz) * zoomFactor;
    newSpan = qBound(minSpan, newSpan, maxSpan);

    const double halfSpan = newSpan / 2.0;
    double newStart = mouseFreqMHz - halfSpan * xRatio * 2.0;
    double newEnd = mouseFreqMHz + halfSpan * (1.0 - xRatio) * 2.0;

    const double globalStart = m_frame.centerFreqMHz - totalSpan / 2.0;
    const double globalEnd = m_frame.centerFreqMHz + totalSpan / 2.0;

    if (newStart < globalStart) {
        newStart = globalStart;
        newEnd = newStart + newSpan;
    }
    if (newEnd > globalEnd) {
        newEnd = globalEnd;
        newStart = newEnd - newSpan;
    }

    m_zoomStartMHz = newStart;
    m_zoomEndMHz = newEnd;
    m_isZoomed = (newSpan < maxSpan * 0.95);

    emit zoomChanged(m_zoomStartMHz, m_zoomEndMHz);

    update();
    event->accept();
}