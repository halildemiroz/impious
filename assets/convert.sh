#!/bin/bash

set -e

TARGET_DIR="../nitrofiles/assets"
mkdir -p "$TARGET_DIR"

for img in *.png; do
    base="${img%.png}"

    echo "Converting $base..."

    case "$base" in
        # 1. HUGE BACKGROUNDS (*bg*)
        # Keep reduction ON to save VRAM
        *bg*)
            grit "$img" \
                -ftb \
                -fh! \
                -gB8 \
                -gt \
                -mRtf \
                -mLs \
                -o"$base"
            ;;

        # 2. HARDCODED BLOCKS (block*)
        # Turn reduction OFF (-mR!) so Tile IDs 1, 2, 3, and 4 stay intact
        block*)
            grit "$img" \
                -ftb \
                -fh! \
                -gB8 \
                -gt \
                -mR! \
                -m \
                -o"$base"
            ;;

        # 3. SPRITES (character, coin, door)
        # Turn reduction OFF (-mR!), disable map (-m!), use magenta transparency
        *)
            grit "$img" \
                -ftb \
                -fh! \
                -gB8 \
                -gTFF00FF \
                -gt \
                -mR! \
                -m! \
                -o"$base"
            ;;
    esac

    mv "$base.img.bin" "$TARGET_DIR/$base.img"
    mv "$base.pal.bin" "$TARGET_DIR/$base.pal"

    if [ -f "$base.map.bin" ]; then
        mv "$base.map.bin" "$TARGET_DIR/$base.map"
    fi

    rm -f "$base.h"
done

echo "Done!"
