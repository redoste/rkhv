#!/bin/sh
exec clang-format -n -Werror $(git ls-files '*.c' '*.h')
