# Scripts building php-firebird extension (Windows + Linux)

## How it works

These scripts will clone or pull corresponding PHP version(s) from the PHP source repo and build 4 .dll files per PHP version: x86, x64 and ts and non-ts for each architecture.

Do not run scripts with ``-sdk-`` in their files names directly. These are called from php-sdk environment.

Make sure you got ~20GB free disk space to build for all PHP versions.

## Set up

Make sure ``git`` is in you PATH

1. Set up Microsoft Visual Studio vc15, vs16 and vs17 (for PHP8.4+).
2. Set up Firebird 32-bit and 64-bit installations or libraries.
3. Set up PHP-SDK according to https://wiki.php.net/internals/windows/stepbystepbuild_sdk_2
4. Clone php-firebird extension source somewhere.
5. Adjust php-fb-config.bat.
    ``Note: PFB_SOURCE_DIR should point one level up. For example
    PFB_SOURCE_DIR=D:\php-firebird\ then your source should reside in D:\php-firebird\php-firebird\
    ``
6. cd into php-sdk and from there run ``<path_to>\php-fb-build-all.bat`` to build for all PHP versions or run ``php-fb-build.bat 7.4 vc15`` to build for particular version.

---

# Build on Linux

Linux builds are handled by `php-fb-build-linux.sh`. It clones the extension source into a local build folder and builds one `.so` per configured PHP version.

## Prerequisites

- A working C/C++ build toolchain: `build-essential`, `autoconf`, `automake`, `libtool`, `pkg-config`, `g++`
- Firebird client headers + library (`fbclient`) installed
    - Default script paths assume a Firebird install under `/opt/firebird/`:
        - headers: `/opt/firebird/include`
        - library: `/opt/firebird/lib`
- For each PHP version you want to build, the corresponding packages must exist (Debian/Ubuntu naming):
    - `phpX.Y-dev`, `phpX.Y-cli`, `phpX.Y-common`

Note: the script will try to install missing PHP packages via `sudo apt-get`. On non-Debian-based distros, install the equivalents manually and make sure `phpX.Y`, `phpizeX.Y` and `php-configX.Y` exist in `/usr/bin/`.

## Configure

Open `php-fb-build-linux.sh` and adjust if needed:

- `FIREBIRD_INCLUDE_DIR` (default: `/opt/firebird/include`)
- `PHP_VERSIONS` (default: `("7.4" "8.0" "8.1" "8.2" "8.3" "8.4")`)
- `INSTALL_DIR` (default: `../ext` relative to the repo root)

## Run

From the repository root:

```bash
./build_scripts/php-fb-build-linux.sh
```

## Output

Successful builds are copied into the install directory (default: `../ext`) and named like:

`php_<PHP_FULL_VERSION>-interbase-<DRIVER_VERSION>-<os>-<arch>.so`

If a particular PHP version fails to build, the script continues with the next one and reports all failed versions at the end.

