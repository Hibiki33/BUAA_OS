before=$2
after=$3
sed -i 's/'${before}'/'${after}'/g' $1
