# Инструкции для Claude Code — tsunami-ui

## Build & Test
- Configure: `cmake -B build -DTSUNAMI_CORE_DIR=../tsunami-core/MacCormackSimulation`
- Build: `cmake --build build --config Release`
- Запуск: `build/Release/TsunamiSimUI` (или `build_and_run.bat`).

## Зависимость от core
- UI подключает header-only парсеры из tsunami-core. Путь — через `TSUNAMI_CORE_DIR`
  (по умолчанию sibling-checkout или submodule `extern/tsunami-core`).
- НЕ копировать сюда исходники солвера — только подключать.

## Конвенция путей (ОБЯЗАТЕЛЬНО)
- Данные: `$NSKTSH_DATA` (default `../tsunami-data` + `../data-external`)
- Результаты: `$NSKTSH_RESULTS` (default `../results`)
- Без абсолютных путей `F:\`.

## Не трогать
- Каталоги сборки (`build*/`, `.vs/`, `x64/`), бинарники Qt.
- Минимальный набор изменений > крупный рефактор.

## Workflow
- Не коммитить/пушить без явной просьбы.
- Перед завершением: build → smoke-run → diff summary → риски.
- Общение по-русски, код и комментарии на английском.
