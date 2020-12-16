#!/bin/bash

cd $(dirname $0)

if [[ $1 == all ]]; then
  for pid in data/redis-*/*.pid; do
    kill $(cat $pid)
  done
  for pid in data/hive-*/*.pid; do
    kill $(cat $pid)
    rm $pid
  done
else
  while getopts "hr:v:" OPT; do
    case $OPT in
    r)
      REDIS=$(( 6000 + $OPTARG ))
      kill $(cat data/redis-$REDIS/redis-$REDIS.pid)
      ;;
    v)
      CONTROL=$(( 3000 + $OPTARG ))
      kill $(cat data/hive-$CONTROL/hive.pid)
      rm data/hive-$CONTROL/hive.pid
      ;;
    h)
      echo "Usage $0 [-rREDIS|-vHIVE]"
      ;;
    *)
      echo >2 Unknown option $OPT
      exit 1
      ;;
    esac
  done
fi


