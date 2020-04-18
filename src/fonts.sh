#!/bin/bash

set -o nounset -o pipefail -o errexit

cat <<EOF > "$1"
#pragma once

uint8_t fonts_monospace_default[] = {
EOF
xxd -include < /usr/share/fonts/TTF/Inconsolata-Regular.ttf >> "$1"
cat <<EOF >> "$1"
};
EOF
