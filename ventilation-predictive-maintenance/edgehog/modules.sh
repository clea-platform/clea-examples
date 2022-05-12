#!/bin/bash

echo "  ______ _____   _____ ______ _    _  ____   _____                  _                                      _       _           "
echo " |  ____|  __ \ / ____|  ____| |  | |/ __ \ / ____|                (_)                                    | |     | |          "
echo " | |__  | |  | | |  __| |__  | |__| | |  | | |  __ ______ _ __ ___  _  ___ _ __ ___    _ __ ___   ___   __| |_   _| | ___  ___ "
echo " |  __| | |  | | | |_ |  __| |  __  | |  | | | |_ |______| '_ \` _ \| |/ __| '__/ _ \  | '_ \` _ \ / _ \ / _\` | | | | |/ _ \/ __|"
echo " | |____| |__| | |__| | |____| |  | | |__| | |__| |      | | | | | | | (__| | | (_) | | | | | | | (_) | (_| | |_| | |  __/\__ \\"
echo " |______|_____/ \_____|______|_|  |_|\____/ \_____|      |_| |_| |_|_|\___|_|  \___/  |_| |_| |_|\___/ \__,_|\__,_|_|\___||___/"
echo ""
                                                                                                                            
PS3='Please enter your choice: '
options=("All modules (Modem, Battery, Accelerometer)" "SI7006-A20 Temperature & Humidity Sensor" "Modem" "Battery" "Accelerometer" "Clear Modules" "Done")
COLUMNS=1
select opt in "${options[@]}"
do
    case $opt in
        "All modules (Modem, Battery, Accelerometer)")
        git submodule update --init --remote
        git submodule add git@git.seco.com:iot/edgehog-micro-components/d47_accelerometer.git components/d47_accelerometer/
	    ;;
        "SI7006-A20 Temperature & Humidity Sensor")
        git submodule add git@git.seco.com:iot/edgehog-micro-components/d47_si7006-a20.git components/d47_si7006-a20/
        ;;
        "Modem")
	    git submodule update --init --remote components/ehm-modem_simcomm/
        ;;
        "Battery")
	    git submodule update --init --remote components/d47_battery_module/
	    ;;
	    "Accelerometer")
	    git submodule add git@git.seco.com:iot/edgehog-micro-components/d47_accelerometer.git components/d47_accelerometer/
	    ;;
        "Clear Modules")
        rm -rf components/d47_accelerometer/ components/d47_si7006-a20 components/ehm-modem_simcomm/* components/d47_battery_module/*
        ;;
        "Done")
        break
        ;;
        *) echo "invalid option $REPLY";;
    esac
done