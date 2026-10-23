# Async Strings Handler

Динамическая C++ библиотека для асинхронной обработки строк. Библиотека
принимает задачи, приводит строки к нижнему регистру и добавляет задержку
50 мс, имитируя длительную операцию.

Проект содержит:

- `AsyncStringsHandler` — shared library (`.dll`, `.so` или `.dylib`);
- пример приложения-клиента;
- автоматические тесты;
- benchmark для сравнения количества worker-потоков;
- CMake-конфигурацию для Windows, Linux и macOS;
- конфигурации `clang-format` и `clang-tidy`.

## Структура проекта

```text
include/
  async_strings_handler/
    AsyncStringsHandler.hpp    # публичный API
src/
  AsyncStringsHandler.cpp      # реализация библиотеки
examples/
  client/                      # демонстрационное приложение
tests/                         # автоматические тесты
benchmarks/                    # benchmark
cmake/                         # общие CMake-модули
scripts/                       # команды сборки, тестов и benchmark
```

Каталог `build/` является временным и не хранится в репозитории.

## Требования

- CMake 3.20 или новее;
- C++17 compiler;
- Ninja или другой генератор CMake;
- для Windows подойдёт MinGW или Visual Studio;
- `clang-format` и `clang-tidy` опциональны.

## Сборка через CMake Presets

Рекомендуемый вариант:

```bash
cmake --preset default
cmake --build --preset default
ctest --preset default
```

Preset `default` использует Ninja и конфигурацию Debug.

Для установки `Ninja` можно воспользоваться `WinGet` на `Windows`

```powershell
winget install ninja
```
Для PowerShell с MinGW можно использовать скрипт:

```powershell
.\scripts\build.ps1
```

Для Linux/macOS:

```bash
./scripts/build.sh
```

Скрипты не зависят от текущего каталога: их можно запускать из корня
проекта или из каталога `scripts`.

## Запуск примера

После сборки:

```powershell
.\build\examples\client\async_strings_handler_example.exe
```

На Linux/macOS:

```bash
./build/examples/client/async_strings_handler_example
```

## Тесты

```powershell
.\scripts\test.ps1
```

или:

```bash
./scripts/test.sh
```

Тесты проверяют обработку задач, получение результата по `TaskId`, пустую
строку, одновременные вызовы из нескольких клиентских потоков и повторный
вызов `Shutdown()`.

## Benchmark

```powershell
.\scripts\benchmark.ps1
```

или:

```bash
./scripts/benchmark.sh
```

Benchmark принимает количество задач первым аргументом и по умолчанию
использует 100:

```bash
./build/benchmarks/async_strings_handler_benchmark 500
```

Сравнивается обработка с 1, 2 и 4 worker-потоками. Результаты benchmark
зависят от процессора, планировщика ОС и фоновой нагрузки, поэтому их нужно
использовать для сравнения конфигураций на одной машине, а не как абсолютную
характеристику производительности.

## Статический анализ и форматирование

Форматирование:

```bash
clang-format -i include/async_strings_handler/AsyncStringsHandler.hpp
clang-format -i src/AsyncStringsHandler.cpp
clang-format -i examples/client/main.cpp
clang-format -i tests/AsyncStringsHandlerTests.cpp
clang-format -i benchmarks/AsyncStringsHandlerBenchmark.cpp
```

Проверка `clang-format` без изменения файлов:

```bash
clang-format --dry-run --Werror <file>
```

Сборка с `clang-tidy`:

```bash
cmake --preset clang-tidy
cmake --build --preset clang-tidy
```

Если `clang-tidy` не установлен, этот preset завершится с понятной ошибкой.
Сам анализ включён опционально и не мешает обычной сборке.

## Публичный API

```cpp
AsyncStringsHandler::StringsHandler handler(4);

const auto taskId = handler.SubmitTask("HELLO");
const auto result = handler.GetResult(taskId);
```

Если количество worker-потоков не передано или равно `0`, библиотека выбирает
значение на основе `std::thread::hardware_concurrency()`.

## Ограничения текущей версии

Текущая реализация обработки регистра использует `std::tolower` и пока
ориентирована на ASCII. Корректная обработка Unicode/UTF-8 будет отдельным
этапом развития библиотеки.
