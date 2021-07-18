#!/usr/bin/env bash

action=$1

if [ "$action" = "enter" ]; then
    docker exec -it mocker /bin/bash
elif [ "$action" = "up" ]; then
    docker-compose -f docker-compose.yml up -d
elif [ "$action" = "down" ]; then
    docker-compose down
elif [ "$action" = "test" ]; then
    docker-compose exec dev_host sh -c "gdbserver :1234 cmake-build-mocker/test ${*:2}"
fi