#!/bin/bash

PARSER=/home/ijoubair/AIZ-Protocol/runme   # change if needed

run_test () {
    name=$1
    content=$2

    file="test_$name.conf"

    echo "=============================="
    echo "Test: $name"
    echo "=============================="

    echo "$content" > "$file"

    $PARSER "$file"
    echo ""
}

# =========================
# ✅ VALID TESTS
# =========================

run_test "valid_simple_server" "
server {
    listen 80;
}
"

run_test "valid_multiple_directives" "
server {
    listen 8080;
    root /var/www;
    index index.html;
}
"

run_test "valid_nested_blocks" "
server {
    location / {
        root /var/www;
    }
}
"

# =========================
# ❌ SYNTAX ERRORS
# =========================

run_test "missing_semicolon" "
server {
    listen 80
}
"

run_test "missing_brace" "
server {
    listen 80;
"

run_test "extra_closing_brace" "
server {
    listen 80;
}}
"

run_test "random_text" "
hello this is not valid config
server listen 80
"

# =========================
# 🧨 EDGE CASES
# =========================

run_test "empty_file" "
"

run_test "only_spaces" "
     
"

run_test "only_braces" "
{}{}{}
"

run_test "deep_nesting" "
server {
    location / {
        location /a {
            location /b {
                location /c {
                    listen 80;
                }
            }
        }
    }
}
"

# =========================
# 🧠 WEIRD FORMATTING
# =========================

run_test "weird_spacing" "
server{
listen    80;
   root     /var/www;
}
"

run_test "tabs_and_spaces" "$(printf 'server\t{\n\tlisten\t80;\n}')"

# =========================
# 💣 STRESS TEST (BIG INPUT)
# =========================

big_config=""
for i in $(seq 1 200); do
big_config+="server { listen $((8000 + i)); }\n"
done

run_test "large_input" "$big_config"

# =========================
# 🧪 DONE
# =========================

echo "All tests finished."