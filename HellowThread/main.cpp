#include<iostream>
#include<thread>
#include<mutex>  //Ыј
#include<atomic>//д­зг
#include "CELLTimestamp.hpp"
using namespace std;

mutex m;
const int tCount = 4;
atomic<int> sum = 0;
void workFun(int index)
{ 
	
	for (int i = 0;i < 20000000;i++) {
		//lock_guard<mutex> lg(m);

		sum++;
	}
	
}


int main() {

	thread t[tCount];
	for (int i = 0;i < tCount;i++) {
	  
		t[i] = thread(workFun, i);
	
	}
	CELLTimestamp tTime;
	for (int i = 0;i < tCount;i++) {
	  
		t[i].join();
	}

	cout << tTime.getElapsedTimeInMilliSec() << ",sum=" << sum << endl;

	tTime.updata();
	for (int i = 0;i < 80000000;i++) 
	{
	
		sum++;
	}
	cout << tTime.getElapsedTimeInMilliSec() << ",sum" << sum << endl;
	cout << "Hellow,main thread" << endl;
	 return 0;

}