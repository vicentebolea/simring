#include <simring.hh>
#include <assert.h>

int main () {
 uint64_t success = 0;
 uint64_t failed = 0;

 SETcache cache (3, "input1.trash");

 for (int i = 0; i < 6; i++) {
  if (cache.match (i, 0, 5)) success++;
  else                 failed++;
 }
 assert (failed == 6);

 for (int i = 5; i >= 0; i--) {
  if (cache.match (i, 0, 5)) success++;
  else                 failed++;
 }

 assert (success == 3);
}
