#ifndef BBBDHT22_HPP_
#define BBBDHT22_HPP_

#include <mutex>
#include <atomic>
#include <thread>


class DHT22 
{
public:
   DHT22(const std::string& path);
   ~DHT22();
   void start();
   bool is_running();
   float temperature();
   float humidity();
   float error_rate();
   int errors();
   int cycles();
   void halt();
   
private:
   void run();
   std::atomic<bool> halt_;
   std::string path_;
   float temperature_;
   float humidity_;
   int cycles_;
   int errors_;
   std::mutex mtx_;
   unsigned int *data_;
   std::thread process_;
   static const int STOP = 123;
   
};


#endif
