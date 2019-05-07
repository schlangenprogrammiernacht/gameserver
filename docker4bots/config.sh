SPN_DATA_HOSTDIR=$(readlink -f $(dirname $0)/compiled_bots/)
SPN_SHM_HOSTDIR="/mnt/spn_shm"

DOCKER_COMPILE_ARGS="\
	--memory=1G --memory-swap=2G --cpus=1 \
	--tmpfs /run --tmpfs /tmp --network none"

DOCKER_RUN_ARGS="\
	--memory=32M --memory-swap=32M --cpus=1 --read-only \
	--tmpfs /run --tmpfs /tmp --network none"
