#include <uniDQP.h>

Query::Query (const packet& p): packet(p) {
  gettimeofday(&scheduledDate, NULL);
  key = fid + offset + length;
}

Query::Query (const Query& that): packet(that) {
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
