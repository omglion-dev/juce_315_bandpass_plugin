#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>

namespace bandpass {
    FrequencyResponseComponent::FrequencyResponseComponent() {
        setOpaque(false);
    }

    void FrequencyResponseComponent::setParameter(float freqCenterVal, float qVal, float sampleRateVal) {
        freqCenter = freqCenterVal;
        q = qVal;
        sampleRate = sampleRateVal;
        repaint();
    }

    float FrequencyResponseComponent::getMagnitude(const float frequency) const {
        // 二阶带通模拟域：幅频响应
        // H(s) = (ω0/Q * s) / (s² + ω0/Q * s + ω0²)
        // 对复数分式：H = Z1/Z2，H的模等于：|H| = |Z1|/|Z2|，即分子的模除以分母的模
        // |H(jω)| = (ω0/Q * ω) / sqrt((ω0² - ω²)² + (ω0/Q * ω)²)

        // 中心角频率 ω0 = 2π·f_center
        const auto omega0 = juce::MathConstants<float>::twoPi * freqCenter;
        // 当前采样点角频率 ω = 2π·frequency
        const auto omega = juce::MathConstants<float>::twoPi * frequency;
        // 角频率带宽 BW = ω0 / Q
        const auto bw = omega0 / q;

        // 分子：bw * ω
        const auto numerator = bw * omega;
        // 分母：sqrt( (ω0²−ω²)² + (bw·ω)² )
        const auto denominator = std::sqrt(std::pow(omega0 * omega0 - omega * omega, 2.0f)
                                        + std::pow(bw * omega, 2.0f));

        if (denominator < 1e-6f) {
            return 1.0f;
        }
        return numerator / denominator;
    }

