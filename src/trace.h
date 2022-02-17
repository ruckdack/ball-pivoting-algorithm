#define DEBUG

#ifdef DEBUG
#define DBOUT std::cout << "[debug] "
#else
#define DBOUT 0 && std::cout
#endif

#define ERROUT std::cout << "[error] "
#define INFOUT std::cout << "[info] "