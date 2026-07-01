/**
 * @file waterfallwidget.h
 * @brief 频谱瀑布图绘制组件
 * 
 * 本组件负责绘制频谱瀑布图，展示频谱数据随时间的变化趋势。
 * 瀑布图的特点是：
 * - X轴：频率
 * - Y轴：时间（从上到下表示时间从新到旧）
 * - 颜色：功率强度（从黑蓝到青绿到黄红）
 * 
 * 主要功能包括：
 * - 维护瀑布图图像缓冲区
 * - 逐帧更新图像（新帧写入顶部，旧帧下移）
 * - 根据功率值计算颜色
 * - 支持窗口大小调整
 */

#ifndef WATERFALLWIDGET_H
#define WATERFALLWIDGET_H

#include "spectrumsimulator.h"
#include <QWidget>
#include <QImage>

/**
 * @brief 频谱瀑布图绘制组件
 * 
 * 继承自QWidget，使用QImage作为图像缓冲区，逐帧更新瀑布图。
 * 瀑布图展示频谱数据随时间的变化，便于观察信号的时间特性。
 */
class WaterfallWidget : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief 构造函数
     * 
     * @param parent 父对象指针
     */
    explicit WaterfallWidget(QWidget *parent = 0);

    /**
     * @brief 设置功率显示范围
     * 
     * @param minDbm 最小功率值（dBm）
     * @param maxDbm 最大功率值（dBm）
     */
    void setPowerRange(float minDbm, float maxDbm);

    /**
     * @brief 添加一帧频谱数据到瀑布图
     * 
     * 将新帧数据写入瀑布图顶部，旧帧整体下移一行。
     * 
     * @param frame 频谱帧数据
     */
    void addFrame(const SpectrumFrame &frame);

    /**
     * @brief 清空瀑布图
     * 
     * 将图像缓冲区填充为黑色，清除所有历史数据。
     */
    void clear();

    /**
     * @brief 设置框选区域
     * 
     * 设置频谱图中框选的频率范围，瀑布图将在对应位置显示框选标记。
     * 
     * @param startFreqMHz 起始频率（MHz）
     * @param endFreqMHz 结束频率（MHz）
     */
    void setSelection(double startFreqMHz, double endFreqMHz);
    
    /**
     * @brief 设置缩放范围
     * 
     * 设置频率显示范围，与频谱图的缩放保持同步。
     * 
     * @param startFreqMHz 起始频率（MHz）
     * @param endFreqMHz 结束频率（MHz）
     */
    void setZoomRange(double startFreqMHz, double endFreqMHz);

protected:
    /**
     * @brief 绘制事件处理
     * 
     * 重写QWidget的paintEvent，绘制瀑布图图像。
     * 
     * @param event 绘制事件
     */
    void paintEvent(QPaintEvent *event);

    /**
     * @brief 窗口大小变化事件处理
     * 
     * 重写QWidget的resizeEvent，重新调整图像缓冲区大小。
     * 
     * @param event 大小变化事件
     */
    void resizeEvent(QResizeEvent *event);

private:
    /**
     * @brief 根据功率值计算颜色
     * 
     * 将功率值（dBm）映射到颜色：
     * - 低能量：黑蓝色
     * - 中等能量：青绿色
     * - 高能量：黄红色
     * 
     * @param dbm 功率值（dBm）
     * @return QColor 对应的颜色
     */
    QColor colorForPower(float dbm) const;

    /**
     * @brief 确保图像缓冲区有效
     * 
     * 检查图像缓冲区是否存在且大小正确，若不存在则创建。
     * 在窗口大小变化时调用，确保图像与窗口尺寸匹配。
     */
    void ensureImage();

private:
    QImage m_waterfall;       ///< 瀑布图图像缓冲区
    SpectrumFrame m_lastFrame; ///< 最后一帧数据（用于绘制坐标轴标签）
    bool m_hasFrame;          ///< 是否有有效的帧数据
    float m_minDbm;           ///< 功率显示范围最小值（dBm）
    float m_maxDbm;           ///< 功率显示范围最大值（dBm）
    double m_selectionStartMHz; ///< 框选起始频率（MHz）
    double m_selectionEndMHz;   ///< 框选结束频率（MHz）
    double m_zoomStartMHz;      ///< 缩放起始频率（MHz）
    double m_zoomEndMHz;        ///< 缩放结束频率（MHz）
    bool m_isZoomed;            ///< 是否处于缩放状态
};

#endif // WATERFALLWIDGET_H