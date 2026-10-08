#!/usr/bin/env bash

CC=gcc

CFLAGS=(-Iinclude $(pkg-config --cflags raylib))
CFLAGS_DEV=(-g -Wall -Wextra -fsanitize=address "${CFLAGS[@]}")
CFLAGS_REL=(-O3 "${CFLAGS[@]}")

LFLAGS=($(pkg-config --libs raylib))
LFLAGS_DEV=(-g -fsanitize=address "${LFLAGS[@]}")
LFLAGS_REL=("${LFLAGS[@]}")

BUILD=build
DEV_EXEC="$BUILD/dev/gbemulate"
REL_EXEC="$BUILD/rel/gbemulate"

srcs=()
while IFS= read -r -d '' src; do
    srcs+=("$src")
done < <(find src -name '*.c' -print0)

dev_objs=()
rel_objs=()

for src in "${srcs[@]}"; do
    rel=${src#src/}
    rel=${rel%.c}

    dev_objs+=("$BUILD/dev/$rel.o")
    rel_objs+=("$BUILD/rel/$rel.o")
done

mkdirs() {
    mkdir -p "$BUILD/dev" "$BUILD/rel"
}

dev() {
    mkdirs

    for i in "${!srcs[@]}"; do
        src=${srcs[$i]}
        obj=${dev_objs[$i]}

        mkdir -p "$(dirname "$obj")"

        "$CC" "${CFLAGS_DEV[@]}" -c "$src" -o "$obj"
    done

    "$CC" "${LFLAGS_DEV[@]}" -o "$DEV_EXEC" "${dev_objs[@]}"
}

rel() {
    mkdirs

    for i in "${!srcs[@]}"; do
        src=${srcs[$i]}
        obj=${rel_objs[$i]}

        mkdir -p "$(dirname "$obj")"

        "$CC" "${CFLAGS_REL[@]}" -c "$src" -o "$obj"
    done

    "$CC" "${LFLAGS_REL[@]}" -o "$REL_EXEC" "${rel_objs[@]}"
}

clean() {
    rm "$DEV_EXEC" "$REL_EXEC"
    rm -rf "$BUILD"
}

case "$1" in
    dev)
        dev
        ;;
    rel)
        rel
        ;;
    clean)
        clean
        ;;
    *)
        dev
        ;;
esac
