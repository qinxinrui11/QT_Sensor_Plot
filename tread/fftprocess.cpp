#include <QDebug>
#include "fftprocess.h"
#include "fftw/fftw3.h"

/**
 * @brief FFT 运算
 *
 * 此函数调用 fftw 库，
 * 根据输入数据 data 和数据频率 m_FS，数据量 m_nums 自动计算频率幅值
 *
 * @param QVector<double> data
 * @return 无
 */
void FftProcess::FftCalculate(QVector<double> data)
{
    // 数据清零
    freq.clear();
    ampl.clear();

    quint32 nums = *m_nums;
    // 数据存入缓冲区，等达到计算用的数据量时开始计算
    for(int i = 0; i < data.size(); i++)
        dataBuffer.append(data[i]);

    if(dataBuffer.size() >= nums){
        // 保证 fft 输入数据量一定
        quint32 num = dataBuffer.size() - nums;
        for(int i = 0; i < num; i++)
            dataBuffer.pop_front();

        // 根据输入数据大小创建输入向量
        double* in = (double*)fftw_malloc(sizeof(double) * nums);
        // 将输入数据复制到输入向量中
        for (int i = 0; i < nums; i++) {
            in[i] = dataBuffer[i];
        }
        // 创建FFT结果向量
        fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * nums);
        // 创建FFT计划
        fftw_plan p;
        p = fftw_plan_dft_r2c_1d(nums, in, out, FFTW_ESTIMATE);
        // 执行FFT
        fftw_execute(p);

        // 将FFT结果存储到输出数据中
        // 计算幅值
        for (int i = 0; i < nums / 2 + 1; i++) {
            double real_part = out[i][0]; // 实部
            double imag_part = out[i][1]; // 虚部
            double magnitude = sqrt(real_part * real_part + imag_part * imag_part); // 幅值

            ampl.append(2 * magnitude / nums); // 存储幅值
        }
        ampl[0] /= 2;
        // 计算频率，假设数据采样频率为20kHz，Fs*(0:(L/2))/L;
        for(int i = 0; i < nums / 2 + 1; i++)
            freq.append(*m_FS / nums * i);

        // 清理资源
        fftw_destroy_plan(p);
        fftw_free(in);
        fftw_free(out);

        emit FftResult(freq, ampl);
    }
}
