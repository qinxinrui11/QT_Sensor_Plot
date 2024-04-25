#ifndef APPDATA_H
#define APPDATA_H
// 存放全局变量

#include <QObject>

// 注册表 key 名宏定义
#define KEY_SELF_IP         "self/ip"
#define KEY_SELF_PORT       "self/port"
#define KEY_TARGET_IP       "target/ip"
#define KEY_TARGET_PORT     "target/port"
#define KEY_WORK_FREQUENCY  "workFrequency"
#define KEY_DEAD_FREQUENCY  "deadFrequency"
#define KEY_ACC_RANGE       "accRange"

#define PLOT_REFRESH_RATE 40       // Plot 每秒更新的次数
#define WIDGET_REFRESH_RATE 25     // UI 控件每秒更新的次数

extern float workFreq_map[8];
extern float deadFreq_map[7];
extern float accRange_map[3];

struct My_HostAddress{
    QString IP;
    quint16 port;
};

struct PlotRange{
    double xmin;
    double xmax;
    double ymin;
    double ymax;
};

struct RegistryData{
    My_HostAddress self;            // 主机 IP 和端口号
    My_HostAddress target;          // 目标 IP 和端口号

    quint8 workFreqIndex;           // 数据发送频率下标
    quint8 deadFreqIndex;           // 滤波器截止频率下标
    quint8 accRangeIndex;           // 加速度计量程下标
    /**************************************************/
    // 下面的数据不存入注册表，仅作为全局变量值
    double accX;                    // UDP 读出的加速度值
    double accY;
    double accZ;

    PlotRange timeRange;            // 时域图像范围
    PlotRange freqRange;            // 频域图像范围

    quint16 FREQUENCY_PER_SECOND;   // 数据每秒生成的组数
    quint16 DATA_COUNT;             // 每组数据的个数
    quint32 T;                      // 三角函数的周期，等于 FREQUENCY_PER_SECOND * DATA_COUNT 时1s能绘制一个周期
    quint32 FFT_N;                  // FFT 变换所需的数据量
    float FFT_F;                    // FFT 输入数据的频率

    quint32 udpPacket;              // UDP 接收到的报文总数
    quint32 udpN;                   // UDP 接收到的有效报文数
    quint32 udpData;                // UDP 接收到的有效数据总数
    quint32 saveN;                  // 保存的有效数据组数
    quint32 averageN;               // 数据均值所用的数据组数
    quint32 autoAverageN;           // 自动均值所用的数据组数
    /**************************************************/
    // 状态位
    bool UDPConnect_flag;           // UDP 是否连接的标志位
    bool dataSave_flag;             // 开始数据保存的标志位
    bool dataAverage_flag;          // 开始数据均值的标志位
    bool autoAverage_flag;          // 开始自动均值的标志位
    bool startFFT_flag;             // 开启 FFT 运算及绘图的标志位
};

extern RegistryData m_registryData;

#endif // APPDATA_H
