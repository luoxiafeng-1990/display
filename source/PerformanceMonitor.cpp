#include "../include/PerformanceMonitor.hpp"
#include <stdio.h>
#include <string.h>

// ============ 构造函数和析构函数 ============

PerformanceMonitor::PerformanceMonitor()
    : frames_loaded_(0)
    , frames_decoded_(0)
    , frames_displayed_(0)
    , total_load_time_us_(0)
    , total_decode_time_us_(0)
    , total_display_time_us_(0)
    , is_started_(false)
    , is_paused_(false)
    , report_interval_ms_(1000)  // 默认1秒报告一次
    , timer_interval_seconds_(1.0)  // 默认1秒触发一次
    , timer_running_(false)
    , last_frames_loaded_(0)
    , last_frames_decoded_(0)
    , last_frames_displayed_(0)
{
}

PerformanceMonitor::~PerformanceMonitor() {
    // 确保定时器停止
    stopTimer();
}

// ============ 生命周期管理 ============

void PerformanceMonitor::start() {
    start_time_ = std::chrono::steady_clock::now();
    last_report_time_ = start_time_;
    is_started_ = true;
    is_paused_ = false;
    
    printf("📊 PerformanceMonitor started\n");
}

void PerformanceMonitor::reset() {
    frames_loaded_ = 0;
    frames_decoded_ = 0;
    frames_displayed_ = 0;
    total_load_time_us_ = 0;
    total_decode_time_us_ = 0;
    total_display_time_us_ = 0;
    
    start_time_ = std::chrono::steady_clock::now();
    last_report_time_ = start_time_;
    
    printf("📊 PerformanceMonitor reset\n");
}

void PerformanceMonitor::pause() {
    is_paused_ = true;
}

void PerformanceMonitor::resume() {
    is_paused_ = false;
}

// ============ 简单事件记录 ============

void PerformanceMonitor::recordFrameLoaded() {
    if (!is_started_ || is_paused_) {
        return;
    }
    frames_loaded_++;
}

void PerformanceMonitor::recordFrameDecoded() {
    if (!is_started_ || is_paused_) {
        return;
    }
    frames_decoded_++;
}

void PerformanceMonitor::recordFrameDisplayed() {
    if (!is_started_ || is_paused_) {
        return;
    }
    frames_displayed_++;
}

// ============ 带计时的事件记录 ============

void PerformanceMonitor::beginLoadFrame() {
    if (!is_started_ || is_paused_) {
        return;
    }
    load_start_ = std::chrono::steady_clock::now();
}

void PerformanceMonitor::endLoadFrame() {
    if (!is_started_ || is_paused_) {
        return;
    }
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end - load_start_);
    
    total_load_time_us_ += duration.count();
    frames_loaded_++;
}

void PerformanceMonitor::beginDecodeFrame() {
    if (!is_started_ || is_paused_) {
        return;
    }
    decode_start_ = std::chrono::steady_clock::now();
}

void PerformanceMonitor::endDecodeFrame() {
    if (!is_started_ || is_paused_) {
        return;
    }
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end - decode_start_);
    
    total_decode_time_us_ += duration.count();
    frames_decoded_++;
}

void PerformanceMonitor::beginDisplayFrame() {
    if (!is_started_ || is_paused_) {
        return;
    }
    display_start_ = std::chrono::steady_clock::now();
}

void PerformanceMonitor::endDisplayFrame() {
    if (!is_started_ || is_paused_) {
        return;
    }
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
        end - display_start_);
    
    total_display_time_us_ += duration.count();
    frames_displayed_++;
}

// ============ 统计信息获取 ============

int PerformanceMonitor::getLoadedFrames() const {
    return frames_loaded_;
}

int PerformanceMonitor::getDecodedFrames() const {
    return frames_decoded_;
}

int PerformanceMonitor::getDisplayedFrames() const {
    return frames_displayed_;
}

double PerformanceMonitor::getAverageLoadFPS() const {
    return calculateAverageFPS(frames_loaded_);
}

double PerformanceMonitor::getAverageDecodeFPS() const {
    return calculateAverageFPS(frames_decoded_);
}

double PerformanceMonitor::getAverageDisplayFPS() const {
    return calculateAverageFPS(frames_displayed_);
}

double PerformanceMonitor::getTotalTime() const {
    return getTotalDuration();
}

double PerformanceMonitor::getElapsedTime() const {
    if (!is_started_) {
        return 0.0;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - start_time_);
    
    return duration.count() / 1000.0;
}

// ============ 报告输出 ============

