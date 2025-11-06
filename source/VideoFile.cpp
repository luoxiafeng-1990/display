#include "../include/VideoFile.hpp"
#include <stdio.h>
#include <stdlib.h>  // For atoi
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

// ============ 构造函数 ============

VideoFile::VideoFile()
    : fd_(-1)
    , width_(0)
    , height_(0)
    , bytes_per_pixel_(0)
    , frame_size_(0)
    , file_size_(0)
    , total_frames_(0)
    , current_frame_index_(0)
    , is_open_(false)
    , detected_format_(FileFormat::UNKNOWN)
{
    path_[0] = '\0';  // Initialize path as empty string
}

VideoFile::~VideoFile() {
    close();
}

// ============ 文件操作 ============

bool VideoFile::open(const char* path) {
    if (is_open_) {
        printf("⚠️  Warning: File already opened, closing previous file\n");
        close();
    }
    
    // 保存路径
    strncpy(path_, path, MAX_PATH_LENGTH - 1);
    path_[MAX_PATH_LENGTH - 1] = '\0';
    
    printf("📂 Opening video file: %s\n", path);
    printf("   Mode: Auto-detect format\n");
    
    // 打开文件
    fd_ = ::open(path, O_RDONLY);
    if (fd_ < 0) {
        printf("❌ ERROR: Cannot open file: %s\n", strerror(errno));
        return false;
    }
    
    // 检测文件格式
    detected_format_ = detectFileFormat();
    
    switch (detected_format_) {
        case FileFormat::MP4:
            printf("📹 Detected format: MP4\n");
            if (!parseMP4Header()) {
                ::close(fd_);
                fd_ = -1;
                return false;
            }
            break;
            
        case FileFormat::H264:
            printf("📹 Detected format: H.264\n");
            if (!parseH264Header()) {
                ::close(fd_);
                fd_ = -1;
                return false;
            }
            break;
            
        case FileFormat::H265:
            printf("📹 Detected format: H.265\n");
            printf("❌ ERROR: H.265 format not yet supported\n");
            ::close(fd_);
            fd_ = -1;
            return false;
            
        case FileFormat::AVI:
            printf("📹 Detected format: AVI\n");
            printf("❌ ERROR: AVI format not yet supported\n");
            ::close(fd_);
            fd_ = -1;
            return false;
            
        case FileFormat::RAW:
        case FileFormat::UNKNOWN:
            printf("❌ ERROR: No format magic detected\n");
            printf("   This file may be raw format or unsupported encoded format\n");
            printf("   \n");
            printf("   💡 For raw format, please use:\n");
            printf("      openRaw(path, width, height, bytes_per_pixel)\n");
            ::close(fd_);
            fd_ = -1;
            return false;
    }
    
    // 验证文件
    if (!validateFile()) {
        ::close(fd_);
        fd_ = -1;
        return false;
    }
    
    is_open_ = true;
    current_frame_index_ = 0;
    
    printf("✅ Video file opened successfully\n");
    printf("   Format: ");
    switch (detected_format_) {
        case FileFormat::RAW:  printf("RAW\n"); break;
        case FileFormat::MP4:  printf("MP4\n"); break;
        case FileFormat::H264: printf("H.264\n"); break;
        case FileFormat::H265: printf("H.265\n"); break;
        case FileFormat::AVI:  printf("AVI\n"); break;
        default: printf("UNKNOWN\n"); break;
    }
    printf("   Resolution: %dx%d\n", width_, height_);
    printf("   Bytes per pixel: %d\n", bytes_per_pixel_);
    printf("   Frame size: %zu bytes\n", frame_size_);
    printf("   File size: %ld bytes\n", file_size_);
    printf("   Total frames: %d\n", total_frames_);
    
    return true;
}

