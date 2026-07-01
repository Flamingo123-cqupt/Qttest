/**
 * @file spectrumanalyzer.h
 * @brief 频谱分析器模块
 * 
 * 本模块负责对频谱数据进行分析，识别无人机信号特征。
 * 核心功能包括：
 * - 峰值检测与中心频率计算
 * - 带宽估计与活跃频点计数
 * - 跳频特征检测（通过历史峰值轨迹分析）
 * - 信号分类识别（遥控、图传、跳频、未知）
 */
// 每一次频谱扫描，都是与未知信号的对话
// 在噪声的海洋中寻找那一丝规律的涟漪
// 当峰值浮现，便是真相即将揭晓的时刻

// 蔡健雅 - Letting Go
// 这是一封离别信
// 写下我该离开的原因
// 我在你生命中扮演的角色太模糊了
// 你对我常忽冷忽热
// 我到底是情人还是朋友
// 爱你是否不该太认真
// That's why I'm letting go
// 我终于舍得为你放开手
// 因为爱你爱到我心痛
// 但你却不懂
// I'm letting go
// 你对一切的软弱与怠惰
// 让人怀疑你是否爱过我
// 真的爱过我
// 为你再也找不到借口
// That's when we should let it go
// 你是呼吸的空气
// 脱离不了的地心引力
// 在我生命中曾经是我 存在的原因
// 或许就像他们说
// 爱情只会让人变愚蠢
// 自作多情爱得太天真
// That's why I'm letting go
// 我终于舍得为你放开手
// 因为爱你爱到我心痛
// 但你却不懂
// I'm letting go
// 你对一切的软弱与怠惰
// 让人怀疑你是否爱过我
// 真的爱过我
// 为你再也找不到借口
// That's when we should let it go
// 在夜深人静里想着
// 心不安却越沸腾
// 我无助 好想哭 我找不到退路
// 在夜深人静里写着
// 心慢慢就越变冷
// 我不恨 也不哭 我的眼泪 早已哭干了
// Cause I'm letting go
// 我终于舍得为你放开手
// 因为爱你爱到我心痛
// 但你却不懂
// I'm letting go
// 你对一切的软弱与怠惰
// 让人怀疑你是否爱过我
// 真的爱过我
// 为你再也找不到借口
// That's when we should let it go
// We should let it go
// We should let it go

#ifndef SPECTRUMANALYZER_H
#define SPECTRUMANALYZER_H

#include "spectrumsimulator.h"
#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 检测结果数据结构
 * 
 * 存储单次频谱分析的结果，包含检测状态、信号类型、置信度等信息。
 */
struct DetectionResult
{
    bool detected;           ///< 是否检测到信号（功率超过门限）
    QString label;           ///< 信号分类标签（如"疑似无人机遥控信号"）
    QString detail;          ///< 详细描述（信号特征说明）
    double confidence;       ///< 置信度（0.0到1.0之间）
    double peakDbm;          ///< 峰值功率（dBm）
    double centerFreqMHz;    ///< 中心频率（MHz）
    double bandwidthMHz;     ///< 占用带宽（MHz）
    int activeBins;          ///< 活跃频点数量（超过门限的频点）
    int hopCount;            ///< 最近一段时间内的跳频次数
};

/**
 * @brief 频谱分析器类
 * 
 * 负责对频谱帧数据进行分析处理，识别无人机通信信号。
 * 采用规则引擎进行信号分类，可扩展为机器学习模型。
 */
class SpectrumAnalyzer : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 构造函数
     * 
     * @param parent 父对象指针
     */
    explicit SpectrumAnalyzer(QObject *parent = 0);

    /**
     * @brief 设置检测门限
     * 
     * 低于此门限的信号被认为是噪声。
     * 
     * @param thresholdDbm 门限功率值（dBm）
     */
    void setThresholdDbm(double thresholdDbm);

    /**
     * @brief 获取当前检测门限
     * 
     * @return 门限功率值（dBm）
     */
    double thresholdDbm() const;

    /**
     * @brief 分析频谱帧数据
     * 
     * 核心方法，执行完整的频谱分析流程：
     * 1. 统计活跃频点
     * 2. 检测峰值并计算中心频率
     * 3. 估计信号带宽
     * 4. 分析跳频特征
     * 5. 信号分类识别
     * 
     * @param frame 频谱帧数据
     * @return DetectionResult 检测结果
     */
    DetectionResult analyze(const SpectrumFrame &frame);

    /**
     * @brief 重置分析器状态
     * 
     * 清除跳频历史记录，重新开始分析。
     */
    void reset();

private:
    /**
     * @brief 统计活跃频点数量
     * 
     * 计算功率超过检测门限的频点数量，用于判断信号带宽特征。
     * 
     * @param bins 频谱功率数据
     * @return 活跃频点数量
     */
    int countActiveBins(const QVector<float> &bins) const;

    /**
     * @brief 查找峰值频点
     * 
     * 遍历频谱数据，找到功率最大的频点。
     * 
     * @param bins 频谱功率数据
     * @param peakValue 输出参数：峰值功率值
     * @return 峰值频点索引（-1表示数据为空）
     */
    int findPeakBin(const QVector<float> &bins, float *peakValue) const;

    /**
     * @brief 估计信号带宽
     * 
     * 通过找到功率超过门限的第一个和最后一个频点，计算信号占用带宽。
     * 
     * @param frame 频谱帧数据
     * @param firstBin 输出参数：第一个活跃频点索引
     * @param lastBin 输出参数：最后一个活跃频点索引
     * @return 信号带宽（MHz）
     */
    double estimateBandwidthMHz(const SpectrumFrame &frame, int *firstBin, int *lastBin) const;

    /**
     * @brief 更新跳频历史记录
     * 
     * 将当前帧的峰值频点添加到历史记录中，用于检测跳频特征。
     * 
     * @param peakBin 当前帧的峰值频点索引
     */
    void updateHopHistory(int peakBin);

    /**
     * @brief 计算最近跳频次数
     * 
     * 分析历史峰值轨迹，统计峰值在不同频点间切换的次数。
     * 当相邻峰值间距超过阈值（18个频点）时，认为发生了一次跳频。
     * 
     * @return 最近一段时间内的跳频次数
     */
    int recentHopCount() const;

private:
    double m_thresholdDbm;       ///< 检测门限（dBm），默认-68dBm
    QVector<int> m_recentPeakBins; ///< 最近峰值频点历史记录（最多24帧）
};

#endif // SPECTRUMANALYZER_H