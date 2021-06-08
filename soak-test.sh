#!/bin/bash

cd $(dirname $0)

helm repo add leftoverbytes https://leftoverbytes.com/charts
helm repo update
helm upgrade redis-hive leftoverbytes/redis-hive --install --force

iter=0
prev=
errors=0

while true; do
  (( ++iter ))
  echo >&2
  echo >&2 Iteration $iter

  replicas=$(kubectl exec deployment/redis-hive -credis -- redis-cli -h redis-hive-master info | grep connected_slaves | tr -d '\r')

  if [[ $replicas != "connected_slaves:2" ]]; then
    echo >&2 "FATAL: expected two replicas, got '$replicas'"
    exit 1
  fi

  if [[ ! -z $prev ]]; then
    for key in {a..z}; do
      val=$(kubectl exec deployment/redis-hive -credis -- redis-cli -h redis-hive-replica get soak/$key)
      if [[ $val != $prev ]]; then
        echo >&2 "Error: soak/$key expected $prev, actual $val"
        (( ++errors ))
      fi
    done
  fi
  echo >&2 "Total errors $errors"

  now=$(date)

  echo Setting keys to $now

  for key in {a..z}; do
    a=$(kubectl exec deployment/redis-hive -credis -- redis-cli -h redis-hive-master set soak/$key "$now")
    if [[ $a != "OK" ]]; then
      echo >&2 "$a"
    fi
  done

  oldest=$(kubectl get pod -lapp=redis-hive --sort-by=.status.startTime  -ojsonpath={.items[0].metadata.name})
  kubectl delete pod $oldest

  sleep 10
done
