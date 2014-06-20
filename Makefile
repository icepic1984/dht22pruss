LIBPATH=-L ~/prusslib/lib/
INCLUDEPATH=-I ~/prusslib/include/
LIBS+=-lprussdrv
PASM=~/prusslib/pasm
EXECUTABLE=prudht22
DEVICEOVERLAY=DM-GPIO-PRU

all: $(EXECUTABLE)  $(EXECUTABLE).bin $(EXECUTABLE).o $(DEVICEOVERLAY).dtbo
%.o: %.cpp
	g++ -c $(INCLUDEPATH) $< 

% : %.o
	g++  $(LIBPATH)  $(LIBS) $< -o $@

%.bin : %.p
	$(PASM) -b $<

%.dtbo: %.dts
	dtc -O dtb -o $(basename $@)-00A0.dtbo -b 0 -@ $<
	cp $(basename $@)-00A0.dtbo /lib/firmware

clean: 
	-rm -rf $(EXECUTABLE) $(EXECUTABLE).o $(EXECUTABLE).bin
