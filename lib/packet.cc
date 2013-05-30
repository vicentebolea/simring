#include <simring.hh>

packet::packet(){
 fid = offset = length = last = 0;
}

packet::packet(char* line)
{ 
	long v[4];
	toArray(line, v);

	this->fid = v[0];
	this->offset = v[1];
	this->length = v[2]; 
	this->time = v[3];
	this->last = offset + length;
}	

packet::packet(const packet& that)
{
	this->fid = that.fid;
	this->offset = that.offset;
	this->length = that.length;
 	this->last = that.last;
}	

packet& packet::operator=(const packet& that)
{
	if (this == &that)
		return *this;

	this->fid = that.fid;
	this->offset = that.offset;
	this->length = that.length;
 	this->last = that.last;
	return *this;
}

//To check, just compare offsets
int packet::getDistance(const packet& that)
{
	if (this->fid != that.fid)
		return -1;

	if (this->offset < that.offset && this->last > that.offset)
		return this->last - that.offset;

	if (this->offset > that.offset && this->offset < that.last)
		return that.last - this->offset;

	if (this->offset == that.offset){
		if(this->length == that.length)
			return length+1;

		return (this->last > that.last) ? this->last-that.last: that.last-this->last;
	}	
	return -1;
}

double packet::getEMA(){
	return toDouble(fid, offset, length);
}

