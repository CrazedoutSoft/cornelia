#!/bin/bash

ps -ef | grep './cornelia_d' | grep -v grep | awk '{print $2}' | xargs -r kill -9
