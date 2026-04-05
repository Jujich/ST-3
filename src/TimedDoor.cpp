// Copyright 2021 GHA Test Team

#include "TimedDoor.h"

#include <stdexcept>
#ifndef _WIN32
#include <chrono>
#include <thread>
#else
#include <windows.h>
#endif

DoorTimerAdapter::DoorTimerAdapter(TimedDoor& d)
    : door(d), from_timer_(false) {}

void DoorTimerAdapter::Timeout() {
  if (!from_timer_) {
    if (door.timer_) {
      from_timer_ = true;
      door.timer_->tregister(door.getTimeOut(), this);
      from_timer_ = false;
    }
  } else {
    from_timer_ = false;
    if (door.isDoorOpened()) {
      door.throwState();
    }
  }
}

TimedDoor::TimedDoor(int timeout)
    : adapter(new DoorTimerAdapter(*this)),
      iTimeout(timeout),
      isOpened(false),
      timer_(nullptr) {}

TimedDoor::~TimedDoor() {
  delete adapter;
}

bool TimedDoor::isDoorOpened() {
  return isOpened;
}

void TimedDoor::unlock() {
  isOpened = true;
  adapter->Timeout();
}

void TimedDoor::lock() {
  isOpened = false;
}

int TimedDoor::getTimeOut() const {
  return iTimeout;
}

void TimedDoor::throwState() {
  if (isOpened) {
    throw std::runtime_error("Door remained open after timeout");
  }
}

void TimedDoor::setTimer(Timer* timer) {
  timer_ = timer;
}

void Timer::sleep(int delay_ms) {
  if (delay_ms <= 0) {
    return;
  }
#ifndef _WIN32
  std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
#else
  Sleep(static_cast<DWORD>(delay_ms));
#endif
}

void Timer::tregister(int delay, TimerClient* c) {
  client = c;
  sleep(delay);
  if (client != nullptr) {
    client->Timeout();
  }
}
