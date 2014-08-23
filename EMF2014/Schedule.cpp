
#include "Schedule.h"

Schedule::Schedule(Event* aEvents, int32_t aEventCount)
    :mEvents(aEvents), mEventCount(aEventCount)
{}

Schedule::~Schedule() {
    delete mEvents;
}

Schedule::Schedule(const Schedule& that) {
    // deep copy
    this->mEventCount = that.mEventCount;
    this->mEvents = new Event[mEventCount];
    for (int i = 0; i < mEventCount; i++) {
        this->mEvents[i] = that.mEvents[i];
    }
}

int32_t Schedule::getEventCount() const {
    return mEventCount;
}

Event* Schedule::getEvents() const {
    return mEvents;
}