#!/bin/sh

kill `ps -ef | grep windowck | grep -v grep | awk '{ print $2}'`
