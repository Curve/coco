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
    VERSION        1.1.0
    GIT_REPOSITORY "https://github.com/Curve/coco"
  )
  ```

* Using FetchContent
  ```cmake
  include(FetchContent)

  FetchContent_Declare(coco GIT_REPOSITORY "https://github.com/Curve/coco" GIT_TAG v1.1.0)
  FetchContent_MakeAvailable(coco)

  target_link_libraries(<target> cr::coco)
  ```

## 📋 Documentation

### `basic_task`

A simple coroutine primitive. This coroutine is evaluated eagerly and does not return anything.  
The coroutine can be synchronously awaited using it's member function `get`, but cannot be `co_await`'ed.

```cpp
coco::basic_task basic()
{
    co_await something();
}
```

### `task<T>`

This coroutine is evaluated eagerly and returns a result of type `T`.  
Similar to the `basic_task`, the result of this coroutine can be retrieved synchronously and can also be `co_await`'ed.

A convenience member function `then` is also provided.

```cpp
coco::task<int> task()
{
    co_await something();
    co_return 10;
}

coco::basic_task basic()
{
    auto result = co_await task();
    // ...
}

void not_a_coroutine()
{
    auto t      = task();
    auto result = t.get();
    // or...
    t.then([](auto result) { ... });
}
```

### `promise<T> / future<T>`

A replacement for `std::promise<T>` / `std::future<T>` that allows to `co_await` the result.  
Convenience member functions such as `then` and `get` are also provided.

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

coco::basic_task basic()
{
    auto result = co_await compute();
    // ...
}

void not_a_coroutine()
{
    auto fut    = compute();
    auto result = fut.get();
    // or...
    fut.then([](auto result) { ... });
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
