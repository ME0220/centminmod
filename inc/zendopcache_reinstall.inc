#################
zopcachereinstall() {

    echo "*************************************************"
    cecho "* Re-Installing Zend Optimizer Plus OPcache" $boldgreen
    echo "*************************************************"

    cd $DIR_TMP

PHPCURRENTVER=$(php -v | awk -F " " '{print $2}' | head -n1 | cut -d . -f1,2)
PHPMVER=$(echo "$PHP_VERSION" | cut -d . -f1,2)
PHPMUVER=$(echo "$phpver" | cut -d . -f1,2)

echo "-------------------------------------------------------------------"
echo "Current PHP branch: $PHPCURRENTVER"
echo "Current centmin.sh set PHP_VERSION: $PHPMVER"
echo "Current PHP upgrade version set: $PHPMUVER"
echo "-------------------------------------------------------------------"



# only prompt for Zend Opcache version if PHP version is between 5.2 and 5.4 inclusive
if [[ "$(expr $PHPMVER \>= 5.2)" = 1 && "$(expr $PHPMVER \<= 5.4)" = 1 ]] && [[ "$(expr $PHPCURRENTVER \>= 5.2)" = 1 && "$(expr $PHPCURRENTVER \<= 5.4)" = 1 ]]; then
	ZOPCACHEPROMPT='y'
# prompt if ZOPCACHE_OVERRIDE=y for PHP versions between 5.5-5.6
elif [[ "$(expr $PHPMVER \>= 5.5)" = 1 && "$(expr $PHPMVER \<= 5.6)" = 1 && "$ZOPCACHE_OVERRIDE" = [yY] ]] && [[ "$(expr $PHPCURRENTVER \>= 5.5)" = 1 && "$(expr $PHPCURRENTVER \<= 5.6)" = 1 && "$ZOPCACHE_OVERRIDE" = [yY] ]]; then
	ZOPCACHEPROMPT='y'
# prompt if ZOPCACHE_OVERRIDE=y for PHP version 7.0	
elif [[ "$(expr $PHPMVER \= 7.0)" = 1 && "$ZOPCACHE_OVERRIDE" = [yY] ]]; then
	ZOPCACHEPROMPT='y'	
else
	ZOPCACHEPROMPT='n'
fi

# check if this is a PHP upgrade task
if [ "$PHPMUVER" ]; then
	if [[ "$(expr $PHPMUVER \>= 5.2)" = 1 && "$(expr $PHPMUVER \<= 5.4)" = 1 ]]; then
		ZOPCACHEPROMPT='y'
	# prompt if ZOPCACHE_OVERRIDE=y for PHP versions between 5.5-5.6
	elif [[ "$(expr $PHPMUVER \>= 5.5)" = 1 && "$(expr $PHPMUVER \<= 5.6)" = 1 && "$ZOPCACHE_OVERRIDE" = [yY] ]]; then
		ZOPCACHEPROMPT='y'
	# prompt if ZOPCACHE_OVERRIDE=y for PHP version 7.0		
	elif [[ "$(expr $PHPMUVER \= 7.0)" = 1 && "$ZOPCACHE_OVERRIDE" = [yY] ]]; then
		ZOPCACHEPROMPT='y'		
	else
		ZOPCACHEPROMPT='n'
	fi
fi

# only prompt for Zend Opcache version ZOPCACHEPROMPT=y
if [[ "$ZOPCACHEPROMPT" = [yY] ]]; then

echo ""
read -ep "Install which version of Zend OpCache? (version i.e. ${ZOPCACHECACHE_VERSION}}): " zocver
ZENDOPCACHE_LINKFILE="zendopcache-${zocver}.tgz"
ZENDOPCACHE_LINK="https://pecl.php.net/get/${ZENDOPCACHE_LINKFILE}"


    cd $DIR_TMP

        cecho "Download ${ZENDOPCACHE_LINKFILE} ..." $boldyellow
    if [ -s ${ZENDOPCACHE_LINKFILE} ]; then
        cecho " ${ZENDOPCACHE_LINKFILE} Archive found, skipping download..." $boldgreen
    else
        $DOWNLOADAPP ${ZENDOPCACHE_LINK} $WGETRETRY 
ERROR=$?
	if [[ "$ERROR" != '0' ]]; then
	cecho "Error: ${ZENDOPCACHE_LINKFILE} download failed." $boldgreen
checklogdetails
	exit #$ERROR
else 
         cecho "Download done." $boldyellow
#echo ""
	fi
    fi

tar xzf ${ZENDOPCACHE_LINKFILE} 
ERROR=$?
	if [[ "$ERROR" != '0' ]]; then
	cecho "Error: ${ZENDOPCACHE_LINKFILE} extraction failed." $boldgreen
checklogdetails
	exit #$ERROR
else 
         cecho "${ZENDOPCACHE_LINKFILE} valid file." $boldyellow
echo ""
	fi

cd zendopcache-${zocver}
make clean
/usr/local/bin/phpize
./configure --with-php-config=/usr/local/bin/php-config
make${MAKETHREADS}
make install

if [[ -f ${CONFIGSCANDIR}/apc.ini ]]; then
	rm -rf ${CONFIGSCANDIR}/apc.ini
fi

if [[ -f ${CONFIGSCANDIR}/igbinary.ini ]]; then
	sed -i 's/apc.serializer=igbinary/;apc.serializer=igbinary/g' ${CONFIGSCANDIR}/igbinary.ini
fi

if [[ -f ${CONFIGSCANDIR}/xcache.ini ]]; then
	rm -rf ${CONFIGSCANDIR}/xcache.ini
fi

if [[ -f ${CONFIGSCANDIR}/zendopcache.ini ]]; then
	# \cp -af ${CONFIGSCANDIR}/zendopcache.ini ${CONFIGSCANDIR}/zendopcache.ini.bak
	# rm -rf ${CONFIGSCANDIR}/zendopcache.ini
	ZOPHPINIEXISTS=y
fi

######################################################
# Randomize ${N}_opcache.php filename
N=$(od -vAn -N4 -tx < /dev/urandom)
N=$(echo ${N} | sed -e 's/\s//')
if [[ -f /usr/local/nginx/html/opcache.php ]]; then
	mv /usr/local/nginx/html/opcache.php /usr/local/nginx/html/${N}_opcache.php
	chown nginx:nginx /usr/local/nginx/html/${N}_opcache.php
fi

######################################################
PHPEXTDIRD=`cat /usr/local/bin/php-config | awk '/^extension_dir/ {extdir=$1} END {gsub(/\047|extension_dir|=|)/,"",extdir); print extdir}'`
echo "\$PHPEXTDIRD = $PHPEXTDIRD"
#######################################################

# check if ${CONFIGSCANDIR}/zendopcache.ini file exists
if [ ! -f ${CONFIGSCANDIR}/zendopcache.ini ]; then
	if [[ "$lesszendopcachemem" = [yY] ]]; then
		echo $lesszendopcachemem			
		echo -e "\nCopying zendopcache-min.ini > ${CONFIGSCANDIR}/zendopcache.ini\n"
		cat $CUR_DIR/config/zendopcache/zendopcache-min.ini > ${CONFIGSCANDIR}/zendopcache.ini
		replace 'zend_extension=opcache.so' "zend_extension=${PHPEXTDIRD}/opcache.so" -- "${CONFIGSCANDIR}/zendopcache.ini"	
	else
		echo $lesszendopcachemem	
		echo -e "\nCopying zendopcache.ini > ${CONFIGSCANDIR}/zendopcache.ini\n"
		cat $CUR_DIR/config/zendopcache/zendopcache.ini > ${CONFIGSCANDIR}/zendopcache.ini
		replace 'zend_extension=opcache.so' "zend_extension=${PHPEXTDIRD}/opcache.so" -- "${CONFIGSCANDIR}/zendopcache.ini"		
	fi
fi

# check if opcache.so exists in ${CONFIGSCANDIR}/zendopcache.ini
ZOPLUSSOCHECK=$(grep 'opcache.so' ${CONFIGSCANDIR}/zendopcache.ini)

################
# if opcache.so doesn't exist then ZOPPLUSSOCHECK will be empty
if [[ -z $ZOPLUSSOCHECK ]]; then
	echo
	read -ep "Do you want to reset zendopcache.ini settings to default (y) or keep existing zendopcache.ini settings (n)? [y/n]: " resetzocache

    if [[ "$resetzocache" = [yY] ]]; then   
		echo
		if [[ "$lesszendopcachemem" = [yY] ]]; then
			echo $lesszendopcachemem			
			echo -e "\nCopying zendopcache-min.ini > ${CONFIGSCANDIR}/zendopcache.ini\n"
			cat $CUR_DIR/config/zendopcache/zendopcache-min.ini > ${CONFIGSCANDIR}/zendopcache.ini
			replace 'zend_extension=opcache.so' "zend_extension=${PHPEXTDIRD}/opcache.so" -- "${CONFIGSCANDIR}/zendopcache.ini"	
		else
			echo $lesszendopcachemem	
			echo -e "\nCopying zendopcache.ini > ${CONFIGSCANDIR}/zendopcache.ini\n"
			cat $CUR_DIR/config/zendopcache/zendopcache.ini > ${CONFIGSCANDIR}/zendopcache.ini
			replace 'zend_extension=opcache.so' "zend_extension=${PHPEXTDIRD}/opcache.so" -- "${CONFIGSCANDIR}/zendopcache.ini"		
		fi
	else
		# if resetzocache=n
		if [ -f ${CONFIGSCANDIR}/zendopcache.ini ]; then
			ls -lah ${CONFIGSCANDIR}/zendopcache.ini
			
			echo
			# replace 'zend_extension=opcache.so' "zend_extension=${PHPEXTDIRD}/opcache.so" -- "${CONFIGSCANDIR}/zendopcache.ini"
			
			# resetzocache = n
			# update zend_extension path for updated PHP version and update ${CONFIGSCANDIR}/zendopcache.ini
			# with such path while keeping existing ${CONFIGSCANDIR}/zendopcache.ini settings intact
			sed -i "/zend_extension=/c\zend_extension=$PHPEXTDIRD\/opcache.so" ${CONFIGSCANDIR}/zendopcache.ini
		else
			echo
			echo "${CONFIGSCANDIR}/zendopcache.ini doesn't exist"
			echo "May need to run sub menu option 1 to install Zend Opcache"
			echo "instead of sub menu option 2 to reinstall Zend Opcache"
		fi
	fi # resetzocache
fi # check if opcache.so exists in zendopcache.ini
################

# DOUBLE CHECK as opcache.so needs extension path
ZOPLUSSOPATHCHECK=$(grep 'zend_extension=opcache.so' ${CONFIGSCANDIR}/zendopcache.ini)
if [[ $ZOPLUSSOPATHCHECK ]]; then
	echo
	echo "replace 'zend_extension=opcache.so' "zend_extension=${PHPEXTDIRD}/opcache.so" -- ${CONFIGSCANDIR}/zendopcache.ini"
	replace 'zend_extension=opcache.so' "zend_extension=${PHPEXTDIRD}/opcache.so" -- "${CONFIGSCANDIR}/zendopcache.ini"
fi

    cmservice php-fpm restart
    echo "*************************************************"
    cecho "* Zend Optimizer Plus OPcache re-installed" $boldgreen
    echo "*************************************************"

else

	PHPNATIVEOPCACHE='y'
	echo ""
	echo "Detected PHP branch: 5.5 which has native support for Zend OPcache"
	echo "To re-install Zend OPcache run menu option #5 and specify PHP 5.5.0 or higher"
fi # check for PHPMVER !=5.5
}