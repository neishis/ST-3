// Copyright 2021 GHA Test Team
#include "TimedDoor.h"

#include <thread>
#include <chrono>
#include <atomic>
#include <stdexcept>
#include <exception>

// ================= DoorTimerAdapter =================

DoorTimerAdapter::DoorTimerAdapter(TimedDoor& d) : door(d) {}

void DoorTimerAdapter::Timeout() {
  if (door.isDoorOpened()) {
    door.throwState();
  }
}

// ================= Timer =================

Timer::Timer() : client(nullptr), is_active(false) {}

void Timer::sleep(int s) {
  std::this_thread::sleep_for(std::chrono::seconds(s));
}

void Timer::tregister(int timeout, TimerClient* c) {
  stop();

  client = c;
  is_active = true;

  timer_thread = std::thread([this, timeout]() {
    for (int i = 0; i < timeout; ++i) {
      sleep(1);
      if (!is_active) {
        return;
      }
    }

    try {
      if (is_active && client) {
        client->Timeout();
      }
    } catch (...) {
    }
  });
}

void Timer::stop() {
  is_active = false;
  if (timer_thread.joinable()) {
    timer_thread.join();
  }
}

Timer::~Timer() {
  stop();
}

// ================= TimedDoor =================

TimedDoor::TimedDoor(int delay) : iTimeout(delay), isOpened(false) {
  adapter = new DoorTimerAdapter(*this);
}

TimedDoor::~TimedDoor() {
  timer.stop();
  delete adapter;
}

bool TimedDoor::isDoorOpened() {
  return isOpened;
}

void TimedDoor::unlock() {
  isOpened = true;
  timer.tregister(iTimeout, adapter);
}

void TimedDoor::lock() {
  isOpened = false;
  timer.stop();
}

int TimedDoor::getTimeOut() const {
  return iTimeout;
}

void TimedDoor::throwState() {
  throw std::runtime_error("Timeout: the door has been left open!");
}
