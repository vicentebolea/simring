#include <uniDQP.h>

Query::Query (const packet& p): packet(p) {
  gettimeofday(&scheduledDate, NULL);
}

Query::Query (const Query& that): packet(that) {
  //queryPoint = that.queryPoint;
  scheduledDate = that.scheduledDate;
  startDate = that.startDate;
  finishedDate = that.finishedDate;
}

void Query::setStartDate() {
  gettimeofday (&startDate, NULL);
}

void Query::setFinishedDate() {
  gettimeofday (&finishedDate, NULL);
}

uint64_t Query::getWaitTime() {
  return timediff (&startDate, &scheduledDate);
}

uint64_t Query::getExecTime() {
  return timediff (&finishedDate, &startDate);
}