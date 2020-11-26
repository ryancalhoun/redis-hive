#!/bin/bash

for pid in data/redis-*/*.pid; do
  kill $(cat $pid)
done
for pid in data/hive-*/*.pid; do
  kill $(cat $pid)
done
