#include <simring.hh>

long*
toArray (char* line)
{
  static long token [4];

	token[0] = atol (strtok (line, " "));
	token[1] = atol (strtok (NULL, " "));
	token[2] = atol (strtok (NULL, " "));
	token[3] = atol (strtok (NULL, " "));

	return token;
}

//long*
//toArray (char* line, size_t _size)
//{
//  static long token [_size];
//
//	token[0] = atol (strtok (line, " "));
//
//  for (int i = 1; i < _size; i++)
//		token[i] = atol (strtok (NULL, " "));
//
//	return token;
//}

void
toArray (char* line, long* token)
{
	token[0] = atol (strtok (line, " "));
	token[1] = atol (strtok (NULL, " "));
	token[2] = atol (strtok (NULL, " "));
	token[3] = atol (strtok (NULL, " "));
}

double
toDouble (long *token) {
	return token[0] + (((token[1] + token[2])/2) / 10000000000.0);
}

double
toDouble (long token1, long token2, long token3) {
	return token1 + (((token2 + token3)/2) / 10000000000.0);
}

uint64_t
timediff (struct timeval *end_time, struct timeval *start_time)
{
	return 	(end_time->tv_usec + (1000000 * end_time->tv_sec)) 
				-	(start_time->tv_usec + (1000000 * start_time->tv_sec));
}

void
send_msg (int socket, char* send_data, int msg_len)
{
  send (socket, (char*)&msg_len, sizeof(int), 0); 
  send (socket, send_data, msg_len, 0); 
}

void
recv_msg (int socket, char* recv_data)
{
  int nbytes, bytes_received = 0, r = 0;
  while (r != 4)
		r += recv (socket, (char*) &nbytes+r, sizeof(int)-r,0);

  // read nbytes;
  while (bytes_received < nbytes)
		bytes_received += recv (socket, recv_data+bytes_received, nbytes-bytes_received, 0);

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
rot (int n, int *x, int *y, int rx, int ry) 
{
	if (ry == 0) {
		if (rx == 1) {
			*x = n-1 - *x;
			*y = n-1 - *y;
		}

		//Swap x and y
		int t  = *x;
		*x = *y;
		*y = t;
	}
}

int
xy2d (int n, int x, int y) 
{
	int rx, ry, s, d=0;
	for (s=n/2; s>0; s/=2) {
		rx = (x & s) > 0;
		ry = (y & s) > 0;
		d += s * s * ((3 * rx) ^ ry);
		rot(s, &x, &y, rx, ry);
	}
	return d;
}
