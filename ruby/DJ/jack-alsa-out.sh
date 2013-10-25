DESIRED_DEVICE_NAME=$(cat jack-alsa-out-device.cfg | sed -e "s/\"\([^\"]\+\).*/\1/")
EXTRA_PARAMS=$(cat jack-alsa-out-device.cfg | sed -e "s/\"[^\"]\+\" \(.*\)/\1/")
ALSA_DEVICE_ID=hw:$(cat /proc/asound/cards | grep -o " \([[:digit:]]\) \[$DESIRED_DEVICE_NAME" | sed -e "s/ \([[:digit:]]\).*/\1/")

alsa_out -d ${ALSA_DEVICE_ID} ${EXTRA_PARAMS} -q0
