#!/bin/bash

set -o nounset -o pipefail -o errexit

cat <<EOF > "$1"
#pragma once

const static char build_info_version[] = "0.0.1";
const static char build_info_git_revid[] = "$(git rev-parse HEAD)";
const static char build_info_date[] = "$(date -Is)";

const static char build_info_application_name[] = "stellar-drift";
const static uint32_t build_info_version_integral = 1;
EOF