bool VideoFile::openRaw(const char* path, int width, int height, int bytes_per_pixel) {
    if (is_open_) {
        printf("⚠️  Warning: File already opened, closing previous file\n");
        close();
    }
    
    // 验证参数
    if (width <= 0 || height <= 0 || bytes_per_pixel <= 0) {
        printf("❌ ERROR: Invalid parameters\n");
        printf("   width=%d, height=%d, bytes_per_pixel=%d\n", 
               width, height, bytes_per_pixel);
        return false;
    }
    
    // 保存参数
    strncpy(path_, path, MAX_PATH_LENGTH - 1);
    path_[MAX_PATH_LENGTH - 1] = '\0';
    width_ = width;
    height_ = height;
    bytes_per_pixel_ = bytes_per_pixel;
    frame_size_ = width_ * height_ * bytes_per_pixel_;
    detected_format_ = FileFormat::RAW;
    
    printf("📂 Opening raw video file: %s\n", path);
    printf("   Format: %dx%d, %d bytes per pixel\n", 
           width_, height_, bytes_per_pixel_);
    printf("   Frame size: %zu bytes\n", frame_size_);
    
    // 打开文件
    fd_ = ::open(path, O_RDONLY);
    if (fd_ < 0) {
        printf("❌ ERROR: Cannot open file: %s\n", strerror(errno));
        return false;
    }
    
    // 验证文件
    if (!validateFile()) {
        ::close(fd_);
        fd_ = -1;
        return false;
    }
    
    is_open_ = true;
    current_frame_index_ = 0;
    
    printf("✅ Raw video file opened successfully\n");
    printf("   File size: %ld bytes\n", file_size_);
    printf("   Total frames: %d\n", total_frames_);
    
    return true;
}

void VideoFile::close() {
    if (!is_open_) {
        return;
    }
    
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
    
    is_open_ = false;
    current_frame_index_ = 0;
    
    printf("✅ Video file closed: %s\n", path_);
}

bool VideoFile::isOpen() const {
    return is_open_;
}

// ============ 读取操作 ============

bool VideoFile::readFrameTo(Buffer& dest_buffer) {
    return readFrameTo(dest_buffer.data(), dest_buffer.size());
}

bool VideoFile::readFrameTo(void* dest_buffer, size_t buffer_size) {
    if (!is_open_) {
        printf("❌ ERROR: File not opened\n");
        return false;
    }
    
    if (!dest_buffer) {
        printf("❌ ERROR: Destination buffer is null\n");
        return false;
    }
    
    if (buffer_size < frame_size_) {
        printf("❌ ERROR: Buffer too small (need %zu, got %zu)\n", 
               frame_size_, buffer_size);
        return false;
    }
    
    // 检查是否超出文件范围
    if (current_frame_index_ >= total_frames_) {
        printf("⚠️  Warning: Reached end of file\n");
        return false;
    }
    
    // 读取一帧数据
    ssize_t bytes_read = read(fd_, dest_buffer, frame_size_);
    if (bytes_read != (ssize_t)frame_size_) {
        printf("❌ ERROR: Read failed (expected %zu, got %zd): %s\n",
               frame_size_, bytes_read, strerror(errno));
        return false;
    }
    
    current_frame_index_++;
    return true;
}

bool VideoFile::readFrameAt(int frame_index, Buffer& dest_buffer) {
    return readFrameAt(frame_index, dest_buffer.data(), dest_buffer.size());
}

bool VideoFile::readFrameAt(int frame_index, void* dest_buffer, size_t buffer_size) {
    if (!seek(frame_index)) {
        return false;
    }
    
    return readFrameTo(dest_buffer, buffer_size);
}

// ============ 导航操作 ============

bool VideoFile::seek(int frame_index) {
    if (!is_open_) {
        printf("❌ ERROR: File not opened\n");
        return false;
    }
    
    if (frame_index < 0 || frame_index >= total_frames_) {
        printf("❌ ERROR: Invalid frame index %d (valid: 0-%d)\n",
               frame_index, total_frames_ - 1);
        return false;
    }
    
    // 计算文件偏移
    off_t offset = (off_t)frame_index * frame_size_;
    
    // 执行seek
    if (lseek(fd_, offset, SEEK_SET) < 0) {
        printf("❌ ERROR: lseek failed: %s\n", strerror(errno));
        return false;
    }
    
    current_frame_index_ = frame_index;
    return true;
}

bool VideoFile::seekToBegin() {
    return seek(0);
}

bool VideoFile::seekToEnd() {
    if (!is_open_) {
        printf("❌ ERROR: File not opened\n");
        return false;
    }
    
    if (lseek(fd_, 0, SEEK_END) < 0) {
        printf("❌ ERROR: lseek to end failed: %s\n", strerror(errno));
        return false;
    }
    
    current_frame_index_ = total_frames_;
    return true;
}

bool VideoFile::skip(int frame_count) {
    int target_frame = current_frame_index_ + frame_count;
    return seek(target_frame);
}

// ============ 信息查询 ============

int VideoFile::getTotalFrames() const {
    return total_frames_;
}

int VideoFile::getCurrentFrameIndex() const {
    return current_frame_index_;
}

size_t VideoFile::getFrameSize() const {
    return frame_size_;
}

long VideoFile::getFileSize() const {
    return file_size_;
}

// ============ 元数据 ============

int VideoFile::getWidth() const {
    return width_;
}

