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

Timer::Timer() : client(nullptr), running(false) {}

void Timer::sleep(int s) {
  std::this_thread::sleep_for(std::chrono::seconds(s));
}

void Timer::tregister(int timeout, TimerClient* c) {
  stop();

  client = c;
  running = true;

  worker = std::thread([this, timeout]() {
    int elapsed = 0;
    try {
      while (running && elapsed < timeout) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        elapsed++;
      }

      if (running && client) {
        client->Timeout();
      }
    } catch (...) {
      eptr = std::current_exception();
    }
  });
}

void Timer::stop() {
  running = false;
  if (worker.joinable()) {
    worker.join();
  }
}

Timer::~Timer() {
  stop();
}


// ================= TimedDoor =================

TimedDoor::TimedDoor(int timeout) : iTimeout(timeout), isOpened(false) {
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
  throw std::runtime_error("Door is open too long!");
}
