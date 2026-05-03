#!/bin/bash

# Configuration
FIREBIRD_INCLUDE_DIR="/opt/firebird/include"
INSTALL_DIR="../ext"
REPO_URL="https://github.com/FirebirdSQL/php-firebird.git"
BRANCH_OR_COMMIT="master" # Set to a specific tag or commit if needed
PHP_VERSIONS=("7.4" "8.0" "8.1" "8.2" "8.3" "8.4") # Adjust as needed
BUILD_DIR="php-firebird-build"

set -e
set -o pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
INSTALL_DIR_ABS="$(realpath -m "$ROOT_DIR/$INSTALL_DIR")"
BUILD_DIR_ABS="$ROOT_DIR/$BUILD_DIR"

mkdir -p "$INSTALL_DIR_ABS"
rm -rf "$BUILD_DIR_ABS"
echo "Cloning repository from $REPO_URL (branch: $BRANCH_OR_COMMIT)..."
git clone --depth 1 --branch "$BRANCH_OR_COMMIT" "$REPO_URL" "$BUILD_DIR_ABS"

extract_driver_version() {
    local header="$1"
    local pkgxml="$2"

    if [[ -f "$header" ]]; then
        # Legacy style (if present)
        local legacy
        legacy="$(awk '$1=="#define" && $2=="PHP_INTERBASE_VERSION" { gsub(/\"/,"",$3); print $3; exit }' "$header" 2>/dev/null || true)"
        if [[ -n "$legacy" ]]; then
            echo "$legacy"
            return 0
        fi

        # Current style: parse MAJOR/MINOR/REV and optional PRE
        local major minor rev pre
        major="$(awk '/^#define[[:space:]]+PHP_INTERBASE_VER_MAJOR[[:space:]]+/{print $3; exit}' "$header" 2>/dev/null || true)"
        minor="$(awk '/^#define[[:space:]]+PHP_INTERBASE_VER_MINOR[[:space:]]+/{print $3; exit}' "$header" 2>/dev/null || true)"
        rev="$(awk '/^#define[[:space:]]+PHP_INTERBASE_VER_REV[[:space:]]+/{print $3; exit}' "$header" 2>/dev/null || true)"
        pre="$(awk '/^#define[[:space:]]+PHP_INTERBASE_VER_PRE[[:space:]]+/{print $3; exit}' "$header" 2>/dev/null || true)"
        pre="${pre%\"}"
        pre="${pre#\"}"

        if [[ -n "$major" && -n "$minor" && -n "$rev" ]]; then
            echo "${major}.${minor}.${rev}${pre}"
            return 0
        fi
    fi

    if [[ -f "$pkgxml" ]]; then
        local v
        v="$(grep -oPm1 '(?<=<version>)[^<]+' "$pkgxml" 2>/dev/null || true)"
        if [[ -n "$v" ]]; then
            echo "$v"
            return 0
        fi
    fi

    echo "unknown"
}

DRIVER_VERSION="$(extract_driver_version "$BUILD_DIR_ABS/php_interbase.h" "$BUILD_DIR_ABS/package.xml")"

FAILED_VERSIONS=()

for VERSION in "${PHP_VERSIONS[@]}"; do
    echo "==> Building for PHP $VERSION"

    if (
        PHP_BIN="/usr/bin/php$VERSION"
        PHPIZE="/usr/bin/phpize$VERSION"
        PHP_CONFIG="/usr/bin/php-config$VERSION"

        if [[ ! -x "$PHP_BIN" || ! -x "$PHPIZE" || ! -x "$PHP_CONFIG" ]]; then
            echo "--> Installing missing PHP $VERSION packages..."
            sudo apt-get install -y "php$VERSION-dev" "php$VERSION-cli" "php$VERSION-common"
        fi

        cd "$BUILD_DIR_ABS"

        echo "--> Cleaning previous build (if any)..."
        make clean || true
        echo "--> Running phpize..."
        "$PHPIZE"
        echo "--> Configuring build..."
        CPPFLAGS="-I$FIREBIRD_INCLUDE_DIR" ./configure --with-php-config="$PHP_CONFIG"
        echo "--> Compiling..."
        make -j"$(nproc)"

        PHP_FULL_VERSION=$("$PHP_BIN" -r 'echo PHP_VERSION;')
        ARCH=$(uname -m)
        OS=$(uname -s | tr '[:upper:]' '[:lower:]')

        OUTPUT_FILE="php_${PHP_FULL_VERSION}-interbase-${DRIVER_VERSION}-${OS}-${ARCH}.so"
        mkdir -p "$INSTALL_DIR_ABS"
        echo "--> Copying output to $INSTALL_DIR_ABS/$OUTPUT_FILE"
        cp modules/interbase.so "$INSTALL_DIR_ABS/$OUTPUT_FILE"

        echo "Build complete for PHP $VERSION: $OUTPUT_FILE"
    ); then
        :
    else
        echo "Build FAILED for PHP $VERSION" >&2
        FAILED_VERSIONS+=("$VERSION")
    fi
done

if (( ${#FAILED_VERSIONS[@]} > 0 )); then
    echo "Some builds failed: ${FAILED_VERSIONS[*]}" >&2
    echo "Artifacts (successful builds) are located in: $INSTALL_DIR_ABS"
    exit 1
fi

echo "All builds completed. Files are located in: $INSTALL_DIR_ABS"

