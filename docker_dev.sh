#!/usr/bin/env bash

action=$1

if [ "$action" = "enter" ]; then
    docker exec -it dev_host /bin/bash
elif [ "$action" = "up" ]; then
    docker-compose -f docker-compose.yml up -d
elif [ "$action" = "down" ]; then
    docker-compose down
elif [ "$action" = "debug" ]; then
    docker-compose exec dev_host sh -c "gdbserver :1234 cmake-build-debug/my_project_exec ${*:2}"
fi