    void FrequencyResponseComponent::paint(juce::Graphics& g) {
        const auto w = static_cast<float>(getWidth());
        const auto h = static_cast<float>(getHeight());
        const auto padLeft = 50.f;
        const auto padRight = 20.f;
        const auto padTop = 30.f;
        const auto padBottom = 35.f;
        // plotW：画布宽度，plot 是MATLAB的二维线图函数
        const auto plotW = w - padLeft - padRight;
        // plotH：画布高度
        const auto plotH = h - padTop - padBottom;

        // --- 背景半透明面板 ---
        g.setColour(juce::Colour(0x33000000));
        g.fillRoundedRectangle(padLeft - 10, padTop - 10, plotW + 20, plotH + 20, 6.0f);

        // --- 网络线 ---
        g.setColour(juce::Colour(0x22ffffff));
        // 水平网格（3条）
        for (int i = 0; i < 3; ++i) {
            const auto gy = padTop + plotH * i / 3.0f;
            g.drawHorizontalLine(static_cast<int>(gy), padLeft, padLeft + plotW);
        }
        // 垂直网格（5条）
        for (int i = 0; i < 5; ++i) {
            const auto gx = padLeft + plotW * i / 5.0f;
            g.drawVerticalLine(static_cast<int>(gx), padTop, padTop + plotH);
        }
        // --- 坐标轴标签 ---
        g.setColour(juce::Colour(0xffaaaaaa));
        g.setFont(BandpassLookAndFeel::getAxisFont());
        g.drawText(
            "Gain",
            2,
            static_cast<int>(padTop + plotH * 0.5f - 10),
            45,
            20,
            juce::Justification::centred);

        // --- 绘制频响曲线 ---
        juce::Path curvePath;   // 存储频响轮廓线，用来strokePath描边画线
        juce::Path fillPath;    // 存储“曲线+底边闭合”的封闭多边形，用来fillPath填充颜色

        constexpr auto minFreq = Bandpass::minFreqCenter;
        const auto maxFreq = sampleRate * 0.5f;     // 奈奎斯特频率

        constexpr int numPoints = 200;
        // 注意：必须是小于等于符号，这是为了让t的最大值是1，这样才能取到奈奎斯特点
        // 即实际是201个采样点
        for (int i = 0; i <= numPoints; ++i) {
            // t：归一化横坐标，范围[0.0, 1.0]
            // i = 0 -> t = 0.0：对应最左侧20Hz
            // i = 200 -> t = 1.0：对应最右侧，奈奎斯特频率
            const auto t = static_cast<float>(i) / numPoints;
            // 对数频率映射
            // 公式推导：
            // t=0 → freq = minFreq = 20Hz
            // t=1 → freq = minFreq * (maxFreq/minFreq)^1 = maxFreq →奈奎斯特
            const auto freq = minFreq * std::pow(maxFreq / minFreq, t);
            const auto mag = getMagnitude(freq);

            // 把归一化的t∈[0,1]映射为画布像素 X 坐标：
            // - t=0 → x=padLeft（最左边）
            // - t=1 → x=padLeft+plotW（绘图区最右侧）
            const auto x = padLeft + plotW * t;
            // 同样将mag∈[0,1]映射为画布像素 y 坐标：
            const auto y =
                // 1.f − mag：反转数值：顶部y值小，底部y值大
                padTop + plotH * (1.f - juce::jlimit(0.f, 1.f, mag));

            // 处理第一个点，最左侧的20Hz
            if (i == 0) {
                // 从当前点开始绘制
                curvePath.startNewSubPath(x, y);
                // 填充路径起点不是曲线上的点！是绘图区域左下角。
                fillPath.startNewSubPath(x, padTop + plotH);
                // 从左下角垂直向上走到曲线第一个采样点
                fillPath.lineTo(x, y);
            }
            // 处理其他点
            else {
                // 轮廓线从上一个点连线到当前`(x,y)`，把所有频响采样点连成一条连续曲线。
                curvePath.lineTo(x, y);
                // 填充多边形沿着频响曲线，不断记录每个曲线上的点
                fillPath.lineTo(x, y);
            }
        }
        // 闭合填充路径
        // 从曲线的最右端点，垂直向下走到绘图区域右下角。
        fillPath.lineTo(padLeft + plotW, padTop + plotH);   // 右下角
        // 自动把【右下角】连回路径起点【绘图左下角】
        fillPath.closeSubPath();
        // 至此，fillPath 形成一个完整封闭多边形

        // --- 曲线下方渐变填充 ---
        juce::ColourGradient fillGrad(
            juce::Colour(0x4400aaff),
            0, padTop,
            juce::Colour(0x0000aaff),
            0, padTop + plotH,
            false);
        g.setGradientFill(fillGrad);
        g.fillPath(fillPath);

        // --- 曲线发光效果（外层粗半透明） ---
        g.setColour(juce::Colour(0x3300ccff));
        g.strokePath(curvePath, juce::PathStrokeType(6.0f));

        // --- 曲线核心（亮蓝色细线）---
        g.setColour(juce::Colour(0xff00ccff));
        g.strokePath(curvePath, juce::PathStrokeType(2.0f));

        // --- 峰值点标注 ---
        // 已知真实频率 `freqCenter`，反推它在画布上的归一化横坐标 t_peak，再换算成像素 peakX
        // 和上面的计算freq 正好是逆运算
        const auto peakX = padLeft + plotW * (std::log(freqCenter / minFreq) / std::log(maxFreq / minFreq));
        const auto peakY = padTop + plotH * (1.f - 1.f);  // 峰值 mag = 1，在顶部

        // 峰值圆点
        g.setColour(juce::Colour(0xffffffff));
        g.fillEllipse(peakX - 5.0f, peakY - 5.0f, 10.0f, 10.0f);
        g.setColour(juce::Colour(0x4400aaff));
        g.fillEllipse(peakX - 9.f, peakY - 9.f, 18.0f, 18.0f);

        // 峰值频率文字
        g.setColour(juce::Colour(0xffffffff));
        g.setFont(BandpassLookAndFeel::getPeakFont());
        juce::String peakText;
        if (freqCenter >= 1000.0f) {
            peakText = juce::String(freqCenter / 1000.0f, 1) + " kHz";
        }
        else {
            peakText = juce::String(static_cast<int>(freqCenter)) + " Hz";
        }
        g.drawText(
            peakText,
            static_cast<int>(peakX - 40),
            static_cast<int>(peakY - 33),
            80,
            20,
            juce::Justification::centred,
            false);

        // 峰值垂直虚线
        g.setColour(juce::Colour(0x44ffffff));
        constexpr float dashLengths[2] = {4.0f, 4.0f};
        g.drawDashedLine(
            juce::Line<float>(peakX, peakY, peakX, padTop + plotH),
            dashLengths,
            2,
            1.0f);

    }
}
