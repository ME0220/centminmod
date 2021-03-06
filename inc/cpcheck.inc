selinxcheck() {
	# check if selinux is disabled, if not abort and prompt to reboot

	if [ ! -f /usr/sbin/sestatus ]; then
		yum -q -y install policycoreutils
	fi

	if [ ! -f /usr/sbin/setenforce ]; then
		yum -q -y install libselinux-utils
	fi	

	if [[ -z $(sestatus | grep disabled) && -f /etc/selinux/config ]]; then
		echo "---------------------------------------------------------------"
		echo "SELINUX linux detected"
		echo "---------------------------------------------------------------"
		echo "disabling SELINUX..."
		echo "will require a server reboot before running initial install"
		sleep 3
		sed -i 's/SELINUX=enforcing/SELINUX=disabled/g' /etc/selinux/config && setenforce 0
		sed -i 's/SELINUX=permissive/SELINUX=disabled/g' /etc/selinux/config && setenforce 0
		echo
		echo "SELINUX disabled, please reboot server and rerun install"
		echo "on reboot you can find the centmin.sh file located at:"
		echo "echo $(pwd)"
		echo "---------------------------------------------------------------"
		exit 0
	fi
}

centaltoff() {
	if [[ -f /etc/yum.repos.d/centalt.repo ]]; then
		sed -i 's/enabled=1/enabled=0/g' /etc/yum.repos.d/centalt.repo
		yum clean all -q
	fi
}

csfipsetcheck() {
	if [[ ! -f /proc/user_beancounters && "$(lsmod | grep ip_set)" ]]; then
		if [[ ! -f /usr/sbin/ipset || ! -f /usr/include/libipset/data.h ]]; then
			yum -y -q install ipset ipset-devel
			if [ -f /etc/csf/csf.conf ]; then
			sed -i 's/LF_IPSET = "0"/LF_IPSET = "1"/g' /etc/csf/csf.conf
			fi
		fi
	else
		if [ -f /etc/csf/csf.conf ]; then
		sed -i 's/LF_IPSET = \"1\"/LF_IPSET = \"0\"/' /etc/csf/csf.conf
		fi
	fi
}

checkaliases() {
	if [[ "$(grep 'centminmod-123.08centos7beta01' /root/.bashrc)" && -d /usr/local/src/centminmod-123.08centos7beta02 ]]; then
        sed -i 's/centminmod-123.08centos7beta01/centminmod-123.08centos7beta02/' /root/.bashrc
    fi
}

checkcmdircmd() {
	if [[ "$(grep 'cmdir=' /root/.bashrc)" || "$(grep 'centmin=' /root/.bashrc)" ]]; then
		sed -i '/cmdir=/d' /root/.bashrc
		sed -i '/centmin=/d' /root/.bashrc
		rm -rf /usr/bin/cmdir
		alias cmdir="pushd ${SCRIPT_DIR}"
		echo "alias cmdir='pushd ${SCRIPT_DIR}'" >> /root/.bashrc
cat > "/usr/bin/centmin" << EOF
#!/bin/bash
pushd $SCRIPT_DIR; bash centmin.sh
EOF
		chmod 0700 /usr/bin/centmin
	fi
}

cpcheck() {

if [ -f /var/cpanel/cpanel.config ]; then
echo "WHM/Cpanel detected.. centmin mod NOT compatible"
echo "aborting script..."
exit
fi

if [ -f /etc/psa/.psa.shadow ]; then
echo "Plesk detected.. centmin mod NOT compatible"
echo "aborting script..."
exit
fi

if [ -f /etc/init.d/directadmin ]; then
echo "DirectAdmin detected.. centmin mod NOT compatible"
echo "aborting script..."
exit
fi

selinxcheck
centaltoff
csfipsetcheck
checkaliases
checkcmdircmd

}

histformat() {
	if [[ -z "$(grep HISTTIMEFORMAT /root/.bashrc)" ]]; then
		echo "HISTTIMEFORMAT=\"[%d.%m.%y] %T   \"" >> /root/.bashrc
		echo "export HISTSIZE=10000" >> /root/.bashrc
		echo "export HISTTIMEFORMAT" >> /root/.bashrc
	fi	
}

setupdate() {
cat > "/usr/bin/cminfo_updater"<<EOF
#!/bin/bash
rm -rf /usr/bin/cminfo
#wget -q --no-check-certificate -O /usr/bin/cminfo https://gist.githubusercontent.com/centminmod/828a8ae1add0397e740b/raw/cminfo.sh
wget -q --no-check-certificate -O /usr/bin/cminfo https://raw.githubusercontent.com/centminmod/centminmod/123.08centos7beta01/tools/cminfo.sh
chmod 0700 /usr/bin/cminfo
EOF

chmod 0700 /usr/bin/cminfo_updater

# insert itself into cronjob for auto updates
if [ ! -f /usr/bin/crontab ]; then
	yum -y -q install cronie
fi

if [[ -z "$(crontab -l 2>&1 | grep cminfo_updater)" ]]; then
    crontab -l > cronjoblist
    echo "0 */4 * * * /usr/bin/cminfo_updater" >> cronjoblist
    crontab cronjoblist
    rm -rf cronjoblist
    crontab -l
fi
}