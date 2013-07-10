#include <simring.hh>

packet::packet () : point (0) {}
packet::packet (uint64_t p) : point (p) {}	
packet::packet (uint64_t p, double e, double l, double u) 
 : point (p), EMA (e), low_b (l), upp_b (u) {}	
packet::packet (const packet& that) : point (that.point) {}

packet& packet::operator= (const packet& that) {
	if (this == &that)
		return *this;

	this->point = that.point;
	return *this;
}

uint64_t packet::get_point (void) { return this->point; }
