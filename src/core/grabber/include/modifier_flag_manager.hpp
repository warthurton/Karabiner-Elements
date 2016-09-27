#pragma once

#include "types.hpp"
#include <CoreGraphics/CoreGraphics.h>
#include <thread>
#include <vector>

namespace manipulator {
class modifier_flag_manager final {
public:
  modifier_flag_manager(void) {
    states_.resize(static_cast<size_t>(krbn::modifier_flag::prepared_modifier_flag_end_));

    states_[static_cast<size_t>(krbn::modifier_flag::caps_lock)] = std::make_unique<state>("caps lock", "⇪");
    states_[static_cast<size_t>(krbn::modifier_flag::left_control)] = std::make_unique<state>("control", "⌃");
    states_[static_cast<size_t>(krbn::modifier_flag::left_shift)] = std::make_unique<state>("shift", "⇧");
    states_[static_cast<size_t>(krbn::modifier_flag::left_option)] = std::make_unique<state>("option", "⌥");
    states_[static_cast<size_t>(krbn::modifier_flag::left_command)] = std::make_unique<state>("command", "⌘");
    states_[static_cast<size_t>(krbn::modifier_flag::right_control)] = std::make_unique<state>("control", "⌃");
    states_[static_cast<size_t>(krbn::modifier_flag::right_shift)] = std::make_unique<state>("shift", "⇧");
    states_[static_cast<size_t>(krbn::modifier_flag::right_option)] = std::make_unique<state>("option", "⌥");
    states_[static_cast<size_t>(krbn::modifier_flag::right_command)] = std::make_unique<state>("command", "⌘");
    states_[static_cast<size_t>(krbn::modifier_flag::fn)] = std::make_unique<state>("fn", "fn");
    states_[static_cast<size_t>(krbn::modifier_flag::hyper)] = std::make_unique<state>("hyper", "hyper");
    states_[static_cast<size_t>(krbn::modifier_flag::hypers)] = std::make_unique<state>("hypers", "hypers"); 
  }

  void reset(void) {
    for (const auto& s : states_) {
      if (s) {
        s->reset();
      }
    }
  }

  void unlock(void) {
    for (const auto& s : states_) {
      if (s) {
        s->unlock();
      }
    }
  }

  enum class operation {
    increase,
    decrease,
    lock,
    unlock,
    toggle_lock,
  };

  void manipulate(krbn::modifier_flag k, operation operation) {
    auto i = static_cast<size_t>(k);
    if (states_[i]) {
      switch (operation) {
      case operation::increase:
        states_[i]->increase();
        break;
      case operation::decrease:
        states_[i]->decrease();
        break;
      case operation::lock:
        states_[i]->lock();
        break;
      case operation::unlock:
        states_[i]->unlock();
        break;
      case operation::toggle_lock:
        states_[i]->toggle_lock();
        break;
      }
    }
  }

  bool pressed(krbn::modifier_flag m) const {
    if (m == krbn::modifier_flag::zero) {
      return true;
    }

    auto i = static_cast<size_t>(m);
    if (i < states_.size() && states_[i]) {
      return states_[i]->pressed();
    }
    return false;
  }

  bool pressed(const std::vector<krbn::modifier_flag>& modifier_flags) {
    // return true if all modifier flags are pressed.
    for (const auto& m : modifier_flags) {
      if (!pressed(m)) {
        return false;
      }
    }
    return true;
  }

  uint8_t get_hid_report_bits(void) const {
    uint8_t bits = 0;

    if (pressed(krbn::modifier_flag::left_control)) {
      bits |= (0x1 << 0);
    }
    if (pressed(krbn::modifier_flag::left_shift)) {
      bits |= (0x1 << 1);
    }
    if (pressed(krbn::modifier_flag::left_option)) {
      bits |= (0x1 << 2);
    }
    if (pressed(krbn::modifier_flag::left_command)) {
      bits |= (0x1 << 3);
    }
    if (pressed(krbn::modifier_flag::right_control)) {
      bits |= (0x1 << 4);
    }
    if (pressed(krbn::modifier_flag::right_shift)) {
      bits |= (0x1 << 5);
    }
    if (pressed(krbn::modifier_flag::right_option)) {
      bits |= (0x1 << 6);
    }
    if (pressed(krbn::modifier_flag::right_command)) {
      bits |= (0x1 << 7);
    }
    if (pressed(krbn::modifier_flag::hyper)) {
      bits |= (0x1 << 0) | (0x1 << 2) | (0x1 << 3);
    }
    if (pressed(krbn::modifier_flag::hypers)) {
      bits |= (0x1 << 0) | (0x1 << 1) | (0x1 << 2) | (0x1 << 3);
    }

    return bits;
  }

