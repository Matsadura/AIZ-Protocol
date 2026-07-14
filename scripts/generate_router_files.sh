#!/usr/bin/env bash

# builds a filesystem sandbox under /tmp/webserv_test
#
# Safe to re-run: wipes and rebuilds /tmp/webserv_test each time.
# Pair it with router_test.conf, which points every `location` at the
# paths this script creates.

set -euo pipefail

ROOT=/tmp/webserv_test
rm -rf "$ROOT"

# ---------------------------------------------------------------------------
# location /  ->  $ROOT/www/html   (index present, autoindex on)
# ---------------------------------------------------------------------------
mkdir -p "$ROOT/www/html/normaldir"
mkdir -p "$ROOT/www/html/emptydir"

echo "<h1>index</h1>"            > "$ROOT/www/html/index.html"
echo "<h1>about</h1>"            > "$ROOT/www/html/about.html"
echo "just a file, no listing"   > "$ROOT/www/html/normaldir/file1.txt"

# file the process can't read -> 403 "File permission denied"
echo "top secret"                > "$ROOT/www/html/secret.txt"
chmod 000 "$ROOT/www/html/secret.txt"

# symlink that escapes the web root entirely -> tests whether stat()/open()
# in your server follow symlinks out of root (classic symlink-escape check)
ln -sf /etc/passwd "$ROOT/www/html/passwd-link"

# ---------------------------------------------------------------------------
# location /noindex -> autoindex off, no index file -> 403 "no index / listing off"
# ---------------------------------------------------------------------------
mkdir -p "$ROOT/www/noindexdir"
echo "hidden.txt exists but the dir can never be listed or auto-served" \
    > "$ROOT/www/noindexdir/hidden.txt"
echo "this file can be server as an index file" > "$ROOT/www/noindexdir/index.php"

# ---------------------------------------------------------------------------
# location /locked-listing -> autoindex on, but the dir itself is unreadable
# -> 403 "Directory is unreadable for listing"
# ---------------------------------------------------------------------------
mkdir -p "$ROOT/www/lockeddir"
echo "you will never see this listed" > "$ROOT/www/lockeddir/nope.txt"
chmod 000 "$ROOT/www/lockeddir"

# ---------------------------------------------------------------------------
# location /uploads -> POST/DELETE target + storage dir
# ---------------------------------------------------------------------------
mkdir -p "$ROOT/www/uploads/storage"
echo "Done!"
