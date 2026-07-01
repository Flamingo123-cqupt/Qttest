/**
 * @file spectrumsimulator.cpp
 * @brief 频谱数据模拟器实现
 * 
 * 实现频谱数据的模拟生成逻辑，包括噪声生成、高斯信号、平顶信号等。
 * 支持四种模拟场景：环境底噪、遥控窄带链路、图传宽带链路、跳频链路。
 */

#include "spectrumsimulator.h"
#include <QtMath>
#include <QRandomGenerator>

/**
 * @brief 构造函数
 * 
 * 初始化频谱模拟器的参数，默认场景为无人机遥控链路，中心频率2400MHz。
 * 
 * @param parent 父对象指针
 */
SpectrumSimulator::SpectrumSimulator(QObject *parent)
    : QObject(parent)
    , m_scenario(DroneControl)      // 默认场景：无人机遥控链路
    , m_binCount(512)               // 频点数量：512个
    , m_centerFreqMHz(2400.0)       // 中心频率：2400MHz（2.4GHz ISM频段）
    , m_spanMHz(100.0)              // 分析带宽：100MHz
    , m_timeSec(0.0)                // 初始模拟时间：0秒
    , m_hopIndex(0)                 // 跳频索引：0
{
}

/**
 * @brief 设置当前模拟场景
 * 
 * 切换模拟场景，并重置跳频索引。
 * 
 * @param scenario 场景枚举值
 */
void SpectrumSimulator::setScenario(SpectrumSimulator::Scenario scenario)
{
    m_scenario = scenario;
    m_hopIndex = 0;   // 切换场景时重置跳频状态
}

/**
 * @brief 获取当前模拟场景
 * 
 * @return 当前场景枚举值
 */
SpectrumSimulator::Scenario SpectrumSimulator::scenario() const
{
    return m_scenario;
}

/**
 * @brief 生成下一帧频谱数据
 * 
 * 核心方法，根据当前场景生成一帧模拟频谱数据：
 * 1. 创建频点数组并填充噪声
 * 2. 根据场景添加对应的信号分量
 * 3. 组装频谱帧结构并返回
 * 
 * @return SpectrumFrame 频谱帧数据
 */
SpectrumFrame SpectrumSimulator::nextFrame()
{
    // 1. 创建频点数组（初始化为0）
    QVector<float> bins(m_binCount);
    
    // 2. 添加环境噪声（所有场景都包含底噪）
    addNoise(bins);

    // 3. 根据当前场景添加特定信号分量
    QString name;
    switch (m_scenario) {
    case IdleNoise:
        // 空闲场景：仅底噪，不添加额外信号
        name = QStringLiteral("环境底噪");
        break;
        
    case DroneControl:
        // 遥控链路场景：窄带持续信号
        name = QStringLiteral("无人机遥控窄带链路");
        // 主信号：带小幅正弦摆动的高斯峰，模拟遥控信号的多普勒频移或调制特性
        addGaussianSignal(bins, m_binCount * 0.42 + 8.0 * qSin(m_timeSec * 1.3), 5.0, -48.0f);
        // 副信号：固定位置的高斯峰，模拟遥控链路的辅助通道
        addGaussianSignal(bins, m_binCount * 0.61, 3.0, -55.0f);
        break;
        
    case DroneVideo:
        // 图传链路场景：宽带平台信号
        name = QStringLiteral("无人机图传宽带链路");
        // 主信号：宽带平顶信号，模拟视频传输的高带宽特性
        addFlatSignal(bins, m_binCount * 0.50 + 14.0 * qSin(m_timeSec * 0.5), 70.0, -50.0f);
        // 副信号：叠加在平顶上的高斯峰，模拟图传信号的载波分量
        addGaussianSignal(bins, m_binCount * 0.50, 35.0, -54.0f);
        break;
        
    case FrequencyHopping: {
        // 跳频链路场景：多个频点间快速切换
        name = QStringLiteral("无人机跳频链路");
        // 预设6个跳频频点位置
        const int hopBins[] = {90, 145, 210, 270, 330, 405};
        // 每0.2秒切换一次跳频位置（10Hz采样率，每2帧切换）
        if (((int)(m_timeSec * 10.0)) % 2 == 0) {
            m_hopIndex = (m_hopIndex + 1) % 6;
        }
        // 当前跳频点主信号
        addGaussianSignal(bins, hopBins[m_hopIndex], 4.0, -45.0f);
        // 前一个跳频点的残留信号（模拟切换延迟）
        addGaussianSignal(bins, hopBins[(m_hopIndex + 2) % 6], 3.0, -58.0f);
        break;
    }
    }

    // 4. 组装频谱帧结构
    SpectrumFrame frame;
    frame.powerDbm = bins;           // 功率数据
    frame.centerFreqMHz = m_centerFreqMHz;  // 中心频率
    frame.spanMHz = m_spanMHz;       // 分析带宽
    frame.timeSec = m_timeSec;       // 时间戳
    frame.scenarioName = name;       // 场景名称

    // 5. 更新模拟时间（帧间隔约0.08秒，对应约12.5Hz的帧速率）
    m_timeSec += 0.08;
    
    return frame;
}

