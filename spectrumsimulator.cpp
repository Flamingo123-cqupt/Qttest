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
 * 模拟信号特征基于实际无人机通信技术：
 * - 遥控链路：GFSK调制，窄带（2MHz），跳频扩频
 * - 图传链路：OFDM调制，宽带（20MHz），多载波特征
 * - 数传链路：GFSK/LoRa，窄带（<1MHz）
 * - 跳频链路：多频点快速切换（50跳/秒）
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
    const double freqStep = m_spanMHz / (double)(m_binCount - 1);
    
    switch (m_scenario) {
    case IdleNoise:
        name = QStringLiteral("环境底噪");
        break;
        
    case DroneControl: {
        name = QStringLiteral("无人机遥控窄带链路");
        const double ctrlCenter = m_centerFreqMHz;
        const double ctrlSpan = 2.0;
        const int ctrlBin = (int)((ctrlCenter - (m_centerFreqMHz - m_spanMHz/2.0)) / freqStep);
        
        addGFSKSignal(bins, ctrlBin, ctrlSpan / freqStep, -45.0f);
        addGFSKSignal(bins, ctrlBin + 15, ctrlSpan / freqStep * 0.6, -52.0f);
        break;
    }
        
    case DroneVideo: {
        name = QStringLiteral("无人机图传宽带链路");
        const double videoCenter = m_centerFreqMHz;
        const double videoSpan = 20.0;
        const int videoBin = (int)((videoCenter - (m_centerFreqMHz - m_spanMHz/2.0)) / freqStep);
        
        addOFDMFrame(bins, videoBin, videoSpan / freqStep, -48.0f);
        break;
    }
        
    case FrequencyHopping: {
        name = QStringLiteral("无人机跳频链路");
        const double hopRange = 50.0;
        const double hopStart = m_centerFreqMHz - hopRange / 2.0;
        
        const int hopCount = 8;
        const double hopSpacing = hopRange / (double)(hopCount - 1);
        
        if (((int)(m_timeSec * 50.0)) % 2 == 0) {
            m_hopIndex = (m_hopIndex + 1) % hopCount;
        }
        
        const double currentHopFreq = hopStart + m_hopIndex * hopSpacing;
        const int currentHopBin = (int)((currentHopFreq - (m_centerFreqMHz - m_spanMHz/2.0)) / freqStep);
        
        addGFSKSignal(bins, currentHopBin, 2.0 / freqStep, -42.0f);
        
        const int prevHopIndex = (m_hopIndex + hopCount - 1) % hopCount;
        const double prevHopFreq = hopStart + prevHopIndex * hopSpacing;
        const int prevHopBin = (int)((prevHopFreq - (m_centerFreqMHz - m_spanMHz/2.0)) / freqStep);
        addGFSKSignal(bins, prevHopBin, 2.0 / freqStep, -60.0f);
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
    const double startMHz = m_centerFreqMHz - m_spanMHz / 2.0;
    return startMHz + (double)bin * m_spanMHz / (double)(m_binCount - 1);
}

void SpectrumSimulator::setCenterFreqMHz(double centerFreqMHz)
{
    const double startMHz = m_centerFreqMHz - m_spanMHz / 2.0;
    const double endMHz = m_centerFreqMHz + m_spanMHz / 2.0;
    
    if (centerFreqMHz - m_spanMHz / 2.0 < 500.0) {
        m_centerFreqMHz = 500.0 + m_spanMHz / 2.0;
    } else if (centerFreqMHz + m_spanMHz / 2.0 > 6000.0) {
        m_centerFreqMHz = 6000.0 - m_spanMHz / 2.0;
    } else {
        m_centerFreqMHz = centerFreqMHz;
    }
}

double SpectrumSimulator::centerFreqMHz() const
{
    return m_centerFreqMHz;
}

void SpectrumSimulator::setSpanMHz(double spanMHz)
{
    const double minSpan = 1.0;
    const double maxSpan = 5500.0;
    m_spanMHz = qBound(minSpan, spanMHz, maxSpan);
    
    const double endMHz = m_centerFreqMHz + m_spanMHz / 2.0;
    if (endMHz > 6000.0) {
        m_centerFreqMHz = 6000.0 - m_spanMHz / 2.0;
    }
    
    const double startMHz = m_centerFreqMHz - m_spanMHz / 2.0;
    if (startMHz < 500.0) {
        m_centerFreqMHz = 500.0 + m_spanMHz / 2.0;
    }
}

double SpectrumSimulator::spanMHz() const
{
    return m_spanMHz;
}

void SpectrumSimulator::setStartFreqMHz(double startFreqMHz)
{
    const double clampedStart = qBound(500.0, startFreqMHz, 6000.0 - m_spanMHz);
    m_centerFreqMHz = clampedStart + m_spanMHz / 2.0;
}

double SpectrumSimulator::startFreqMHz() const
{
    return m_centerFreqMHz - m_spanMHz / 2.0;
}

double SpectrumSimulator::endFreqMHz() const
{
    return m_centerFreqMHz + m_spanMHz / 2.0;
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
    const double sigma = qMax(1.0, widthBins);
    
    for (int i = 0; i < bins.size(); ++i) {
        const double d = ((double)i - centerBin) / sigma;
        const float shape = (float)qExp(-0.5 * d * d);
        const float value = -95.0f + (peakDbm + 95.0f) * shape;
        bins[i] = qMax(bins[i], value + randomFloat(-1.2f, 1.2f));
    }
}

/**
 * @brief 添加GFSK调制信号
 * 
 * 模拟高斯频移键控（GFSK）信号的频谱特征：
 * - 高斯脉冲成形导致频谱平滑滚降
 * - 调制指数约0.5-1.0，频谱主瓣较窄
 * - 旁瓣衰减快，符合实际遥控信号特征
 * 
 * GFSK是无人机遥控器最常用的调制方式（如2.4GHz频段）。
 * 
 * @param bins 频谱数据数组（输入/输出）
 * @param centerBin 信号中心频点位置
 * @param widthBins 信号带宽（以频点为单位）
 * @param peakDbm 信号峰值功率（dBm）
 */
void SpectrumSimulator::addGFSKSignal(QVector<float> &bins, double centerBin, double widthBins, float peakDbm) const
{
    const double sigma = widthBins / 2.5;
    
    for (int i = 0; i < bins.size(); ++i) {
        const double d = ((double)i - centerBin) / sigma;
        const double d2 = d * d;
        const float shape = (float)(qExp(-d2 / 2.0) * (1.0 - d2 / 6.0 + d2 * d2 / 120.0));
        const float value = -95.0f + (peakDbm + 95.0f) * qMax(0.0f, shape);
        bins[i] = qMax(bins[i], value + randomFloat(-0.8f, 0.8f));
    }
}

/**
 * @brief 添加OFDM帧信号
 * 
 * 模拟正交频分复用（OFDM）信号的频谱特征：
 * - 整体呈近似矩形的频谱形状（OFDM的主瓣叠加）
 * - 内部有轻微的子载波波动（梳状谱特征）
 * - 频谱边缘有滚降（根升余弦滤波器效果）
 * - 功率谱密度相对平坦
 * 
 * OFDM是现代无人机图传的主流调制方式（如5.8GHz高清图传）。
 * 
 * @param bins 频谱数据数组（输入/输出）
 * @param centerBin 信号中心频点位置
 * @param widthBins 信号带宽（以频点为单位）
 * @param peakDbm 信号峰值功率（dBm）
 */
void SpectrumSimulator::addOFDMFrame(QVector<float> &bins, double centerBin, double widthBins, float peakDbm) const
{
    const double halfWidth = widthBins / 2.0;
    const double rolloffWidth = widthBins * 0.12;
    
    for (int i = 0; i < bins.size(); ++i) {
        const double distance = qAbs((double)i - centerBin);
        
        if (distance <= halfWidth + rolloffWidth) {
            double power = 0.0;
            
            if (distance <= halfWidth) {
                power = 0.95 + randomFloat(-0.08f, 0.08f);
            } else {
                const double edge = (halfWidth + rolloffWidth - distance) / rolloffWidth;
                power = (0.95 + randomFloat(-0.08f, 0.08f)) * edge * edge;
            }
            
            const float value = -95.0f + (peakDbm + 95.0f) * (float)power;
            bins[i] = qMax(bins[i], value + randomFloat(-1.5f, 1.5f));
        }
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