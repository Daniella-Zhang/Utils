class Event {
 public:
  void Set() {
    std::lock_guard<std::mutex> lock(event_mutex_);
    event_status_ = true;
    event_cond_.notify_one();
  }

  bool Wait() {
    std::unique_lock<std::mutex> lock(event_mutex_);
    bool result = false;
    if (std::cv_status::timeout !=
        event_cond_.wait_for(lock, std::chrono::seconds(10))) {
      result = true;
    }

    event_status_ = false;
    return result;
  }

  std::mutex event_mutex_;
  std::condition_variable event_cond_;
  bool event_status_ = false;
};

int main() {
  Event done;
  task_runner->PostTask([&done] { done.Set(); });
  done.Wait();
}
