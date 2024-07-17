#pragma once

#include <mutex>
#include <queue>
#include <functional>
#include <vector>
#include <condition_variable>
#include <future>

class ThreadPool
{
public:
	ThreadPool(size_t numThreads = std::jthread::hardware_concurrency())
	{
		for (size_t i = 0; i < numThreads; ++i) {
			workers.emplace_back([this](std::stop_token stop_token) { WorkerThread(stop_token); });
		}
	}

	~ThreadPool()
	{
		{
			std::scoped_lock lock(queueMutex);
			stopping = true;
		}

		for (auto& worker : workers)
			worker.request_stop();

		condition.notify_all();

		for (auto& worker : workers) {
			if (worker.joinable()) {
				worker.join();
			}
		}
	}

	template<class F, class... Args>
	auto Enqueue(F&& f, Args&&... args) -> std::future<typename std::invoke_result_t<F, Args...>> {
		using return_type = typename std::invoke_result_t<F, Args...>;

		auto task = std::make_shared<std::packaged_task<return_type()>>(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
		);

		std::future<return_type> res = task->get_future();
		{
			std::scoped_lock lock(queueMutex);

			if (stopping)
				throw std::runtime_error("Enqueue on stopped ThreadPool");

			tasks.emplace([task]() { (*task)(); });
		}
		condition.notify_one();
		return res;
	}

private:

	void WorkerThread(std::stop_token stop_token) {
		while (true) {
			std::function<void()> task;
			{
				std::unique_lock lock(queueMutex);
				condition.wait(lock, stop_token, [this] { return stopping || !tasks.empty(); });
				if (tasks.empty() && stop_token.stop_requested())
					return;
				if (!tasks.empty())
				{
					task = std::move(tasks.front());
					tasks.pop();
				}
			}
			if(task)
				task();
		}
	}

	std::vector <std::jthread> workers;
	std::queue<std::function<void()>> tasks;
	std::mutex queueMutex;
	std::condition_variable_any condition;
	bool stopping = false;
};