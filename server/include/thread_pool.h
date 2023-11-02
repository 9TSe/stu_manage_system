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
		unsigned short _initsize; //��ʼ�̳߳ش�С
		using Task = function<void()>; //������
		vector<thread> _pool; //�߳�����
		queue<Task> _tasks; //�������
		mutex _lock; //����������е���
		mutex _lockGrow; //�����̵߳���
		condition_variable _task_cv; //
		atomic<bool> _run{true}; //�����Ƿ�����
		atomic<int> _spa_trd_num{0}; //��ǰ�߳�����

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

		//�������ύ���̳߳��е����߳�
		template<typename F, typename... Args> //FΪ��������,argsΪ��������
		auto commit(F&& f, Args&& ...args) -> future<decltype(f(args...))> //auto �ƶ�Ϊf�������ú�ķ���ֵ���� 
		{
			//future�����þ��Ǵ洢�������͵�ֵ
			if (!_run)
				throw runtime_error{"commit auto stop"};
			using RetType = decltype(f(args...)); //RetType���ǵ�ǰ��������ֵ����
			//����һ������Ϊtask,ָ��һ����װ���������͵�,�԰�����Ϊ���������ָ��
			//�������ڰ󶨺�����ַ�Ͳ���(ռλ��)
			auto task = make_shared<packaged_task<RetType()>>(bind(forward<F>(f), forward<Args>(args)...));
			future<RetType> future = task->get_future(); //ȡ��������ֵ
			{
				lock_guard<mutex> lock{_lock};
				//������м���һ��function<void()>�ĺ���,���������һ��lambda���ʽ,������֮��,���ð�װ��(��task����)
				_tasks.emplace([task]() {(*task)(); }); 
			}
			if (_spa_trd_num < 1 && _pool.size() < THREADPOOL_MAX_NUM) //���������������к�,�����ǰû�����߳�
				Add_Thread(1); //�Ӹ����߳�
			_task_cv.notify_one(); //�������������Ķ���
			return future; //���ص�ǰ�����ķ���ֵ
		}

		template<typename F>
		void commit2(F&& f) //�޲������ͺ���
		{
			if (!_run)
				return;
			{
				lock_guard<mutex> lock{_lock};
				_tasks.emplace(forward<F>(f)); //�������ֱ�Ӽ��뺯��
			}
			if (_spa_trd_num < 1 && _pool.size() < THREADPOOL_MAX_NUM)
				Add_Thread(1);
			_task_cv.notify_one();
		}

		int idlCount() { return _spa_trd_num; }
		int thrCount() { return _pool.size(); }

	private:
		void Add_Thread(unsigned short size) //���߳�
		{
			if (!_run)
				throw runtime_error{"Add_Thread stop"};
			unique_lock<mutex> lockgrow{_lockGrow}; //�������̴߳����ĺ���
			for (; _pool.size() < THREADPOOL_MAX_NUM && size > 0; --size) //��������Լ����߳�
			{
				_pool.emplace_back([this] //�߳������������lambda���ʽ,lambda���ʽ�����߳�
					{
						while (true)
						{
							Task task; //ÿ���̲߳���ȡ��������е�����
							{
								unique_lock<mutex> lock{_lock};
								//wait ����false����(�����˻���������в�Ϊ��,���Ըɻ��˾ͽ������)
								_task_cv.wait(lock, [this] {return !_run || !_tasks.empty(); });
								//����������Ϊ�ղ��Ҳ�����,��ô����߳̾�ֱ�ӽ���
								if (!_run && _tasks.empty()) 
									return;
								_spa_trd_num--; //��ǰ�߳���--(��ΪҪִ��������)
								task = move(_tasks.front()); //�ƶ�������ȡ����
								_tasks.pop(); //�������ɾ��һ��
							}
							task(); //ִ������
							//�����ǰ�߳������ڳ�ʼ����ֵ,�߳�Ҳ��ɱ��,����ǰ�߳������ټ���
							if (_spa_trd_num > 0 && _pool.size() > _initsize) 
								return;
							{
								unique_lock<mutex> lock{_lock};
								_spa_trd_num++;//ִ������������,��ǰ�߳���++
							}
						}
					});
				{
					unique_lock<mutex> lock{_lock};
					_spa_trd_num++; //����������һ�����߳�,��ǰ�߳���++
				}
			}
		}
	};
}