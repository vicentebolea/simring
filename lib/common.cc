#include <simring.hh>

uint64_t
timediff (struct timeval *end_time, struct timeval *start_time)
{
	return 	(end_time->tv_usec + (1000000 * end_time->tv_sec)) 
				-	(start_time->tv_usec + (1000000 * start_time->tv_sec));
}

void
send_msg (int socket, const char* send_data)
{
  int msg_len = strlen (send_data);
  send (socket, &msg_len, sizeof(int), 0); 
  send (socket, send_data, msg_len, 0); 
}

void
recv_msg (int socket, char* recv_data)
{
  int nbytes, bytes_received = 0, r = 0;
  while (r != 4)
		r += recv (socket, (char*) &nbytes+r, sizeof(int)-r, MSG_WAITALL);

  // read nbytes;
  while (bytes_received < nbytes)
		bytes_received += recv (socket, recv_data+bytes_received, nbytes-bytes_received, MSG_WAITALL);

  recv_data [bytes_received] = 0;
}

int
poisson (double c)
{	
	int x = 0;
	srand (time(NULL));

  for (double t = .0; t <= 1.0; x++) {
    t -= log ((double)(rand()%1000) / 1000.0) / c;
  }
	return x;
}

//rotate/flip a quadrant appropriately
void
rot (int64_t n, int64_t  *x, int64_t *y, int64_t  rx, int64_t ry) 
{
	if (ry == 0) {
		if (rx == 1) {
			*x = n - 1 - *x;
			*y = n - 1 - *y;
		}

		//Swap x and y
		int64_t t  = *x;
		*x = *y;
		*y = t;
	}
}

int64_t
hilbert (int64_t n, int64_t x, int64_t y) 
{
	int64_t rx, ry, s, d = 0;
	for (s = n / 2; s > 0; s /= 2) {
		rx = (x & s) > 0;
		ry = (y & s) > 0;
		d += s * s * ((3 * rx) ^ ry);
		rot (s, &x, &y, rx, ry);
	}
	return d;
}

uint64_t prepare_input (char* in) {
 int64_t a, b, ret;
 sscanf (in, "%" SCNi64 " %" SCNi64 , &a , &b );
 a /= 2000;
 b /= 2000;
 ret = hilbert (1024, a, b);
 return ret;
}
