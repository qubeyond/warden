# Сборка

```bash
mkdir build && cd build
cmake .. && cmake --build .
```

# GUI Мониторинг

```bash
sudo ./build/warden_cli --gui monitor ~/my_folder
```

# Основные режимы

Сканирование одного файла: `./warden_cli scan path/to/file`
Мониторинг папки: `sudo ./warden_cli monitor path/to/folder`
Интерактивный дашборд: `--gui`

# Глобальные настройки

`-t, --set-threshold` — порог детекции.
`--set-log-level` — уровень логов (debug, info, warn, error).
`--set-max-chunks` — ограничение на количество проверяемых блоков в больших файлах.
`--save` — сохранить текущие настройки в конфиг-файлы.

# Настройка конфигов

`-a`, `--app-config` — путь к app_config.json.
`-m`, `--model-config` — путь к весам модели model_config.json.
`-p`, `--prop-config` — путь к properties.json.

# Готовые тестовые случаи

Запустить в разных консолях

```bash
./tests/scripts/gui_test.sh ~/warden_monitor_test
```

```bash
sudo -E ./build/warden_cli --gui monitor ~/warden_monitor_test
```