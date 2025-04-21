<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="assets/logo-dark.svg">
    <img src="assets/logo-light.svg" width="600">
  </picture>
</p>

## 📃 Description

_Coco_ is a C++20 coroutine library that aims to be convenient and simple to use.  

## 📦 Installation

* Using [CPM](https://github.com/cpm-cmake/CPM.cmake)
  ```cmake
  CPMFindPackage(
    NAME           coco
    VERSION        2.0.0
    GIT_REPOSITORY "https://github.com/Curve/coco"
  )
  ```

* Using FetchContent
  ```cmake
  include(FetchContent)

  FetchContent_Declare(coco GIT_REPOSITORY "https://github.com/Curve/coco" GIT_TAG v2.0.0)
  FetchContent_MakeAvailable(coco)

  target_link_libraries(<target> cr::coco)
  ```

## 📋 Documentation

### `stray`

A simple coroutine primitive. This coroutine is evaluated eagerly and does not return anything.  

```cpp
coco::stray basic()
{
    co_await something();
}
```

### `task<T>`

This coroutine is evaluated eagerly and returns a result of type `T`.  
It is also possible to suspend the task until it is awaited (i.e. make it lazy evaluated) by calling `co_await task<T>::wake_on_await{};` from within it.

```cpp
coco::task<int> task()
{
    co_await something();
    co_return 10;
}

coco::task<int> task_lazy()
{
    co_await coco::task<int>::wake_on_await{};
    co_return 10;
}

coco::stray basic()
{
    auto result      = co_await task();
    auto lazy_result = co_await task_lazy();
    // ...
}

void not_a_coroutine()
{
    auto result = coco::await(task());
    // or...
    coco::then(task(), [](int result) { /* ... */ });
}
```

### `promise<T> / future<T>`

A replacement for `std::promise<T>` / `std::future<T>` that allows to `co_await` the result.  

```cpp
coco::future<int> compute()
{
    auto promise = coco::promise<int>{};
    auto future  = promise.get_future();

 
    std::thread thread{[promise = std::move(promise)]() mutable
                        {
                            std::this_thread::sleep_for(std::chrono::milliseconds(500));
                            promise.set_value(10);
                        }};

    thread.detach();

    return future;
}

coco::stray basic()
{
    auto result = co_await compute();
    // ...
}

void not_a_coroutine()
{
    auto result = coco::await(compute());
    // or...
    coco::then(compute(), [](int result) { /* ... */ });
}
```

### `generator<T>`

A simple generator with iterator support.  
This generator does not support yielding another generator (i.e. no support for `ranges::elements_of`).  
Convenience member functions such as `find_if`, `find` and `skip` are also provided.

```cpp
coco::generator<int> generator()
{
    for (auto i = 0; 10 > i; ++i)
    {
        co_yield i;
    }
}

void not_a_coroutine()
{
    for (const auto& value : generator())
    {
        // ...
    }
}
```

### `forget` / `await` / `then`

Convenience functions for awaitable objects.

```cpp
task<int> some_task();

void not_a_coroutine()
{
    coco::forget(some_task());                      // Safely discard a `task<T>`
    auto result = coco::await(some_task());         // Synchronously await any awaitable
    coco::then(some_task(), [](int) { /* ... */ }); // Invoke callback when awaitable is resolved
}
```
