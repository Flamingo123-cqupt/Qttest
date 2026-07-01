/**
 * @file waterfallwidget.cpp
 * @brief 频谱瀑布图绘制组件实现
 * 
 * 实现瀑布图的绘制逻辑，包括图像缓冲区管理、帧更新、颜色映射等。
 * 使用QImage作为图像缓冲区，逐帧更新瀑布图。
 */

#include "waterfallwidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QtMath>

/**
 * @brief 构造函数
 * 
 * 初始化功率显示范围（-100dBm到-35dBm），设置最小尺寸和背景填充方式。
 * 
 * @param parent 父对象指针
 */
WaterfallWidget::WaterfallWidget(QWidget *parent)
    : QWidget(parent)
    , m_hasFrame(false)          // 默认无帧数据
    , m_minDbm(-100.0f)          // 功率范围最小值：-100dBm
    , m_maxDbm(-35.0f)           // 功率范围最大值：-35dBm
    , m_selectionStartMHz(0.0)   // 默认框选起始频率
    , m_selectionEndMHz(0.0)     // 默认框选结束频率
    , m_zoomStartMHz(0.0)        // 默认缩放起始频率
    , m_zoomEndMHz(0.0)          // 默认缩放结束频率
    , m_isZoomed(false)          // 默认未缩放
{
    setMinimumSize(640, 420);    // 设置最小尺寸
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
void WaterfallWidget::setPowerRange(float minDbm, float maxDbm)
{
    m_minDbm = minDbm;
    m_maxDbm = qMax(minDbm + 1.0f, maxDbm);  // 确保范围有效
    update();  // 触发重绘
}

/**
 * @brief 添加一帧频谱数据到瀑布图
 * 
 * 将新帧数据写入瀑布图顶部，旧帧整体下移一行。
 * 具体步骤：
 * 1. 确保图像缓冲区存在
 * 2. 将旧图像整体向下移动一行（丢弃最底部一行）
 * 3. 将新帧数据转换为颜色，写入图像顶部一行
 * 4. 更新最后一帧数据记录，触发重绘
 * 
 * @param frame 频谱帧数据
 */
void WaterfallWidget::addFrame(const SpectrumFrame &frame)
{
    // 帧数据为空时不处理
    if (frame.powerDbm.isEmpty()) {
        return;
    }

    // 确保图像缓冲区存在
    ensureImage();
    if (m_waterfall.isNull()) {
        return;
    }

    // 创建图像绘图对象
    QPainter imagePainter(&m_waterfall);
    
    // 将旧图像向下移动一行：从(0,0)复制到(0,1)，高度减少1
    imagePainter.drawImage(QPoint(0, 1), m_waterfall, QRect(0, 0, m_waterfall.width(), m_waterfall.height() - 1));

    // 将新帧数据写入图像顶部一行
    for (int x = 0; x < m_waterfall.width(); ++x) {
        // 将图像X坐标映射到频谱帧的频点索引
        const int rawBin = (int)((qint64)x * frame.powerDbm.size() / qMax(1, m_waterfall.width()));
        const int bin = qBound(0, rawBin, frame.powerDbm.size() - 1);  // 边界检查
        
        // 根据功率值计算颜色并设置像素
        m_waterfall.setPixelColor(x, 0, colorForPower(frame.powerDbm.at(bin)));
    }

    // 更新最后一帧记录（用于绘制坐标轴标签）
    m_lastFrame = frame;
    m_hasFrame = true;
    update();  // 触发重绘
}

/**
 * @brief 清空瀑布图
 * 
 * 将图像缓冲区填充为深黑色，清除所有历史数据。
 */
void WaterfallWidget::clear()
{
    if (!m_waterfall.isNull()) {
        m_waterfall.fill(QColor(8, 12, 18));  // 填充深黑色背景
    }
    m_hasFrame = false;
    m_selectionStartMHz = 0.0;
    m_selectionEndMHz = 0.0;
    update();  // 触发重绘
}

/**
 * @brief 设置框选区域
 * 
 * 设置频谱图中框选的频率范围，瀑布图将在对应位置显示框选标记。
 * 
 * @param startFreqMHz 起始频率（MHz）
 * @param endFreqMHz 结束频率（MHz）
 */
void WaterfallWidget::setSelection(double startFreqMHz, double endFreqMHz)
{
    m_selectionStartMHz = startFreqMHz;
    m_selectionEndMHz = endFreqMHz;
    update();  // 触发重绘
}

/**
 * @brief 设置缩放范围
 * 
 * 设置频率显示范围，与频谱图的缩放保持同步。
 * 
 * @param startFreqMHz 起始频率（MHz）
 * @param endFreqMHz 结束频率（MHz）
 */
void WaterfallWidget::setZoomRange(double startFreqMHz, double endFreqMHz)
{
    m_zoomStartMHz = startFreqMHz;
    m_zoomEndMHz = endFreqMHz;
    m_isZoomed = (startFreqMHz != endFreqMHz && qAbs(endFreqMHz - startFreqMHz) < m_lastFrame.spanMHz * 0.95);
    update();  // 触发重绘
}

/**
 * @brief 绘制事件处理
 * 
 * 重写QWidget的paintEvent，执行瀑布图绘制流程：
 * 1. 确保图像缓冲区存在
 * 2. 创建绘图对象，填充背景
 * 3. 绘制瀑布图图像
 * 4. 绘制边框和坐标轴标签
 * 
 * @param event 绘制事件
 */
void WaterfallWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);  // 忽略事件参数
    
    // 确保图像缓冲区存在
    ensureImage();

    // 创建绘图对象
    QPainter painter(this);
    painter.fillRect(rect(), QColor(18, 22, 28));  // 填充背景

    // 定义边距
    const int leftMargin = 58;    // 左边距（时间标签）
    const int rightMargin = 16;   // 右边距
    const int topMargin = 18;     // 上边距
    const int bottomMargin = 34;  // 下边距（频率标签）
    
    // 计算绘图区域矩形
    const QRect plotRect(leftMargin, topMargin,
                         qMax(1, width() - leftMargin - rightMargin),
                         qMax(1, height() - topMargin - bottomMargin));

    // 获取全局频率范围
    const double globalStart = m_lastFrame.centerFreqMHz - m_lastFrame.spanMHz / 2.0;
    const double globalEnd = m_lastFrame.centerFreqMHz + m_lastFrame.spanMHz / 2.0;
    
    // 根据缩放状态确定显示范围
    double displayStart, displayEnd;
    if (m_isZoomed) {
        displayStart = m_zoomStartMHz;
        displayEnd = m_zoomEndMHz;
    } else {
        displayStart = globalStart;
        displayEnd = globalEnd;
    }

    // 绘制瀑布图图像（如果图像有效）
    if (!m_waterfall.isNull() && m_hasFrame) {
        const double totalSpan = globalEnd - globalStart;
        const double zoomStartRatio = (displayStart - globalStart) / totalSpan;
        const double zoomEndRatio = (displayEnd - globalStart) / totalSpan;
        
        const int sourceX = (int)(zoomStartRatio * m_waterfall.width());
        const int sourceWidth = (int)((zoomEndRatio - zoomStartRatio) * m_waterfall.width());
        
        if (sourceWidth > 0) {
            painter.drawImage(plotRect, m_waterfall, QRect(sourceX, 0, sourceWidth, m_waterfall.height()));
        }
    }

    // 绘制边框
    painter.setPen(QColor(95, 110, 130));
    painter.drawRect(plotRect);

    // 设置字体
    painter.setPen(QColor(180, 190, 205));
    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);
    
    // 绘制时间标签（Y轴方向：上为最新，下为历史）
    painter.drawText(8, topMargin + 12, QStringLiteral("最新"));
    painter.drawText(8, plotRect.bottom(), QStringLiteral("历史"));

    // 绘制频率标签（X轴）
    if (m_hasFrame) {
        const double centerFreqMHz = (displayStart + displayEnd) / 2.0;
        
        painter.drawText(plotRect.left(), height() - 10, QString::number(displayStart, 'f', 1) + QStringLiteral(" MHz"));
        painter.drawText(plotRect.center().x() - 42, height() - 10, QString::number(centerFreqMHz, 'f', 1) + QStringLiteral(" MHz"));
        painter.drawText(plotRect.right() - 78, height() - 10, QString::number(displayEnd, 'f', 1) + QStringLiteral(" MHz"));
    }

    // 绘制框选区域
    if (m_hasFrame && m_selectionStartMHz != m_selectionEndMHz) {
        const double totalSpan = displayEnd - displayStart;
        
        const double startRatio = (m_selectionStartMHz - displayStart) / totalSpan;
        const double endRatio = (m_selectionEndMHz - displayStart) / totalSpan;
        
        const int x1 = plotRect.left() + (int)(startRatio * plotRect.width());
        const int x2 = plotRect.left() + (int)(endRatio * plotRect.width());
        
        if (qAbs(x2 - x1) >= 5) {
            painter.save();
            
            painter.setPen(QPen(QColor(255, 180, 60), 1.5));
            painter.setBrush(QColor(255, 180, 60, 20));
            painter.drawRect(QRect(x1, plotRect.top(), x2 - x1, plotRect.height()));
            
            painter.restore();
        }
    }
}

