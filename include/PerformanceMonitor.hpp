#ifndef PERFORMANCE_MONITOR_HPP
#define PERFORMANCE_MONITOR_HPP

#include <stddef.h>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

/**
 * PerformanceMonitor - 性能监控类
 * 
 * 用于统计和监控播放性能，包括：
 * - 帧数统计（加载、解码、显示）
 * - 时间统计
 * - FPS计算
 * - 实时报告（带节流）
 * 
 * 使用场景：
 * - 视频播放性能测试
 * - 解码性能评估
 * - 显示性能分析
 */
class PerformanceMonitor {
private:
    // ============ 时间记录 ============
    std::chrono::steady_clock::time_point start_time_;
    std::chrono::steady_clock::time_point last_report_time_;
    
    // ============ 帧数统计 ============
    int frames_loaded_;               // 加载的帧数
    int frames_decoded_;              // 解码的帧数（如果有解码）
    int frames_displayed_;            // 显示的帧数
    
    // ============ 时间统计 ============
    long long total_load_time_us_;    // 总加载时间（微秒）
    long long total_decode_time_us_;  // 总解码时间（微秒）
    long long total_display_time_us_; // 总显示时间（微秒）
    
    // ============ 临时计时器 ============
    std::chrono::steady_clock::time_point load_start_;
    std::chrono::steady_clock::time_point decode_start_;
    std::chrono::steady_clock::time_point display_start_;
    
    // ============ 状态标志 ============
    bool is_started_;
    bool is_paused_;
    
    // ============ 节流控制（用于实时打印）============
    int report_interval_ms_;          // 报告间隔（毫秒）
    
    // ============ 定时器相关 ============
    std::thread timer_thread_;        // 定时器后台线程
    std::mutex timer_mutex_;          // 定时器互斥锁
    std::condition_variable timer_cv_; // 定时器条件变量
    double timer_interval_seconds_;   // 定时器间隔（秒）
    bool timer_running_;              // 定时器运行标志
    
    // 定时器增量统计（用于计算每个时间间隔内的帧数）
    int last_frames_loaded_;          // 上次统计时的加载帧数
    int last_frames_decoded_;         // 上次统计时的解码帧数
    int last_frames_displayed_;       // 上次统计时的显示帧数
    std::chrono::steady_clock::time_point last_timer_trigger_time_; // 上次触发时间
    
    // ============ 内部辅助方法 ============
    
    /**
     * 计算平均FPS
     */
    double calculateAverageFPS(int frame_count) const;
    
    /**
     * 获取从开始到现在的总时长（秒）
     */
    double getTotalDuration() const;
    
    /**
     * 定时器线程函数
     * 在后台线程中周期性地打印统计信息
     */
    void timerThreadFunction();

public:
    PerformanceMonitor();
    ~PerformanceMonitor();
    
    // ============ 生命周期管理 ============
    
    /**
     * 开始监控
     */
    void start();
    
    /**
     * 重置所有统计数据
     */
    void reset();
    
    /**
     * 暂停监控
     */
    void pause();
    
    /**
     * 恢复监控
     */
    void resume();
    
    // ============ 简单事件记录 ============
    
    /**
     * 记录一次帧加载
     */
    void recordFrameLoaded();
    
    /**
     * 记录一次帧解码
     */
    void recordFrameDecoded();
    
    /**
     * 记录一次帧显示
     */
    void recordFrameDisplayed();
    
    // ============ 带计时的事件记录 ============
    
    /**
     * 开始计时：帧加载
     */
    void beginLoadFrame();
    
    /**
     * 结束计时：帧加载
     */
    void endLoadFrame();
    
    /**
     * 开始计时：帧解码
     */
    void beginDecodeFrame();
    
    /**
     * 结束计时：帧解码
     */
    void endDecodeFrame();
    
    /**
     * 开始计时：帧显示
     */
    void beginDisplayFrame();
    
    /**
     * 结束计时：帧显示
     */
    void endDisplayFrame();
    
    // ============ 统计信息获取 ============
    
    /**
     * 获取已加载的帧数
     */
    int getLoadedFrames() const;
    
    /**
     * 获取已解码的帧数
     */
    int getDecodedFrames() const;
    
    /**
     * 获取已显示的帧数
     */
    int getDisplayedFrames() const;
    
    /**
     * 获取平均加载FPS
     */
    double getAverageLoadFPS() const;
    
    /**
     * 获取平均解码FPS
     */
    double getAverageDecodeFPS() const;
    
    /**
     * 获取平均显示FPS
     */
    double getAverageDisplayFPS() const;
    
    /**
     * 获取总运行时间（秒）
     */
    double getTotalTime() const;
    
    /**
     * 获取从开始到现在的时间（秒）
     */
    double getElapsedTime() const;
    
    // ============ 报告输出 ============
    
    /**
     * 打印完整的统计报告
     */
    void printStatistics() const;
    
    /**
     * 实时打印统计（带节流，避免频繁打印）
     * 默认每1秒最多打印一次
     */
    void printRealTimeStats();
    
    /**
     * 生成统计报告字符串
     */
    void generateReport(char* buffer, size_t buffer_size) const;
    
    // ============ 配置 ============
    
    /**
     * 设置实时报告的间隔（毫秒）
     * @param interval_ms 间隔时间（毫秒）
     */
    void setReportInterval(int interval_ms);
    
    // ============ 定时器控制 ============
    
    /**
     * 设置定时器间隔
     * 
     * 配置定时器的触发间隔时间。
     * 注意：需要在 startTimer() 之前调用才能生效。
     * 
     * @param seconds 间隔时间（秒），例如 0.5、1.0、2.5 等
     */
    void setTimerInterval(double seconds);
    
    /**
     * 启动定时器
     * 
     * 启动一个后台线程，周期性地打印性能统计信息。
     * 定时器会按照 setTimerInterval() 设置的间隔触发。
     * 
     * 注意：
     * - 定时器在后台线程运行，不会阻塞主流程
     * - 定时器触发时会自动调用统计函数打印 FPS 等信息
     * - 如果定时器已经在运行，调用此方法无效
     */
    void startTimer();
    
    /**
     * 停止定时器
     * 
     * 停止后台定时器线程。
     * 
     * 注意：
     * - 此方法会等待定时器线程完全退出
     * - 调用后可以再次调用 startTimer() 重新启动
     */
    void stopTimer();
};

#endif // PERFORMANCE_MONITOR_HPP

