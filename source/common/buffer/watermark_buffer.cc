#include "common/buffer/watermark_buffer.h"

#include "common/common/assert.h"

namespace Envoy {
namespace Buffer {

void WatermarkBuffer::add(const void* data, uint64_t size) {
  OwnedImpl::add(data, size);
  checkHighWatermark();
}

void WatermarkBuffer::add(const std::string& data) {
  OwnedImpl::add(data);
  checkHighWatermark();
}

void WatermarkBuffer::add(const Instance& data) {
  OwnedImpl::add(data);
  checkHighWatermark();
}

void WatermarkBuffer::commit(RawSlice* iovecs, uint64_t num_iovecs) {
  OwnedImpl::commit(iovecs, num_iovecs);
  checkHighWatermark();
}

void WatermarkBuffer::drain(uint64_t size) {
  OwnedImpl::drain(size);
  checkLowWatermark();
}

void WatermarkBuffer::move(Instance& rhs) {
  OwnedImpl::move(rhs);
  checkHighWatermark();
}

void WatermarkBuffer::move(Instance& rhs, uint64_t length) {
  OwnedImpl::move(rhs, length);
  checkHighWatermark();
}

std::tuple<int, int> WatermarkBuffer::read(int fd, uint64_t max_length) {
  std::tuple<int, int> result = OwnedImpl::read(fd, max_length);
  checkHighWatermark();
  return result;
}

uint64_t WatermarkBuffer::reserve(uint64_t length, RawSlice* iovecs, uint64_t num_iovecs) {
  uint64_t bytes_reserved = OwnedImpl::reserve(length, iovecs, num_iovecs);
  checkHighWatermark();
  return bytes_reserved;
}

std::tuple<int, int> WatermarkBuffer::write(int fd) {
  std::tuple<int, int> result = OwnedImpl::write(fd);
  checkLowWatermark();
  return result;
}

void WatermarkBuffer::setWatermarks(uint32_t low_watermark, uint32_t high_watermark) {
  ASSERT(low_watermark < high_watermark || (high_watermark == 0 && low_watermark == 0));
  low_watermark_ = low_watermark;
  high_watermark_ = high_watermark;
  checkHighWatermark();
  checkLowWatermark();
}

void WatermarkBuffer::checkLowWatermark() {
  if (!above_high_watermark_called_ ||
      (high_watermark_ != 0 && OwnedImpl::length() >= low_watermark_)) {
    return;
  }

  above_high_watermark_called_ = false;
  below_low_watermark_();
}

void WatermarkBuffer::checkHighWatermark() {
  if (above_high_watermark_called_ || high_watermark_ == 0 ||
      OwnedImpl::length() <= high_watermark_) {
    return;
  }

  above_high_watermark_called_ = true;
  above_high_watermark_();
}

} // namespace Buffer
} // namespace Envoy
