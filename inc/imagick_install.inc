checkphpext() {

cecho "Check for php extensions" $boldyellow
if [[ ! -f "${DIR_TMP}/imagick-${IMAGICKPHP_VER}.tgz" ]]; then
echo "Downloading imagick extension"
imagickphpexttarball
fi

}

imagickinstall() {

checkphpext

    echo "*************************************************"
    cecho "* Installing imagick PHP Extension" $boldgreen
    echo "*************************************************"

    pwd
    echo "cd $DIR_TMP"
    cd $DIR_TMP

php -v | awk -F " " '{print $2}' | head -n1 | cut -d . -f1,2 | grep '7.0'
PHPSEVEN_CHECKVER=$?
echo $PHPSEVEN_CHECKVER

if [[ "$PHPMUVER" = '7.0' || "$PHPMUVER" = 'NGDEBUG' || "$PHPSEVEN_CHECKVER" = '0' ]]; then
    if [[ -d "imagick-php7" && -d "imagick-php7/.git" ]]; then       
        cd imagick-php7
        git stash
        git pull
        git log -3
    elif [[ -d "imagick-php7" && ! -d "imagick-php7/.git" ]]; then
        rm -rf imagick-php7
        git clone -b phpseven https://github.com/mkoppanen/imagick.git imagick-php7
    else
        rm -rf imagick-php7
        git clone -b phpseven https://github.com/mkoppanen/imagick.git imagick-php7
    fi
    echo
    echo "compiling imagick PHP extension for PHP 7.x ..."
    cd imagick-php7
    make clean
    /usr/local/bin/phpize
    ./configure --with-php-config=/usr/local/bin/php-config
    make${MAKETHREADS}
    make install
else    
    cd imagick-${IMAGICKPHP_VER}
    make clean
    /usr/local/bin/phpize
    ./configure --with-php-config=/usr/local/bin/php-config
    make${MAKETHREADS}
    make install
fi # php 7 or not

#######################################################
# check if imagick.so exists in php.ini

IMAGICKSOCHECK=`grep 'extension=imagick.so' /usr/local/lib/php.ini`
IMAGICKSOCHECKB=$(grep 'extension=imagick.so' ${CONFIGSCANDIR}/imagick.ini)

if [[ -z $IMAGICKSOCHECK || -z $IMAGICKSOCHECKB ]]; then

echo -e "\nCopying imagick.ini > ${CONFIGSCANDIR}/imagick.ini\n"
echo "extension=imagick.so" > ${CONFIGSCANDIR}/imagick.ini

fi #check if imagick.so exists in php.ini

    cmservice php-fpm restart
    echo "*************************************************"
    cecho "* imagick PHP installed" $boldgreen
    echo "*************************************************"

}