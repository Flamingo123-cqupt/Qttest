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
{
    setMinimumHeight(230);       // 设置最小高度
    setAutoFillBackground(false); // 禁用自动背景填充（手动绘制）
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
}

/**
 * @brief 将频点数据转换为绘图坐标
 * 
 * 根据频点索引和功率值，计算在绘图区域中的坐标位置：
 * - X坐标：频点索引线性映射到绘图区域宽度
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
    
    // 计算X轴比例（频点索引 / 总频点数）
    const double xRatio = (count > 1) ? (double)bin / (double)(count - 1) : 0.0;
    
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
        const double startMHz = m_frame.centerFreqMHz - m_frame.spanMHz / 2.0;
        const double endMHz = m_frame.centerFreqMHz + m_frame.spanMHz / 2.0;
        
        painter.drawText(plotRect.left(), height() - 10, QString::number(startMHz, 'f', 1) + QStringLiteral(" MHz"));
        painter.drawText(plotRect.center().x() - 42, height() - 10, QString::number(m_frame.centerFreqMHz, 'f', 1) + QStringLiteral(" MHz"));
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