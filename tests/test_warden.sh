#!/bin/bash

CLI="$HOME/course/build/warden_cli"
ORIGINAL_PDF="$HOME/test.pdf"
TEST_DIR="$HOME/warden_tests"
PASSWORD="123"

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' 

mkdir -p "$TEST_DIR"
cd "$TEST_DIR" || exit

echo -e "${YELLOW}--- Подготовка расширенных тестовых файлов ---${NC}"

# 1. Оригинал (PDF)
cp "$ORIGINAL_PDF" "1_original.pdf"

# 2. Архив (ZIP)
zip -q "2_archived.zip" "1_original.pdf"

# 3. Зашифрованный (OpenSSL AES-256)
openssl enc -aes-256-cbc -salt -in "1_original.pdf" -out "3_encrypted.enc" -k "$PASSWORD" -pbkdf2

# 4. Архив + Архив
zip -q "4_double_zip.zip" "2_archived.zip"

# 5. Шифр + Архив (Сложный тест: шифр внутри легитимного контейнера)
zip -q "5_enc_then_zip.zip" "3_encrypted.enc"

# 6. Архив + Шифр (Шифр поверх архива - подпись ZIP пропадет)
openssl enc -aes-256-cbc -salt -in "2_archived.zip" -out "6_zip_then_enc.enc" -k "$PASSWORD" -pbkdf2

# 7. Текст (Низкая энтропия)
echo "This is a simple text file. It has very low entropy and should be safe." > "7_text.txt"
for i in {1..100}; do echo "Adding more lines to make it large enough for multiple chunks..." >> "7_text.txt"; done

# 8. Имитация видео (Высокая энтропия + валидный Header)
printf "\x00\x00\x00\x20\x66\x74\x79\x70\x69\x73\x6f\x6d" > "8_fake_video.mp4"
head -c 50000 /dev/urandom >> "8_fake_video.mp4"

# 9. Чистый шум (Высокая энтропия БЕЗ заголовка)
head -c 50000 /dev/urandom > "9_pure_noise.bin"

echo -e "${YELLOW}--- Запуск сканирования ---${NC}"
echo -e "VERDICT        | TYPE    | CONF     | CHUNKS   | FILE"
echo "------------------------------------------------------------------------"

FILES=(
    "1_original.pdf" 
    "2_archived.zip" 
    "3_encrypted.enc" 
    "4_double_zip.zip" 
    "5_enc_then_zip.zip" 
    "6_zip_then_enc.enc"
    "7_text.txt"
    "8_fake_video.mp4"
    "9_pure_noise.bin"
)

for FILE in "${FILES[@]}"; do
    if [ -f "$FILE" ]; then
        OUTPUT=$(cd "$HOME/course/build" && ./warden_cli "$TEST_DIR/$FILE" | grep -E "^\[")
        
        if [[ $OUTPUT == *"[!]"* ]]; then
            echo -e "${RED}$OUTPUT${NC}"
        elif [[ $OUTPUT == *"[?]"* ]]; then
            echo -e "${YELLOW}$OUTPUT${NC}"
        else
            echo -e "${GREEN}$OUTPUT${NC}"
        fi
    fi
done

echo -e "------------------------------------------------------------------------"
echo -e "${YELLOW}Тесты завершены.${NC}"