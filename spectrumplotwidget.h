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

protected:
    /**
     * @brief 绘制事件处理
     * 
     * 重写QWidget的paintEvent，执行频谱图绘制。
     * 
     * @param event 绘制事件
     */
    void paintEvent(QPaintEvent *event);

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

private:
    SpectrumFrame m_frame;       ///< 当前频谱帧数据
    DetectionResult m_result;    ///< 当前检测结果
    bool m_hasFrame;             ///< 是否有有效的帧数据
    float m_minDbm;              ///< 功率显示范围最小值（dBm）
    float m_maxDbm;              ///< 功率显示范围最大值（dBm）
};

#endif // SPECTRUMPLOTWIDGET_H