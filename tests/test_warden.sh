#!/bin/bash

# Пути к файлам
CLI="../build/warden_cli"
ORIGINAL_PDF="$HOME/test.pdf"
TEST_DIR="$HOME/warden_tests"
PASSWORD="123"

# Создаем папку для тестов
mkdir -p "$TEST_DIR"
cd "$TEST_DIR" || exit

echo "--- Подготовка тестовых файлов ---"

# 1. Оригинал
cp "$ORIGINAL_PDF" "1_original.pdf"

# 2. Архив (ZIP)
zip -q "2_archived.zip" "1_original.pdf"

# 3. Зашифрованный (OpenSSL AES-256)
openssl enc -aes-256-cbc -salt -in "1_original.pdf" -out "3_encrypted.enc" -k "$PASSWORD" -pbkdf2

# 4. Архив + Архив
zip -q "4_double_zip.zip" "2_archived.zip"

# 5. Шифр + Архив (Шифруем, потом пакуем в зип)
zip -q "5_enc_then_zip.zip" "3_encrypted.enc"

# 6. Архив + Шифр (Пакуем в зип, потом шифруем результат)
openssl enc -aes-256-cbc -salt -in "2_archived.zip" -out "6_zip_then_enc.enc" -k "$PASSWORD" -pbkdf2

echo "--- Запуск сканирования ---"

# Список всех созданных файлов
FILES=("1_original.pdf" "2_archived.zip" "3_encrypted.enc" "4_double_zip.zip" "5_enc_then_zip.zip" "6_zip_then_enc.enc")

for FILE in "${FILES[@]}"; do
    if [ -f "$FILE" ]; then
        echo "Анализ файла: $FILE"
        # Запускаем CLI из папки build, чтобы он видел конфиги
        (cd "$HOME/course/build" && ./warden_cli "$TEST_DIR/$FILE")
        echo "------------------------------------------"
    fi
done

echo "Тесты завершены."