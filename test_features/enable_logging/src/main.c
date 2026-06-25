#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(test_logging, LOG_LEVEL_DBG);

int main(void)
{
    printk("=== Start of the test_enable_logging application ===\n");

    LOG_ERR("This is an error message");
    LOG_WRN("This is a warning message");
    LOG_INF("This is an info message");

#ifdef CONFIG_LOG
    LOG_DBG("This is a debug message");
#endif

    // print a long log line exceed 128 bytes
    LOG_INF("This is a long log message that exceeds 128 bytes. It is used to test the logging functionality in Zephyr RTOS. The message should be properly handled and displayed without any issues.");

    return 0;
}