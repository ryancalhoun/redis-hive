#!/bin/bash

cd $(dirname $0)

N=${1:-3}

for i in $(seq 1 $N); do
  REDIS=$(( 6000 + i ))
  PROXY=$(( 7000 + i ))
  CONTROL=$(( 3000 + i ))
  mkdir -p data/redis-$REDIS

  if [[ ! -f data/redis-$REDIS/redis-$REDIS.pid ]]; then
    if [[ -z $REDIS_AUTH ]]; then
      redis-server <(sed 's/{{PORT}}/'$REDIS'/g' redis.conf.in | grep -v '{{PASS}}')
    else
      redis-server <(sed 's/{{PORT}}/'$REDIS'/g;s/{{PASS}}/'$REDIS_AUTH'/g' redis.conf.in)
    fi
  fi

  mkdir -p data/hive-$CONTROL

  if [[ ! -f data/hive-$CONTROL/hive.pid ]] ||
     [[ $(cat /proc/$(cat data/hive-$CONTROL/hive.pid)/comm 2>/dev/null) != redis-hive ]]; then
    if [[ -z $REDIS_AUTH ]]; then
      ./bin/redis-hive -r$REDIS -p$PROXY -c$CONTROL -mlocalhost $(seq 3001 $(( 3000 + N)) | sed 's/^/-o/') >> data/hive-$CONTROL/hive.log 2>&1 &
    else
      ./bin/redis-hive -r$REDIS -p$PROXY -aREDIS_AUTH -c$CONTROL -mlocalhost $(seq 3001 $(( 3000 + N)) | sed 's/^/-o/') >> data/hive-$CONTROL/hive.log 2>&1 &
    fi
    echo $! > data/hive-$CONTROL/hive.pid
  fi
done