void PerformanceMonitor::printStatistics() const {
    printf("\n");
    printf("═══════════════════════════════════════════════════════\n");
    printf("          Performance Statistics\n");
    printf("═══════════════════════════════════════════════════════\n");
    
    double total_time = getTotalDuration();
    
    // 帧数统计
    if (frames_loaded_ > 0) {
        printf("📥 Loaded Frames:    %d frames\n", frames_loaded_);
        printf("   Average Load FPS: %.2f fps\n", getAverageLoadFPS());
        if (total_load_time_us_ > 0) {
            double avg_load_time = (double)total_load_time_us_ / frames_loaded_ / 1000.0;
            printf("   Average Load Time: %.2f ms/frame\n", avg_load_time);
        }
    }
    
    if (frames_decoded_ > 0) {
        printf("\n🎬 Decoded Frames:   %d frames\n", frames_decoded_);
        printf("   Average Decode FPS: %.2f fps\n", getAverageDecodeFPS());
        if (total_decode_time_us_ > 0) {
            double avg_decode_time = (double)total_decode_time_us_ / frames_decoded_ / 1000.0;
            printf("   Average Decode Time: %.2f ms/frame\n", avg_decode_time);
        }
    }
    
    if (frames_displayed_ > 0) {
        printf("\n📺 Displayed Frames: %d frames\n", frames_displayed_);
        printf("   Average Display FPS: %.2f fps\n", getAverageDisplayFPS());
        if (total_display_time_us_ > 0) {
            double avg_display_time = (double)total_display_time_us_ / frames_displayed_ / 1000.0;
            printf("   Average Display Time: %.2f ms/frame\n", avg_display_time);
        }
    }
    
    printf("\n⏱️  Total Time:       %.2f seconds\n", total_time);
    printf("═══════════════════════════════════════════════════════\n\n");
}

void PerformanceMonitor::printRealTimeStats() {
    if (!is_started_) {
        return;
    }
    
    // 节流：检查距离上次报告的时间
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - last_report_time_);
    
    if (duration.count() < report_interval_ms_) {
        return;  // 未到报告时间
    }
    
    // 更新上次报告时间
    last_report_time_ = now;
    
    // 打印实时统计
    printf("📊 Real-time Stats: ");
    
    if (frames_loaded_ > 0) {
        printf("Loaded=%d (%.1f fps) ", frames_loaded_, getAverageLoadFPS());
    }
    
    if (frames_decoded_ > 0) {
        printf("Decoded=%d (%.1f fps) ", frames_decoded_, getAverageDecodeFPS());
    }
    
    if (frames_displayed_ > 0) {
        printf("Displayed=%d (%.1f fps) ", frames_displayed_, getAverageDisplayFPS());
    }
    
    printf("Time=%.1fs\n", getElapsedTime());
}

void PerformanceMonitor::generateReport(char* buffer, size_t buffer_size) const {
    if (!buffer || buffer_size == 0) {
        return;
    }
    
    int offset = 0;
    double total_time = getTotalDuration();
    
    offset += snprintf(buffer + offset, buffer_size - offset,
                      "Performance Report:\n");
    
    if (frames_loaded_ > 0) {
        offset += snprintf(buffer + offset, buffer_size - offset,
                          "  Loaded: %d frames, %.2f fps\n",
                          frames_loaded_, getAverageLoadFPS());
    }
    
    if (frames_decoded_ > 0) {
        offset += snprintf(buffer + offset, buffer_size - offset,
                          "  Decoded: %d frames, %.2f fps\n",
                          frames_decoded_, getAverageDecodeFPS());
    }
    
    if (frames_displayed_ > 0) {
        offset += snprintf(buffer + offset, buffer_size - offset,
                          "  Displayed: %d frames, %.2f fps\n",
                          frames_displayed_, getAverageDisplayFPS());
    }
    
    snprintf(buffer + offset, buffer_size - offset,
             "  Total time: %.2f seconds\n", total_time);
}

// ============ 配置 ============

void PerformanceMonitor::setReportInterval(int interval_ms) {
    report_interval_ms_ = interval_ms;
}

// ============ 内部辅助方法 ============

double PerformanceMonitor::calculateAverageFPS(int frame_count) const {
    if (!is_started_ || frame_count == 0) {
        return 0.0;
    }
    
    double duration = getTotalDuration();
    if (duration <= 0.0) {
        return 0.0;
    }
    
    return frame_count / duration;
}

double PerformanceMonitor::getTotalDuration() const {
    if (!is_started_) {
        return 0.0;
    }
    
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - start_time_);
    
    return duration.count() / 1000.0;
}

