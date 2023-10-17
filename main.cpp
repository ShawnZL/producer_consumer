#include <iostream>
#include <mutex> // 锁
#include <queue> // 任务队列
#include <thread>
#include <functional> // bind
#include <condition_variable> // 条件变量

// 使用互斥锁保护共享资源buffer_q，并使用条件变量控制消费者与生产者同步
uint64_t num = 0;
class Producer_consumer {
public:
    Producer_consumer() {
        producer = new std::thread(std::bind(&Producer_consumer::produce, this));
        consumer = new std::thread(std::bind(&Producer_consumer::consume, this));
    }
    ~Producer_consumer() {
        if (producer->joinable()) {
            producer->join();
        }
        if (consumer->joinable()) {
            consumer->join();
        }
        produce_is_not_finished_flag = false;
        std::cout << "producer_consumer end" << std::endl;
    }
    void produce() {
        int produced_production_count = 0;
        while (produce_is_not_finished_flag) {
            std::unique_lock<std::mutex> lockGuard(mutexLock); // 创建一个独占的对象并锁住mutexlock
            while (buffer_q.size() >= max_buffer_q_size) {
                condition.wait(lockGuard);
            }
            uint64_t data = num++;
            buffer_q.push(data);
            std::cout << "task data = " << data << " produced" << std::endl;
            produced_production_count++;
            if (produced_production_count > total_production_count) {
                produce_is_not_finished_flag = false;
            }
            lockGuard.unlock(); // 释放锁
            condition.notify_all(); // 唤醒其他等待condition条件的线程
        }
    }

    void consume() {
        while (consume_is_not_finished_flag) {
            std::unique_lock<std::mutex> lockGuard(mutexLock);
            while (buffer_q.empty()) {
                condition.wait(lockGuard);
            }
            uint64_t data = buffer_q.front();
            buffer_q.pop();
            std::cout << "task data = " << data << " has been consumed" << std::endl;
            if (!produce_is_not_finished_flag && buffer_q.empty()) {
                consume_is_not_finished_flag = false;
            }
            lockGuard.unlock();
            condition.notify_all();
        }
    }
private:
    std::queue<uint64_t> buffer_q{};
    std::mutex mutexLock{};
    std::condition_variable condition{};
    std::thread * producer;
    std::thread * consumer;
    bool produce_is_not_finished_flag = true;
    bool consume_is_not_finished_flag = true;
    uint64_t total_production_count = 100;
    uint64_t max_buffer_q_size = 10;
};

static Producer_consumer * global_PC;

int main() {
    global_PC = new Producer_consumer();
    delete global_PC;
    return 0;
}
