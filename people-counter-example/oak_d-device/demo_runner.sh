
#!/bin/bash

cd $(dirname $0)

# Creating virtualenv if not existing
if [[ ! -d "./venv" ]] ; then
    virtualenv venv
    source venv/bin/activate
    pip3 install -r requirements.txt
    deactivate
fi

# Creating 'astarte_persistence.d' if not existing
if [[ ! -d "./astarte_persistence.d" ]] ; then
    mkdir astarte_persistence.d
fi

source venv/bin/activate
python3 main.py