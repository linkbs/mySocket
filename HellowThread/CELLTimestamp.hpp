#ifndef _CELLTimestamp_hpp_
#define _CELLTimestamp_hpp_

#include <chrono>
using namespace std::chrono;

class CELLTimestamp
{
public:
	CELLTimestamp()
	{
		updata();
	}
	~CELLTimestamp()
	{}

	void updata()
	{
		_begin = high_resolution_clock::now();
	}
	//��ȡ��
	double getElapsedSecond()
	{
		return getElapsedTimeInMicroSec() * 0.000001;
	
	}

	//��ȡ����
	double getElapsedTimeInMilliSec() {
	
		return this->getElapsedTimeInMicroSec()*0.001;
	}
	//��ȡ΢��
	long long getElapsedTimeInMicroSec() {
	
	 
		return duration_cast<microseconds>(high_resolution_clock::now() - _begin).count();
	}

protected:

	time_point<high_resolution_clock> _begin;

};

#endif