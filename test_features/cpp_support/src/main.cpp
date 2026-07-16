#if defined(CONFIG_CPP_SUPPORT_SIMPLE_TEST)
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

#include <exception>
#include <stdexcept>

template <typename T>
class template_class
{
public:
    template_class(T val) : value(val) {}
    T get_value() const { return value; }

private:
    T value;
};

void func_throw_exp()
{
    throw std::runtime_error("This is a test exception");
}

int main(void)
{
    printk("Zephyr with CPP support\n");

    char* ptr = new char[2048];
    memset(ptr, 0, 2048);
    strcpy(ptr, "Hello, Zephyr with C++17!");
    printk("%s\n", ptr);
    delete[] ptr;
    ptr = nullptr;

    std::unique_ptr<char[]> str(new char[2048]);
    memset(str.get(), 0, 2048);
    strcpy(str.get(), "Hello, Zephyr with C++17!");
    printk("%s\n", str.get());
    str.reset();

    std::string cpp_string = "Hello, Zephyr with C++17!";
    printk("%s\n", cpp_string.c_str());

    std::vector<char> cpp_vector = {'H', 'e', 'l', 'l', 'o', ',', ' ', 'Z', 'e', 'p', 'h', 'y', 'r', '!', '\0'};
    printk("%s\n", cpp_vector.data());

    std::unordered_map<std::string, int> cpp_map = {{"one", 1}, {"two", 2}, {"three", 3}};
    for (const auto& pair : cpp_map)
    {
        printk("%s: %d\n", pair.first.c_str(), pair.second);
    }

    auto lambda = [](int a, int b) { return a + b; };
    int result = lambda(5, 3);
    printk("Lambda result: %d\n", result);

    try
    {
        func_throw_exp();
    }
    catch (const std::exception& e)
    {
        printk("Caught exception: %s\n", e.what());
    }

    template_class<int> obj(42);
    printk("Template class value: %d\n", obj.get_value());

    template_class<int> another_obj = std::move(obj);
    printk("Another template class value: %d\n", another_obj.get_value());

    return 0;
}
#endif // CONFIG_CPP_SUPPORT_SIMPLE_TEST

#if defined(CONFIG_CPP_SUPPORT_ADVANCED_TEST)
/*
 * Demonstrates C++ features safe for Zephyr embedded development:
 * - Classes with two-phase initialization (safe pattern)
 * - RAII mutex guard
 * - std::array (no heap allocation)
 * - constexpr for compile-time constants
 * - enum class for type-safe enums
 * - Static C callback bridging to C++ instance method
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <array>
#include <cstring>
#include <cstdint>

LOG_MODULE_REGISTER(cpp_support, LOG_LEVEL_DBG);

/* ─── constexpr: compile-time constants ─────────────────────────── */
constexpr size_t GPS_BUFFER_SIZE = 256;
constexpr uint32_t GPS_BAUD_RATE = 9600;

static_assert(GPS_BUFFER_SIZE % 2 == 0, "Buffer size must be even");

/* ─── enum class: type-safe enumeration ─────────────────────────── */
enum class GpsParseState : uint8_t
{
    IDLE       = 0,
    COLLECTING = 1,
    COMPLETE   = 2,
    ERROR      = 3
};

/* ─── RAII Mutex Guard ───────────────────────────────────────────── */
class MutexGuard
{
public:
    explicit MutexGuard(struct k_mutex &m) noexcept
        : mutex_(m), locked_(k_mutex_lock(&mutex_, K_FOREVER) == 0)
    {}

    ~MutexGuard() noexcept
    {
        if (locked_)
        {
            k_mutex_unlock(&mutex_);
        }
    }

    bool is_locked() const noexcept { return locked_; }

    /* Non-copyable, non-movable */
    MutexGuard(const MutexGuard &)            = delete;
    MutexGuard &operator=(const MutexGuard &) = delete;
    MutexGuard(MutexGuard &&)                 = delete;
    MutexGuard &operator=(MutexGuard &&)      = delete;

private:
    struct k_mutex &mutex_;
    bool locked_;
};

