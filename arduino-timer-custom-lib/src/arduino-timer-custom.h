#pragma once
#include <stdint.h>
#include <stddef.h>
#include <Arduino.h>

template <typename T = void*>
class Timer {
public:
  using Callback = bool (*)(T);
  struct Task {
    bool active = false;
    uint32_t time = 0;
    uint32_t interval = 0;
    bool repeat = false;
    Callback callback = nullptr;
    T context = nullptr;
  };

  Timer() : tasks_{}, now_{0} {}

  Task* in(uint32_t interval, Callback callback, T context = nullptr) {
    return schedule(interval, callback, context, false);
  }

  Task* every(uint32_t interval, Callback callback, T context = nullptr) {
    return schedule(interval, callback, context, true);
  }

  void cancel(Task* task) {
    if (task) task->active = false;
  }

  bool tick() {
    now_ = millis();
    bool ran = false;

    for (auto& task : tasks_) {
      if (!task.active || now_ < task.time) continue;

      ran = true;
      bool keep = task.callback(task.context);
      if (task.repeat && keep) {
        task.time += task.interval;
      } else {
        task.active = false;
      }
    }

    return ran;
  }

  long remaining(Task* task) {
    if (!task || !task->active) return -1;
    long diff = (long)(task->time - millis());
    return diff >= 0 ? diff : 0;
  }

private:
  static constexpr size_t MAX_TASKS = 10;
  Task tasks_[MAX_TASKS];
  uint32_t now_;

  Task* schedule(uint32_t interval, Callback callback, T context, bool repeat) {
    for (auto& task : tasks_) {
      if (!task.active) {
        task.active = true;
        task.time = millis() + interval;
        task.interval = interval;
        task.repeat = repeat;
        task.callback = callback;
        task.context = context;
        return &task;
      }
    }
    return nullptr;
  }
};

template <typename T = void*>
Timer<T> timer_create_default() {
  return Timer<T>();
}
