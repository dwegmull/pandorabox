#
# ~/.bashrc
#

# If not running interactively, don't do anything
[[ $- != *i* ]] && return

alias ls='ls --color=auto'
PS1='[\u@\h \W]\$ '

amixer cset numid=3 1
amixer cset numid=1 400
ifconfig | grep -A1 wlan0 | grep broadcast
while [ 1 -eq $? ]
do
  sleep 1
  ifconfig | grep broadcast
done
/root/vfd &
/root/buttons >> /root/.config/pianobar/ctl &
pianobar

