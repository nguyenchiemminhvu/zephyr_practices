/*
 * Zephyr C++ Static Library Linking Demo
 *
 * Demonstrates linking the castle 1.0 C++ library as a static lib
 * into a Zephyr application with full C++ support enabled.
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "castle/events/event_config.h"

#include "castle/callbacks/function.h"
#include "castle/callbacks/callback_registry.h"
#include "castle/callbacks/callback_policy.h"
#include "castle/events/event_dispatcher.h"

#include "castle/callbacks/inplace_function.h"
#include "castle/callbacks/inplace_callback_registry.h"
#include "castle/events/inplace_event_dispatcher.h"

#include "castle/events/sigslot.h"

#include <cstdint>

using namespace castle;

struct timer_expired_tag {};
struct button_pressed_tag {};

void on_timer_expired(uint32_t timestamp)
{
    printk("Timer expired at timestamp: %u\n", timestamp);
}

void on_button_pressed(uint32_t key_id, uint32_t timestamp)
{
    printk("Button pressed: key_id=%u, timestamp=%u\n", key_id, timestamp);
}

using timer_expired_signature = void(uint32_t);
using button_pressed_signature = void(uint32_t, uint32_t);

using event_dispatcher_t = events::event_dispatcher<
    events::event_config<timer_expired_tag, 2, timer_expired_signature>,
    events::event_config<button_pressed_tag, 2, button_pressed_signature>
>;

using inplace_event_dispatcher_t = events::inplace_event_dispatcher<
    events::event_config<timer_expired_tag, 2, timer_expired_signature>,
    events::event_config<button_pressed_tag, 2, button_pressed_signature>
>;

using signal_timer_expired_t = sigslot::signal<2, timer_expired_signature>;
using signal_button_pressed_t = sigslot::signal<2, button_pressed_signature>;

void test_event_dispatcher()
{
    event_dispatcher_t dispatcher;

    callbacks::function_ct<&on_timer_expired> timer_expired_func;
    callbacks::function_ct<&on_button_pressed> button_pressed_func;

    auto sub_timer_expired = dispatcher.register_callback<timer_expired_tag>(&timer_expired_func);
    sub_timer_expired
    ? printk("Successfully registered timer expired callback.\n")
    : printk("Failed to register timer expired callback.\n");

    auto sub_button_pressed = dispatcher.register_callback<button_pressed_tag>(&button_pressed_func);
    sub_button_pressed
    ? printk("Successfully registered button pressed callback.\n")
    : printk("Failed to register button pressed callback.\n");

    dispatcher.dispatch_event<timer_expired_tag>(1234);
    dispatcher.dispatch_event<button_pressed_tag>(1, 1235);

    sub_timer_expired.unsubscribe();
    sub_button_pressed.unsubscribe();
}

void test_inplace_event_dispatcher()
{
    inplace_event_dispatcher_t dispatcher;

    callbacks::inplace_function<timer_expired_signature> timer_expired_func(&on_timer_expired);
    callbacks::inplace_function<button_pressed_signature> button_pressed_func(&on_button_pressed);

    auto sub_timer_expired = dispatcher.register_callback<timer_expired_tag>(std::move(timer_expired_func));
    sub_timer_expired
    ? printk("Successfully registered timer expired callback (inplace).\n")
    : printk("Failed to register timer expired callback (inplace).\n");

    auto sub_button_pressed = dispatcher.register_callback<button_pressed_tag>(std::move(button_pressed_func));
    sub_button_pressed
    ? printk("Successfully registered button pressed callback (inplace).\n")
    : printk("Failed to register button pressed callback (inplace).\n");

    dispatcher.dispatch_event<timer_expired_tag>(1234);
    dispatcher.dispatch_event<button_pressed_tag>(1, 1235);

    sub_timer_expired.unsubscribe();
    sub_button_pressed.unsubscribe();
}

void test_sigslot()
{
    signal_timer_expired_t signal_timer_expired;
    signal_button_pressed_t signal_button_pressed;

    auto timer_expired_conn = signal_timer_expired.connect(&on_timer_expired);
    timer_expired_conn
    ? printk("Successfully connected timer expired signal.\n")
    : printk("Failed to connect timer expired signal.\n");

    auto button_pressed_conn = signal_button_pressed.connect(&on_button_pressed);
    button_pressed_conn
    ? printk("Successfully connected button pressed signal.\n")
    : printk("Failed to connect button pressed signal.\n");

    signal_timer_expired(1234);
    signal_button_pressed(1, 1235);

    timer_expired_conn.disconnect();
    button_pressed_conn.disconnect();
}

/* ── Zephyr application entry point ─────────────────────────────────────── */
int main(void)
{
    printk("=== Zephyr C++ Static Library Linking Demo ===\n");
    printk("    Library: castle 1.0\n\n");

    test_event_dispatcher();
    test_inplace_event_dispatcher();
    test_sigslot();

    printk("\nDone.\n");
    return 0;
}
