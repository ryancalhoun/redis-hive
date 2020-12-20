#!/bin/bash

cd $(dirname $0)

N=${1:-3}

for i in $(seq 1 $N); do
  REDIS=$(( 6000 + i ))
  PROXY=$(( 7000 + i ))
  CONTROL=$(( 3000 + i ))
  mkdir -p data/redis-$REDIS

  if [[ ! -f data/redis-$REDIS/redis-$REDIS.pid ]]; then
    redis-server <(sed 's/{{PORT}}/'$REDIS'/g' redis.conf.in)
  fi

  mkdir -p data/hive-$CONTROL

  if [[ ! -f data/hive-$CONTROL/hive.pid ]] ||
     [[ $(cat /proc/$(cat data/hive-$CONTROL/hive.pid)/comm 2>/dev/null) != redis-hive ]]; then
    ./bin/redis-hive -r$REDIS -p$PROXY -c$CONTROL -mlocalhost $(seq 3001 $(( 3000 + N)) | sed 's/^/-o/') >> data/hive-$CONTROL/hive.log 2>&1 &
    echo $! > data/hive-$CONTROL/hive.pid
  fi
done
