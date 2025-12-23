#!/bin/bash

TARGET_DIR=${1:-"$HOME/warden_new_test"}
PROJECT_ROOT="$HOME/course"
ORIGINAL_FILE="/etc/hosts"
PASSWORD="123"

BLUE='\033[0;34m'
YELLOW='\033[1;33m'
GREEN='\033[0;32m'
RED='\033[0;31m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m'

# Списки расширений для разнообразия
EXT_TEXT=("txt" "log" "conf" "cfg" "sh" "md")
EXT_BIN=("bin" "dat" "sys" "exe" "dll" "so")
EXT_DOC=("pdf" "doc" "docx" "xlsx")

echo -e "${BLUE}=== Warden GUI Ultimate Variety Generator ===${NC}"
echo -e "${BLUE}[*] Целевая директория: $TARGET_DIR${NC}"

mkdir -p "$TARGET_DIR"

generate_file() {
    local type=$1
    local name=$2
    local ext_idx=$(( RANDOM % 6 ))

    case $type in
        "text_vary")
            # Текстовые файлы разного размера (от 1КБ до 500КБ)
            local ext=${EXT_TEXT[$ext_idx]}
            local lines=$(( (RANDOM % 1000) + 10 ))
            for ((i=0; i<$lines; i++)); do echo "Line $i: Some safe content $(date +%N)" >> "$TARGET_DIR/$name.$ext"; done
            echo -e "${GREEN}[+] Текст ($ext): $name.$ext (~$lines строк)${NC}"
            ;;
            
        "malware_mix")
            # Смешанный файл: 70% текста + 30% шума (даст средний процент вероятности)
            local ext=${EXT_BIN[$ext_idx]}
            echo "Header: Valid application data" > "$TARGET_DIR/$name.$ext"
            head -c $(( (RANDOM % 2000) + 500 )) /dev/urandom >> "$TARGET_DIR/$name.$ext"
            echo "Footer: End of segment" >> "$TARGET_DIR/$name.$ext"
            echo -e "${RED}[!] Смешанный бинарник ($ext): $name.$ext${NC}"
            ;;

        "heavy_vary")
            # Большие файлы (до 50МБ) с разными расширениями
            local ext=${EXT_DOC[$(( RANDOM % 4 ))]}
            local size=$(( (RANDOM % 40) + 10 ))
            echo -e "${PURPLE}[#] ТЯЖЕЛЫЙ файл ($size MB) as .$ext: $name.$ext${NC}"
            head -c "${size}M" /dev/urandom > "$TARGET_DIR/$name.$ext"
            ;;

        "random_noise")
            # Чистый шум (100% угроза)
            local ext=${EXT_BIN[$ext_idx]}
            head -c $(( (RANDOM % 10000) + 1000 )) /dev/urandom > "$TARGET_DIR/$name.$ext"
            echo -e "${RED}[!] Чистая угроза ($ext): $name.$ext${NC}"
            ;;

        "encrypted")
            openssl enc -aes-256-cbc -salt -in "$ORIGINAL_FILE" -out "$TARGET_DIR/$name.enc" -k "$PASSWORD" -pbkdf2 2>/dev/null
            echo -e "${YELLOW}[?] Шифрование: $name.enc${NC}"
            ;;
    esac
}

echo -e "${YELLOW}[!] Нажми Ctrl+C для остановки.${NC}"
echo -e "------------------------------------------------------------"

count=1
while true; do
    # 5 вариантов типов
    choice=$(( count % 5 ))
    
    if [ $choice -eq 0 ]; then generate_file "text_vary" "note_$count"
    elif [ $choice -eq 1 ]; then generate_file "malware_mix" "app_$count"
    elif [ $choice -eq 2 ]; then generate_file "heavy_vary" "report_$count"
    elif [ $choice -eq 3 ]; then generate_file "random_noise" "driver_$count"
    else generate_file "encrypted" "vault_$count"
    fi

    ((count++))
    sleep $(( (RANDOM % 2) + 1 ))
done