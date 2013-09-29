#!/bin/sh

kill `ps -ef | grep wckbuttons | grep -v grep | awk '{ print $2}'`
