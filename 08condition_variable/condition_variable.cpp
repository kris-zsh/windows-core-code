#include <iostream>
#include <condition_variable>
#include <queue>
#include <mutex>
#include <thread>
#include <chrono>
using namespace std;
mutex mx;
condition_variable cv;
queue<int> que;
void Productor()
{
	for (;;)
	{
		unique_lock<mutex> lg(mx);

		int val = rand();
		que.push(val);
		cout << "老母鸡下单了 val "<< val << endl;
		lg.unlock();

		cv.notify_all();
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}
}


void consumer()
{
	for(;;)
	{
		unique_lock<mutex> ul(mx);
		cv.wait(ul, []() {return que.size() > 0; });
		int val = que.front();
		que.pop();
		cout << "thread ID " << std::this_thread::get_id() << " 吃鸡蛋" << val << "啦" << endl;
		ul.unlock();

	}
}

int main(int argc, char* argv[])
{
	std::thread t1(Productor);
	std::thread t2(consumer);
	std::thread t3(consumer);
	std::thread t4(consumer);
	std::thread t5(consumer);
	cout << "t1 threadID" << t1.get_id() << endl;
	cout << "t2 threadID" << t2.get_id() << endl;
	cout << "t3 threadID" << t3.get_id() << endl;
	cout << "t4 threadID" << t4.get_id() << endl;
	cout << "t5 threadID" << t5.get_id() << endl;
	t1.join();
	t2.join();
	t3.join();
}
