#!/bin/sh

res="$(jq -r "$(cat <<EOF
select(.["interface-types"] | contains(["uefi"])) |
select(.targets[].architecture == "x86_64") |
select(.targets[].machines | join("|") | contains("pc-")) |
select(.features | contains(["secure-boot"]) | not) |
.mapping.executable.filename
EOF
)" "${RKHV_QEMU_FIRMWARE_PATH:-/usr/share/qemu/firmware}"/* | head -n1)"

if [ -z "$res" ]; then
	exit 1;
fi
echo "$res"
