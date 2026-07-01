/**
 * @file spectrumanalyzer.cpp
 * @brief 频谱分析器实现
 * 
 * 实现频谱数据的分析处理逻辑，包括峰值检测、带宽估计、跳频分析和信号分类。
 * 采用基于规则的信号识别方法，可扩展为机器学习模型。
 */

#include "spectrumanalyzer.h"
#include <QtMath>

/**
 * @brief 构造函数
 * 
 * 初始化检测门限为-68dBm，这是一个典型的无人机信号检测门限值。
 * 
 * @param parent 父对象指针
 */
SpectrumAnalyzer::SpectrumAnalyzer(QObject *parent)
    : QObject(parent)
    , m_thresholdDbm(-68.0)   // 默认检测门限：-68dBm
{
}

/**
 * @brief 设置检测门限
 * 
 * 更新检测门限值，低于此门限的信号被认为是噪声。
 * 
 * @param thresholdDbm 门限功率值（dBm）
 */
void SpectrumAnalyzer::setThresholdDbm(double thresholdDbm)
{
    m_thresholdDbm = thresholdDbm;
}

/**
 * @brief 获取当前检测门限
 * 
 * @return 门限功率值（dBm）
 */
double SpectrumAnalyzer::thresholdDbm() const
{
    return m_thresholdDbm;
}

/**
 * @brief 分析频谱帧数据
 * 
 * 核心方法，执行完整的频谱分析流程：
 * 1. 初始化检测结果（默认未检测到信号）
 * 2. 统计活跃频点数量
 * 3. 检测峰值并计算中心频率
 * 4. 估计信号带宽
 * 5. 更新跳频历史并计算跳频次数
 * 6. 判断是否超过门限，若超过则进行信号分类
 * 7. 根据信号强度调整置信度
 * 
 * @param frame 频谱帧数据
 * @return DetectionResult 检测结果
 */
DetectionResult SpectrumAnalyzer::analyze(const SpectrumFrame &frame)
{
    // 1. 初始化检测结果（默认状态：未检测到信号）
    DetectionResult result;
    result.detected = false;
    result.label = QStringLiteral("未发现无人机信号");
    result.detail = QStringLiteral("频谱能量低于检测门限");
    result.confidence = 0.0;
    result.peakDbm = -120.0;           // 默认峰值：-120dBm（极低）
    result.centerFreqMHz = frame.centerFreqMHz;  // 默认中心频率
    result.bandwidthMHz = 0.0;
    result.activeBins = countActiveBins(frame.powerDbm);  // 统计活跃频点
    result.hopCount = 0;

    // 2. 检测峰值功率和峰值频点
    float peak = -120.0f;
    const int peakBin = findPeakBin(frame.powerDbm, &peak);
    result.peakDbm = peak;
    
    // 3. 计算中心频率（根据峰值频点位置）
    if (peakBin >= 0) {
        // 将频点索引转换为实际频率：中心频率 - 半带宽 + 比例偏移
        result.centerFreqMHz = frame.centerFreqMHz - frame.spanMHz / 2.0
                + (double)peakBin * frame.spanMHz / qMax(1, frame.powerDbm.size() - 1);
    }

    // 4. 估计信号带宽
    int firstBin = -1;
    int lastBin = -1;
    result.bandwidthMHz = estimateBandwidthMHz(frame, &firstBin, &lastBin);

    // 5. 更新跳频历史并计算跳频次数
    updateHopHistory(peakBin);
    result.hopCount = recentHopCount();

    // 6. 判断是否超过检测门限
    if (peak < m_thresholdDbm) {
        // 峰值低于门限，认为没有有效信号
        return result;
    }

    // 7. 信号超过门限，标记为检测到信号
    result.detected = true;

    // 8. 信号分类识别（规则引擎）
    // 识别优先级：跳频 > 图传 > 遥控 > 未知
    // 跳频特征：跳频次数>=4 且 窄带（带宽<8MHz）
    if (result.hopCount >= 4 && result.bandwidthMHz < 8.0) {
        result.label = QStringLiteral("疑似无人机跳频遥控信号");
        result.confidence = 0.86;
        result.detail = QStringLiteral("窄带峰值在多个频点间快速切换，符合跳频链路特征");
    }
    // 图传特征：宽带（带宽>=10MHz）且 活跃频点多（>=35个）
    else if (result.bandwidthMHz >= 10.0 && result.activeBins >= 35) {
        result.label = QStringLiteral("疑似无人机图传信号");
        result.confidence = 0.82;
        result.detail = QStringLiteral("检测到持续宽带能量平台，疑似视频/数传链路");
    }
    // 遥控特征：窄带（带宽0.4-8MHz）
    else if (result.bandwidthMHz > 0.4 && result.bandwidthMHz < 8.0) {
        result.label = QStringLiteral("疑似无人机遥控信号");
        result.confidence = 0.74;
        result.detail = QStringLiteral("检测到持续窄带高能量峰，疑似遥控控制链路");
    }
    // 其他情况：未知信号
    else {
        result.label = QStringLiteral("未知无线信号");
        result.confidence = 0.45;
        result.detail = QStringLiteral("能量超过门限，但特征不足以归类为典型无人机链路");
    }

    // 9. 根据信号强度调整置信度
    // 信号越强（超过门限越多），置信度越高
    const double margin = qBound(0.0, (result.peakDbm - m_thresholdDbm) / 24.0, 1.0);
    result.confidence = qBound(0.0, result.confidence * 0.75 + margin * 0.25, 0.98);
    
    return result;
}

