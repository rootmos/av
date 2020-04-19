#!/bin/bash

set -o nounset -o pipefail -o errexit

TMP=$(mktemp -d)
trap 'rm -rf $TMP' EXIT

DURATION=5
OUTPUT=$TMP/out.mkv
PLAYER=${PLAYER-mpv}
OPTS=()
while getopts "o:d:-" OPT; do
    case $OPT in
        o) OUTPUT=$OPTARG ;;
        d) DURATION=$OPTARG ;;
        -) break ;;
        ?) exit 2 ;;
    esac
done
shift $((OPTIND-1))

EXE=$1

$EXE -d "$DURATION" -o "$OUTPUT"
$PLAYER "$OUTPUT"
