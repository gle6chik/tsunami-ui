# Инструкции для агентов — tsunami-ui

## Назначение
Qt6-десктоп для просмотра/запуска тсунами-симуляций (front-end к tsunami-core).

## Как верифицировать
- Сборка: `cmake -B build -DTSUNAMI_CORE_DIR=<core> && cmake --build build --config Release`.
- Smoke: запуск приложения, загрузка тестовой бати из `$NSKTSH_DATA`.
- CI Qt6 — headless configure/build.

## Границы
- Только UI-слой; логику солвера не дублировать — подключать core.
- Большие данные/результаты — через конвенцию путей, не в git.

## Чек-лист перед PR
- [ ] Configure+build зелёные  - [ ] Нет build-артефактов в индексе
- [ ] `TSUNAMI_CORE_DIR` не захардкожен  - [ ] README обновлён при смене сборки
