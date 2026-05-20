savedcmd_vringlog.mod := printf '%s\n'   vringlog.o | awk '!x[$$0]++ { print("./"$$0) }' > vringlog.mod
