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