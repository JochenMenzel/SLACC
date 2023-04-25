#! /usr/bin/bash

set -e

# SPDX-FileCopyrightText: 2023 2023 Dipl.-Ing. Jochen Menzel (Jehdar@gmx.de)
#
# SPDX-License-Identifier: GPL-3.0-or-later

CPRGHT_JM="2023 Dipl.-Ing. Jochen Menzel (Jehdar@gmx.de)"
CPRGHT_BM="2012 Frank Bättermann (frank@ich-war-hier.de)"
CPRGHT_MJ="2019 Dr. Martin Jäger (https://libre.solar)"
CPRGHT_ST7032="2019 tomozh http://ore-kb.net/archives/195"
START_PATH=$PWD

reuse addheader --license GPL-3.0-or-later --copyright "$CPRGHT_JM" README.md reusescript.sh

cd "$START_PATH/firmware"
reuse addheader --license GPL-3.0-or-later --copyright "$CPRGHT_JM" makefile

cd "$START_PATH/firmware/inc"

reuse addheader --license GPL-3.0-or-later --copyright "$CPRGHT_JM" --copyright "CPRGHT_BM" \
adc.h csv.h datetime.h fan.h fifo.h linearize.h measurement.h pwm.h time.h timer2.h uart.h

reuse addheader --license Apache-2.0 --copyright "$CPRGHT_MJ" \
charger.h mppt.h

reuse addheader --license GPL-3.0-or-later --copyright "$CPRGHT_JM" \
hmi.h main.h pwr_management.h

cd "$START_PATH/firmware/src"
reuse addheader --license GPL-3.0-or-later --copyright "$CPRGHT_JM" --copyright "CPRGHT_BM" \
adc.c csv.c datetime.c fan.c fifo.c linearize.c measurement.c pwm.c time.c timer2.c uart.c

reuse addheader --license Apache-2.0 --copyright "$CPRGHT_MJ" \
charger.c mppt.c

reuse addheader --license GPL-3.0-or-later --copyright "$CPRGHT_JM" \
hmi.c main.c pwr_management.c

cd "$START_PATH"
reuse addheader --license MIT --copyright "$CPRGHT_ST7032" \
firmware/ST7032-master/ST7032.c \
firmware/ST7032-master/ST7032.h \
firmware/ST7032-master/arduino_st7032_sch.png \
firmware/ST7032-master/examples/Autoscroll/Autoscroll.ino \
firmware/ST7032-master/examples/Blink/Blink.ino \
firmware/ST7032-master/examples/Cursor/Cursor.ino \
firmware/ST7032-master/examples/CustomCharacter/CustomCharacter.ino \
firmware/ST7032-master/examples/Display/Display.ino \
firmware/ST7032-master/examples/HelloWorld/HelloWorld.ino \
firmware/ST7032-master/examples/Icon/Icon.ino \
firmware/ST7032-master/examples/Scroll/Scroll.ino \
firmware/ST7032-master/examples/SerialDisplay/SerialDisplay.ino \
firmware/ST7032-master/examples/TextDirection/TextDirection.ino \
firmware/ST7032-master/examples/setCursor/setCursor.ino

reuse addheader --license CC-BY-4.0 --copyright "2008-20012 eXtreme Electronics, India (https://extremeelectronics.co.in/)" \
firmware/SoftI2CLib/SoftwareI2CLibraryforAVRMCUs.pdf \
firmware/SoftI2CLib/i2csoft.c \
firmware/SoftI2CLib/i2csoft.h

cd "$START_PATH/build_pictures"
reuse addheader --license GPL-3.0-or-later --copyright "$CPRGHT_MJ" \
DSCF5579.JPG DSCF5580.JPG DSCF5581.JPG DSCF5582.JPG DSCF5583.JPG 

cd "$START_PATH/build_pictures/number 1"
reuse addheader --license GPL-3.0-or-later --copyright "$CPRGHT_MJ" \
20170624_163031.jpg 20170715_121228.jpg 20170715_121240.jpg 20170715_121252.jpg 20170716_202529.jpg \
20170716_215857.jpg 20170716_215902.jpg 20170716_222513.jpg 20170716_230438.jpg 20170716_231204.jpg \
20170716_231448.jpg 20170716_233421.jpg 20170718_230503.jpg 20170718_230512.jpg 20170718_230518.jpg \
20170718_230534.jpg 20170718_232755.jpg 20170718_232916.jpg 20170718_232940.jpg 20170718_235255.jpg \
20170718_235302.jpg 20170718_235309.jpg 20170720_231218.jpg 20170720_231225.jpg 20170806_231120.jpg \
20170806_231130.jpg 20170806_231135.jpg 20170806_231148.jpg 20170806_231153.jpg 20170806_231237.jpg

cd "$START_PATH/build_pictures/number 2"
reuse addheader --license GPL-3.0-or-later --copyright "$CPRGHT_MJ" \
20180123_220905.jpg 20180123_220919.jpg 20180123_222154.jpg 20180123_232148.jpg 20180123_232231.jpg \
20180123_234353.jpg 20180125_195158.jpg 20180128_180303.jpg 20180128_180322.jpg 20180128_180336.jpg \
20180128_180549.jpg 20180128_180554.jpg 20180128_181852.jpg 20180128_181918.jpg 20180201_234318.jpg \
20180216_223436.jpg 20180216_223503.jpg 20180224_210309.jpg 20180224_210335.jpg 20180224_210423.jpg \
20180224_215635.jpg 20180225_000537.jpg 20180225_000553.jpg 20180225_000619.jpg 20180225_000647.jpg \
20180225_221730.jpg 20180225_223845.jpg 20180225_223855.jpg 20180225_225313.jpg 20180225_225539.jpg \
20180225_225738.jpg 20180225_232107.jpg 20180225_232110.jpg 20180226_214302.jpg 20180226_220616.jpg \
20180226_220754.jpg 20180226_222540.jpg 20180226_224413.jpg 20180226_224801.jpg 20180226_224823.jpg \
20180226_224837.jpg 20180226_230402.jpg 20180226_231745.jpg 20180226_231749.jpg 20180226_231752.jpg \
20180226_233047.jpg 20180226_233143.jpg 20180302_210728.jpg 20180302_210735.jpg 20180302_231311.jpg \
20180302_231321.jpg 20180308_001425.jpg 20180308_001454.jpg 20180318_142047.jpg 20180320_210632.jpg \
20180320_221804.jpg 20180320_221814.jpg 20190401_214330.jpg 20190401_214337.jpg 20190401_214409.jpg

cd "$START_PATH/build_pictures/number 3"
reuse addheader --license GPL-3.0-or-later --copyright "$CPRGHT_MJ" \
20190312_184743.jpg 20190312_185648.jpg 20190312_191633.jpg 20190312_202053.jpg 20190312_202104.jpg \
20190312_202109.jpg 20190312_204302.jpg 20190312_204337.jpg 20190312_204341.jpg 20190313_145729.jpg \
20190313_145737.jpg 20190314_191201.jpg 20190314_191216.jpg 20190314_191220.jpg 20190314_191243.jpg \
20190401_200742.jpg 20190401_202641.jpg

cd "$START_PATH"
mv firmware/.cproject firmware/.cproject.xml 
mv firmware/.project firmware/.project.xml
reuse addheader --license CC0-1.0 --copyright "2019 Jane Doe <jane@example.com>" \
firmware/.cproject.xml \
firmware/.project.xml \
.gitignore
mv firmware/.cproject.xml firmware/.cproject
mv firmware/.project.xml firmware/.project

