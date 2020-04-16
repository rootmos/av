#!/bin/bash

set -o nounset -o pipefail -o errexit

OUT=README.md

cat <<EOF > "$OUT"
# Audio and Visuals

Sandbox for audio and video synthesis.

## Progress
- [x] Use [ffmpeg](https://ffmpeg.org/) to stream simple frames to
    [Twitch](https://www.twitch.tv/rootmos2)
- [ ] Generate simple audio and attach it to the stream
- [ ] Add an interface for a higher level language (Python?) to control the
    synthesis live (using Python's repl maybe?)

## Usage
\`\`\`
EOF
build/av -h >> "$OUT" 2>&1

cat <<EOF >> "$OUT"
\`\`\`
EOF
