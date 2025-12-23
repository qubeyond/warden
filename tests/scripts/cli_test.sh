#!/bin/bash

# Настройки путей
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
CLI="$PROJECT_ROOT/build/warden_cli"
TEST_DIR="$HOME/warden_tests"
LOG_DIR="/var/log/warden"
PASS="HshHd829"

# Цвета
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[1;34m'
NC='\033[0m'

# Подготовка
mkdir -p "$TEST_DIR"
echo "$PASS" | sudo -S mkdir -p "$LOG_DIR"
echo "$PASS" | sudo -S chmod 777 "$LOG_DIR"

echo -e "${BLUE}=== ГЕНЕРАЦИЯ ТЕСТОВЫХ ФАЙЛОВ ===${NC}"
echo "clean text file" > "$TEST_DIR/safe.txt"
head -c 100000 /dev/urandom > "$TEST_DIR/malware.bin"

# Переходим в корень проекта для доступа к configs/
cd "$PROJECT_ROOT" || exit

echo -e "\n${BLUE}=== ТЕСТ 1: SCAN (с sudo для записи логов) ===${NC}"
# Используем sudo -E чтобы сохранить переменные окружения пользователя
echo "$PASS" | sudo -S -E "$CLI" scan "$TEST_DIR/safe.txt"

echo -e "\n${BLUE}=== ТЕСТ 2: THRESHOLD OVERRIDE ===${NC}"

echo -e "${YELLOW}[*] Порог 0.95 для файла с шумом (Должно быть SAFE)${NC}"
echo "$PASS" | sudo -S -E "$CLI" --set-threshold 0.95 scan "$TEST_DIR/malware.bin"

echo -e "${YELLOW}[*] Порог 0.10 для файла с шумом (Должно быть MALICIOUS)${NC}"
echo "$PASS" | sudo -S -E "$CLI" --set-threshold 0.10 scan "$TEST_DIR/malware.bin"

echo -e "\n${BLUE}=== ТЕСТ 3: LOG LEVEL OVERRIDE ===${NC}"
echo -e "${YELLOW}[*] Запуск с DEBUG уровнем логов${NC}"
echo "$PASS" | sudo -S -E "$CLI" --set-log-level debug scan "$TEST_DIR/malware.bin"

echo -e "\n${BLUE}=== ТЕСТ 4: MONITOR (Real-time) ===${NC}"
echo -e "${YELLOW}[*] Запуск монитора на 3 секунды...${NC}"
# Запускаем монитор в фоне
echo "$PASS" | sudo -S -E timeout 3s "$CLI" monitor "$TEST_DIR" &
MONITOR_PID=$!

sleep 1
echo "Создаем файл для триггера..."
touch "$TEST_DIR/trigger.file"
sleep 2

echo -e "${GREEN}[OK] Тесты завершены.${NC}"