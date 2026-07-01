/**
 * @file spectrumsimulator.h
 * @brief 频谱数据模拟器模块
 * 
 * 本模块负责生成模拟的无人机频谱数据，用于测试和演示频谱检测算法。
 * 支持四种模拟场景：
 * - 环境底噪：仅包含高斯噪声和慢起伏
 * - 无人机遥控窄带链路：模拟无人机遥控信号的窄带特征
 * - 无人机图传宽带链路：模拟无人机图传信号的宽带特征
 * - 无人机跳频链路：模拟跳频通信的频率切换特征
 */

#ifndef SPECTRUMSIMULATOR_H
#define SPECTRUMSIMULATOR_H

#include <QObject>
#include <QVector>
#include <QString>

/**
 * @brief 频谱帧数据结构
 * 
 * 表示一帧频谱数据，包含功率谱密度、频率参数和时间戳信息。
 */
struct SpectrumFrame
{
    QVector<float> powerDbm;      ///< 每个频点的功率值，单位 dBm
    double centerFreqMHz;         ///< 当前分析带宽的中心频率，单位 MHz
    double spanMHz;               ///< 当前分析带宽范围，单位 MHz
    double timeSec;               ///< 模拟时间戳，单位秒
    QString scenarioName;         ///< 当前模拟场景的名称
};

/**
 * @brief 频谱模拟器类
 * 
 * 负责生成不同场景下的模拟频谱数据，为频谱分析器提供测试数据源。
 */
class SpectrumSimulator : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 模拟场景枚举
     * 
     * 定义四种不同的频谱模拟场景，对应无人机通信的不同工作模式。
     */
    enum Scenario
    {
        IdleNoise = 0,       ///< 空闲场景：只有环境底噪
        DroneControl,        ///< 遥控链路：窄带持续信号
        DroneVideo,          ///< 图传链路：宽带平台信号
        FrequencyHopping     ///< 跳频链路：多个频点间快速切换
    };

    /**
     * @brief 构造函数
     * 
     * @param parent 父对象指针
     */
    explicit SpectrumSimulator(QObject *parent = 0);

    /**
     * @brief 设置当前模拟场景
     * 
     * @param scenario 场景枚举值
     */
    void setScenario(Scenario scenario);

    /**
     * @brief 获取当前模拟场景
     * 
     * @return 当前场景枚举值
     */
    Scenario scenario() const;

    /**
     * @brief 生成下一帧频谱数据
     * 
     * 根据当前场景生成一帧模拟频谱数据，包含噪声和信号分量。
     * 
     * @return SpectrumFrame 频谱帧数据
     */
    SpectrumFrame nextFrame();

    /**
     * @brief 获取频点数量
     * 
     * @return 频点数量
     */
    int binCount() const;

    /**
     * @brief 将频点索引转换为频率值
     * 
     * @param bin 频点索引（0到binCount-1）
     * @return 对应的频率值，单位 MHz
     */
    double binToFreqMHz(int bin) const;

private:
    /**
     * @brief 添加高斯噪声到频谱数据
     * 
     * 为每个频点添加随机噪声和慢起伏分量，模拟真实环境噪声。
     * 
     * @param bins 频谱数据数组（输入/输出）
     */
    void addNoise(QVector<float> &bins) const;

    /**
     * @brief 添加高斯形状的信号
     * 
     * 在指定位置添加一个高斯形状的信号峰，模拟窄带通信信号。
     * 
     * @param bins 频谱数据数组（输入/输出）
     * @param centerBin 信号中心频点位置
     * @param widthBins 信号带宽（以频点为单位）
     * @param peakDbm 信号峰值功率（dBm）
     */
    void addGaussianSignal(QVector<float> &bins, double centerBin, double widthBins, float peakDbm) const;

    /**
     * @brief 添加平顶形状的信号
     * 
     * 在指定位置添加一个平顶平台信号，模拟宽带通信信号（如图传）。
     * 
     * @param bins 频谱数据数组（输入/输出）
     * @param centerBin 信号中心频点位置
     * @param widthBins 信号带宽（以频点为单位）
     * @param peakDbm 信号峰值功率（dBm）
     */
    void addFlatSignal(QVector<float> &bins, double centerBin, double widthBins, float peakDbm) const;

    /**
     * @brief 生成指定范围内的随机浮点数
     * 
     * @param minValue 最小值
     * @param maxValue 最大值
     * @return 随机浮点数
     */
    float randomFloat(float minValue, float maxValue) const;

private:
    Scenario m_scenario;       ///< 当前模拟场景
    int m_binCount;            ///< 频点数量（默认512）
    double m_centerFreqMHz;    ///< 中心频率（默认2400MHz，即2.4GHz ISM频段）
    double m_spanMHz;          ///< 分析带宽（默认100MHz）
    double m_timeSec;          ///< 当前模拟时间
    int m_hopIndex;            ///< 跳频场景下的当前跳频索引
};

#endif // SPECTRUMSIMULATOR_H