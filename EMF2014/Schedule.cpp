
#include <debug.h>
#include "Schedule.h"

Event::Event()
    : speaker(NULL), title(NULL)
{}

Event::~Event() {
    delete[] speaker;
    delete[] title;
}

Event& Event::operator=(const Event& that) {
    this->locationId = that.locationId;
    this->typeId = that.typeId;
    this->startTimestamp = that.startTimestamp;
    this->endTimestamp = that.endTimestamp;
    delete[] this->speaker;
    this->speaker = new char[strlen(that.speaker) + 1];
    strcpy(this->speaker, that.speaker);
    delete[] this->title;
    this->title = new char[strlen(that.title) + 1];
    strcpy(this->title, that.title);
    return *this;
}

Schedule::Schedule(Event* aEvents, int32_t aEventCount)
    :mEvents(aEvents), mEventCount(aEventCount)
{}

Schedule::~Schedule() {
    delete[] mEvents;
}

Schedule::Schedule(const Schedule& that) {
    // deep copy
    this->mEventCount = that.mEventCount;
    this->mEvents = new Event[mEventCount];
    for (int i = 0; i < mEventCount; i++) {
        this->mEvents[i] = that.mEvents[i];
    }
}

char* Schedule::getStageName(uint8_t aLocationId) {
    switch (aLocationId) {
        case LOCATION_STAGE_A:      return "Stage A";
        case LOCATION_STAGE_B:      return "Stage B";
        case LOCATION_STAGE_C:      return "Stage C";
        case LOCATION_WORKSHOPS:    return "Workshops";
        case LOCATION_KIDS:         return "Kids";
        case LOCATION_VILLAGES:     return "Villages";
        case LOCATION_LOUNGE:       return "Lounge";
        case LOCATION_BAR:          return "Bar";
        case LOCATION_MODEL_FLYING: return "Model Flying";
        case LOCATION_CATERING:     return "Catering";
        case LOCATION_EMFFM:        return "EMF.FM";
        case LOCATION_OTHER:        return "Other";
    }

    return "Unknown";
}

int32_t Schedule::getEventCount() const {
    return mEventCount;
}

Event* Schedule::getEvents() const {
    return mEvents;
}