/**
 * @brief 重置分析器状态
 * 
 * 清除跳频历史记录，用于场景切换或用户手动重置时。
 */
void SpectrumAnalyzer::reset()
{
    m_recentPeakBins.clear();
}

/**
 * @brief 统计活跃频点数量
 * 
 * 遍历所有频点，计算功率超过检测门限的频点数量。
 * 活跃频点数量反映了信号的带宽特性。
 * 
 * @param bins 频谱功率数据
 * @return 活跃频点数量
 */
int SpectrumAnalyzer::countActiveBins(const QVector<float> &bins) const
{
    int count = 0;
    for (int i = 0; i < bins.size(); ++i) {
        if (bins.at(i) >= m_thresholdDbm) {
            ++count;
        }
    }
    return count;
}

/**
 * @brief 查找峰值频点
 * 
 * 遍历频谱数据，找到功率最大的频点索引和对应的峰值功率。
 * 
 * @param bins 频谱功率数据
 * @param peakValue 输出参数：峰值功率值（可为NULL）
 * @return 峰值频点索引（-1表示数据为空）
 */
int SpectrumAnalyzer::findPeakBin(const QVector<float> &bins, float *peakValue) const
{
    if (bins.isEmpty()) {
        return -1;
    }

    int peakBin = 0;
    float peak = bins.at(0);
    
    // 遍历寻找最大值
    for (int i = 1; i < bins.size(); ++i) {
        if (bins.at(i) > peak) {
            peak = bins.at(i);
            peakBin = i;
        }
    }

    // 输出峰值功率值（如果需要）
    if (peakValue) {
        *peakValue = peak;
    }
    
    return peakBin;
}

/**
 * @brief 估计信号带宽
 * 
 * 通过找到功率超过门限的第一个和最后一个频点，计算信号占用的带宽。
 * 带宽计算公式：(last - first + 1) * spanMHz / binCount
 * 
 * @param frame 频谱帧数据
 * @param firstBin 输出参数：第一个活跃频点索引（可为NULL）
 * @param lastBin 输出参数：最后一个活跃频点索引（可为NULL）
 * @return 信号带宽（MHz），无信号时返回0
 */
double SpectrumAnalyzer::estimateBandwidthMHz(const SpectrumFrame &frame, int *firstBin, int *lastBin) const
{
    int first = -1;
    int last = -1;
    
    // 遍历所有频点，记录第一个和最后一个超过门限的频点
    for (int i = 0; i < frame.powerDbm.size(); ++i) {
        if (frame.powerDbm.at(i) >= m_thresholdDbm) {
            if (first < 0) {
                first = i;  // 第一个活跃频点
            }
            last = i;  // 更新最后一个活跃频点
        }
    }

    // 输出参数（如果需要）
    if (firstBin) {
        *firstBin = first;
    }
    if (lastBin) {
        *lastBin = last;
    }
    
    // 边界检查：没有活跃频点或频点数量不足
    if (first < 0 || last < first || frame.powerDbm.size() <= 1) {
        return 0.0;
    }

    // 计算带宽：(活跃频点数量) * 每频点带宽
    return (double)(last - first + 1) * frame.spanMHz / (double)frame.powerDbm.size();
}

/**
 * @brief 更新跳频历史记录
 * 
 * 将当前帧的峰值频点添加到历史记录中，用于检测跳频特征。
 * 历史记录最多保存24帧，超过时移除最旧的记录。
 * 
 * @param peakBin 当前帧的峰值频点索引
 */
void SpectrumAnalyzer::updateHopHistory(int peakBin)
{
    // 无效频点不记录
    if (peakBin < 0) {
        return;
    }
    
    // 添加当前峰值频点到历史
    m_recentPeakBins.append(peakBin);
    
    // 保持历史记录长度不超过24帧
    while (m_recentPeakBins.size() > 24) {
        m_recentPeakBins.removeFirst();
    }
}

/**
 * @brief 计算最近跳频次数
 * 
 * 分析历史峰值轨迹，统计峰值在不同频点间切换的次数。
 * 当相邻峰值间距超过阈值（18个频点，约3.5MHz）时，认为发生了一次跳频。
 * 
 * @return 最近一段时间内的跳频次数
 */
int SpectrumAnalyzer::recentHopCount() const
{
    // 历史记录不足2帧，无法判断跳频
    if (m_recentPeakBins.size() < 2) {
        return 0;
    }

    int changes = 0;
    int last = m_recentPeakBins.first();
    
    // 遍历历史记录，统计跳频次数
    for (int i = 1; i < m_recentPeakBins.size(); ++i) {
        const int current = m_recentPeakBins.at(i);
        // 频点变化超过18个（约3.5MHz），认为是跳频
        if (qAbs(current - last) > 18) {
            ++changes;
            last = current;
        }
    }
    
    return changes;
}