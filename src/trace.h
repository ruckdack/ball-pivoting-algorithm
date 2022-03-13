// #define DEBUG

#ifdef DEBUG
#define DBOUT std::cout << "[debug] "
#define ASSERTM(eq, msg) assert(((void)msg, eq))
#define ASSERT(eq) assert(eq)
#else
#define DBOUT 0 && std::cout
#define ASSERTM(eq, msg)
#define ASSERT(eq)
#endif


#define ERROUT std::cout << "[error] "
#define INFOUT std::cout << "[info] "