/**
 * @brief 窗口大小变化事件处理
 * 
 * 重写QWidget的resizeEvent，在窗口大小变化时：
 * 1. 清空现有图像缓冲区
 * 2. 重新创建与窗口尺寸匹配的图像缓冲区
 * 
 * @param event 大小变化事件
 */
void WaterfallWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);  // 调用父类处理
    
    // 清空现有图像，触发重新创建
    m_waterfall = QImage();
    ensureImage();  // 重新创建图像缓冲区
}

/**
 * @brief 根据功率值计算颜色
 * 
 * 将功率值（dBm）线性映射到0-1范围，然后分段映射到颜色：
 * - t < 0.25：黑蓝色（低能量）
 * - 0.25 <= t < 0.55：青绿色（中等能量）
 * - 0.55 <= t < 0.80：黄绿色（较高能量）
 * - t >= 0.80：黄红色（高能量）
 * 
 * @param dbm 功率值（dBm）
 * @return QColor 对应的颜色
 */
QColor WaterfallWidget::colorForPower(float dbm) const
{
    // 将功率值线性映射到0-1范围
    const float t = qBound(0.0f, (dbm - m_minDbm) / (m_maxDbm - m_minDbm), 1.0f);

    // 分段颜色映射
    if (t < 0.25f) {
        // 黑蓝色：从(5,10,35)过渡到(5,60,125)
        const float k = t / 0.25f;
        return QColor(5, 10 + (int)(50 * k), 35 + (int)(90 * k));
    }
    if (t < 0.55f) {
        // 青绿色：从(0,60,130)过渡到(0,210,220)
        const float k = (t - 0.25f) / 0.30f;
        return QColor(0, 60 + (int)(150 * k), 130 + (int)(90 * k));
    }
    if (t < 0.80f) {
        // 黄绿色：从(0,210,80)过渡到(240,245,40)
        const float k = (t - 0.55f) / 0.25f;
        return QColor((int)(240 * k), 210 + (int)(35 * k), 80 - (int)(40 * k));
    }
    // 黄红色：从(240,170,35)过渡到(255,30,10)
    const float k = (t - 0.80f) / 0.20f;
    return QColor(240 + (int)(15 * k), 170 - (int)(140 * k), 35 - (int)(25 * k));
}

