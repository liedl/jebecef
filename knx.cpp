#include "knx.h"

void (*debug)(const char[]);

void setDebugCallback(void (*cb)(const char[])) {
  debug = cb;
}
