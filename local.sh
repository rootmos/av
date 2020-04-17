#!/bin/bash

set -o nounset -o pipefail -o errexit

INPUT_URL=rtmp://localhost/ingest
OUTPUT_URL=rtmp://localhost:$(shuf -i 2000-65000 -n 1)/$(uuidgen)
PLAYER=${PLAYER-mpv}
while getopts "i:o:-" OPT; do
    case $OPT in
        i) INPUT_URL=$OPTARG ;;
        o) OUTPUT_URL=$OPTARG ;;
        -) break ;;
        ?) exit 2 ;;
    esac
done
shift $((OPTIND-1))

log() {
    cat > "local.log"
}

server() {
    ffmpeg \
        -f flv -listen 1 -i "$INPUT_URL" \
        -c copy \
        -f flv -listen 1 "$OUTPUT_URL" \
        2>&1 | log
}

player() {
    while sleep 0.2; do
        set +o errexit
        $PLAYER "$OUTPUT_URL" 2>&1 | log
        set -o errexit
    done
}

player &
PLAYER_PID=$!

trap 'pkill $PLAYER_PID' EXIT
server
