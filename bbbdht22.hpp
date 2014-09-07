#ifndef BBBDHT22_HPP_
#define BBBDHT22_HPP_

#include <mutex>
#include <atomic>
#include <thread>

enum class Pru {BPRU0,BPRU1};

class DHT22;
typedef std::shared_ptr<DHT22> DHT22Ptr_t;

class DHT22 
{
public:
   
   DHT22(const std::string& path, const Pru& pru);
   DHT22(const DHT22&) = delete;
   DHT22& operator=(const DHT22&) = delete;
   ~DHT22();

   void start();
   bool is_running();
   float temperature();
   float humidity();
   float error_rate();
   int errors();
   int cycles();
   void halt();
   std::string message();
   
private:
   enum DataField
   {
	   TEMP,
	   HUM,
	   ERROR,
	   CYCLE,
	   HALT,
	   STATUS
   };
   
   void run();
   unsigned int read_from_pru(DataField field);
   void write_to_pru(DataField field, unsigned int value);
   
   static bool constructed_;
   
   std::atomic<bool> halt_;
   std::string path_;
   Pru pru_;
   float temperature_;
   float humidity_;
   int cycles_;
   int errors_;
   std::mutex mtx_;
   unsigned int *data_;
   std::thread process_;
   static const int STOP = 123;
   
};

template<typename Func, typename... Args>
void pruss_wrapper(Func func, const std::string& name, Args... args)
{
	int ret = func(args...);
	if(ret == -1)
	   throw std::runtime_error(name);
}

#endif
