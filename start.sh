#!/bin/bash

N=${1:-3}

for i in $(seq 1 $N); do
  REDIS=$(( 6000 + i ))
  PROXY=$(( 7000 + i ))
  CONTROL=$(( 3000 + i ))
  mkdir -p data/redis-$REDIS
  redis-server <(sed 's/{{PORT}}/'$REDIS'/g' redis.conf.in)

  mkdir -p data/hive-$CONTROL
  ./bin/redis-hive $REDIS $PROXY $CONTROL $(seq 3001 $(( 3000 + N)) ) > data/hive-$CONTROL/hive.log 2>&1 &
  echo $! > data/hive-$CONTROL/hive.pid
done
