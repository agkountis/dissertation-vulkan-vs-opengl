#include "thread_pool.h"
#include <string>
#include "logger.h"

// WorkerThread ---------------------------------------------------------------------------------
void WorkerThread::WaitAndExecute() noexcept
{
	while (true) {
		Task task;

		{
			std::unique_lock<std::mutex> lock{ m_TaskQueueMutex };

			m_ConditionVariable.wait(lock, [this]() -> bool { 
				return !m_TaskQueue.empty() || m_Terminating; 
			});

			if (m_Terminating) {
				return;
			}

			task = m_TaskQueue.front();
		}

		task();

		{
			std::lock_guard<std::mutex> lock{ m_TaskQueueMutex };

			m_TaskQueue.pop();

			m_ConditionVariable.notify_one();
		}
	}
}

WorkerThread::WorkerThread()
{
	// Have to pass this as an argument because WaitAndExecute is a member function.
	m_Worker = std::thread{ &WorkerThread::WaitAndExecute, this };
}

WorkerThread::~WorkerThread()
{
	if (m_Worker.joinable()) {
		Wait();

		{
			std::lock_guard<std::mutex> lock{ m_TaskQueueMutex };
			m_Terminating = true;
			m_ConditionVariable.notify_one();
		}

		m_Worker.join();
	}
}

void WorkerThread::AddTask(const Task& task) noexcept
{
	std::lock_guard<std::mutex> lock{ m_TaskQueueMutex };
	m_TaskQueue.push(std::move(task));
	m_ConditionVariable.notify_one();
}

void WorkerThread::Wait() noexcept
{
	std::unique_lock<std::mutex> lock{ m_TaskQueueMutex };
	m_ConditionVariable.wait(lock, [this]() -> bool {
		return m_TaskQueue.empty(); 
	});
}

// ThreadPool -----------------------------------------------------------------------------------------------
bool ThreadPool::Initialize()
{
	LOG("Initializing thread pool...");

	/**
	* Get the system's supported thread count.
	*/
	int thread_count = std::thread::hardware_concurrency();

	if (!thread_count) {
		ERROR_LOG("Not able to detect the system's available thread count!");
		return false;
	}

	LOG("Available system threads: " + std::to_string(thread_count));

	LOG("Creating workers...");

	/**
	* Spawn the worker threads.
	*/
	for (int i = 0; i < thread_count; i++) {
		/**
		* The workers will execute an infinite loop function
		* and will wait for a job to enter the job queue. Once a job is in the the queue
		* the threads will wake up to acquire and execute it.
		*/
		m_Workers.push_back(std::make_unique<WorkerThread>());
	}

	return true;
}


void ThreadPool::Wait() noexcept
{
	for (auto& worker : m_Workers) {
		worker->Wait();
	}
}

void ThreadPool::AddTask(int workerIndex, const Task& task) noexcept
{
	m_Workers[workerIndex]->AddTask(std::move(task));
}

size_t ThreadPool::GetWorkerCount() const noexcept
{
	return m_Workers.size();
}
