#!/bin/bash

case $1 in
on)
	echo "enable forwarding";

	echo 1 > /proc/sys/net/ipv4/ip_forward;

	iptables -I FORWARD 1 -s 192.169.100.41 -j ACCEPT;
	iptables -I FORWARD 1 -s 192.169.100.41 -j ACCEPT;

	iptables -I FORWARD 1 -s 192.169.100.42 -j ACCEPT;
	iptables -I FORWARD 1 -s 192.169.100.42 -j ACCEPT;

	;;
off)
	echo "disable forwarding";

	echo 0 > /proc/sys/net/ipv4/ip_forward;

	iptables -F FORWARD;

	;;
*)
	echo "only [ on || off ] is acceptable";
	;;
esac;
