/**
 * @file spectrumplotwidget.h
 * @brief 实时频谱图绘制组件
 * 
 * 本组件负责绘制实时频谱图，展示当前帧的频谱数据。
 * 主要功能包括：
 * - 绘制网格和坐标轴标签
 * - 绘制频谱曲线和填充区域
 * - 绘制峰值标记和检测结果指示
 * - 支持功率范围配置
 */

#ifndef SPECTRUMPLOTWIDGET_H
#define SPECTRUMPLOTWIDGET_H

#include "spectrumsimulator.h"
#include "spectrumanalyzer.h"
#include <QWidget>
#include <QMouseEvent>
#include <QPoint>
#include <QWheelEvent>

/**
 * @brief 实时频谱图绘制组件
 * 
 * 继承自QWidget，使用QPainter绘制频谱图。
 * 显示当前帧的功率谱密度曲线，支持检测结果标记。
 */
class SpectrumPlotWidget : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief 构造函数
     * 
     * @param parent 父对象指针
     */
    explicit SpectrumPlotWidget(QWidget *parent = 0);

    /**
     * @brief 设置功率显示范围
     * 
     * @param minDbm 最小功率值（dBm）
     * @param maxDbm 最大功率值（dBm）
     */
    void setPowerRange(float minDbm, float maxDbm);

    /**
     * @brief 设置当前帧数据
     * 
     * 更新频谱帧和检测结果，触发重绘。
     * 
     * @param frame 频谱帧数据
     * @param result 检测结果
     */
    void setFrame(const SpectrumFrame &frame, const DetectionResult &result);

    /**
     * @brief 清空显示
     * 
     * 清除当前帧数据，显示"等待频谱数据"提示。
     */
    void clear();

signals:
    /**
     * @brief 框选完成信号
     * 
     * 当用户左键框选频谱区域完成后发出此信号。
     * 
     * @param startFreqMHz 框选起始频率（MHz）
     * @param endFreqMHz 框选结束频率（MHz）
     */
    void selectionChanged(double startFreqMHz, double endFreqMHz);
    
    /**
     * @brief 缩放变化信号
     * 
     * 当用户通过滚轮缩放频谱图时发出此信号，用于同步瀑布图的缩放状态。
     * 
     * @param startFreqMHz 缩放后的起始频率（MHz）
     * @param endFreqMHz 缩放后的结束频率（MHz）
     */
    void zoomChanged(double startFreqMHz, double endFreqMHz);

protected:
    /**
     * @brief 绘制事件处理
     * 
     * 重写QWidget的paintEvent，执行频谱图绘制。
     * 
     * @param event 绘制事件
     */
    void paintEvent(QPaintEvent *event);

    /**
     * @brief 鼠标按下事件处理
     * 
     * 重写QWidget的mousePressEvent，记录框选起始点。
     * 
     * @param event 鼠标按下事件
     */
    void mousePressEvent(QMouseEvent *event);

    /**
     * @brief 鼠标移动事件处理
     * 
     * 重写QWidget的mouseMoveEvent，实现鼠标悬浮显示幅度和频率标签，以及框选矩形绘制。
     * 
     * @param event 鼠标移动事件
     */
    void mouseMoveEvent(QMouseEvent *event);

    /**
     * @brief 鼠标释放事件处理
     * 
     * 重写QWidget的mouseReleaseEvent，完成框选操作并发出信号。
     * 
     * @param event 鼠标释放事件
     */
    void mouseReleaseEvent(QMouseEvent *event);

    /**
     * @brief 鼠标离开事件处理
     * 
     * 重写QWidget的leaveEvent，隐藏悬浮标签。
     * 
     * @param event 鼠标离开事件
     */
    void leaveEvent(QEvent *event);

    /**
     * @brief 滚轮事件处理
     * 
     * 重写QWidget的wheelEvent，实现横坐标缩放功能。
     * 滚轮向前滚动放大，向后滚动缩小，以鼠标位置为中心进行缩放。
     * 
     * @param event 滚轮事件
     */
    void wheelEvent(QWheelEvent *event);

private:
    /**
     * @brief 将频点数据转换为绘图坐标
     * 
     * 根据频谱帧数据和绘图区域，计算指定频点的绘制坐标。
     * 
     * @param plotRect 绘图区域矩形
     * @param bin 频点索引
     * @param dbm 功率值（dBm）
     * @return QPointF 绘图坐标
     */
    QPointF pointForBin(const QRect &plotRect, int bin, float dbm) const;

    /**
     * @brief 绘制网格和坐标轴
     * 
     * 绘制背景网格线、边框和坐标轴标签。
     * 
     * @param painter 绘图对象
     * @param plotRect 绘图区域矩形
     */
    void drawGrid(QPainter &painter, const QRect &plotRect) const;

    /**
     * @brief 绘制频谱曲线
     * 
     * 绘制频谱曲线、填充区域和采样点标记。
     * 
     * @param painter 绘图对象
     * @param plotRect 绘图区域矩形
     */
    void drawSpectrum(QPainter &painter, const QRect &plotRect) const;

    /**
     * @brief 绘制检测结果标记
     * 
     * 绘制峰值标记线、峰值点和峰值信息文字。
     * 
     * @param painter 绘图对象
     * @param plotRect 绘图区域矩形
     */
    void drawDetectionMarker(QPainter &painter, const QRect &plotRect) const;

    /**
     * @brief 绘制鼠标悬浮光标和标签
     * 
     * 当鼠标在控件内时，绘制垂直光标线和显示幅度、频率信息的悬浮标签。
     * 
     * @param painter 绘图对象
     * @param plotRect 绘图区域矩形
     */
    void drawMouseCursor(QPainter &painter, const QRect &plotRect) const;

    /**
     * @brief 绘制框选区域
     * 
     * 绘制当前正在框选或已完成框选的区域矩形，以及带宽信息标签。
     * 
     * @param painter 绘图对象
     * @param plotRect 绘图区域矩形
     */
    void drawSelection(QPainter &painter, const QRect &plotRect) const;

private:
    SpectrumFrame m_frame;       ///< 当前频谱帧数据
    DetectionResult m_result;    ///< 当前检测结果
    bool m_hasFrame;             ///< 是否有有效的帧数据
    float m_minDbm;              ///< 功率显示范围最小值（dBm）
    float m_maxDbm;              ///< 功率显示范围最大值（dBm）
    QPoint m_mousePos;           ///< 当前鼠标位置
    bool m_mouseInWidget;        ///< 鼠标是否在控件内
    double m_zoomStartMHz;       ///< 缩放后的起始频率（MHz）
    double m_zoomEndMHz;         ///< 缩放后的结束频率（MHz）
    bool m_isZoomed;             ///< 是否处于缩放状态
    bool m_isSelecting;          ///< 是否正在框选
    QPoint m_selectionStart;     ///< 框选起始点
    QPoint m_selectionEnd;       ///< 框选结束点
    double m_selectedStartMHz;   ///< 当前选中的起始频率（MHz）
    double m_selectedEndMHz;     ///< 当前选中的结束频率（MHz）
};

#endif // SPECTRUMPLOTWIDGET_H