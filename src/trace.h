// #define DEBUG
// #define ASSERTIONS

#ifdef ASSERTIONS
#define ASSERTM(eq, msg) assert(((void)msg, eq))
#define ASSERT(eq) assert(eq)
#else
#define ASSERTM(eq, msg)
#define ASSERT(eq)
#endif

#ifdef DEBUG
#define DBOUT std::cout << "[debug] "
#else
#define DBOUT 0 && std::cout
#endif


#define ERROUT std::cout << "[error] "
#define INFOUT std::cout << "[info] "