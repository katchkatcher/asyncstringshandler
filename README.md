# Async Strings Handler

Динамическая C++17 библиотека для асинхронной обработки строк. Библиотека
принимает задачи, приводит поддерживаемые буквы к нижнему регистру и добавляет
задержку 50 мс, имитируя длительную операцию.

## Статус тестового задания

| Требование | Статус |
| --- | --- |
| Динамическая библиотека (`.dll`, `.so`, `.dylib`) | Выполнено |
| Асинхронная постановка задач в очередь | Выполнено |
| Получение результата по `TaskId` | Выполнено |
| Приведение ASCII и кириллицы к нижнему регистру | Выполнено |
| Задержка 50 мс для каждой задачи | Выполнено |
| Клиент с `N` потоками из аргумента командной строки | Выполнено |
| Каждый клиентский поток отправляет `X` строк | Выполнено, `X = 5` |
| Складирование результатов в общий контейнер | Выполнено |

Пример-клиент принимает `N` из командной строки, создаёт `N` клиентских
потоков, отправляет по пять hardcoded-строк от каждого потока и складывает
полученные результаты в общий `std::vector`, защищённый mutex.

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

Каталоги `build/` и `build-clang-tidy/` являются временными и не хранятся в
репозитории.

## Требования

- CMake 3.20 или новее;
- C++17 compiler;
- Ninja или другой генератор CMake;
- для Windows подойдёт MinGW или Visual Studio;
- `clang-format`, `clang-tidy` и `cppcheck` опциональны.

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
.\build\examples\client\async_strings_handler_example.exe 4
```

На Linux/macOS:

```bash
./build/examples/client/async_strings_handler_example 4
```

Аргумент `4` означает четыре клиентских потока. Каждый поток отправляет пять
строк, поэтому ожидается 20 результатов.

## Тесты

```powershell
.\scripts\test.ps1
```

или:

```bash
./scripts/test.sh
```

Тесты проверяют обработку задач, получение результата по `TaskId`, пустую
строку, UTF-8 и обработку невалидного UTF-8, одновременные вызовы из
нескольких клиентских потоков, повторное получение результата и повторный
вызов `Shutdown()`.

## Continuous Integration

GitHub Actions проверяет проект на каждом `push` и `pull_request`:

- `ubuntu-latest`;
- `macos-latest`;
- `windows-latest`.

Для каждой платформы CI выполняет:

```text
cmake --preset default
cmake --build --preset default --parallel
ctest --preset default --output-on-failure
```

После тестов запускаются пример с четырьмя клиентскими потоками и benchmark
с 100 задачами. Отдельная job на Ubuntu собирает проект с `clang-tidy` и
`cppcheck`, а затем также запускает CTest.

Workflow находится в
[.github/workflows/ci.yml](.github/workflows/ci.yml). Локальный результат,
полностью совпадающий с основной CI-проверкой, можно получить командами
сборки и тестирования из раздела выше. Для статического анализа используются
команды:

```bash
cmake --preset clang-tidy
cmake --build --preset clang-tidy --parallel
ctest --test-dir build-clang-tidy --output-on-failure
```

## Benchmark

```powershell
.\scripts\benchmark.ps1
```

или:

```bash
./scripts/benchmark.sh
```

Benchmark принимает количество задач первым аргументом и по умолчанию
использует 40:

```bash
./build/benchmarks/async_strings_handler_benchmark 500
```

Сравнивается обработка с 1, 2, 4, 8, 16, 32 и 64 worker-потоками. Результаты benchmark
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

Если `clang-tidy` или `cppcheck` не установлен, этот preset завершится с понятной ошибкой.
Сам анализ включён опционально и не мешает обычной сборке.

## Публичный API

```cpp
AsyncStringsHandler::StringsHandler handler(4);

const auto taskId = handler.SubmitTask("HELLO");
const auto result = handler.GetResult(taskId);
```

Если количество worker-потоков не передано или равно `0`, библиотека выбирает
значение на основе `std::thread::hardware_concurrency()`.

## Семантика API

`SubmitTask` принимает задачу до начала остановки и возвращает уникальный
`TaskId`. `GetResult` блокируется до завершения конкретной задачи. После
успешного получения результат удаляется из внутреннего хранилища, а повторный
вызов для того же `TaskId` выбрасывает `std::logic_error`. Ошибка обработки
не маскируется строкой: `GetResult` повторно выбрасывает исключение,
возникшее в worker-потоке. `Shutdown` запрещает новые задачи, но даёт уже
принятым задачам завершиться и дожидается worker-потоков.

## Ограничения текущей версии

Библиотека проверяет входные данные как UTF-8 и приводит к нижнему регистру
ASCII и базовую кириллицу, включая `Ё`. Это не полноценная Unicode case
mapping-реализация: для остальных алфавитов и специальных Unicode-правил
поддержка пока не гарантируется.

При невалидном UTF-8 worker-поток не завершается аварийно: исключение
сохраняется для задачи и повторно выбрасывается из `GetResult`.

На Windows консоль должна быть переведена в UTF-8 перед выводом текста на
экран, иначе отображение кириллицы будет ломаться даже при корректной
обработке строк внутри библиотеки.