int VideoFile::getHeight() const {
    return height_;
}

int VideoFile::getBytesPerPixel() const {
    return bytes_per_pixel_;
}

const char* VideoFile::getPath() const {
    return path_;
}

// ============ 状态查询 ============

bool VideoFile::hasMoreFrames() const {
    return current_frame_index_ < total_frames_;
}

bool VideoFile::isAtEnd() const {
    return current_frame_index_ >= total_frames_;
}

// ============ 内部辅助方法 ============

bool VideoFile::validateFile() {
    // 获取文件大小
    struct stat st;
    if (fstat(fd_, &st) < 0) {
        printf("❌ ERROR: Cannot get file size: %s\n", strerror(errno));
        return false;
    }
    
    file_size_ = st.st_size;
    
    // 检查文件大小
    if (file_size_ == 0) {
        printf("❌ ERROR: File is empty\n");
        return false;
    }
    
    // 计算总帧数
    total_frames_ = file_size_ / frame_size_;
    
    if (total_frames_ == 0) {
        printf("❌ ERROR: File too small (size=%ld, frame_size=%zu)\n",
               file_size_, frame_size_);
        return false;
    }
    
    // 检查是否有不完整的帧
    if (file_size_ % frame_size_ != 0) {
        printf("⚠️  Warning: File size (%ld) not aligned to frame size (%zu)\n",
               file_size_, frame_size_);
        printf("   Last frame may be incomplete\n");
    }
    
    return true;
}

// ============ 格式检测辅助方法 ============

VideoFile::FileFormat VideoFile::detectFileFormat() {
    unsigned char header[32];
    ssize_t bytes_read = readFileHeader(header, sizeof(header));
    
    if (bytes_read < 16) {
        printf("⚠️  Warning: Cannot read enough header data\n");
        return FileFormat::UNKNOWN;
    }
    
    // 检测 MP4 (ftyp box)
    // MP4 格式: 00 00 00 xx 66 74 79 70
    if (bytes_read >= 8 && 
        header[4] == 0x66 && header[5] == 0x74 && 
        header[6] == 0x79 && header[7] == 0x70) {
        return FileFormat::MP4;
    }
    
    // 检测 AVI (RIFF header)
    // AVI 格式: 52 49 46 46 ... 41 56 49 20
    if (bytes_read >= 12 &&
        header[0] == 0x52 && header[1] == 0x49 && 
        header[2] == 0x46 && header[3] == 0x46 &&
        header[8] == 0x41 && header[9] == 0x56 && 
        header[10] == 0x49 && header[11] == 0x20) {
        return FileFormat::AVI;
    }
    
    // 检测 H.264 (NAL unit start code)
    // H.264 格式: 00 00 00 01 或 00 00 01
    if (bytes_read >= 4) {
        if ((header[0] == 0x00 && header[1] == 0x00 && 
             header[2] == 0x00 && header[3] == 0x01) ||
            (header[0] == 0x00 && header[1] == 0x00 && header[2] == 0x01)) {
            
            // 进一步检查 NAL unit type (第4或第3字节)
            int nal_byte_idx = (header[3] == 0x01) ? 4 : 3;
            if (bytes_read > nal_byte_idx) {
                unsigned char nal_type = header[nal_byte_idx] & 0x1F;
                // NAL types for H.264: 1-21
                if (nal_type >= 1 && nal_type <= 21) {
                    return FileFormat::H264;
                }
                // NAL types for H.265: 0-40
                if (nal_type <= 40) {
                    return FileFormat::H265;
                }
            }
        }
    }
    
    // 没有检测到已知格式
    return FileFormat::UNKNOWN;
}

ssize_t VideoFile::readFileHeader(unsigned char* header, size_t size) {
    if (fd_ < 0) {
        return -1;
    }
    
    // 保存当前文件位置
    off_t current_pos = lseek(fd_, 0, SEEK_CUR);
    
    // 回到文件开头
    if (lseek(fd_, 0, SEEK_SET) < 0) {
        return -1;
    }
    
    // 读取文件头
    ssize_t bytes_read = read(fd_, header, size);
    
    // 恢复文件位置
    lseek(fd_, current_pos, SEEK_SET);
    
    return bytes_read;
}

bool VideoFile::parseMP4Header() {
    printf("⚠️  MP4 format detected but not yet fully supported\n");
    printf("   Please use a tool to extract raw frames, or provide format info\n");
    return false;
}

bool VideoFile::parseH264Header() {
    printf("⚠️  H.264 format detected but not yet fully supported\n");
    printf("   Please use a tool to extract raw frames, or provide format info\n");
    return false;
}

