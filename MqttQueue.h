#pragma once
#include <Arduino.h>

struct Msg { String topic; String payload; };

extern volatile int qHead;
extern volatile int qTail;
extern const int QSIZE;
extern Msg queueMsg[];

void enqueueMsg(const char* topic, const char* buf, unsigned len);
