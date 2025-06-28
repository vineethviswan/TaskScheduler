# 🧵 Task Scheduler

**Language & Tools:** C++20/23, CMake, CLion or Visual Studio (via CMake)

---

### 🎯 Objective

Design and implement a lightweight, modular, and extensible task scheduler in modern C++ that supports asynchronous task execution, delayed and recurring tasks, and clean task management using modern language features.

---

### 📌 Scope of Work

#### ✅ Functional Requirements

- Implement a `Task` abstraction using `std::function<void()>`
- Create a thread-safe task queue
- Develop a thread pool using `std::jthread`
- Support:
  - One-shot tasks
  - Recurring tasks
  - Delayed tasks
  - Graceful shutdown
- Optional: Task dependencies or priorities

#### ⚙️ Non-Functional Requirements

- Modular code using C++ modules (if supported)
- Unit tests with Catch2 or GoogleTest
- clang-tidy integration and CMake presets
- Cross-platform compatibility

---

### 🧠 Modern C++ Features to Use

| Feature              | Purpose                          |
|----------------------|----------------------------------|
| `std::jthread`       | Simplified thread management     |
| `std::stop_token`    | Cooperative cancellation         |
| `co_await` / `co_yield` | Coroutine-based async execution |
| `concepts`           | Constrain template parameters    |
| `ranges`             | Filter/transform task collections|
| `std::expected` / `optional` | Clean error/result handling |
| `modules`            | Fast-compiling, reusable code units |

---

### 📦 Deliverables

- `CMakeLists.txt` with modern configuration
- `TaskScheduler` class with:
  - `addTask()`, `start()`, `stop()` methods
  - Support for delayed and recurring tasks
- Example usage in `main.cpp`
- Unit tests in `tests/`
- Documentation (`README.md`)

---

### 🚀 Stretch Goals

- Task chaining (`.then()` style)
- Executors and Senders (C++23)
- CLI or GUI timeline visualization

---