  IOOptionBits get_io_option_bits(void) const {
    IOOptionBits bits = 0;
    if (pressed(krbn::modifier_flag::caps_lock)) {
      bits |= NX_ALPHASHIFTMASK;
    }
    if (pressed(krbn::modifier_flag::left_control) ||
        pressed(krbn::modifier_flag::right_control)) {
      bits |= NX_CONTROLMASK;
    }
    if (pressed(krbn::modifier_flag::left_shift) ||
        pressed(krbn::modifier_flag::right_shift)) {
      bits |= NX_SHIFTMASK;
    }
    if (pressed(krbn::modifier_flag::left_option) ||
        pressed(krbn::modifier_flag::right_option)) {
      bits |= NX_ALTERNATEMASK;
    }
    if (pressed(krbn::modifier_flag::left_command) ||
        pressed(krbn::modifier_flag::right_command)) {
      bits |= NX_COMMANDMASK;
    }
    if (pressed(krbn::modifier_flag::fn)) {
      bits |= NX_SECONDARYFNMASK;
    }
    if (pressed(krbn::modifier_flag::hyper)) {
      bits |= NX_CONTROLMASK | NX_ALTERNATEMASK | NX_COMMANDMASK;
    }
    if (pressed(krbn::modifier_flag::hypers)) {
      bits |= NX_CONTROLMASK | NX_SHIFTMASK | NX_ALTERNATEMASK | NX_COMMANDMASK;
    }
    return bits;
  }