// ============ 定时器控制实现 ============

void PerformanceMonitor::setTimerInterval(double seconds) {
    timer_interval_seconds_ = seconds;
    printf("⏱️  Timer interval set to %.2f seconds\n", seconds);
}

void PerformanceMonitor::startTimer() {
    // 如果定时器已经在运行，忽略
    if (timer_running_) {
        printf("⚠️  Timer is already running\n");
        return;
    }
    
    // 初始化增量统计的基准点
    last_frames_loaded_ = frames_loaded_;
    last_frames_decoded_ = frames_decoded_;
    last_frames_displayed_ = frames_displayed_;
    last_timer_trigger_time_ = std::chrono::steady_clock::now();
    
    // 设置运行标志
    timer_running_ = true;
    
    // 启动后台线程
    timer_thread_ = std::thread(&PerformanceMonitor::timerThreadFunction, this);
    
    printf("✅ Timer started (interval: %.2f seconds)\n", timer_interval_seconds_);
}

void PerformanceMonitor::stopTimer() {
    // 如果定时器未运行，直接返回
    if (!timer_running_) {
        return;
    }
    
    // 设置停止标志
    {
        std::lock_guard<std::mutex> lock(timer_mutex_);
        timer_running_ = false;
    }
    
    // 唤醒定时器线程（如果它在等待）
    timer_cv_.notify_one();
    
    // 等待线程退出
    if (timer_thread_.joinable()) {
        timer_thread_.join();
    }
    
    printf("⏹️  Timer stopped\n");
}

void PerformanceMonitor::timerThreadFunction() {
    printf("🧵 Timer thread started\n\n");
    
    while (true) {
        // 等待指定的时间间隔
        {
            std::unique_lock<std::mutex> lock(timer_mutex_);
            
            // 使用 wait_for 实现定时等待，同时可以被 notify_one 中断
            auto wait_duration = std::chrono::duration<double>(timer_interval_seconds_);
            timer_cv_.wait_for(lock, wait_duration);
            
            // 检查是否需要退出
            if (!timer_running_) {
                break;
            }
        }
        
        // 检查再次确认（避免在 wait_for 超时后才设置 timer_running_ = false 的情况）
        if (!timer_running_) {
            break;
        }
        
        // 定时器触发：计算增量统计
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - last_timer_trigger_time_);
        double actual_interval = duration.count() / 1000.0;
        
        // 计算这个时间间隔内的帧数增量
        int loaded_delta = frames_loaded_ - last_frames_loaded_;
        int decoded_delta = frames_decoded_ - last_frames_decoded_;
        int displayed_delta = frames_displayed_ - last_frames_displayed_;
        
        // 计算这个时间间隔内的FPS
        double load_fps = (actual_interval > 0) ? (loaded_delta / actual_interval) : 0.0;
        double decode_fps = (actual_interval > 0) ? (decoded_delta / actual_interval) : 0.0;
        double display_fps = (actual_interval > 0) ? (displayed_delta / actual_interval) : 0.0;
        
        // 打印统计信息
        printf("┌─────────────────────────────────────────────────────┐\n");
        printf("│      ⏱️  过去 %.1f 秒内的性能统计               │\n", actual_interval);
        printf("└─────────────────────────────────────────────────────┘\n");
        
        if (!is_started_) {
            printf("⚠️  Monitor not started yet\n");
        } else {
            // 显示增量统计
            if (displayed_delta > 0 || frames_displayed_ > 0) {
                printf("📺 显示帧数: %d 帧 (%.1f fps) | 累计: %d 帧\n", 
                       displayed_delta, display_fps, frames_displayed_);
            }
            
            if (decoded_delta > 0 || frames_decoded_ > 0) {
                printf("🎬 解码帧数: %d 帧 (%.1f fps) | 累计: %d 帧\n", 
                       decoded_delta, decode_fps, frames_decoded_);
            }
            
            if (loaded_delta > 0 || frames_loaded_ > 0) {
                printf("📥 加载帧数: %d 帧 (%.1f fps) | 累计: %d 帧\n", 
                       loaded_delta, load_fps, frames_loaded_);
            }
            
            printf("⏱️  总运行时间: %.2f 秒\n", getElapsedTime());
        }
        
        printf("\n");
        
        // 更新基准点，为下次统计做准备
        last_frames_loaded_ = frames_loaded_;
        last_frames_decoded_ = frames_decoded_;
        last_frames_displayed_ = frames_displayed_;
        last_timer_trigger_time_ = now;
    }
    
    printf("🧵 Timer thread exited\n");
}

