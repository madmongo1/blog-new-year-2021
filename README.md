# An easy to use C++ Boost-based HTTP library with connection pool inside

Yes you read this right, Boost finally got internal wget/curl (almost)!

But better read an [article from Boost.Beast maintainer - Richard Hodges](https://cppalliance.org/richard/2021/01/01/RichardsNewYearUpdate.html). Basically this repository just packs his code into a library.

# Requirements

* Boost 1.75
* OpenSSL

# Build

Regular cmake build:
```bash
mkdir build; cd build; cmake .. && cmake --build . -j 8
```
If you need just library and no demo tools pass `-DBUILD_TOOLS=OFF` to cmake.

# Install

```
cmake --install . # from the build directory
```
It will install static library and tools.

# Usage

```c++
http::connection_cache cache(co_await net::this_coro::executor);
auto result = co_await cache.call(http::verb::get, "https://yourwebhost.com");
std::cout << result->body();
// don't forget to catch exceptions
```
