#!/bin/bash

# Пути
PROJECT_ROOT="$HOME/course"
CLI="$PROJECT_ROOT/build/warden_cli"
TEST_DIR="$HOME/warden_monitor_test"
LOG_FILE="$HOME/warden_monitor.log"
ORIGINAL_PDF="$HOME/test.pdf"
PASSWORD="123"

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Проверка окружения
if [ ! -f "$CLI" ]; then
    echo -e "${RED}[!] Ошибка: Бинарник не найден в $CLI. Сначала скомпилируй проект.${NC}"
    exit 1
fi

if [ ! -d "$PROJECT_ROOT/configs" ]; then
    echo -e "${RED}[!] Ошибка: Папка configs не найдена в $PROJECT_ROOT.${NC}"
    exit 1
fi

# Очистка
rm -rf "$TEST_DIR"
rm -f "$LOG_FILE"
mkdir -p "$TEST_DIR"

echo -e "${BLUE}[*] Запуск Warden Monitor на директорию: $TEST_DIR${NC}"

# ВАЖНО: Переходим в корень проекта, чтобы пути к конфигам внутри программы (configs/...) были валидны
cd "$PROJECT_ROOT" || exit

# Запуск монитора в фоне. Используем абсолютный путь к CLI
sudo "$CLI" monitor "$TEST_DIR" > "$LOG_FILE" 2>&1 &
MONITOR_PID=$!

cleanup() {
    echo -e "\n${YELLOW}[*] Завершение работы монитора (PID: $MONITOR_PID)...${NC}"
    # Убиваем процесс и всех его потомков (sudo создает дочерний процесс)
    sudo kill $MONITOR_PID 2>/dev/null
    exit
}
trap cleanup SIGINT SIGTERM

echo -e "${YELLOW}[*] Ожидание инициализации монитора...${NC}"
sleep 2

# Проверка, не упал ли процесс сразу
if ! ps -p $MONITOR_PID > /dev/null; then
    echo -e "${RED}[!] Монитор упал сразу после запуска. Проверь лог: $LOG_FILE${NC}"
    cat "$LOG_FILE"
    exit 1
fi

echo -e "${YELLOW}--- Имитация активности в директории ---${NC}"

echo -e "${BLUE}[+] Создание: 1_safe.txt${NC}"
echo "This is a simple safe text file." > "$TEST_DIR/1_safe.txt"
sleep 0.8

echo -e "${BLUE}[+] Создание: 2_encrypted.enc${NC}"
openssl enc -aes-256-cbc -salt -in "$ORIGINAL_PDF" -out "$TEST_DIR/2_encrypted.enc" -k "$PASSWORD" -pbkdf2 2>/dev/null
sleep 0.8

echo -e "${BLUE}[+] Создание: 3_archive.zip${NC}"
zip -q "$TEST_DIR/3_archive.zip" "$TEST_DIR/1_safe.txt"
sleep 0.8

echo -e "${BLUE}[+] Создание: 4_noise.bin${NC}"
head -c 100000 /dev/urandom > "$TEST_DIR/4_noise.bin"
sleep 0.8

echo -e "${YELLOW}--- Результаты детекции в реальном времени ---${NC}"
echo -e "------------------------------------------------------------------------"

# Читаем лог и раскрашиваем
# Используем седы, чтобы подсветить важные ключевые слова
grep --line-buffered "" "$LOG_FILE" | while read -r line; do
    if [[ $line == *"MALICIOUS"* ]]; then
        echo -e "${RED}$line${NC}"
    elif [[ $line == *"SUSPICIOUS"* ]]; then
        echo -e "${YELLOW}$line${NC}"
    elif [[ $line == *"SAFE"* ]]; then
        echo -e "${GREEN}$line${NC}"
    else
        echo -e "$line"
    fi
done &
GREP_PID=$!

# Даем время дочитать последние события
sleep 2
kill $GREP_PID 2>/dev/null

echo -e "------------------------------------------------------------------------"
echo -e "${BLUE}[*] Лог сохранен в: $LOG_FILE${NC}"
cleanup