  CGEventFlags get_cg_event_flags(CGEventFlags original_flags, krbn::key_code key_code) const {
    original_flags = CGEventFlags(original_flags & ~(kCGEventFlagMaskAlphaShift | kCGEventFlagMaskControl | kCGEventFlagMaskShift | kCGEventFlagMaskAlternate | kCGEventFlagMaskCommand | kCGEventFlagMaskNumericPad | kCGEventFlagMaskSecondaryFn));

    if (pressed(krbn::modifier_flag::caps_lock)) {
      original_flags = CGEventFlags(original_flags | kCGEventFlagMaskAlphaShift);
    }
    if (pressed(krbn::modifier_flag::left_control) ||
        pressed(krbn::modifier_flag::right_control)) {
      original_flags = CGEventFlags(original_flags | kCGEventFlagMaskControl);
    }
    if (pressed(krbn::modifier_flag::left_shift) ||
        pressed(krbn::modifier_flag::right_shift)) {
      original_flags = CGEventFlags(original_flags | kCGEventFlagMaskShift);
    }
    if (pressed(krbn::modifier_flag::left_option) ||
        pressed(krbn::modifier_flag::right_option)) {
      original_flags = CGEventFlags(original_flags | kCGEventFlagMaskAlternate);
    }
    if (pressed(krbn::modifier_flag::left_command) ||
        pressed(krbn::modifier_flag::right_command)) {
      original_flags = CGEventFlags(original_flags | kCGEventFlagMaskCommand);
    }
    if (pressed(krbn::modifier_flag::fn)) {
      original_flags = CGEventFlags(original_flags | kCGEventFlagMaskSecondaryFn);
    }
    if (pressed(krbn::modifier_flag::hyper)) {
      original_flags = CGEventFlags(original_flags | kCGEventFlagMaskControl | kCGEventFlagMaskAlternate | kCGEventFlagMaskCommand);
    }
    if (pressed(krbn::modifier_flag::hypers)) {
      original_flags = CGEventFlags(original_flags | kCGEventFlagMaskControl | kCGEventFlagMaskShift | kCGEventFlagMaskAlternate | kCGEventFlagMaskCommand);
    }
    // Add kCGEventFlagMaskNumericPad, kCGEventFlagMaskSecondaryFn by key_code.
    //
    // Note:
    //   Microsoft Remote Client will fail to treat shift-arrow keys unless these flags.
    //   Alfred will fail f16 key as trigger key unless fn flag.

    switch (key_code) {
    case krbn::key_code::keypad_slash:
    case krbn::key_code::keypad_asterisk:
    case krbn::key_code::keypad_hyphen:
    case krbn::key_code::keypad_plus:
    case krbn::key_code::keypad_enter:
    case krbn::key_code::keypad_1:
    case krbn::key_code::keypad_2:
    case krbn::key_code::keypad_3:
    case krbn::key_code::keypad_4:
    case krbn::key_code::keypad_5:
    case krbn::key_code::keypad_6:
    case krbn::key_code::keypad_7:
    case krbn::key_code::keypad_8:
    case krbn::key_code::keypad_9:
    case krbn::key_code::keypad_0:
    case krbn::key_code::keypad_period:
    case krbn::key_code::keypad_equal_sign:
    case krbn::key_code::keypad_comma:
      original_flags = CGEventFlags(original_flags | kCGEventFlagMaskNumericPad);
      break;

    case krbn::key_code::right_arrow:
    case krbn::key_code::left_arrow:
    case krbn::key_code::down_arrow:
    case krbn::key_code::up_arrow:
      original_flags = CGEventFlags(original_flags | kCGEventFlagMaskNumericPad | kCGEventFlagMaskSecondaryFn);
      break;

    case krbn::key_code::f1:
    case krbn::key_code::f2:
    case krbn::key_code::f3:
    case krbn::key_code::f4:
    case krbn::key_code::f5:
    case krbn::key_code::f6:
    case krbn::key_code::f7:
    case krbn::key_code::f8:
    case krbn::key_code::f9:
    case krbn::key_code::f10:
    case krbn::key_code::f11:
    case krbn::key_code::f12:
    case krbn::key_code::f13:
    case krbn::key_code::f14:
    case krbn::key_code::f15:
    case krbn::key_code::f16:
    case krbn::key_code::f17:
    case krbn::key_code::f18:
    case krbn::key_code::f19:
    case krbn::key_code::f20:
    case krbn::key_code::f21:
    case krbn::key_code::f22:
    case krbn::key_code::f23:
    case krbn::key_code::f24:
      original_flags = CGEventFlags(original_flags | kCGEventFlagMaskSecondaryFn);
      break;

    default:
      break;
    }

    return original_flags;
  }

private:
  class state final {
  public:
    state(const std::string& name, const std::string& symbol) : name_(name),
                                                                symbol_(symbol),
                                                                count_(0),
                                                                lock_count_(0) {}

    const std::string& get_name(void) const { return name_; }
    const std::string& get_symbol(void) const { return symbol_; }

    bool pressed(void) {
      std::lock_guard<std::mutex> guard(mutex_);

      return (count_ + lock_count_) > 0;
    }

    void reset(void) {
      std::lock_guard<std::mutex> guard(mutex_);

      count_ = 0;
    }

    void increase(void) {
      std::lock_guard<std::mutex> guard(mutex_);

      ++count_;
    }

    void decrease(void) {
      std::lock_guard<std::mutex> guard(mutex_);

      --count_;
    }

    void lock(void) {
      std::lock_guard<std::mutex> guard(mutex_);

      lock_count_ = 1;
    }

    void unlock(void) {
      std::lock_guard<std::mutex> guard(mutex_);

      lock_count_ = 0;
    }

    void toggle_lock(void) {
      std::lock_guard<std::mutex> guard(mutex_);

      lock_count_ = lock_count_ == 0 ? 1 : 0;
    }

  private:
    std::string name_;
    std::string symbol_;
    int count_;
    int lock_count_;

    std::mutex mutex_;
  };

  std::vector<std::unique_ptr<state>> states_;
};
}