/**
 * @brief 确保图像缓冲区有效
 * 
 * 检查图像缓冲区是否存在且大小正确：
 * - 如果图像不存在或尺寸不匹配，创建新图像
 * - 新图像填充为深黑色背景
 * - 如果旧图像存在，尝试将其内容复制到新图像（保持历史数据）
 */
void WaterfallWidget::ensureImage()
{
    // 定义边距（与paintEvent保持一致）
    const int leftMargin = 58;
    const int rightMargin = 16;
    const int topMargin = 18;
    const int bottomMargin = 34;
    
    // 计算图像尺寸（绘图区域大小）
    const int imageWidth = qMax(1, width() - leftMargin - rightMargin);
    const int imageHeight = qMax(1, height() - topMargin - bottomMargin);

    // 图像尺寸已经正确，无需处理
    if (m_waterfall.size() == QSize(imageWidth, imageHeight)) {
        return;
    }

    // 创建新图像（RGB32格式，支持alpha通道）
    QImage newImage(imageWidth, imageHeight, QImage::Format_RGB32);
    newImage.fill(QColor(8, 12, 18));  // 填充深黑色背景
    
    // 如果旧图像存在，尝试将其内容复制到新图像
    if (!m_waterfall.isNull()) {
        QPainter painter(&newImage);
        painter.drawImage(QRect(0, 0, imageWidth, imageHeight), m_waterfall);
    }
    
    // 使用新图像替换旧图像
    m_waterfall = newImage;
}