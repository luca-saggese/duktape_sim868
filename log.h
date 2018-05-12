#define LOG_DEBUG(fmt, ...) eat_trace("[%d][DBG][%s:%d %s]"fmt, eat_get_task_id(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) eat_trace("[%d][INF][%s:%d %s]"fmt, eat_get_task_id(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) eat_trace("[%d][ERR][%s:%d %s]"fmt, eat_get_task_id(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define LOG_HEX(data, len)  log_hex(data, len);

void log_hex(const char* data, int length);