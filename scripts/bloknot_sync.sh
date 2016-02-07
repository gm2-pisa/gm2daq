#!/bin/bash

rsync -av \
    /home/daq/DAQ/www/bloknot/* \
    --exclude config.php \
    --exclude /home/daq/DAQ/www/bloknot/notes/*/cache \
    --exclude /home/daq/DAQ/www/bloknot/notes/*/editor* \
    --exclude /home/daq/DAQ/www/bloknot/notes/*/entry* \
    --exclude /home/daq/DAQ/www/bloknot/notes/*/icons \
    --exclude /home/daq/DAQ/www/bloknot/notes/*/latex \
    --exclude /home/daq/DAQ/www/bloknot/notes/*/tinymce \
    --exclude /home/daq/DAQ/www/bloknot/notes/*/.git \
    --exclude /home/daq/DAQ/www/bloknot/notes/*/figures \
    --exclude notes/*/cache \
    --exclude notes/*/editor* \
    --exclude notes/*/entry* \
    --exclude notes/*/icons \
    --exclude notes/*/latex \
    --exclude notes/*/tinymce \
    --exclude notes/*/.git \
    --exclude notes/*/figures \
    g2@gluon.pa.uky.edu:public_html/bloknot_MTest2012 > ~/tmp/bloknot_sync.dat

