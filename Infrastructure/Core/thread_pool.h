#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <memory>
#include <functional>
#include <condition_variable>

using Task = std::function<void()>;

using TaskQueue = std::queue<Task>;

class WorkerThread {
private:
	std::thread m_Worker;
	
	TaskQueue m_TaskQueue;

	std::mutex m_TaskQueueMutex;

	std::condition_variable m_ConditionVariable;

	bool m_Terminating{ false };

	void WaitAndExecute() noexcept;

public:
	WorkerThread();

	WorkerThread(const WorkerThread& other) = delete;

	WorkerThread& operator=(const WorkerThread& other) = delete;

	~WorkerThread();

	void AddTask(const Task& task) noexcept;

	void Wait() noexcept;
};

class ThreadPool {
private:
	std::vector<std::unique_ptr<WorkerThread>> m_Workers;

public:
	bool Initialize();

	void Wait() noexcept;

	void AddTask(int workerIndex, Task task) noexcept;

	void AddTask(Task task) noexcept;

	void AddTasks(std::vector<Task> tasks) noexcept;

	size_t GetWorkerCount() const noexcept;
};


#endif //THREAD_POOL_H
