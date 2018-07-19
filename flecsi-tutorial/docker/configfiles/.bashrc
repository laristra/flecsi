################################################################################
# Bash Configuration
################################################################################

#------------------------------------------------------------------------------#
# Determine if this is an interactive shell
#------------------------------------------------------------------------------#

if [ -t 0 ] ; then

	echo "Interactive Session"

	#---------------------------------------------------------------------------#
	# Source configuration files
	#---------------------------------------------------------------------------#

	[ -f ~/.bash/colors ] && . ~/.bash/colors
	[ -f ~/.bash/functions ] && . ~/.bash/functions

	#---------------------------------------------------------------------------#
	# environment modules
	#---------------------------------------------------------------------------#

	if [ -f /usr/share/Modules/init/bash ] ; then
		. /usr/share/Modules/init/bash
	fi

	export MODULEPATH=$MODULEPATH:$HOME/.modulefiles
	print_env_var "MODULEPATH" $MODULEPATH

	module load aliases

	#---------------------------------------------------------------------------#
	# PATH
	#---------------------------------------------------------------------------#

	#if [ `whoami` != "root" ] ; then
	#fi

        print_env_var "PATH" $PATH

	#---------------------------------------------------------------------------#
	# host and OS
	#---------------------------------------------------------------------------#

	host=`hostname | sed 's,administrators-,,g;s,-air,,g;s,\.local,,g'`

	if [ $host == *cn* ] ; then
		host_simple=`echo $host | sed 's,\..*$,,g;s,[0-9],,g'`
	else
		host_simple=`echo $host | sed 's,\..*$,,g'`
	fi

	#---------------------------------------------------------------------------#
	# enable core files
	#---------------------------------------------------------------------------#

	ulimit -c unlimited

	#---------------------------------------------------------------------------#
	# HISTCONTROL
	#---------------------------------------------------------------------------#

	export HISTCONTROL=ignoreboth
	print_env_var "HISTCONTROL" $HISTCONTROL

   #---------------------------------------------------------------------------#
	# GNU Colors
	#---------------------------------------------------------------------------#
	export GCC_COLORS='error=01;31:warning=01;35:note=01;36:caret=01;32:locus=01:quote=01'

	#---------------------------------------------------------------------------#
	# dircolors setup
	#---------------------------------------------------------------------------#

	eval `dircolors -b ~/.dircolors`

	#---------------------------------------------------------------------------#
	# less setup for manpages
	#---------------------------------------------------------------------------#

	export LESS_TERMCAP_mb=$'\E[01;31m'       # begin blinking
	export LESS_TERMCAP_md=$'\E[01;38;5;74m'  # begin bold
	export LESS_TERMCAP_me=$'\E[0m'           # end mode
	export LESS_TERMCAP_se=$'\E[0m'           # end standout-mode
	export LESS_TERMCAP_so=$'\E[38;5;246m'    # begin standout-mode - info box
	export LESS_TERMCAP_ue=$'\E[0m'           # end underline
	export LESS_TERMCAP_us=$'\E[04;38;5;146m' # begin underline

	#---------------------------------------------------------------------------#
	# source the host-specific configuration
	#---------------------------------------------------------------------------#

	extra_pinfo=""
	[ -f ~/.bash/$host_simple ] &&
		echo -e "$FG_DCYAN""Configuration""$FG_GREEN"" $host_simple""$NEUTRAL" &&
		. ~/.bash/$host_simple

	#---------------------------------------------------------------------------#
	# Set LANL proxies
	#---------------------------------------------------------------------------#

	if timeout 0.2 bash -c ": > /dev/tcp/proxyout/8080" &>/dev/null; then
		export http_proxy="http://proxyout.lanl.gov:8080"
		export https_proxy="$http_proxy"
		export ftp_proxy="$http_proxy"
		export HTTP_PROXY="$http_proxy"
		export HTTPS_PROXY="$http_proxy"
		export FTP_PROXY="$http_proxy"
		export no_proxy="lanl.gov"
		export RSYNC_PROXY="proxyout.lanl.gov:8080"
		export GIT_PROXY_COMMAND="/usr/libexec/git-core/git-proxy"
		export ECVS_PROXY=proxyout.lanl.gov
		export ECVS_PROXY_PORT=8080
	else
		unset http_proxy https_proxy ftp_proxy no_proxy RSYNC_PROXY GIT_PROXY_COMMAND ECVS_PROXY ECVS_PROXY_PORT
	fi

	#---------------------------------------------------------------------------#
	# set prompt
	#---------------------------------------------------------------------------#

	if [ `whoami` = "root" ] ; then
		export PROMPT_COMMAND='echo -e "$RP_BG$RP_FG" `date +%H:%M` " flecsi@$host$TOEND $BG_GREY ROOT WINDOW $RP_BG$RP_FG" `pwd` "$NEUTRAL"'
		PS1="$R1>$R2>$R3>$R4>$R5>$R6>$P "
	else
		export PROMPT_COMMAND='echo -e "$P_TBG$P_FG_DATE" `date +%H:%M:%S` "$P_HBG$P_FG_USER flecsi$P_FG_AT@$P_FG_HOST$host $P_BG$P_FG_EXTRA $extra_pinfo $P_BG$P_FG_PWD$TOEND\n$P_BG_PWD$P_FG_PWD$TOEND" `pwd` "$NEUTRAL"'

		PS1="$P1>$P2>$P3>$P4>$P5>$P6>$P "
	fi

fi # interactive

# vim: set syntax=sh ts=3 :
