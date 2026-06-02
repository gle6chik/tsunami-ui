# Инструкции для Claude Code — tsunami-ui

## Build & Test
- Configure: `cmake -B build -DCMAKE_PREFIX_PATH=<path-to-Qt6>` (core берётся из соседнего `../tsunami-core`).
  Если core не сосед — `-DTSUNAMI_CORE_DIR=<path-to-tsunami-core>`.
- Build: `cmake --build build --config Release`.
- Запуск: `build/Release/TsunamiSimUI` (или `build_and_run.bat`).

## Зависимость от core
- UI подключает header-only таргет `NskTSH::tsunami-io` из `tsunami-core` (**соседний репозиторий** по конвенции, НЕ submodule, НЕ DLL).
- НЕ копировать сюда исходники солвера/парсеров — только подключать таргет.

## Симулятор — опционален
- UI работает как standalone-просмотрщик: запуск НЕ требует симулятора, просмотр результатов всегда доступен.
- Симулятор (`tsunami-simulator` .exe) ищется лениво (saved → app-local `simulator/` → `$TSUNAMI_SIMULATOR_HOME` → соседний build → PATH);
  нет — кнопка Launch disabled.

## Конвенция путей (ОБЯЗАТЕЛЬНО)
- Данные: `$NSKTSH_DATA` (default `../Data` — папка уровнем выше репо).
- Результаты: `$NSKTSH_RESULTS` (default `../Results`).
- Без абсолютных путей `F:\`.

## Не трогать
- Каталоги сборки (`build*/`, `.vs/`, `x64/`), бинарники Qt.
- Минимальный набор изменений > крупный рефактор.

## Workflow
- Не коммитить/пушить без явной просьбы.
- Перед завершением: build → smoke-run → diff summary → риски.
- Общение по-русски, код и комментарии на английском.
