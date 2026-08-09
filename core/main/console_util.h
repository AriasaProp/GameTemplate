#ifndef CONSOLE_UTIL_INCLUDED_
#define CONSOLE_UTIL_INCLUDED_

#include <stdio.h>

#define RESET       "\033[0m"
#define BLACK       "\033[30m"        /* Black */
#define RED         "\033[31m"        /* Red */
#define GREEN       "\033[32m"        /* Green */
#define YELLOW      "\033[33m"        /* Yellow */
#define BLUE        "\033[34m"        /* Blue */
#define MAGENTA     "\033[35m"        /* Magenta */
#define CYAN        "\033[36m"        /* Cyan */
#define WHITE       "\033[37m"        /* White */
#define BOLDBLACK   "\033[1m\033[30m" /* Bold Black */
#define BOLDRED     "\033[1m\033[31m" /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m" /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m" /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m" /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m" /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m" /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m" /* Bold White */

#define PRINT_LOG(...) fprintf(stdout, CYAN   "[LOG] " RESET __VA_ARGS__ "\n")
#define PRINT_INF(...) fprintf(stdout, WHITE  "[INF] " RESET __VA_ARGS__ "\n")
#define PRINT_WRN(...) fprintf(stdout, YELLOW "[WRN] " RESET __VA_ARGS__ " at " __FILE__ ":" __LINE__ "\n")
#define PRINT_ERR(...) fprintf(stderr, RED    "[ERR] " RESET __VA_ARGS__ " at " __FILE__ ":" __LINE__ "\n")

#endif // CONSOLE_UTIL_INCLUDED_
