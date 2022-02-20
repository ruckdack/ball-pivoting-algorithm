#define DEBUG

#ifdef DEBUG
#define DBOUT std::cout << "[debug] "
#else
#define DBOUT 0 && std::cout
#endif

#define ASSERTM(eq, msg) assert(((void)msg, eq))
#define ASSERT(eq) assert(eq)

#define ERROUT std::cout << "[error] "
#define INFOUT std::cout << "[info] "