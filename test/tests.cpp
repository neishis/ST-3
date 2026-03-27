// Copyright 2021 GHA Test Team

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <cstdint>
#include <thread>
#include <atomic>
#include <chrono>
#include "TimedDoor.h"

using ::testing::Return;
using ::testing::_;

class MockDoor : public Door {
 public:
  MOCK_METHOD(void, lock, (), (override));
  MOCK_METHOD(void, unlock, (), (override));
  MOCK_METHOD(bool, isDoorOpened, (), (override));
};

class MockTimerClient : public TimerClient {
 public:
  MOCK_METHOD(void, Timeout, (), (override));
};


class TimedDoorInterfaceTest : public ::testing::Test {
 protected:
  MockDoor mockDoor;
};

class TimedDoorTest : public ::testing::Test {
 protected:
  TimedDoor* door;

  void SetUp() override {
    door = new TimedDoor(2);
  }

  void TearDown() override {
    delete door;
  }
};

TEST_F(TimedDoorInterfaceTest, LockCallsStopAndSetsIsOpenedFalse) {
  EXPECT_CALL(mockDoor, lock())
    .Times(1);
  EXPECT_CALL(mockDoor, isDoorOpened())
    .WillRepeatedly(Return(false));
  mockDoor.lock();

  EXPECT_FALSE(mockDoor.isDoorOpened());
}

TEST_F(TimedDoorInterfaceTest, UnlockCallsTregisterAndSetsIsOpenedTrue) {
  EXPECT_CALL(mockDoor, unlock())
    .Times(1);

  EXPECT_CALL(mockDoor, isDoorOpened())
    .WillRepeatedly(Return(true));

  mockDoor.unlock();

  EXPECT_TRUE(mockDoor.isDoorOpened());
}

TEST_F(TimedDoorInterfaceTest, IsDoorOpenedWorks) {
  EXPECT_CALL(mockDoor, isDoorOpened())
    .Times(2)
    .WillRepeatedly(Return(true));

  mockDoor.isDoorOpened();
  mockDoor.isDoorOpened();
}


TEST_F(TimedDoorTest, DoorInitiallyClosed) {
  EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, DoorOpensCorrectly) {
  door->unlock();
  EXPECT_TRUE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, DoorClosesCorrectly) {
  door->unlock();
  door->lock();
  EXPECT_FALSE(door->isDoorOpened());
}

TEST_F(TimedDoorTest, TimerDoesNotTriggerWhenDoorClosed) {
  door->unlock();
  std::this_thread::sleep_for(std::chrono::seconds(1));
  door->lock();

  std::this_thread::sleep_for(std::chrono::seconds(2));

  SUCCEED();
}

TEST_F(TimedDoorTest, TimerRestartsAfterReopen) {
  door->unlock();
  std::this_thread::sleep_for(std::chrono::seconds(1));

  door->lock();
  door->unlock();

  std::this_thread::sleep_for(std::chrono::seconds(1));

  door->lock();

  SUCCEED();
}


TEST(DoorAdapterTest, TimeoutCallsThrowStateWhenDoorOpen) {
  TimedDoor door(1);
  DoorTimerAdapter adapter(door);

  door.unlock();

  EXPECT_THROW(adapter.Timeout(), std::runtime_error);
}

TEST(DoorAdapterTest, TimeoutDoesNothingWhenDoorClosed) {
  TimedDoor door(1);
  DoorTimerAdapter adapter(door);

  door.lock();

  EXPECT_NO_THROW(adapter.Timeout());
}


TEST(TimerTest, TimerCallsTimeout) {
  Timer timer;
  MockTimerClient mock;

  EXPECT_CALL(mock, Timeout())
      .Times(1);

  timer.tregister(1, &mock);

  std::this_thread::sleep_for(std::chrono::seconds(2));

  timer.stop();
}

TEST(TimerTest, TimerDoesNotCallTimeoutIfStopped) {
  Timer timer;
  MockTimerClient mock;

  EXPECT_CALL(mock, Timeout())
      .Times(0);

  timer.tregister(2, &mock);

  std::this_thread::sleep_for(std::chrono::seconds(1));
  timer.stop();

  std::this_thread::sleep_for(std::chrono::seconds(2));
}

TEST(TimerTest, MultipleStartStopWorks) {
  Timer timer;
  MockTimerClient mock;

  EXPECT_CALL(mock, Timeout())
    .Times(1);
  timer.tregister(1, &mock);
  std::this_thread::sleep_for(std::chrono::seconds(2));
  timer.stop();

  ::testing::Mock::VerifyAndClearExpectations(&mock);

  EXPECT_CALL(mock, Timeout())
    .Times(1);
  timer.tregister(1, &mock);
  std::this_thread::sleep_for(std::chrono::seconds(2));
  timer.stop();
}

TEST(TimedDoorTestExtra, GetTimeOut) {
  TimedDoor door(1);
  EXPECT_EQ(door.getTimeOut(), 1);
}

TEST(TimedDoorTestExtra, ThrowStateFunction) {
  TimedDoor door(1);
  EXPECT_THROW(door.throwState(), std::runtime_error);
}

TEST(TimedDoorTestExtra, OpenDoorCauseTimeoutExceptionIndirectly) {
  TimedDoor door(1);

  door.unlock();

  std::this_thread::sleep_for(std::chrono::seconds(2));

  SUCCEED();
}