/* ─── Two-phase Init Pattern ─────────────────────────────────────── */
class GpsFrameBuffer
{
public:
    /* Default constructor does NOTHING — safe at global/static init time */
    GpsFrameBuffer() noexcept = default;

    /* Real initialization — called from main(), not at static init time */
    bool init() noexcept
    {
        buf_.fill(0);       /* zero the std::array */
        head_ = 0;
        state_ = GpsParseState::IDLE;
        initialized_ = true;
        return true;
    }

    /* Simulate feeding one byte (e.g., from UART) */
    bool feed(char c) noexcept
    {
        if (!initialized_) {
            return false;
        }

        MutexGuard guard(mutex_);
        if (!guard.is_locked())
        {
            return false;
        }

        switch (state_)
        {
        case GpsParseState::IDLE:
            if (c == '$')
            {
                head_ = 0;
                buf_[head_++] = c;
                state_ = GpsParseState::COLLECTING;
            }
            break;

        case GpsParseState::COLLECTING:
            if (head_ < GPS_BUFFER_SIZE - 1)
            {
                buf_[head_++] = c;
                if (c == '\n')
                {
                    buf_[head_] = '\0';
                    state_ = GpsParseState::COMPLETE;
                }
            }
            else
            {
                state_ = GpsParseState::ERROR;
                head_ = 0;
            }
            break;

        default:
            break;
        }
        return true;
    }

    bool is_complete() const noexcept
    {
        return state_ == GpsParseState::COMPLETE;
    }

    const char *get_frame() const noexcept
    {
        return buf_.data();
    }

    void reset() noexcept
    {
        MutexGuard guard(mutex_);
        head_ = 0;
        state_ = GpsParseState::IDLE;
    }

    GpsParseState get_state() const noexcept { return state_; }

private:
    std::array<char, GPS_BUFFER_SIZE> buf_;   /* Fixed-size — no heap */
    size_t head_ = 0;
    GpsParseState state_ = GpsParseState::IDLE;
    bool initialized_ = false;
    struct k_mutex mutex_ = {};               /* Zero-initialized POD */
};

/* ─── Global object — safe because default constructor does nothing ── */
static GpsFrameBuffer gps_buffer;

/* ─── Main ──────────────────────────────────────────────────────── */
extern "C" int main(void)
{
    LOG_INF("=== C++ Support Demo ===");
    LOG_INF("GPS_BUFFER_SIZE: %zu", GPS_BUFFER_SIZE);
    LOG_INF("GPS_BAUD_RATE: %u", GPS_BAUD_RATE);

    /* Two-phase initialization: real work done here, not in constructor */
    if (!gps_buffer.init())
    {
        LOG_ERR("GpsFrameBuffer init failed");
        return -1;
    }
    LOG_INF("GpsFrameBuffer initialized successfully");

    /* Simulate UART byte-by-byte feeding */
    const char *test_sentence = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,,,,*47\r\n";
    for (size_t i = 0; i < strlen(test_sentence); i++)
    {
        gps_buffer.feed(test_sentence[i]);
    }

    if (gps_buffer.is_complete())
    {
        LOG_INF("Complete NMEA frame captured: %s", gps_buffer.get_frame());
    }
    else
    {
        LOG_WRN("Frame not complete — state: %d",
                static_cast<int>(gps_buffer.get_state()));
    }

    /* Demonstrate enum class type safety */
    GpsParseState state = gps_buffer.get_state();
    switch (state)
    {
    case GpsParseState::COMPLETE:
        LOG_INF("State: COMPLETE");
        break;
    case GpsParseState::ERROR:
        LOG_ERR("State: ERROR");
        break;
    default:
        LOG_WRN("State: other (%d)", static_cast<int>(state));
        break;
    }

    LOG_INF("C++ demo complete");
    return 0;
}
#endif // CONFIG_CPP_SUPPORT_ADVANCED_TEST