#include <uniDQP.h>

server::server(packet& p, double alpha)
{
	this->ema = (1-alpha)*p.getEMA();
	this->alpha = alpha;
}

double server::getDistance(packet& p)
{
	double pEMA = p.getEMA();
	return this->ema > pEMA ? this->ema - pEMA: pEMA - this->ema;
}

void server::updateEMA(packet& p)
{
	double pEMA = p.getEMA();
	this->ema = (1.0 - alpha)*this->ema + alpha*pEMA;
}
