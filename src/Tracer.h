#ifndef TRACER_H
#define TRACER_H

#include <ctime>
#include <string>

enum LogLevel
{
  DBG,
  WRN,
  ERR
};

class Tracer
{
public:
  Tracer(const std::string & lvl)
  {
    char buf[32] = {};
    time_t t = time(nullptr);
    tm * ptime = nullptr;
    ptime = gmtime(&t);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ptime);
    std::cout << buf << " [" << lvl << "] ";
  }

  ~Tracer() { std::cout << "\n"; }

  template<class T>
  std::ostream & operator <<(const T & val)
  {
    return std::cout << val;
  }
};

#define TRACE(lvl) Tracer(#lvl)

#endif // TRACER_H