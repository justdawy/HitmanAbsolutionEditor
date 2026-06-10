# Hitman Absolution Editor (Localization Fork)

*[Українська версія нижче / Ukrainian version below]*

This project is a fork of the original **Hitman Absolution Editor**. The main goal of this fork is to add full Ukrainian localization to the game *Hitman: Absolution*.

The original tool allows for unpacking, viewing, and editing game resources. In this version, specific changes and enhancements were introduced to ensure proper handling of Cyrillic fonts and translation tools.

## What was added and modified:

- **Global Deep Search:** Added the ability to search for any text (e.g., "English" or Cyrillic text) across all 16,000+ game files simultaneously in a background thread, preventing UI freezing.
- **Cyrillic Support in Editor:** The editor's UI (via ImGui) now correctly renders Ukrainian and Russian characters, both in search results and within the text reading panels.
- **Critical Memory Bug Fixes:** Fixed `Undefined Behavior` issues related to memory allocation for resource arrays, which previously caused application crashes (heap corruption) during mass file scanning.
- **Safe Double-Clicking:** Fixed a bug where double-clicking an unsupported or unknown resource type would crash the editor due to a Null Pointer Exception. The editor now safely ignores these files and logs a warning in the console.
- **Application Icon:** Added a custom Hitman icon to the executable and the window title bar for better OS integration.

## How to Build

1. Clone the repository.
2. Ensure you have **Visual Studio 2022**, **CMake**, and **vcpkg** installed.
3. Run the `build.bat` script in the root folder of the project.
4. After a successful compilation, the ready-to-use `HitmanAbsolutionEditor.exe` will appear in the `build/x64-Release` folder.

## License and Copyright

This project is based on the original Hitman Absolution Editor, which **has no explicit open license**. 
Under GitHub's Terms of Service, by publishing the code, the original author allows other users to view it and create forks within the GitHub platform. 
However, the absence of a license means that **you are not allowed to use this code for commercial purposes or distribute it as your own standalone product**. 
This fork was created exclusively for non-commercial purposes (translating the game into Ukrainian) within the bounds of Fair Use and GitHub's Terms of Service.

---

# Hitman Absolution Editor (Форк для локалізації)

Цей проект є форком оригінального **Hitman Absolution Editor**. Основна мета цього форку — додавання повноцінної української локалізації в гру *Hitman: Absolution*.

Оригінальний інструмент дозволяє розпаковувати, переглядати та редагувати ігрові ресурси. У цій версії були внесені спеціальні зміни та доповнення для коректної роботи з кириличними шрифтами та інструментами перекладу.

## Що було додано та змінено:

- **Глобальний глибокий пошук (Deep Search):** Додано можливість шукати будь-який текст (наприклад, "English" або "люблю") одразу по всіх 16 000+ файлах гри у фоновому потоці без зависання інтерфейсу.
- **Підтримка кирилиці в редакторі:** Інтерфейс редактора (через ImGui) тепер коректно відображає українські та російські літери як у результатах пошуку, так і в самій панелі читання текстів.
- **Виправлення критичних багів пам'яті:** Полагоджено `Undefined Behavior` у роботі з виділенням пам'яті для масивів ресурсів, що викликало вильоти програми (heap corruption) при масовому скануванні файлів.
- **Безпечний подвійний клік:** Усунуто баг, коли подвійний клік на непідтримуваний або невідомий тип ресурсу призводив до падіння редактора через Null Pointer Exception. Тепер редактор безпечно ігнорує такі файли, попереджаючи у консолі.
- **Іконка програми:** Додано кастомну іконку Хітмена до виконуваного файлу та вікна програми.

## Як зібрати проект (Build)

1. Клонуйте репозиторій.
2. Переконайтесь, що у вас встановлено **Visual Studio 2022**, **CMake** та **vcpkg**.
3. Запустіть скрипт `build.bat` у кореневій папці проекту.
4. Після успішної компіляції готовий `HitmanAbsolutionEditor.exe` з'явиться у папці `build/x64-Release`.

## Про ліцензію та авторське право

Цей проект базується на оригінальному Hitman Absolution Editor, який **не має явної відкритої ліцензії** (no license). 
За правилами GitHub (Terms of Service), публікуючи код, автор дозволяє іншим користувачам переглядати його та створювати форки (fork) всередині платформи GitHub. 
Однак, відсутність ліцензії означає, що **ви не маєте права використовувати цей код у комерційних цілях або розповсюджувати його як свій власний**. 
Цей форк створено виключно з некомерційною метою (переклад гри українською) у рамках Fair Use та умов використання GitHub.
