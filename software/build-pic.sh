#!/bin/sh
# Build production firmware with XC8, without generated MPLAB makefiles.
set -eu

if test "$#" -ne 3; then
	printf '%s\n' "Usage: XC8_CC=/path/to/xc8-cc PIC_DFP=/path/to/pack/xc8 $0 application device output-directory" >&2
	exit 2
fi
pic_app=$1
pic_device=$2
pic_output=$3
pic_root=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
pic_cc=${XC8_CC:-xc8-cc}
# -O1 avoids the additional optimizer stack allowance seen at -Os.
pic_optimization=${PIC_OPTIMIZATION:--O1}
case "$pic_optimization" in
	-Os|-O0|-O1|-O2|-O3) ;;
	*) printf '%s\n' "Unsupported optimization: $pic_optimization" >&2; exit 2 ;;
esac
: "${PIC_DFP:?Set PIC_DFP to the device pack's xc8 directory}"
PIC_DFP=$(CDPATH= cd -- "$PIC_DFP" && pwd)

case "$pic_device" in
	16F877A) pic_hardware=HW_CATGENIE120 ;;
	16F1939) pic_hardware=HW_CATGENIE120PLUS ;;
	*) printf '%s\n' "Unsupported device: $pic_device" >&2; exit 2 ;;
esac

case "$pic_app" in
	catgenius)
		set -- catgenius/catgenius.c catgenius/userinterface.c \
			catgenius/litterlanguage.c catgenius/romwashprogram.c \
			common/catgenie120.c common/catsensor.c common/cmdline.c \
			common/rtc.c common/serial.c common/timer.c \
			common/water.c common/waterquality.c ;;
	geniediag)
		set -- geniediag/geniediag.c geniediag/userinterface.c \
			common/catgenie120.c common/catsensor.c common/cmdline.c \
			common/cmdline_box.c common/cmdline_tag.c common/cr14.c \
			common/i2c.c common/serial.c common/timer.c \
			common/water.c common/waterquality.c ;;
	iotester)
		set -- iotester/iotester.c common/catgenie120.c common/cmdline.c \
			common/cmdline_gpio.c common/serial.c ;;
	*) printf '%s\n' "Unsupported application: $pic_app" >&2; exit 2 ;;
esac
for pic_source do
	set -- "$@" "$pic_root/$pic_source"
	shift
done

# A failed build must never leave an older HEX file looking like a new result.
if test -e "$pic_output"; then
	printf '%s\n' "Use a new output directory: $pic_output" >&2
	exit 2
fi
mkdir -p -- "$pic_output"
cd -- "$pic_output"
"$pic_cc" --version
# A compiled stack gives a link-time RAM bound; no heap or reentrant stack.
"$pic_cc" -mcpu="$pic_device" -mdfp="$PIC_DFP" -std=c90 "$pic_optimization" \
	-mstack=compiled -mno-keep-startup -msummary=mem \
	-mchp-stack-usage -Wl,-Map=firmware.map \
	-D"$pic_hardware" \
	-I"$pic_root/common" --memorysummary=memory.xml \
	-o "$pic_app.elf" "$@"
python3 "$pic_root/check-pic-memory.py" "$pic_device" "$pic_app"
