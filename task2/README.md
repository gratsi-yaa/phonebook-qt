# Телефонный справочник: задача 2 (графический интерфейс Qt, хранение в файле или PostgreSQL)

Приложение на Qt 6 Widgets, построенное на базе логики из задачи 1. Контакты могут храниться либо в текстовом файле, либо в базе данных PostgreSQL; выбор способа хранения задаётся в файле `config.ini`.

## Возможности

- Таблица контактов с сортировкой по нажатию на заголовок любого столбца
- Поиск по произвольному тексту сразу в одном или нескольких выбранных полях (меню «Search in fields»)
- Добавление, редактирование (двойной клик или кнопка) и удаление контактов
- Диалоговое окно контакта с валидацией каждого поля с помощью регулярных выражений; некорректные поля подсвечиваются с пояснением ошибки
- Ввод даты рождения с помощью `QDateEdit` и всплывающего календаря (только даты строго раньше сегодняшнего дня)
- Неограниченное количество телефонных номеров для каждого контакта (рабочий, домашний, служебный)

## Архитектура

Бизнес-логика **используется совместно с задачей 1** (`../task1`, собирается в виде статической библиотеки): `Contact`, `Validator`, `PhoneBook`, `IContactStorage`, `FileStorage`.

| Класс (`task2/src`) | Роль |
|---|---|
| `PostgresStorage` | Реализация `IContactStorage` для PostgreSQL (Qt SQL, драйвер QPSQL) |
| `AppConfig`, `createStorage` | Чтение `config.ini`, создание выбранного хранилища |
| `ContactTableModel` | Модель `QAbstractTableModel` поверх `PhoneBook` |
| `ContactFilterProxy` | Сортировка (через `QSortFilterProxyModel`) и поиск по нескольким полям |
| `ContactDialog` | Диалог создания и редактирования контакта с валидацией |
| `MainWindow` | Главное окно: таблица, строка поиска, кнопки управления |

Поскольку `PhoneBook` зависит только от абстрактного интерфейса `IContactStorage`, переключение между файлом и базой данных не затрагивает остальной код.

## Выбор хранилища: `config.ini`

    [storage]
    type=file          ; или: postgres

    [file]
    path=data/contacts.txt

    [postgres]
    host=localhost
    port=5432
    database=phonebook
    user=phonebook
    password=phonebook

При сборке CMake копирует `config.ini` рядом с исполняемым файлом. Другой конфигурационный файл можно передать первым аргументом командной строки. Если файл отсутствует, по умолчанию используется файловое хранилище. Активное хранилище отображается в заголовке окна.

## Настройка PostgreSQL

    psql -U postgres -h localhost -c "CREATE USER phonebook WITH PASSWORD 'phonebook';"
    psql -U postgres -h localhost -c "CREATE DATABASE phonebook OWNER phonebook ENCODING 'UTF8' TEMPLATE template0;"

Таблицы создаются автоматически при первом запуске программы (структура описана в `sql/schema.sql`: `contacts` 1 --- N `phones`). В Windows папка `bin` из директории установки PostgreSQL (содержащая `libpq.dll`) должна быть добавлена в переменную среды `PATH`.

## Сборка и запуск

Требования: Qt 6 (Widgets, Sql), CMake >= 3.16, компилятор с поддержкой C++17.

Через Qt Creator: откройте `task2/CMakeLists.txt`, выберите комплект (kit) Qt 6 и нажмите Run.

Из терминала (при необходимости скорректируйте путь к Qt):

    cmake -S task2 -B build-task2 -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=C:/Qt/6.12.1/mingw_64
    cmake --build build-task2
    ./build-task2/phonebook_qt
