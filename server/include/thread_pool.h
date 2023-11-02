#pragma once
#include<queue>
#include<thread>
#include<condition_variable>
#include<atomic>
#include<stdexcept>
#include<future>
#include<vector>
#include<functional>
#include<mutex>

namespace std
{
	#define THREADPOOL_MAX_NUM 16
	class threadpool
	{
		unsigned short _initsize; //初始线程池大小
		using Task = function<void()>; //任务函数
		vector<thread> _pool; //线程容器
		queue<Task> _tasks; //任务队列
		mutex _lock; //增加任务队列的锁
		mutex _lockGrow; //增加线程的锁
		condition_variable _task_cv; //
		atomic<bool> _run{true}; //程序是否运行
		atomic<int> _spa_trd_num{0}; //当前线程数量

	public:
		inline threadpool(unsigned short size = 4)
		{
			_initsize = size;
			Add_Thread(size);
		}
		inline ~threadpool()
		{
			_run = false;
			_task_cv.notify_all();
			for (thread& thread : _pool)
			{
				if (thread.joinable())
					thread.join();
			}
		}

		//把任务提交给线程池中的子线程
		template<typename F, typename... Args> //F为函数类型,args为参数类型
		auto commit(F&& f, Args&& ...args) -> future<decltype(f(args...))> //auto 推断为f函数调用后的返回值类型 
		{
			//future的作用就是存储任意类型的值
			if (!_run)
				throw runtime_error{"commit auto stop"};
			using RetType = decltype(f(args...)); //RetType就是当前函数返回值类型
			//创建一个名字为task,指向一个包装任务器类型的,以绑定器作为构造的智能指针
			//绑定器用于绑定函数地址和参数(占位符)
			auto task = make_shared<packaged_task<RetType()>>(bind(forward<F>(f), forward<Args>(args)...));
			future<RetType> future = task->get_future(); //取函数返回值
			{
				lock_guard<mutex> lock{_lock};
				//任务队列加入一个function<void()>的函数,这个函数是一个lambda表达式,解引用之后,调用包装器(即task函数)
				_tasks.emplace([task]() {(*task)(); }); 
			}
			if (_spa_trd_num < 1 && _pool.size() < THREADPOOL_MAX_NUM) //将任务加入任务队列后,如果当前没有子线程
				Add_Thread(1); //加个子线程
			_task_cv.notify_one(); //唤醒条件变量的堵塞
			return future; //返回当前函数的返回值
		}

		template<typename F>
		void commit2(F&& f) //无参数类型函数
		{
			if (!_run)
				return;
			{
				lock_guard<mutex> lock{_lock};
				_tasks.emplace(forward<F>(f)); //任务队列直接加入函数
			}
			if (_spa_trd_num < 1 && _pool.size() < THREADPOOL_MAX_NUM)
				Add_Thread(1);
			_task_cv.notify_one();
		}

		int idlCount() { return _spa_trd_num; }
		int thrCount() { return _pool.size(); }

	private:
		void Add_Thread(unsigned short size) //加线程
		{
			if (!_run)
				throw runtime_error{"Add_Thread stop"};
			unique_lock<mutex> lockgrow{_lockGrow}; //锁整个线程创建的函数
			for (; _pool.size() < THREADPOOL_MAX_NUM && size > 0; --size) //如果还可以加入线程
			{
				_pool.emplace_back([this] //线程容器里面加入lambda表达式,lambda表达式即子线程
					{
						while (true)
						{
							Task task; //每个线程不断取任务队列中的任务
							{
								unique_lock<mutex> lock{_lock};
								//wait 返回false阻塞(不跑了或者任务队列不为空,可以干活了就解除阻塞)
								_task_cv.wait(lock, [this] {return !_run || !_tasks.empty(); });
								//如果任务队列为空并且不跑了,那么这个线程就直接结束
								if (!_run && _tasks.empty()) 
									return;
								_spa_trd_num--; //当前线程数--(因为要执行任务了)
								task = move(_tasks.front()); //移动拷贝获取任务
								_tasks.pop(); //任务队列删除一个
							}
							task(); //执行任务
							//如果当前线程数大于初始构造值,线程也被杀死,即当前线程数不再加了
							if (_spa_trd_num > 0 && _pool.size() > _initsize) 
								return;
							{
								unique_lock<mutex> lock{_lock};
								_spa_trd_num++;//执行玩任务栏了,当前线程数++
							}
						}
					});
				{
					unique_lock<mutex> lock{_lock};
					_spa_trd_num++; //即将创建下一个子线程,提前线程数++
				}
			}
		}
	};
}