/**
 * @brief 获取频点数量
 * 
 * @return 频点数量
 */
int SpectrumSimulator::binCount() const
{
    return m_binCount;
}

/**
 * @brief 将频点索引转换为频率值
 * 
 * 根据中心频率和分析带宽，将频点索引线性映射到实际频率。
 * 
 * @param bin 频点索引（0到binCount-1）
 * @return 对应的频率值，单位 MHz
 */
double SpectrumSimulator::binToFreqMHz(int bin) const
{
    if (m_binCount <= 1) {
        return m_centerFreqMHz;
    }
    // 计算起始频率（中心频率减去半带宽）
    const double startMHz = m_centerFreqMHz - m_spanMHz / 2.0;
    // 线性插值计算当前频点对应的频率
    return startMHz + (double)bin * m_spanMHz / (double)(m_binCount - 1);
}

/**
 * @brief 添加高斯噪声到频谱数据
 * 
 * 为每个频点添加：
 * - 基础底噪：-92dBm
 * - 慢起伏：正弦波调制的幅度变化（模拟环境噪声的缓慢变化）
 * - 随机噪声：均匀分布的随机波动
 * 
 * @param bins 频谱数据数组（输入/输出）
 */
void SpectrumSimulator::addNoise(QVector<float> &bins) const
{
    for (int i = 0; i < bins.size(); ++i) {
        // 慢起伏分量：随时间和频率缓慢变化的噪声基底
        const float slowRipple = 2.5f * (float)qSin((double)i * 0.035 + m_timeSec * 0.8);
        // 总噪声 = 基础底噪 + 慢起伏 + 随机波动
        bins[i] = -92.0f + slowRipple + randomFloat(-4.0f, 4.0f);
    }
}

/**
 * @brief 添加高斯形状的信号
 * 
 * 在指定位置添加一个高斯函数形状的信号峰，公式：
 *   value = -95 + (peakDbm + 95) * exp(-0.5 * ((i - centerBin)/sigma)^2)
 * 
 * 高斯信号特点：中心功率最高，向两侧逐渐衰减，适合模拟窄带通信信号。
 * 
 * @param bins 频谱数据数组（输入/输出）
 * @param centerBin 信号中心频点位置
 * @param widthBins 信号带宽（以频点为单位）
 * @param peakDbm 信号峰值功率（dBm）
 */
void SpectrumSimulator::addGaussianSignal(QVector<float> &bins, double centerBin, double widthBins, float peakDbm) const
{
    // 计算标准差（sigma），确保带宽参数合理
    const double sigma = qMax(1.0, widthBins);
    
    for (int i = 0; i < bins.size(); ++i) {
        // 计算当前频点相对于中心的归一化距离
        const double d = ((double)i - centerBin) / sigma;
        // 高斯函数形状因子（0到1之间）
        const float shape = (float)qExp(-0.5 * d * d);
        // 计算信号值（叠加在-95dBm基底上）
        const float value = -95.0f + (peakDbm + 95.0f) * shape;
        // 取最大值（信号不能低于已有噪声），并添加小幅随机波动
        bins[i] = qMax(bins[i], value + randomFloat(-1.2f, 1.2f));
    }
}

/**
 * @brief 添加平顶形状的信号
 * 
 * 在指定位置添加一个平顶平台信号，特点是：
 * - 中心区域：功率平坦（固定峰值）
 * - 过渡区域：功率线性衰减
 * - 外部区域：不影响
 * 
 * 平顶信号适合模拟宽带通信信号，如图传、数传等。
 * 
 * @param bins 频谱数据数组（输入/输出）
 * @param centerBin 信号中心频点位置
 * @param widthBins 信号带宽（以频点为单位）
 * @param peakDbm 信号峰值功率（dBm）
 */
void SpectrumSimulator::addFlatSignal(QVector<float> &bins, double centerBin, double widthBins, float peakDbm) const
{
    // 计算半带宽
    const double half = widthBins / 2.0;
    
    for (int i = 0; i < bins.size(); ++i) {
        // 计算当前频点到中心的距离
        const double distance = qAbs((double)i - centerBin);
        
        if (distance <= half) {
            // 中心区域：平坦功率，添加随机波动
            bins[i] = qMax(bins[i], peakDbm + randomFloat(-3.0f, 2.0f));
        } else if (distance <= half + 18.0) {
            // 过渡区域：线性衰减，18个频点的过渡带
            const double edge = 1.0 - (distance - half) / 18.0;
            bins[i] = qMax(bins[i], -92.0f + (peakDbm + 92.0f) * (float)edge);
        }
        // 外部区域：不做处理
    }
}

/**
 * @brief 生成指定范围内的随机浮点数
 * 
 * 使用Qt的全局随机数生成器，生成[minValue, maxValue)范围内的均匀分布随机数。
 * 
 * @param minValue 最小值（包含）
 * @param maxValue 最大值（不包含）
 * @return 随机浮点数
 */
float SpectrumSimulator::randomFloat(float minValue, float maxValue) const
{
    // 获取[0, 1)范围内的随机双精度数
    const double u = QRandomGenerator::global()->generateDouble();
    // 线性映射到指定范围
    return minValue + (maxValue - minValue) * (float)u;
}