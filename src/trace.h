// #define DEBUG
// #define ASSERTIONS

#ifdef DEBUG
#define DBOUT std::cout << "[debug] "
#define ASSERTIONS
#else
#define DBOUT 0 && std::cout
#endif

#ifdef ASSERTIONS
#define ASSERTM(eq, msg) assert(((void)msg, eq))
#define ASSERT(eq) assert(eq)
#else
#define ASSERTM(eq, msg)
#define ASSERT(eq)
#endif

#define ERROUT std::cout << "[error] "
#define INFOUT std::cout << "[info] "